#include "saguinus.h"

static u32 randomU32(u32 sd = 1){
    static u32 seed = sd;
    seed = xorshift(seed);
    return seed;
}

static u8* allocateLongTermMemory(GameState* state, u32 amount){
    u8* ptr = state->storage.longTermBufferPointer;
    state->storage.longTermBufferStorageTaken += amount;
    state->storage.longTermBufferPointer += amount;
    return ptr;
}

static Vector2 getTextDimensions(Font* font, const s8* str, f32 scale, f32 border = 0){
    const s8* c = str;
    Vector2 dim(0);

    while(*c != '\0'){
        u32 charIndex = binarySearch(font->characterCodes, *c, 0, font->totalCharacters, font->missingCharacterCodeIndex);
        f32 cW = font->width[charIndex];
        f32 cH = font->height[charIndex];   
        dim.x += cW * scale + font->kerning[charIndex];
        f32 ny = cH * scale;
        dim.y = ny > dim.y ? ny : dim.y;
        c++;
    }

    f32 mul = (border * scale * 2);
    return Vector2(dim.x + mul, dim.y + mul);
}

static void addQuadToBuffer(GameState* state, Vector2 position, Vector2 scale, Vector2 textureOffset, Vector2 textureScale, Vector4 color, Texture2D texture, bool isText){
    CanvasBuffer* buffer = &state->canvasBuffer;
    u32 idx = buffer->totalGUIItems++;
    buffer->positions[idx] = position;
    buffer->scales[idx] = scale;
    buffer->textureOffsets[idx] = textureOffset;
    buffer->textureScales[idx] = textureScale;
    buffer->isText[idx] = isText;
    buffer->colors[idx] = color;
    buffer->textures[idx] = texture;
}

static void addQuadToBuffer(GameState* state, Vector2 position, Vector2 scale, Texture2D texture){
    addQuadToBuffer(state, position, scale, Vector2(0), Vector2(1), Vector4(1), texture, false);
}

static void addQuadToBuffer(GameState* state, Vector2 position, Vector2 scale, Vector4 color){
   addQuadToBuffer(state, position, scale, Vector2(0), Vector2(1), color, state->canvasBuffer.defaultTexture, false);
}

static void addTextToCanvas(GameState* state, const s8* str, Vector2 position, f32 scale, Vector4 color){
    Font* font = state->currentFont;
    CanvasBuffer* buffer = &state->canvasBuffer;

    const s8* c = str;
    f32 xStart = position.x;
    while(*c != '\0'){
        u32 charIndex = binarySearch(font->characterCodes, *c, 0, font->totalCharacters, font->missingCharacterCodeIndex);
                
        f32 bmX = font->bitmapX[charIndex];
        f32 bmY = font->bitmapY[charIndex];
        f32 bmW = font->bitmapCharacterWidth[charIndex];
        f32 bmH = font->bitmapCharacterHeight[charIndex];
        f32 cW = font->width[charIndex];
        f32 cH = font->height[charIndex];

        addQuadToBuffer(state, position, Vector2(cW * scale, cH * scale), Vector2(bmX, bmY), Vector2(bmW, bmH), color, font->bitmap, true);
        position.x += (cW * scale) + font->kerning[charIndex];
        c++;
    }
}

static void addTextQuad(GameState* state, const s8* str, Vector2 position, Vector4 textColor, Vector4 quadColor, f32 scale, f32 border){
    Vector2 dim = getTextDimensions(state->currentFont, str, scale, border);
    f32 amt = (border * scale);
    addQuadToBuffer(state, position, dim, quadColor);
    addTextToCanvas(state, str, Vector2(position.x + amt, position.y + amt), scale, textColor);
}

static void addTextQuad(GameState* state, const s8* str, Vector2 position, Vector2 quadDim, Vector4 textColor, Vector4 quadColor, f32 scale, f32 border){
    f32 amt = (border * scale);
    addQuadToBuffer(state, position, quadDim, quadColor);
    addTextToCanvas(state, str, Vector2(position.x + amt, position.y + amt), scale, textColor);
}

