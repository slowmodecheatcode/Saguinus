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
static IXAudio2* XAudio2Pointer;
static IXAudio2MasteringVoice* XAudio2MasterVoice;

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

static void initializeKeyCodes(GameState* state){
    state->inputCodes.KEY_0 = 0x30;
    state->inputCodes.KEY_1 = 0x31;
    state->inputCodes.KEY_2 = 0x32;
    state->inputCodes.KEY_3 = 0x33;
    state->inputCodes.KEY_4 = 0x34;
    state->inputCodes.KEY_5 = 0x35;
    state->inputCodes.KEY_6 = 0x36;
    state->inputCodes.KEY_7 = 0x37;
    state->inputCodes.KEY_8 = 0x38;
    state->inputCodes.KEY_9 = 0x39;
    state->inputCodes.KEY_A = 0x41;
    state->inputCodes.KEY_B = 0x42;
    state->inputCodes.KEY_C = 0x43;
    state->inputCodes.KEY_D = 0x44;
    state->inputCodes.KEY_E = 0x45;
    state->inputCodes.KEY_F = 0x46;
    state->inputCodes.KEY_G = 0x47;
    state->inputCodes.KEY_H = 0x48;
    state->inputCodes.KEY_I = 0x49;
    state->inputCodes.KEY_J = 0x4A;
    state->inputCodes.KEY_K = 0x4B;
    state->inputCodes.KEY_L = 0x4C;
    state->inputCodes.KEY_M = 0x4D;
    state->inputCodes.KEY_N = 0x4E;
    state->inputCodes.KEY_O = 0x4F;
    state->inputCodes.KEY_P = 0x50;
    state->inputCodes.KEY_Q = 0x51;
    state->inputCodes.KEY_R = 0x52;
    state->inputCodes.KEY_S = 0x53;
    state->inputCodes.KEY_T = 0x54;
    state->inputCodes.KEY_U = 0x55;
    state->inputCodes.KEY_V = 0x56;
    state->inputCodes.KEY_W = 0x57;
    state->inputCodes.KEY_X = 0x58;
    state->inputCodes.KEY_Y = 0x59;
    state->inputCodes.KEY_Z = 0x5A;
    state->inputCodes.KEY_F1 = VK_F1;
    state->inputCodes.KEY_F2 = VK_F2;
    state->inputCodes.KEY_F3 = VK_F3;
    state->inputCodes.KEY_F4 = VK_F4;
    state->inputCodes.KEY_F5 = VK_F5;
    state->inputCodes.KEY_F6 = VK_F6;
    state->inputCodes.KEY_F7 = VK_F7;
    state->inputCodes.KEY_F8 = VK_F8;
    state->inputCodes.KEY_F9 = VK_F9;
    state->inputCodes.KEY_F10 = VK_F10;
    state->inputCodes.KEY_F11 = VK_F11;
    state->inputCodes.KEY_F12 = VK_F12;
    state->inputCodes.KEY_LEFT_SHIFT = VK_LSHIFT;
    state->inputCodes.KEY_RIGHT_SHIFT = VK_RSHIFT;
    state->inputCodes.KEY_LEFT_CTRL = VK_LCONTROL;
    state->inputCodes.KEY_RIGHT_CTRL = VK_RCONTROL;
    state->inputCodes.KEY_UP = VK_UP; 
    state->inputCodes.KEY_DOWN = VK_DOWN;
    state->inputCodes.KEY_LEFT = VK_LEFT;
    state->inputCodes.KEY_RIGHT = VK_RIGHT;
    state->inputCodes.KEY_SPACE = VK_SPACE;
    state->inputCodes.GAMEPAD_A = 0;
    state->inputCodes.GAMEPAD_B = 1;
    state->inputCodes.GAMEPAD_X = 2;
    state->inputCodes.GAMEPAD_Y = 3;
    state->inputCodes.GAMEPAD_LB = 4;
    state->inputCodes.GAMEPAD_RB = 5;
    state->inputCodes.GAMEPAD_L3 = 6;
    state->inputCodes.GAMEPAD_R3 = 7;
    state->inputCodes.GAMEPAD_D_UP = 8;
    state->inputCodes.GAMEPAD_D_DONW = 9;
    state->inputCodes.GAMEPAD_D_LEFT = 10;
    state->inputCodes.GAMEPAD_D_RIGHT = 11;
    state->inputCodes.GAMEPAD_START = 12;
    state->inputCodes.GAMEPAD_BACK = 13;
}