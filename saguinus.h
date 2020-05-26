#pragma once

#include "debug_font.h"
#include "graphics_utilities.h"

#define MOUSE_BUTTON_LEFT 0 
#define MOUSE_BUTTON_MIDDLE 1 
#define MOUSE_BUTTON_RIGHT 2 

#define MAX_STRINGS 256
#define MAX_STRING_LENGTH 256
#define MAX_DEBUG_CUBES 256
#define MAX_DEBUG_LINES 256
#define MAX_TEXTURED_MESHES 256
#define MAX_GUI_ITEMS 256

enum GameMode {
    GAME_MODE_INTRO,
    GAME_MODE_PLAYING,
    GAME_MODE_OVER,
    GAME_MODE_DEBUG,
    
    GAME_MODE_TOTAL,
};

struct InputCodes {
    u8 KEY_0;
    u8 KEY_1;
    u8 KEY_2;
    u8 KEY_3;
    u8 KEY_4;
    u8 KEY_5;
    u8 KEY_6;
    u8 KEY_7;
    u8 KEY_8;
    u8 KEY_9;
    u8 KEY_A;
    u8 KEY_B;
    u8 KEY_C;
    u8 KEY_D;
    u8 KEY_E;
    u8 KEY_F;
    u8 KEY_G;
    u8 KEY_H;
    u8 KEY_I;
    u8 KEY_J;
    u8 KEY_K;
    u8 KEY_L;
    u8 KEY_M;
    u8 KEY_N;
    u8 KEY_O;
    u8 KEY_P;
    u8 KEY_Q;
    u8 KEY_R;
    u8 KEY_S;
    u8 KEY_T;
    u8 KEY_U;
    u8 KEY_V;
    u8 KEY_W;
    u8 KEY_X;
    u8 KEY_Y;
    u8 KEY_Z;
    u8 KEY_F1;
    u8 KEY_F2;
    u8 KEY_F3;
    u8 KEY_F4;
    u8 KEY_F5;
    u8 KEY_F6;
    u8 KEY_F7;
    u8 KEY_F8;
    u8 KEY_F9;
    u8 KEY_F10;
    u8 KEY_F11;
    u8 KEY_F12;
    u8 KEY_LEFT_SHIFT;
    u8 KEY_RIGHT_SHIFT;
    u8 KEY_LEFT_CTRL;
    u8 KEY_RIGHT_CTRL;
    u8 KEY_UP;
    u8 KEY_DOWN;
    u8 KEY_LEFT;
    u8 KEY_RIGHT;
    u8 KEY_SPACE;
    u8 GAMEPAD_A;
    u8 GAMEPAD_B;
    u8 GAMEPAD_X;
    u8 GAMEPAD_Y;
    u8 GAMEPAD_LB;
    u8 GAMEPAD_RB;
    u8 GAMEPAD_L3;
    u8 GAMEPAD_R3;
    u8 GAMEPAD_D_UP;
    u8 GAMEPAD_D_DONW;
    u8 GAMEPAD_D_LEFT;
    u8 GAMEPAD_D_RIGHT;
    u8 GAMEPAD_START;
    u8 GAMEPAD_BACK;
};

struct Texture2D {
    void* texture;
};

struct AudioEmitter {
    void* emitter;
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
    Texture2D texture;
    u32 indexCount;
    u32 indexOffset;
    u32 indexAddon;
};

struct Bone {
    Quaternion orientaion;
    Vector3 position;
    u32 parentIndex;
};

struct MeshAnimation {
    Bone* poses;
    f32 frameRate;
    u32 totalBones;
    u32 totalPoses;
    u32 currentKeyframe;
};

struct AnimatedMesh {
    TexturedMesh mesh;
    MeshAnimation* animation;
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

struct CanvasBuffer {
    Vector4 colors[MAX_GUI_ITEMS];
    Vector2 positions[MAX_GUI_ITEMS];
    Vector2 scales[MAX_GUI_ITEMS];
    Vector2 textureOffsets[MAX_GUI_ITEMS];
    Vector2 textureScales[MAX_GUI_ITEMS];
    Texture2D textures[MAX_GUI_ITEMS];
    bool isText[MAX_GUI_ITEMS];

    Texture2D defaultTexture;
    f32 debugPrinterX;
    f32 debugPrinterY;
    f32 debugPrinterStartY;
    u32 totalGUIItems;

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
    void (*updateAudioEmitterDynamics)(AudioEmitter ae, Vector3 epos, Vector3 lpos, Vector3 lrgt);
    void (*playAudioEmitter)(AudioEmitter ae, s8* buffer, u32 bufferSize);
    void (*setMasterAudioVolume)(f32 v);
    bool (*readFileIntoBuffer)(const s8* fileName, void* data, u32* fileLength);
    bool (*writeToFile)(const s8* fileName, void* data, u32 dataSize);
    Texture2D (*createTexture2DFromFile)(const s8* fileName, u32 bytesPerPixel);
    Texture2D (*createTexture2DFromData)(u8* data, u32 width, u32 height, u32 bytesPerPixel);
    TexturedMesh (*createTexturedMesh)(const s8* fileName);
    AudioEmitter (*createAudioEmitter)();
};

struct MemoryStorage {
    u8* tempMemoryBuffer;
    u8* longTermBuffer;
    u8* longTermBufferPointer;
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

struct Obstacle {
    Vector3 position;
    Vector3 scale;
    Vector4 color;
    Quaternion orientation;
    TexturedMesh mesh;
    f32 xVelocity;
    f32 xStartPosition;
};

struct Player {
    Vector3 position;
    Vector3 scale;
    Quaternion orientation;
    TexturedMesh mesh;
    f32 yVelocity;
    bool isJumping;
};

#define MAX_OBSTACLES 256

struct GameState {
    //TextBuffer textBuffer;
    CanvasBuffer canvasBuffer;
    DebugBuffer debugBuffer;
    TexturedMeshBuffer txtdMeshBuffer;

    Font* currentFont;

    Texture2D testTex1;
    Texture2D testTex2;

    InputCodes inputCodes;
    OSFunctions osFunctions;
    MemoryStorage storage;

    AudioEmitter emitters[2];

    Camera camera;
    PointLight light;

    Obstacle obstacles[MAX_OBSTACLES];
    Player player;

    Gamepad gamepad1;

    Vector4 clearColor;
    Vector2 mousePosition;

    Vector2 windowDimenstion;
    Vector2 gameResolution;

    GameMode mode;

    bool keyTracking[128];
    bool mouseButtonTracking[4];

    s8* soundBuffer1;
    s8* soundBuffer2;
    u32 buffer1Size;
    u32 buffer2Size;

    bool* keyInputs;
    bool* mouseInputs;
    f32 deltaTime;
    f32 gravity;
    f32 gameTime;
    u32 score;
    u32 hiScore;
    u32 totalObstacles;
    s32 mouseScrollDelta;

    bool updateCamera;
    bool isInitialized;
    bool gameOver;
} *gameState;