static void debugPrint(GameState* state, s8* text, ...){
    CanvasBuffer* buffer = &state->canvasBuffer;
    va_list argptr;
    va_start(argptr, text);
    s8 buf[MAX_STRING_LENGTH];
    createDebugString(buf, text, argptr);

    addTextQuad(state, buf, Vector2(buffer->debugPrinterX, buffer->debugPrinterY), Vector4(0.7, 0.7, 0.7, 0.7), Vector4(0.3, 0.3, 0.3, 0.7), 2, 1);
    buffer->debugPrinterY -= 15;

    va_end(argptr);
}

static void debugCube(GameState* state, Vector3 position, Vector3 scale, Vector4 color){
    DebugBuffer* buffer = &state->debugBuffer;
    u32 idx = buffer->totalCubes++;
    buffer->cubePositions[idx] = position;
    buffer->cubeScales[idx] = scale;
    buffer->cubeColors[idx] = color;
}

static void debugLine(GameState* state, Vector3 start, Vector3 end, Vector4 color, f32 lineWidth){
    DebugBuffer* buffer = &state->debugBuffer;
    u32 idx = buffer->totalLines++;
    buffer->lineStarts[idx] = start;
    buffer->lineEnds[idx] = end;
    buffer->lineWidths[idx] = lineWidth;
    buffer->lineColors[idx] = color;
}

static void debugBox(GameState* state, Vector3 position, Vector3 scale, Vector4 color, f32 lineWidth){
    Vector3 halfScale = scale * 0.5;
    Vector3 p1(position.x - halfScale.x, position.y - halfScale.y, position.z - halfScale.z);
    Vector3 p2(position.x - halfScale.x, position.y + halfScale.y, position.z - halfScale.z);
    Vector3 p3(position.x + halfScale.x, position.y + halfScale.y, position.z - halfScale.z);
    Vector3 p4(position.x + halfScale.x, position.y - halfScale.y, position.z - halfScale.z);

    Vector3 p5(position.x - halfScale.x, position.y - halfScale.y, position.z + halfScale.z);
    Vector3 p6(position.x - halfScale.x, position.y + halfScale.y, position.z + halfScale.z);
    Vector3 p7(position.x + halfScale.x, position.y + halfScale.y, position.z + halfScale.z);
    Vector3 p8(position.x + halfScale.x, position.y - halfScale.y, position.z + halfScale.z);

    debugLine(state, p1, p2, color, lineWidth);
    debugLine(state, p2, p3, color, lineWidth);
    debugLine(state, p3, p4, color, lineWidth);
    debugLine(state, p4, p1, color, lineWidth);

    debugLine(state, p5, p6, color, lineWidth);
    debugLine(state, p6, p7, color, lineWidth);
    debugLine(state, p7, p8, color, lineWidth);
    debugLine(state, p8, p5, color, lineWidth);

    debugLine(state, p1, p5, color, lineWidth);
    debugLine(state, p2, p6, color, lineWidth);
    debugLine(state, p3, p7, color, lineWidth);
    debugLine(state, p4, p8, color, lineWidth);
}

static bool guiButton(GameState* state, Vector2 position, const s8* text, f32 scale, f32 buttonBorder = 2){
    Vector2 mp = state->mousePosition;
    Vector2 dim = getTextDimensions(state->currentFont, text, scale, buttonBorder);

    if(mp.x < position.x || mp.x > position.x + dim.x 
    || mp.y < position.y || mp.y > position.y + dim.y){
        addTextQuad(state, text, position, dim, Vector4(1), Vector4(0, 0, 1, 1), scale, buttonBorder);
        state->mouseButtonTracking[MOUSE_BUTTON_LEFT]  = false;
    }else{
        if(state->mouseInputs[MOUSE_BUTTON_LEFT]){
            addTextQuad(state, text, position, dim, Vector4(1), Vector4(1, 0, 0, 1), scale, buttonBorder);
            state->mouseButtonTracking[MOUSE_BUTTON_LEFT]  = true;
        }else if(state->mouseButtonTracking[MOUSE_BUTTON_LEFT] && !state->mouseInputs[MOUSE_BUTTON_LEFT]){
            addTextQuad(state, text, position, dim, Vector4(1), Vector4(1, 0, 0, 1), scale, buttonBorder);
            state->mouseButtonTracking[MOUSE_BUTTON_LEFT]  = false;
            return true;
        }else{
            addTextQuad(state, text, position, dim, Vector4(1), Vector4(0, 1, 0, 1), scale, buttonBorder);
        }
    }
    return false;
}

