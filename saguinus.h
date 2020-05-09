#pragma once

#define MAX_STRINGS 256
#define MAX_STRING_LENGTH 256
#define MAX_DEBUG_CUBES 256
#define MAX_DEBUG_LINES 256
#define MAX_TEXTURED_MESHES 256

static u32 KEY_SPACE;

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

struct GameState {
    TextBuffer textBuffer;
    DebugBuffer debugBuffer;
    TexturedMeshBuffer txtdMeshBuffer;

    Camera camera;
    PointLight light;

    TexturedMesh mesh;

    Vector4 clearColor;
    Vector2 mousePosition;

    bool* keyInputs;

    u32 windowWidth;
    u32 windowHeight;
}gameState;