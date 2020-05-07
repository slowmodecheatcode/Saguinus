#pragma once

#define MAX_STRINGS 256
#define MAX_STRING_LENGTH 256

struct TextBuffer {
    s8 strings[MAX_STRINGS][MAX_STRING_LENGTH];
    Vector4 colors[MAX_STRINGS];
    f32 xPositions[MAX_STRINGS];
    f32 yPositions[MAX_STRINGS];
    f32 scales[MAX_STRINGS];
    u32 totalStrings;
    f32 debugPrinterX;
    f32 debugPrinterY;
    f32 debugPrinterStartY;
};

struct GameState {
    TextBuffer textBuffer;

    u32 windowWidth;
    u32 windowHeight;
};