static void addTexturedMeshToBuffer(GameState* state, TexturedMesh* mesh, Vector3 position, Vector3 scale, Quaternion orientation){
    TexturedMeshBuffer* tmb = &state->txtdMeshBuffer;
    u32 idx = tmb->totalMeshes++;
    tmb->positions[idx] = position;
    tmb->scales[idx] = scale;
    tmb->orientations[idx] = orientation;
    tmb->textures[idx] = mesh->texture;
    tmb->indexCounts[idx] = mesh->indexCount;
    tmb->indexOffsets[idx] = mesh->indexOffset;
    tmb->indexAddons[idx] = mesh->indexAddon;
}

static void updateMeshAnimation(MeshAnimation* mesh, f32 deltaTime){
    mesh->currentPoseElapsed += deltaTime;
    if(mesh->currentPoseElapsed > mesh->currentPoseTime){
        u32 ckf = mesh->currentKeyframe;
        u32 nkf = mesh->nextKeyframe;
        if(nkf < mesh->totalPoses - 1){
            ckf++;
            nkf++;
        }else{
            ckf = 0;
            nkf = 1;
        }
        mesh->currentPoseElapsed -= mesh->currentPoseTime;
        mesh->currentPoseTime = (mesh->keyframes[nkf] - mesh->keyframes[ckf]) / mesh->frameRate;
        mesh->currentKeyframe = ckf;
        mesh->nextKeyframe = nkf;
    }
}

static void updateCameraWithMouse(GameState* state){
    Camera* camera = &state->camera;
    f32 deltaTime = state->deltaTime;
    if(state->updateCamera){
        f32 xDif = state->mousePosition.x - state->windowDimenstion.x * 0.5;
        f32 yDif = state->mousePosition.y - state->windowDimenstion.y * 0.5;
        rotate(&camera->orientation, camera->up, deltaTime * xDif * camera->mouseSensitivity);
        rotate(&camera->orientation, camera->right, deltaTime * -yDif * camera->mouseSensitivity);
        state->updateCamera = false;
    }
}

static void updateCameraWithKeyboard(GameState* state){
    f32 deltaTime = state->deltaTime;
    bool* keyInputs = state->keyInputs;
    Camera* camera = &state->camera;
    InputCodes* c = &state->inputCodes;
    if(keyInputs[c->KEY_W]){
        camera->position += camera->forward * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_S]){
        camera->position -= camera->forward * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_A]){
       camera->position -= camera->right * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_D]){
        camera->position += camera->right * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_R]){
       camera->position += camera->up * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_F]){
        camera->position -= camera->up * deltaTime * camera->moveSpeed;
    }

    if(keyInputs[c->KEY_UP]){
        rotate(&camera->orientation, camera->right, -deltaTime * camera->rotateSpeed);
    }
    if(keyInputs[c->KEY_DOWN]){
        rotate(&camera->orientation, camera->right, deltaTime * camera->rotateSpeed);
    }
    if(keyInputs[c->KEY_LEFT]){
        rotate(&camera->orientation, camera->up, -deltaTime * camera->rotateSpeed);
    }
    if(keyInputs[c->KEY_RIGHT]){
        rotate(&camera->orientation, camera->up, deltaTime * camera->rotateSpeed);
    }
    if(keyInputs[c->KEY_Q]){
        rotate(&camera->orientation, camera->forward, deltaTime * camera->rotateSpeed);
    }
    if(keyInputs[c->KEY_E]){
        rotate(&camera->orientation, camera->forward, -deltaTime * camera->rotateSpeed);
    }
}

