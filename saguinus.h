#pragma once

#define MAX_STRINGS 256
#define MAX_STRING_LENGTH 256
#define MAX_DEBUG_CUBES 256
#define MAX_DEBUG_LINES 256
#define MAX_TEXTURED_MESHES 256

static u8 KEY_0;
static u8 KEY_1;
static u8 KEY_2;
static u8 KEY_3;
static u8 KEY_4;
static u8 KEY_5;
static u8 KEY_6;
static u8 KEY_7;
static u8 KEY_8;
static u8 KEY_9;
static u8 KEY_A;
static u8 KEY_B;
static u8 KEY_C;
static u8 KEY_D;
static u8 KEY_E;
static u8 KEY_F;
static u8 KEY_G;
static u8 KEY_H;
static u8 KEY_I;
static u8 KEY_J;
static u8 KEY_K;
static u8 KEY_L;
static u8 KEY_M;
static u8 KEY_N;
static u8 KEY_O;
static u8 KEY_P;
static u8 KEY_Q;
static u8 KEY_R;
static u8 KEY_S;
static u8 KEY_T;
static u8 KEY_U;
static u8 KEY_V;
static u8 KEY_W;
static u8 KEY_X;
static u8 KEY_Y;
static u8 KEY_Z;
static u8 KEY_F1;
static u8 KEY_F2;
static u8 KEY_F3;
static u8 KEY_F4;
static u8 KEY_F5;
static u8 KEY_F6;
static u8 KEY_F7;
static u8 KEY_F8;
static u8 KEY_F9;
static u8 KEY_F10;
static u8 KEY_F11;
static u8 KEY_F12;
static u8 KEY_LEFT_SHIFT;
static u8 KEY_RIGHT_SHIFT;
static u8 KEY_LEFT_CTRL;
static u8 KEY_RIGHT_CTRL;
static u8 KEY_UP;
static u8 KEY_DOWN;
static u8 KEY_LEFT;
static u8 KEY_RIGHT;
static u8 KEY_SPACE;

static u8 GAMEPAD_A;
static u8 GAMEPAD_B;
static u8 GAMEPAD_X;
static u8 GAMEPAD_Y;
static u8 GAMEPAD_LB;
static u8 GAMEPAD_RB;
static u8 GAMEPAD_L3;
static u8 GAMEPAD_R3;
static u8 GAMEPAD_D_UP;
static u8 GAMEPAD_D_DONW;
static u8 GAMEPAD_D_LEFT;
static u8 GAMEPAD_D_RIGHT;
static u8 GAMEPAD_START;
static u8 GAMEPAD_BACK;

struct Texture2D {
    void* texture;
};

struct Font {
    u32 bitmapWidth;
    u32 bitmapHeight;
    u32 totalCharacters;
    u32 missingCharacterCodeIndex;
    u16* characterCodes;
    f32* xOffset;
    f32* yOffset;
    f32* width;
    f32* height;
    f32* bitmapX;
    f32* bitmapY;
    f32* bitmapCharacterWidth;
    f32* bitmapCharacterHeight;
    f32* kerning;
    Texture2D bitmap;
};

struct TexturedMesh {
    Quaternion orientation;
    Vector3 position;
    Vector3 scale;
    Texture2D texture;
    u32 indexCount;
    u32 indexOffset;
    u32 indexAddon;
};

struct TexturedMeshBuffer {
    Vector3 positions[MAX_TEXTURED_MESHES];
    Vector3 scales[MAX_TEXTURED_MESHES];
    Quaternion orientations[MAX_TEXTURED_MESHES];
    Texture2D textures[MAX_TEXTURED_MESHES];
    u32 indexCounts[MAX_TEXTURED_MESHES];
    u32 indexOffsets[MAX_TEXTURED_MESHES];
    u32 indexAddons[MAX_TEXTURED_MESHES];
    u32 totalMeshes;
};

struct TextBuffer {
    s8 strings[MAX_STRINGS][MAX_STRING_LENGTH];
    Vector4 colors[MAX_STRINGS];
    f32 xPositions[MAX_STRINGS];
    f32 yPositions[MAX_STRINGS];
    f32 scales[MAX_STRINGS];
    f32 debugPrinterX;
    f32 debugPrinterY;
    f32 debugPrinterStartY;
    u32 totalStrings;
};

struct DebugBuffer {
    Vector3 cubePositions[MAX_DEBUG_CUBES];
    Vector3 cubeScales[MAX_DEBUG_CUBES];
    Vector3 lineStarts[MAX_DEBUG_LINES];
    Vector3 lineEnds[MAX_DEBUG_CUBES];
    Vector4 lineColors[MAX_DEBUG_LINES];
    Vector4 cubeColors[MAX_DEBUG_CUBES];
    f32 lineWidths[MAX_DEBUG_LINES];
    u32 totalCubes;
    u32 totalLines;
};

struct OSFunctions {
    void* (*allocateMemory)(u32 amount);
    void (*readFileIntoBuffer)(const s8* fileName, void* data, u32* fileLength);
    Texture2D (*createTexture2D)(const s8* fileName, u32 bytesPerPixel);
    TexturedMesh (*createTexturedMesh)(const s8* fileName);
};

struct MemoryStorage {
    u8* tempMemoryBuffer;
};

struct Gamepad {
    bool buttons[16];
    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;
    f32 leftTrigger;
    f32 rightTrigger;
};

struct GameState {
    TextBuffer textBuffer;
    DebugBuffer debugBuffer;
    TexturedMeshBuffer txtdMeshBuffer;

    OSFunctions osFunctions;
    MemoryStorage storage;

    Camera camera;
    PointLight light;

    Gamepad gamepad1;

    TexturedMesh mesh;

    Vector4 clearColor;
    Vector2 mousePosition;

    Vector2 windowDimenstion;
    Vector2 gameResolution;

    bool* keyInputs;
    bool* mouseInputs;
    f32 deltaTime;
    s32 mouseScrollDelta;

    bool updateCamera;
}gameState;