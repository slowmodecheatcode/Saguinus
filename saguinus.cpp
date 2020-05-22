#include "saguinus.h"

static u32 randomU32(u32 sd = 1){
    static u32 seed = sd;
    seed = xorshift(seed);
    return seed;
}

static Vector2 getTextDimensions(Font* font, const s8* str, f32 scale){
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

    return dim;
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
    Vector2 dim = getTextDimensions(state->currentFont, str, scale);
    f32 amt = (border * scale);
    f32 amtX2 = amt * 2;
    dim = Vector2(dim.x + amtX2, dim.y + amtX2);
    addQuadToBuffer(state, position, dim, quadColor);
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

static bool guiButton(GameState* state, Vector2 position, const s8* text, f32 scale){
    static bool action;

    Vector2 mp = state->mousePosition;
    Vector2 dim = getTextDimensions(state->currentFont, text, scale);

    if(mp.x < position.x || mp.x > position.x + dim.x 
    || mp.y < position.y || mp.y > position.y + dim.y){
        addTextQuad(state, text, position, Vector4(1), Vector4(0, 0, 1, 1), scale, 0);
        action = false;
    }else{
        if(state->mouseInputs[MOUSE_BUTTON_LEFT]){
            addTextQuad(state, text, position, Vector4(1), Vector4(1, 0, 0, 1), scale, 0);
            action = true;
        }else if(action && !state->mouseInputs[MOUSE_BUTTON_LEFT]){
            addTextQuad(state, text, position, Vector4(1), Vector4(1, 0, 0, 1), scale, 0);
            action = false;
            return true;
        }else{
            addTextQuad(state, text, position, Vector4(1), Vector4(0, 1, 0, 1), scale, 0);
            action = false;
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

static void updateCameraWithMouse(GameState* state){
    Camera* camera = &state->camera;
    f32 deltaTime = state->deltaTime;
    if(state->updateCamera){
        f32 xDif = state->mousePosition.x - state->windowDimenstion.x * 0.5;
        f32 yDif = state->mousePosition.y - state->windowDimenstion.y * 0.5;
        rotate(&camera->orientation, camera->up, deltaTime * xDif * camera->mouseSensitivity);
        rotate(&camera->orientation, camera->right, deltaTime * yDif * camera->mouseSensitivity);
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
        }
    }
    
}

static void updatePlayer(GameState* state){
    InputCodes* c = &state->inputCodes;
    bool* keyInputs = state->keyInputs;
    Player* p = &state->player;
    f32 gravity = state->gravity;
    f32 dt = state->deltaTime;

    if(!p->isJumping && keyPressedOnce(state, c->KEY_W)){
        p->yVelocity = 50;
        p->isJumping = true;
    }

    if(p->isJumping){
        p->yVelocity += gravity * dt;
        p->position.y += p->yVelocity * dt;
        if(p->position.y < 0){
            p->position = 0;
            p->isJumping = false;
        }else if(keyInputs[c->KEY_S]){
            p->yVelocity -= 5;
        }
    }
   
}

static void renderGame(GameState* state){
    Camera* cam = &state->camera;
    Player* p = &state->player;
    u32 tot = state->totalObstacles;
    debugCube(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.8, 0.5, 0.2, 1));
    debugBox(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.4, 0.25, 0.1, 1), 0.25);
    debugCube(state, state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1));
    for(u32 i = 0; i < tot; i++){
        Obstacle* o = &state->obstacles[i];
        debugCube(state, o->position, o->scale, o->color);
    }
    // addTexturedMeshToBuffer(state, &p->mesh, p->position, p->scale, p->orientation);
    // addTexturedMeshToBuffer(state, &o->mesh, o->position, o->scale, o->orientation);
    debugCube(state, p->position, p->scale, Vector4(0, 0, 1, 1));


    
    s8 buf[64];
    createDebugString(buf, "Score: %i", state->score);
    addTextToCanvas(state, buf, Vector2(450, 650), 3, Vector4(0, 0, 0, 1));
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
    state->camera.moveSpeed = 3;
    state->camera.rotateSpeed = 1;
    state->camera.mouseSensitivity = 0.1;

    state->camera.projection = createPerspectiveProjection(70.0, (f32)state->windowDimenstion.x / (f32)state->windowDimenstion.y, 0.001, 1000.0);

    state->light = PointLight();
    state->light.position = Vector3(5, 15, 15);
    state->light.diffuse = Vector3(1, 1, 1);

    state->storage.tempMemoryBuffer = (u8*)state->osFunctions.allocateMemory(MEGABYTE(32));
    state->storage.longTermBuffer = (u8*)state->osFunctions.allocateMemory(GIGABYTE(1));

    state->clearColor = Vector4(0.3, 0.5, 0.8, 1);
    state->gameResolution = Vector2(800, 450);

    state->player.orientation = Quaternion();
    rotate(&state->player.orientation, Vector3(0, 1, 0), HALF_PI);
    state->player.scale = Vector3(1);
    state->player.mesh = state->osFunctions.createTexturedMesh("character.texmesh");
    state->gravity = -100;

    state->totalObstacles = 8;

    for(u32 i = 0; i < state->totalObstacles; i++){
        state->obstacles[i].orientation = Quaternion();
        rotate(&state->obstacles[i].orientation, Vector3(0, 1, 0), -HALF_PI);
        state->obstacles[i].xStartPosition = (i + 1) * 35;
        state->obstacles[i].position = Vector3(state->obstacles[i].xStartPosition, (randomU32() % 10) + 0.5, 0);
        state->obstacles[i].scale = Vector3(1);
        state->obstacles[i].mesh = state->osFunctions.createTexturedMesh("suzanne.texmesh");
        state->obstacles[i].mesh.texture = state->osFunctions.createTexture2DFromFile("suzanne.texpix", 4);
        state->obstacles[i].xVelocity = -((s32)randomU32() % 10) - 15;
        u32 max = -1;
        f32 maxf = max;
        state->obstacles[i].color = Vector4((f32)randomU32() / maxf, (f32)randomU32() / maxf, (f32)randomU32() / maxf, 1);
    }

    state->mode = GameMode::GAME_MODE_INTRO;

    state->gameTime = 0;
    state->score = 0;

    state->gameOver = false;
    state->isInitialized = true;
}
u32 zzz = 0;
extern "C" void updateGameState(GameState* state){
    if(!state->isInitialized){
        initializeGameState(state);
    }

    Camera* cam = &state->camera;
    InputCodes* c = &state->inputCodes;
    Player* p = &state->player;

    // switch (state->mode) {
    //     case GameMode::GAME_MODE_DEBUG : {
    //         updateCameraWithMouse(state);
    //         updateCameraWithKeyboard(state);
    //         updateCameraWithGamepad(state);
    //         renderGame(state);
    //         break;
    //     }
    //     case GameMode::GAME_MODE_PLAYING : {
    //         updatePlayer(state);
    //         updateObstacles(state);
            
    //         renderGame(state);
    //         state->gameTime += state->deltaTime;
    //         state->score = (state->gameTime * 100);
    //         break;
    //     }
    //     case GameMode::GAME_MODE_OVER : {
    //         addTextToBuffer(state, "GAME OVER", 350, 300, 8, Vector4(1, 0, 0, 1));
    //         addTextToBuffer(state, "Press R to restart", 350, 250, 4, Vector4(1, 0, 0, 1));
    //         renderGame(state);
    //         if(keyPressedOnce(state, c->KEY_R)){
    //             resetGame(state);
    //         }
    //         break;
    //     }
    //     case GameMode::GAME_MODE_INTRO : {
    //         addTextToBuffer(state, "GAME TITLE", 300, 500, 10, Vector4(1, 1, 1, 1));
    //         addTextToBuffer(state, "Press SPACE to Begin", 300, 450, 6, Vector4(1, 1, 1, 1));
    //         if(keyPressedOnce(state, c->KEY_SPACE)){
    //             state->mode = GameMode::GAME_MODE_PLAYING;
    //         }
    //         break;
    //     }
    // }

    if(keyPressedOnce(state, c->KEY_B)){
        if(state->mode != GameMode::GAME_MODE_DEBUG){
            state->mode = GameMode::GAME_MODE_DEBUG;
        }else{
            initializeGameState(state);
        }
    }

    //updateCameraView(&state->camera);
    lookAt(&state->camera, Vector3(-20, 40, -50), Vector3(0));
    renderGame(state);

    if(guiButton(state, Vector2(250, 250), "CLICK ME!!!!", 5)){
        zzz++;
    }
    debugPrint(state, "%i", zzz);


    debugPrint(state, "debug mode:%b", state->debugMode);
    debugPrint(state, "delta time: %f4", state->deltaTime);
}