static void updateCameraWithGamepad(GameState* state){
    f32 deltaTime = state->deltaTime;
    Camera* camera = &state->camera;
    Gamepad* gamepad1 = &state->gamepad1;
    InputCodes* c = &state->inputCodes;
    if(gamepad1->rightStickX > 0.05 || gamepad1->rightStickX < -0.05){
        rotate(&camera->orientation, camera->up, deltaTime * camera->rotateSpeed * gamepad1->rightStickX);
    }
    if(gamepad1->rightStickY > 0.05 || gamepad1->rightStickY < -0.05){
        rotate(&camera->orientation, camera->right, deltaTime * camera->rotateSpeed * gamepad1->rightStickY);
    }
    if(gamepad1->leftTrigger > 0.05){
        rotate(&camera->orientation, camera->forward, deltaTime * camera->rotateSpeed * gamepad1->leftTrigger);
    }
    if(gamepad1->rightTrigger > 0.05){
        rotate(&camera->orientation, camera->forward, -deltaTime * camera->rotateSpeed * gamepad1->rightTrigger);
    }
    if(gamepad1->leftStickX > 0.05 || gamepad1->leftStickX < -0.05){
        camera->position += camera->right * deltaTime * camera->moveSpeed * gamepad1->leftStickX;
    }
    if(gamepad1->leftStickY > 0.05 || gamepad1->leftStickY < -0.05){
        camera->position += camera->forward * deltaTime * camera->moveSpeed * gamepad1->leftStickY;
    }
    if(gamepad1->buttons[c->GAMEPAD_LB]){
        camera->position -= camera->up * deltaTime * camera->moveSpeed;
    }
    if(gamepad1->buttons[c->GAMEPAD_RB]){
        camera->position += camera->up * deltaTime * camera->moveSpeed;
    }
}

static bool keyPressedOnce(GameState* state, u32 key){
    bool* keyInputs = state->keyInputs;
    if(keyInputs[key] && !state->keyTracking[key]){
        state->keyTracking[key] = true;
        return true;
    }else if(!keyInputs[key]){
        state->keyTracking[key] = false;
    }
    return false;
}

static void updateObstacles(GameState* state){
    Player* p = &state->player;
    u32 tot = state->totalObstacles;
    f32 dt = state->deltaTime;
    for(u32 i = 0; i < tot; i++){
        Obstacle* o = &state->obstacles[i];
        o->position.x += dt * o->xVelocity;
        if(o->position.x < -10){
            o->position.x = o->xStartPosition;
        }
        f32 len = length(o->position - p->position);
        if(len < 1){
            state->clearColor = Vector4(0, 0, 0, 1);
            state->mode = GameMode::GAME_MODE_OVER;

            if(state->score > state->hiScore){
                state->hiScore = state->score;
                state->osFunctions.writeToFile("hiscore", &state->hiScore, sizeof(u32));
            }
        }
    }
}

static f32 getVerticalPositionInTerrain(Terrain* t, Vector2 xz){
    xz.x -= t->position.x;
    xz.y -= t->position.z;
    s32 xi = (s32)xz.x;
    s32 zi = (s32)xz.y;
    s32 xm = xi % t->width;
    s32 zm = zi % t->height;
    s32 xm2 = (xm + 1) % t->width;
    s32 zm2 = (zm + 1) % t->height;
    f32 xt = xz.x - (f32)xi;
    f32 zt = xz.y - (f32)zi;

    return bilinearInterpolate(t->heightmap[zm][xm],
                               t->heightmap[zm][xm2],
                               t->heightmap[zm2][xm],
                               t->heightmap[zm2][xm2], xt, zt);
}

static void generateTerrainMap(f32** map){
    const u32 size = 256;
    f32 noise[size][size];
    u32 v = xorshift(1);
    for(u32 i = 0; i < size; i++){
        for(u32 j = 0; j < size; j++){
            noise[i][j] = (f32)v / (f32)MAX_U32;
            v = xorshift(v);
        }
    }

    for(u32 i = 0; i < size; i++){
        for(u32 j = 0; j < size; j++){
            f32 amp = 0;
            f32 scale = 1;
            f32 total = 0;
            for(u32 d = 0; d < 8; d++){
                u32 div = size >> d;
                u32 xi1 = (j / div) * div;
                u32 yi1 = (i / div) * div;
                u32 xi2 = (xi1 + div) % size;
                u32 yi2 = (yi1 + div) % size;

                f32 xt = (f32)(j - xi1) / (f32)div;
                f32 yt = (f32)(i - yi1) / (f32)div;

                f32 p = bilinearInterpolate(noise[yi1][xi1],
                                            noise[yi1][xi2],
                                            noise[yi2][xi1],
                                            noise[yi2][xi2], xt, yt);                
                amp += p * scale;
                total += scale;
                scale *= 0.5;
            }
            map[i][j] = (amp / total) * 100 - 50;
        }
    }
}

