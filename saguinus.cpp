#include "saguinus.h"

static void addTextToBuffer(GameState* state, const s8* str, f32 x, f32 y, f32 scale, Vector4 color){
    TextBuffer* buffer = &state->textBuffer;

    const s8* c = str;
    u32 ctr = 0;
    u32 idx = buffer->totalStrings++;
    concatenateCharacterStrings(buffer->strings[idx], (s8*)str, &ctr);
    buffer->colors[idx] = color;
    buffer->xPositions[idx] = x;
    buffer->yPositions[idx] = y;
    buffer->scales[idx] = scale;
}

static void debugPrint(GameState* state, s8* text, ...){
    TextBuffer* buffer = &state->textBuffer;
    va_list argptr;
    va_start(argptr, text);
    s8 buf[MAX_STRING_LENGTH];
    createDebugString(buf, text, argptr);

    addTextToBuffer(state, buf, buffer->debugPrinterX, buffer->debugPrinterY, 2, Vector4(0, 0, 0, 1));
    buffer->debugPrinterY -= 25;

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

static void updateObstacle(GameState* state){
    Obstacle* o = &state->obstacle;
    f32 dt = state->deltaTime;
    o->position.x += dt * o->xVelocity;
    if(o->position.x < -10){
        o->position.x = o->xStartPosition;
    }
}

static void updatePlayer(GameState* state){
    InputCodes* c = &state->inputCodes;
    bool* keyInputs = state->keyInputs;
    Player* p = &state->player;
    f32 gravity = state->gravity;
    f32 dt = state->deltaTime;

    if(!p->isJumping && keyPressedOnce(state, c->KEY_W)){
        p->yVelocity = 30;
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

static void checkForPlayerCollision(GameState* state){
    Player* p = &state->player;
    Obstacle* o = &state->obstacle;
    f32 len = length(o->position - p->position);
    if(len < 1){
        state->clearColor = Vector4(0, 0, 0, 1);
        state->mode = GameMode::GAME_MODE_OVER;
    }
}

static void initializeGameState(GameState* state){
    TextBuffer* tb = &state->textBuffer;
    tb->debugPrinterStartY = state->windowDimenstion.y - 50;
    tb->debugPrinterX = 25;
    tb->debugPrinterY = tb->debugPrinterStartY;

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

    state->obstacle.orientation = Quaternion();
    rotate(&state->obstacle.orientation, Vector3(0, 1, 0), -HALF_PI);
    state->obstacle.xStartPosition = 35;
    state->obstacle.position = Vector3(state->obstacle.xStartPosition, 0.5, 0);
    state->obstacle.scale = Vector3(1);
    state->obstacle.mesh = state->osFunctions.createTexturedMesh("suzanne.texmesh");
    state->obstacle.mesh.texture = state->osFunctions.createTexture2D("suzanne.texpix", 4);
    state->obstacle.xVelocity = -15;

    state->mode = GameMode::GAME_MODE_INTRO;

    state->gameTime = 0;
    state->score = 0;

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
    Obstacle* o = &state->obstacle;

    switch (state->mode) {
        case GameMode::GAME_MODE_DEBUG : {
            updateCameraWithMouse(state);
            updateCameraWithKeyboard(state);
            updateCameraWithGamepad(state);
            break;
        }
        case GameMode::GAME_MODE_PLAYING : {
            updatePlayer(state);
            updateObstacle(state);
            checkForPlayerCollision(state);
            debugCube(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.8, 0.5, 0.2, 1));
            debugBox(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.4, 0.25, 0.1, 1), 0.25);
            debugCube(state, state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1));

            addTexturedMeshToBuffer(state, &p->mesh, p->position, p->scale, p->orientation);
            addTexturedMeshToBuffer(state, &o->mesh, o->position, o->scale, o->orientation);
            s8 buf[32];
            createDebugString(buf, "Score: %i", state->score);
            addTextToBuffer(state, buf, 200, 550, 3, Vector4(0, 0, 0, 1));
            // debugPrint(state, "score: %i", state->score);
            state->gameTime += state->deltaTime;
            state->score = (state->gameTime * 100);
            break;
        }
        case GameMode::GAME_MODE_OVER : {
            addTextToBuffer(state, "GAME OVER", 350, 300, 8, Vector4(1, 0, 0, 1));
            addTextToBuffer(state, "Press R to restart", 350, 250, 4, Vector4(1, 0, 0, 1));
            debugCube(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.8, 0.5, 0.2, 1));
            debugBox(state, Vector3(0, -1, 0), Vector3(100, 1, 10), Vector4(0.4, 0.25, 0.1, 1), 0.25);
            debugCube(state, state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1));

            addTexturedMeshToBuffer(state, &p->mesh, p->position, p->scale, p->orientation);
            addTexturedMeshToBuffer(state, &o->mesh, o->position, o->scale, o->orientation);
            s8 buf[32];
            createDebugString(buf, "Score: %i", state->score);
            addTextToBuffer(state, buf, 200, 550, 3, Vector4(1, 0, 0, 1));
            if(keyPressedOnce(state, c->KEY_R)){
                state->obstacle.position = Vector3(state->obstacle.xStartPosition, 0.5, 0);
                state->mode = GameMode::GAME_MODE_PLAYING;
                state->clearColor = Vector4(0.3, 0.5, 0.8, 1);
                state->score = 0;
                state->gameTime = 0;
            }
            break;
        }
        case GameMode::GAME_MODE_INTRO : {
            addTextToBuffer(state, "GAME TITLE", 300, 500, 10, Vector4(1, 1, 1, 1));
            addTextToBuffer(state, "Press SPACE to Begin", 300, 450, 6, Vector4(1, 1, 1, 1));
            if(keyPressedOnce(state, c->KEY_SPACE)){
                state->mode = GameMode::GAME_MODE_PLAYING;
            }
            break;
        }
    }

    if(keyPressedOnce(state, c->KEY_B)){
        if(state->mode != GameMode::GAME_MODE_DEBUG){
            state->mode = GameMode::GAME_MODE_DEBUG;
        }else{
            initializeGameState(state);
        }
    }

    updateCameraView(&state->camera);

    debugPrint(state, "debug mode:%b", state->debugMode);
    debugPrint(state, "delta time: %f4", state->deltaTime);
}