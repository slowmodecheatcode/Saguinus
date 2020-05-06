#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <xinput.h>

#define KEY_0 0x30
#define KEY_1 0x31
#define KEY_2 0x32
#define KEY_3 0x33
#define KEY_4 0x34
#define KEY_5 0x35
#define KEY_6 0x36
#define KEY_7 0x37
#define KEY_8 0x38
#define KEY_9 0x39
#define KEY_A 0x41
#define KEY_B 0x42
#define KEY_C 0x43
#define KEY_D 0x44
#define KEY_E 0x45
#define KEY_F 0x46
#define KEY_G 0x47
#define KEY_H 0x48
#define KEY_I 0x49
#define KEY_J 0x4A
#define KEY_K 0x4B
#define KEY_L 0x4C
#define KEY_M 0x4D
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_P 0x50
#define KEY_Q 0x51
#define KEY_R 0x52
#define KEY_S 0x53
#define KEY_T 0x54
#define KEY_U 0x55
#define KEY_V 0x56
#define KEY_W 0x57
#define KEY_X 0x58
#define KEY_Y 0x59
#define KEY_Z 0x5A
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_F6 VK_F6
#define KEY_F7 VK_F7
#define KEY_F8 VK_F8
#define KEY_F9 VK_F9
#define KEY_F10 VK_F10
#define KEY_F11 VK_F11
#define KEY_F12 VK_F12
#define KEY_LEFT_SHIFT VK_LSHIFT
#define KEY_RIGHT_SHIFT VK_RSHIFT
#define KEY_LEFT_CTRK VK_LCTRL
#define KEY_RIGHT_CTRK VK_RCTRL
#define KEY_UP VK_UP
#define KEY_DOWN VK_DOWN
#define KEY_LEFT VK_LEFT
#define KEY_RIGHT VK_RIGHT

#define GAMEPAD_A 0
#define GAMEPAD_B 1
#define GAMEPAD_X 2
#define GAMEPAD_Y 3
#define GAMEPAD_LB 4
#define GAMEPAD_RB 5
#define GAMEPAD_L3 6
#define GAMEPAD_R3 7
#define GAMEPAD_D_UP 8
#define GAMEPAD_D_DONW 9
#define GAMEPAD_D_LEFT 10
#define GAMEPAD_D_RIGHT 11
#define GAMEPAD_START 12
#define GAMEPAD_BACK 13

#define GAMEPAD_STICK_MAX 32767
#define GAMEPAD_STICK_MIN -32768
#define GAMEPAD_TRIGGER_MAX 255

#define DEBUG_PRINT_SIZE 2.3
#define DEBUG_PRINT_Y_MOVEMENT 25

struct Texture2D {
    ID3D11ShaderResourceView* resourceView;
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

struct TexturedMeshRenderer {
    struct VertexConstants {
        Matrix4 modelMatrix;
        Matrix4 cameraMatrix;
    } vertexConstants;

    struct PixelConstants {
        Vector3 cameraPosition;
        f32 pad1;
        Vector3 lightPosition;
        f32 pad2;
        Vector3 ambient;
        f32 pad3;
        Vector3 diffuse;
        f32 pad4;
        Vector3 specular;
        f32 pad5;
    } pixelConstants;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* vertexConstBuffer;
    ID3D11Buffer* pixelConstBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    Texture2D defaultTexture;

    u32 vertexStride;
    u32 vertexOffset;
    u32 vertexDataUsed;
    u32 indexDataUsed;
};

struct TextRenderer {
    struct VertexConstants {
        Matrix4 projectionMatrix;
    } vertexConstants;

    struct PixelConstants {
        Vector4 color;
    } pixelConstants;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* vertexConstBuffer;
    ID3D11Buffer* pixelConstBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    Texture2D defaultTexture;
    Font* currentFont;

    u32 vertexStride;
    u32 vertexOffset;
};

struct DebugRenderer {
    struct VertexConstants {
        Matrix4 cameraMatrix;
    } vertexConstants;

    struct PixelConstants {
        Vector4 color;
    } pixelConstants;

    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* vertexConstBuffer;
    ID3D11Buffer* pixelConstBuffer;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;

    u32 vertexStride;
    u32 vertexOffset;
    u32 currentBufferCount;
    u32 currentIndexCount;
};

struct Gamepad {
    XINPUT_STATE state;

    bool buttons[16];

    s64 lastPacket;
    s64 index;

    f32 leftStickX;
    f32 leftStickY;
    f32 rightStickX;
    f32 rightStickY;
    f32 leftTrigger;
    f32 rightTrigger;
};

static ID3D11Device* d3d11Device;
static ID3D11DeviceContext* d3d11Context;
ID3D11SamplerState *pointSampler;
ID3D11SamplerState *linearSampler;

Font debugFont;

static TexturedMeshRenderer texturedMeshRenderer;
static TextRenderer textRenderer;
static DebugRenderer debugRenderer;

static bool keyInputs[128];

static POINT mousePosition;
static POINT screenCenter;

static u8* tempStorageBuffer;

static f32 debugPrinterX;
static f32 debugPrinterY;
static f32 debugPrinterStartY;

static Gamepad gamepad1;