static void updatePlayer(GameState* state){
    InputCodes* c = &state->inputCodes;
    bool* keyInputs = state->keyInputs;
    Player* p = &state->player;
    f32 gravity = state->gravity;
    f32 dt = state->deltaTime;
    Camera* cam = &state->camera;

    

    p->turnAmount = state->mousePositionDelta.x * dt * cam->mouseSensitivity;
    p->turnAngle += p->turnAmount;

    rotate(&p->orientation, Vector3(0, -1, 0), p->turnAmount);

    p->forwardXZ = Vector2(sin(p->turnAngle), cos(p->turnAngle));
    Vector2 rgtVec = Vector2(sin(p->turnAngle + HALF_PI), cos(p->turnAngle + HALF_PI));
    normalize(&p->forwardXZ);

    f32 y = getVerticalPositionInTerrain(&state->terrain, Vector2(p->position.x, p->position.z));
    

    if(keyInputs[c->KEY_D]){
        p->position.x -= rgtVec.x * dt * 10;
        p->position.z += rgtVec.y * dt * 10;
    }
    if(keyInputs[c->KEY_A]){
        p->position.x += rgtVec.x * dt * 10;
        p->position.z -= rgtVec.y * dt * 10;
    }
    if(keyInputs[c->KEY_W]){
        p->position.x -= p->forwardXZ.x * dt * 10;
        p->position.z += p->forwardXZ.y * dt * 10;
    }
    if(keyInputs[c->KEY_S]){
        p->position.x += p->forwardXZ.x * dt * 10;
        p->position.z -= p->forwardXZ.y * dt * 10;
    }

    

    if(!p->isJumping && keyPressedOnce(state, c->KEY_SPACE)){
        p->yVelocity = 50;
        p->isJumping = true;
        p->mesh.animation = &p->squatAnimation;
    }

    if(p->isJumping){
        p->yVelocity += gravity * dt;
        p->position.y += p->yVelocity * dt;
        if(p->position.y < y){
            p->position.y = y;
            p->isJumping = false;
            p->mesh.animation = &p->runAnimation;
        }else if(keyInputs[c->KEY_S]){
            p->yVelocity -= 5;
        }
    }else{
        p->position.y = y;
    }
   
}

static void renderGame(GameState* state){
    Camera* cam = &state->camera;
    Player* p = &state->player;
    u32 tot = state->totalObstacles;

    debugCube(state, state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1));
    
    addTexturedMeshToBuffer(state, &state->terrain.mesh, state->terrain.position, Vector3(1), Quaternion());

    updateMeshAnimation(p->mesh.animation, state->deltaTime);
    state->osFunctions.renderAnimatedMesh(&p->mesh, p->position, p->scale, p->orientation);
}

static void resetGame(GameState* state){
    state->mode = GameMode::GAME_MODE_PLAYING;
    state->clearColor = Vector4(0.3, 0.5, 0.8, 1);
    state->score = 0;
    state->gameTime = 0;

    f32 tot = state->totalObstacles;
    for(u32 i = 0; i < tot; i++){
        Obstacle* o = &state->obstacles[i];
        o->position = Vector3(o->xStartPosition, (randomU32() % 10) + 0.5, 0);
        o->xVelocity = -((s32)randomU32() % 10) - 15;
    }
}

