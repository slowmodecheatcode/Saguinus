#include "saguinus.h"

static void addTextToBuffer(const s8* str, f32 x, f32 y, f32 scale, Vector4 color, GameState* state){
    TextBuffer* buffer = &state->textBuffer;

    const s8* c = str;
    u32 ctr = 0;
    u32 idx = buffer->totalStrings;
    while(*c != '\0'){
        buffer->strings[idx][ctr++] = *c;
        c++;
    }
    buffer->colors[idx] = color;
    buffer->xPositions[idx] = x;
    buffer->yPositions[idx] = y;
    buffer->scales[idx] = scale;
    buffer->totalStrings++;
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

static void initialzeGameState(GameState* state){
    TextBuffer* tb = &state->textBuffer;
    tb->debugPrinterStartY = state->windowHeight - 50;
    tb->debugPrinterX = 25;
    tb->debugPrinterY = tb->debugPrinterStartY;
    
}

static void updateGameState(GameState* state){
    debugPrint(state, "TESTTRRSRS");
    debugPrint(state, "T%f", 1235.1541864);
    debugPrint(state, "TESTTRRSRS");
    debugPrint(state, "TESTTRRSRS");
}