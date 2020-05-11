#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <xinput.h>
#include <xaudio2.h>

#define MOUSE_BUTTON_LEFT 0 
#define MOUSE_BUTTON_MIDDLE 1 
#define MOUSE_BUTTON_RIGHT 2 

#define GAMEPAD_STICK_MAX 32767
#define GAMEPAD_STICK_MIN -32768
#define GAMEPAD_TRIGGER_MAX 255

#define DEBUG_PRINT_SIZE 2.3
#define DEBUG_PRINT_Y_MOVEMENT 25

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

struct WindowsGamepad {
    XINPUT_STATE state;
    s64 lastPacket;
    s64 index;
};

static ID3D11Device* d3d11Device;
static ID3D11DeviceContext* d3d11Context;
static ID3D11SamplerState *pointSampler;
static ID3D11SamplerState *linearSampler;

static Font debugFont;

static TexturedMeshRenderer texturedMeshRenderer;
static TextRenderer textRenderer;
static DebugRenderer debugRenderer;

static bool keyInputs[128];
static bool mouseInputs[4];

static POINT mousePosition;
static POINT screenCenter;

static u8* tempStorageBuffer;

static WindowsGamepad gamepad1;

static void initializeKeyCodes(){
    KEY_0 = 0x30;
    KEY_1 = 0x31;
    KEY_2 = 0x32;
    KEY_3 = 0x33;
    KEY_4 = 0x34;
    KEY_5 = 0x35;
    KEY_6 = 0x36;
    KEY_7 = 0x37;
    KEY_8 = 0x38;
    KEY_9 = 0x39;
    KEY_A = 0x41;
    KEY_B = 0x42;
    KEY_C = 0x43;
    KEY_D = 0x44;
    KEY_E = 0x45;
    KEY_F = 0x46;
    KEY_G = 0x47;
    KEY_H = 0x48;
    KEY_I = 0x49;
    KEY_J = 0x4A;
    KEY_K = 0x4B;
    KEY_L = 0x4C;
    KEY_M = 0x4D;
    KEY_N = 0x4E;
    KEY_O = 0x4F;
    KEY_P = 0x50;
    KEY_Q = 0x51;
    KEY_R = 0x52;
    KEY_S = 0x53;
    KEY_T = 0x54;
    KEY_U = 0x55;
    KEY_V = 0x56;
    KEY_W = 0x57;
    KEY_X = 0x58;
    KEY_Y = 0x59;
    KEY_Z = 0x5A;
    KEY_F1 = VK_F1;
    KEY_F2 = VK_F2;
    KEY_F3 = VK_F3;
    KEY_F4 = VK_F4;
    KEY_F5 = VK_F5;
    KEY_F6 = VK_F6;
    KEY_F7 = VK_F7;
    KEY_F8 = VK_F8;
    KEY_F9 = VK_F9;
    KEY_F10 = VK_F10;
    KEY_F11 = VK_F11;
    KEY_F12 = VK_F12;
    KEY_LEFT_SHIFT = VK_LSHIFT;
    KEY_RIGHT_SHIFT = VK_RSHIFT;
    KEY_LEFT_CTRL = VK_LCONTROL;
    KEY_RIGHT_CTRL = VK_RCONTROL;
    KEY_UP = VK_UP; 
    KEY_DOWN = VK_DOWN;
    KEY_LEFT = VK_LEFT;
    KEY_RIGHT = VK_RIGHT;
    KEY_SPACE = VK_SPACE;
    GAMEPAD_A = 0;
    GAMEPAD_B = 1;
    GAMEPAD_X = 2;
    GAMEPAD_Y = 3;
    GAMEPAD_LB = 4;
    GAMEPAD_RB = 5;
    GAMEPAD_L3 = 6;
    GAMEPAD_R3 = 7;
    GAMEPAD_D_UP = 8;
    GAMEPAD_D_DONW = 9;
    GAMEPAD_D_LEFT = 10;
    GAMEPAD_D_RIGHT = 11;
    GAMEPAD_START = 12;
    GAMEPAD_BACK = 13;
}