static void initializeGameState(GameState* state){
    CanvasBuffer* cb = &state->canvasBuffer;
    u8 tdat[] = {255, 255, 255, 255};
    cb->defaultTexture = state->osFunctions.createTexture2DFromData(tdat, 1, 1, 4);

    u8 tt1[] = {255, 0, 0, 255, 255, 255, 0, 255,
                255, 255, 0, 255, 255, 0, 0, 255};
    state->testTex1 = state->osFunctions.createTexture2DFromData(tt1, 2, 2, 4);
    u8 tt2[] = {255, 0, 255, 255, 0, 255, 0, 255,
                0, 255, 0, 255, 255, 0, 255, 255};
    state->testTex2 = state->osFunctions.createTexture2DFromData(tt2, 2, 2, 4);

    cb->debugPrinterStartY = state->windowDimenstion.y - 20;
    cb->debugPrinterX = 15;
    cb->debugPrinterY = cb->debugPrinterStartY;

    state->camera = Camera();
    state->camera.position = Vector3(13.75, 6, 25);
    state->camera.orientation = Quaternion();
    state->camera.moveSpeed = 10;
    state->camera.rotateSpeed = 1;
    state->camera.mouseSensitivity = 0.1;

    state->camera.projection = createPerspectiveProjection(70.0, (f32)state->windowDimenstion.x / (f32)state->windowDimenstion.y, 0.001, 1000.0);

    state->light = PointLight();
    state->light.position = Vector3(5, 100, 15);
    state->light.diffuse = Vector3(1, 1, 1);

    

    state->clearColor = Vector4(0.3, 0.5, 0.8, 1);
    state->gameResolution = Vector2(800, 450);

    state->player.orientation = Quaternion();
    state->player.scale = Vector3(1);
    state->player.mesh = state->osFunctions.createAnimatedMesh("character.animesh");
    state->gravity = -100;

    state->player.runAnimation = state->osFunctions.createMeshAnimation("run.animdat");
    state->player.squatAnimation = state->osFunctions.createMeshAnimation("squat.animdat");
    state->player.mesh.animation = &state->player.runAnimation;

    state->totalObstacles = 8;

    for(u32 i = 0; i < state->totalObstacles; i++){
        state->obstacles[i].orientation = Quaternion();
        rotate(&state->obstacles[i].orientation, Vector3(0, 1, 0), -HALF_PI);
        state->obstacles[i].xStartPosition = (i + 1) * 35;
        state->obstacles[i].position = Vector3(state->obstacles[i].xStartPosition, (randomU32() % 10) + 0.5, 0);
        state->obstacles[i].scale = Vector3(1);
        // state->obstacles[i].mesh = state->osFunctions.createTexturedMeshFromFile("suzanne.texmesh");
        // state->obstacles[i].mesh.texture = state->osFunctions.createTexture2DFromFile("suzanne.texpix", 4);
        state->obstacles[i].xVelocity = -((s32)randomU32() % 10) - 15;
        u32 max = -1;
        f32 maxf = max;
        state->obstacles[i].color = Vector4((f32)randomU32() / maxf, (f32)randomU32() / maxf, (f32)randomU32() / maxf, 1);
    }

    state->mode = GameMode::GAME_MODE_DEBUG;

    state->gameTime = 0;
    state->score = 0;

    u32 lll;
    if(!state->osFunctions.readFileIntoBuffer("hiscore", &state->hiScore, &lll)){
        state->osFunctions.writeToFile("hiscore", &state->hiScore, sizeof(u32));
    }

    u32 tWidth = 256;
    u32 tHeight = 256;
    state->terrain.width = tWidth;
    state->terrain.height = tHeight;
    u32 tPosSize = tWidth * tHeight * sizeof(Vector3);
    u32 terVertSize = tHeight * tWidth * 8 * sizeof(f32);
    u32 ctr = 0;
    f32 *terrainVerts = (f32*)state->storage.tempMemoryBuffer;
    u32 *terrainElms = (u32*)state->storage.tempMemoryBuffer + terVertSize;
    state->terrain.heightmap = (f32**)allocateLongTermMemory(state, sizeof(f32*) * tHeight);
    for(u32 i = 0; i < tHeight; i++){
        state->terrain.heightmap[i] = (f32*)allocateLongTermMemory(state, sizeof(f32) * tWidth);
    }

    generateTerrainMap(state->terrain.heightmap);

    for(u32 i = 0; i < tHeight; i++){
        for(u32 j = 0; j < tWidth; j++){
            Vector3 pos(j, state->terrain.heightmap[i][j], i);
            Vector3 norm;

            if(i == 0){
                if(j == 0){
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    norm = cross(vlo - pos, vrt - pos);
                }else if(j == tWidth - 1){
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    Vector3 vll(j - 1, state->terrain.heightmap[i + 1][j - 1], i + 1);
                    Vector3 n1 = cross(vll - vlt, pos - vlt);
                    Vector3 n2 = cross(pos - vlo, vlt - vlo);
                    norm = (n1 + n2) * 0.5;
                }else{
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    Vector3 vll(j - 1, state->terrain.heightmap[i + 1][j - 1], i + 1);
                    Vector3 n1 = cross(vll - vlt, pos - vlt);
                    Vector3 n2 = cross(pos - vlo, vlt - vlo);
                    Vector3 n3 = cross(vlo - pos, vrt - pos);
                    norm = (n1 + n2 + n3) * 0.3333333333;
                }
            }else if(i == tHeight - 1){
                if(j == 0){
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    Vector3 vhr(j + 1, state->terrain.heightmap[i - 1][j + 1], i - 1);
                    Vector3 n1 = cross(pos - vhi, vhr - vhi);
                    Vector3 n2 = cross(vhr - vrt, pos - vrt);
                    norm = (n1 + n2) * 0.5;
                }else if(j == tWidth - 1){
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    norm = cross(vhi - pos, vlt - pos);
                }else{
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    Vector3 vhr(j + 1, state->terrain.heightmap[i - 1][j + 1], i - 1);
                    Vector3 n1 = cross(vhi - pos, vlt - pos);
                    Vector3 n2 = cross(pos - vhi, vhr - vhi);
                    Vector3 n3 = cross(vhr - vrt, pos - vrt);
                    norm = (n1 + n2 + n3) * 0.3333333333;
                }
            }else{
                if(j == 0){
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    Vector3 vhr(j + 1, state->terrain.heightmap[i - 1][j + 1], i - 1);
                    Vector3 n1 = cross(pos - vhi, vhr - vhi);
                    Vector3 n2 = cross(vhr - vrt, pos - vrt);
                    Vector3 n3 = cross(vlo - pos, vrt - pos);
                    norm = (n1 + n2 + n3) * 0.3333333333;
                }else if(j == tWidth - 1){
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vll(j - 1, state->terrain.heightmap[i + 1][j - 1], i + 1);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    Vector3 n1 = cross(vhi - pos, vlt - pos);
                    Vector3 n2 = cross(vll - vlt, pos - vlt);
                    Vector3 n3 = cross(pos - vlo, vll - vlo);
                    norm = (n1 + n2 + n3) * 0.3333333333;
                }else{
                    Vector3 vlt(j - 1, state->terrain.heightmap[i][j - 1], i);
                    Vector3 vll(j - 1, state->terrain.heightmap[i + 1][j - 1], i + 1);
                    Vector3 vhi(j, state->terrain.heightmap[i - 1][j], i - 1);
                    Vector3 vlo(j, state->terrain.heightmap[i + 1][j], i + 1);
                    Vector3 vhr(j + 1, state->terrain.heightmap[i - 1][j + 1], i - 1);
                    Vector3 vrt(j + 1, state->terrain.heightmap[i][j + 1], i);
                    Vector3 n1 = cross(vll - vlt, pos - vlt);
                    Vector3 n2 = cross(vhi - pos, vlt - pos);
                    Vector3 n3 = cross(pos - vhi, vhr - vhi);
                    Vector3 n4 = cross(vhr - vrt, pos - vrt);
                    Vector3 n5 = cross(vrt - pos, vlo - pos);
                    Vector3 n6 = cross(pos - vlo, vll - vlo);
                    norm = (n1 + n2 + n3 + n4 + n5 + n6) * (1.0/6.0);
                }
            }

            norm = normalOf(norm);

            terrainVerts[ctr++] = pos.x;
            terrainVerts[ctr++] = pos.y;
            terrainVerts[ctr++] = pos.z;
            terrainVerts[ctr++] = norm.x;
            terrainVerts[ctr++] = norm.y;
            terrainVerts[ctr++] = norm.z;
            terrainVerts[ctr++] = (f32)j / (f32)tWidth;
            terrainVerts[ctr++] = (f32)i / (f32)tHeight;
        }
    }

    state->terrain.position = Vector3(-128, 0, -128);

    ctr = 0;
    u32 totElmSize = (tWidth - 1) * (tHeight - 1) * 6 * sizeof(u32);
    for(u32 i = 0; i < tHeight - 1; i++){
        for(u32 j = 0; j < tWidth - 1; j++){
            terrainElms[ctr++] = (i * tWidth) + tWidth + j;
            terrainElms[ctr++] = i * tWidth + j;
            terrainElms[ctr++] = (i * tWidth + j) + 1;
            terrainElms[ctr++] = (i * tWidth + j) + 1;
            terrainElms[ctr++] = (i * tWidth) + tWidth + 1 + j;
            terrainElms[ctr++] = (i * tWidth) + tWidth + j;
        }
    }

    state->terrain.mesh = state->osFunctions.createTexturedMeshFromData(terrainVerts, terVertSize, terrainElms, totElmSize);
    state->terrain.mesh.texture = state->osFunctions.createTexture2DFromFile("terrain.texpix", 4);
    state->gameOver = false;
    state->isInitialized = true;
}

