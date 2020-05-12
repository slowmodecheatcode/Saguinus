#include "saguinus.h"

static void addTextToBuffer(const s8* str, f32 x, f32 y, f32 scale, Vector4 color, GameState* state){
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

    addTextToBuffer(buf, buffer->debugPrinterX, buffer->debugPrinterY, 2, Vector4(0, 0, 0, 1), state);
    buffer->debugPrinterY -= 25;

    va_end(argptr);
}

static void debugCube(Vector3 position, Vector3 scale, Vector4 color, GameState* state){
    DebugBuffer* buffer = &state->debugBuffer;
    u32 idx = buffer->totalCubes++;
    buffer->cubePositions[idx] = position;
    buffer->cubeScales[idx] = scale;
    buffer->cubeColors[idx] = color;
}

static void debugLine(Vector3 start, Vector3 end, Vector4 color, f32 lineWidth, GameState* state){
    DebugBuffer* buffer = &state->debugBuffer;
    u32 idx = buffer->totalLines++;
    buffer->lineStarts[idx] = start;
    buffer->lineEnds[idx] = end;
    buffer->lineWidths[idx] = lineWidth;
    buffer->lineColors[idx] = color;
}

static void debugBox(Vector3 position, Vector3 scale, Vector4 color, f32 lineWidth, GameState* state){
    Vector3 halfScale = scale * 0.5;
    Vector3 p1(position.x - halfScale.x, position.y - halfScale.y, position.z - halfScale.z);
    Vector3 p2(position.x - halfScale.x, position.y + halfScale.y, position.z - halfScale.z);
    Vector3 p3(position.x + halfScale.x, position.y + halfScale.y, position.z - halfScale.z);
    Vector3 p4(position.x + halfScale.x, position.y - halfScale.y, position.z - halfScale.z);

    Vector3 p5(position.x - halfScale.x, position.y - halfScale.y, position.z + halfScale.z);
    Vector3 p6(position.x - halfScale.x, position.y + halfScale.y, position.z + halfScale.z);
    Vector3 p7(position.x + halfScale.x, position.y + halfScale.y, position.z + halfScale.z);
    Vector3 p8(position.x + halfScale.x, position.y - halfScale.y, position.z + halfScale.z);

    debugLine(p1, p2, color, lineWidth, state);
    debugLine(p2, p3, color, lineWidth, state);
    debugLine(p3, p4, color, lineWidth, state);
    debugLine(p4, p1, color, lineWidth, state);

    debugLine(p5, p6, color, lineWidth, state);
    debugLine(p6, p7, color, lineWidth, state);
    debugLine(p7, p8, color, lineWidth, state);
    debugLine(p8, p5, color, lineWidth, state);

    debugLine(p1, p5, color, lineWidth, state);
    debugLine(p2, p6, color, lineWidth, state);
    debugLine(p3, p7, color, lineWidth, state);
    debugLine(p4, p8, color, lineWidth, state);
}

static void addTexturedMeshToBuffer(TexturedMesh* mesh, Vector3 position, Vector3 scale, Quaternion orientation, GameState* state){
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
        camera->position -= camera->forward * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_S]){
        camera->position += camera->forward * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_A]){
       camera->position += camera->right * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_D]){
        camera->position -= camera->right * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_R]){
       camera->position -= camera->up * deltaTime * camera->moveSpeed;
    }
    if(keyInputs[c->KEY_F]){
        camera->position += camera->up * deltaTime * camera->moveSpeed;
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
        camera->position -= camera->right * deltaTime * camera->moveSpeed * gamepad1->leftStickX;
    }
    if(gamepad1->leftStickY > 0.05 || gamepad1->leftStickY < -0.05){
        camera->position -= camera->forward * deltaTime * camera->moveSpeed * gamepad1->leftStickY;
    }
    if(gamepad1->buttons[c->GAMEPAD_LB]){
        camera->position += camera->up * deltaTime * camera->moveSpeed;
    }
    if(gamepad1->buttons[c->GAMEPAD_RB]){
        camera->position -= camera->up * deltaTime * camera->moveSpeed;
    }
}

static void initialzeGameState(GameState* state){
    TextBuffer* tb = &state->textBuffer;
    tb->debugPrinterStartY = state->windowDimenstion.y - 50;
    tb->debugPrinterX = 25;
    tb->debugPrinterY = tb->debugPrinterStartY;

    state->camera.position.z -= 5;
    state->camera.moveSpeed = 3;
    state->camera.rotateSpeed = 1;
    state->camera.mouseSensitivity = 0.1;

    state->camera.projection = createPerspectiveProjection(70.0, (f32)state->windowDimenstion.x / (f32)state->windowDimenstion.y, 0.001, 1000.0);

    state->light.position = Vector3(5, 5, -3);
    state->light.diffuse = Vector3(1, 1, 1);

    state->storage.tempMemoryBuffer = (u8*)state->osFunctions.allocateMemory(MEGABYTE(32));

    state->clearColor = Vector4(0.3, 0.5, 0.8, 1);
    state->gameResolution = Vector2(800, 450);

    state->mesh = state->osFunctions.createTexturedMesh("character.texmesh");
}

extern "C" void updateGameState(GameState* state){
    InputCodes* c = &state->inputCodes;
    updateCameraWithMouse(state);
    updateCameraWithKeyboard(state);
    updateCameraWithGamepad(state);
    
    if(state->keyInputs[c->KEY_SPACE]){
        debugPrint(state, "SPACE IS PRESSED");
    }
    
    debugCube(state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1), state);

    addTexturedMeshToBuffer(&state->mesh, Vector3(0), Vector3(1), Quaternion(), state);
}