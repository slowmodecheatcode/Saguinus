#include "saguinus.h"

static void addTextToBuffer(const s8* str, f32 x, f32 y, f32 scale, Vector4 color, GameState* state){
    TextBuffer* buffer = &state->textBuffer;

    const s8* c = str;
    u32 ctr = 0;
    u32 idx = buffer->totalStrings++;
    while(*c != '\0'){
        buffer->strings[idx][ctr++] = *c;
        c++;
    }
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

static void initialzeGameState(GameState* state){
    TextBuffer* tb = &state->textBuffer;
    tb->debugPrinterStartY = state->windowHeight - 50;
    tb->debugPrinterX = 25;
    tb->debugPrinterY = tb->debugPrinterStartY;
}

static void updateGameState(GameState* state){
    debugCube(state->light.position, Vector3(0.25), Vector4(0.9, 0.9, 1, 1), state);
    debugLine(Vector3(-3, -3, -3), Vector3(3, 3, 3), Vector4(1, 0, 0, 1), 0.5, state);
    debugBox(Vector3(-1, -1, -1), Vector3(3, 4, 2), Vector4(0, 1, 0, 1), 0.25, state);

    addTexturedMeshToBuffer(&state->mesh, Vector3(-4, 0, 0), Vector3(1), Quaternion(), state);

    debugPrint(state, "TESTTRRSRS");
    debugPrint(state, "T%f", 1235.1541864);

    addTextToBuffer("MOAR TESTSTS13#@$(#@", 300, 300, 4, Vector4(.7, .4, .2, 1), state);
    addTextToBuffer("MOAR TESTSTS13#@$(#@", 300, 250, 3.5, Vector4(0, .4, .7, 1), state);
}