extern "C" void updateGameState(GameState* state){
    if(!state->isInitialized){
        initializeGameState(state);
    }

    Camera* cam = &state->camera;
    InputCodes* c = &state->inputCodes;
    Player* p = &state->player;
    f32 deltaTime = state->deltaTime;

    state->mousePositionDelta = Vector2(state->mousePosition.x - state->windowDimenstion.x * 0.5, 
                                        state->mousePosition.y - state->windowDimenstion.y * 0.5);
    switch (state->mode) {
        case GameMode::GAME_MODE_DEBUG : { 
            updateCameraWithMouse(state);
            updateCameraWithKeyboard(state);
            updateCameraWithGamepad(state);
            updateCameraView(&state->camera);
            renderGame(state);
            
            break;
        }
        case GameMode::GAME_MODE_PLAYING : {
            updatePlayer(state);

            p->cameraHeight += (s32)state->mousePositionDelta.y * deltaTime;
            
            cam->position = p->position;
            cam->position.x += p->forwardXZ.x * 10;
            cam->position.y = p->cameraHeight;
            cam->position.z += -p->forwardXZ.y * 10;
            lookAt(cam, cam->position, p->position);

            renderGame(state);
            state->gameTime += state->deltaTime;
            state->score = (state->gameTime * 100);
            break;
        }
        case GameMode::GAME_MODE_OVER : {
            addTextToCanvas(state, "GAME OVER", Vector2(350, 300), 8, Vector4(1, 0, 0, 1));
            addTextToCanvas(state, "Press R to restart", Vector2(350, 250), 4, Vector4(1, 0, 0, 1));
            renderGame(state);
            if(keyPressedOnce(state, c->KEY_R)){
                resetGame(state);
            }
            break;
        }
        case GameMode::GAME_MODE_INTRO : {
            addTextToCanvas(state, "GAME TITLE", Vector2(300, 500), 10, Vector4(1, 1, 1, 1));
            addTextToCanvas(state, "Press SPACE to Begin", Vector2(300, 450), 6, Vector4(1, 1, 1, 1));
            if(keyPressedOnce(state, c->KEY_SPACE) || guiButton(state, Vector2(450, 350), "START", 5)){
                state->mode = GameMode::GAME_MODE_PLAYING;
            }
            break;
        }
    }

    if(keyPressedOnce(state, c->KEY_B)){
        if(state->mode == GameMode::GAME_MODE_DEBUG){
            state->mode = GameMode::GAME_MODE_PLAYING;
        }else{
            initializeGameState(state);
            state->mode = GameMode::GAME_MODE_DEBUG;
        }
    }


    //lookAt(&state->camera, Vector3(-20, 40, -50), Vector3(0));
    //renderGame(state);


    debugPrint(state, "debug mode:%b", state->mode == GameMode::GAME_MODE_DEBUG);
    debugPrint(state, "delta time: %f4", state->deltaTime);
}