#include "mathematics.h"

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

static unsigned int width = 800;
static unsigned int height = 450;

#define KEY_0 0x30
#define KEY_A 0x41
#define KEY_D 0x44
#define KEY_S 0x53
#define KEY_W 0x57



bool keyInputs[128];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
        case WM_QUIT:
        case WM_CLOSE:{
            exit(0);
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);    
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = "Saguinus";
    wc.hCursor = LoadCursorA(0, IDC_ARROW);

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(
        0,                              
        wc.lpszClassName,                     
        wc.lpszClassName,    
        WS_OVERLAPPEDWINDOW,
        100, 
        100, 
        width, 
        height,
        0,          
        0,    
        GetModuleHandle(0),  
        0       
    );

    if (hwnd == 0){
        return 0;
    }


    ID3D11Device* d3d11Device;
    ID3D11DeviceContext* d3d11Context;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.Windowed = true;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SampleDesc.Count = 1;      
    swapChainDesc.SampleDesc.Quality = 0;   
    swapChainDesc.OutputWindow = hwnd;
    
    IDXGISwapChain* swapChain;
    D3D_FEATURE_LEVEL d3dFeatureLevels[] = {D3D_FEATURE_LEVEL_11_0};
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        0,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        0,
        d3dFeatureLevels,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain,
        &d3d11Device,
        0,
        &d3d11Context
    );

    if(hr != S_OK)  MessageBox(0, "Error initializing device and swapchain", "ERROR", 0);

    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backBuffer);
    if(hr != S_OK)  MessageBox(0, "Error initializing back buffer", "ERROR", 0);
    D3D11_TEXTURE2D_DESC backBufferDescriptor;
    backBuffer->GetDesc(&backBufferDescriptor);

    D3D11_VIEWPORT viewPort = {};
    viewPort.Height = height;
    viewPort.Width = width;
    viewPort.MinDepth = 0;
    viewPort.MaxDepth = 1;

    d3d11Context->RSSetViewports(1, &viewPort);

    ID3D11RenderTargetView* renderTargetView;
    hr = d3d11Device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
    if(hr != S_OK)  MessageBox(0, "Error creating render target view", "ERROR", 0);

    const float color [] = { 0.5, 0.2, 0.8, 1 };
    d3d11Context->ClearRenderTargetView(renderTargetView, color);
    
    d3d11Context->OMSetRenderTargets(1, &renderTargetView, 0);

    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"basic_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"basic_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex buffer", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel buffer", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UVCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ID3D11InputLayout* inputLayout;
    hr = d3d11Device->CreateInputLayout(layoutDesc, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &inputLayout);
    d3d11Context->IASetInputLayout(inputLayout);

    d3d11Context->VSSetShader(vertexShader, 0, 0);
    d3d11Context->PSSetShader(pixelShader, 0, 0);

    f32 vertices[] = {
        -0.5, -0.5, 0.0,        0.0, 1.0,
         0.0,  0.5, 0.0,        0.5, 0.0,
         0.5, -0.5, 0.0,        1.0, 1.0,
    };

    CD3D11_BUFFER_DESC vertexDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    ID3D11Buffer* vertexBuffer;
    hr = d3d11Device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    unsigned int stride = sizeof(float) * 5;
    unsigned int offset = 0;
    d3d11Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    unsigned short indices[] = {
        0, 1, 2
    };

    CD3D11_BUFFER_DESC indexDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    ID3D11Buffer* indexBuffer;
    hr = d3d11Device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating index buffer", "ERROR", 0);

    d3d11Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    Matrix4 modelMatrix = createIdentityMatrix();
    modelMatrix.m2[3][2] = -5;
    Matrix4 projectionMatrix = createPerspectiveProjection(70.0, (f32)width / (f32)height, 0.001, 1000.0);
    Matrix4 transformMatrix = multiply(projectionMatrix, modelMatrix);

    CD3D11_BUFFER_DESC constBufDesc(sizeof(transformMatrix), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &transformMatrix;

    ID3D11Buffer* vertexConstBuffer;
    hr = d3d11Device->CreateBuffer(&constBufDesc, &constBufData, &vertexConstBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating constant buffer", "ERROR", 0);

    d3d11Context->VSSetConstantBuffers(0, 1, &vertexConstBuffer);

    unsigned char tex[] = {
        0, 255, 0, 255,     0, 0, 255, 255,
        0, 0, 255, 255,     0, 255, 0, 255
    };

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = tex;
    texData.SysMemPitch = sizeof(char) * 8;

    ID3D11Texture2D* texture;
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 2;
    texDesc.Height = 2; 
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SAMPLER_DESC samplerDescripter = {};
    samplerDescripter.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescripter.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescripter.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescripter.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; 
    samplerDescripter.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescripter.MinLOD = 0;
    samplerDescripter.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState *samplerState;
    d3d11Device->CreateSamplerState(&samplerDescripter, &samplerState);
    d3d11Context->PSSetSamplers(0, 1, &samplerState);


    hr = d3d11Device->CreateTexture2D(&texDesc, &texData, &texture);
    if(hr != S_OK)  MessageBox(0, "Error creating texture", "ERROR", 0);

    CD3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc(texture, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1, 0, 0);

    ID3D11ShaderResourceView* resourceView;
    hr = d3d11Device->CreateShaderResourceView(texture, &resViewDesc, &resourceView);
    if(hr != S_OK)  MessageBox(0, "Error shader resource view", "ERROR", 0);
    d3d11Context->PSSetShaderResources(0, 1, &resourceView);

    ShowWindow(hwnd, SW_SHOW);
    bool isRunning = true;
    while(isRunning){
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
            if(keyInputs[VK_ESCAPE]){
                isRunning = false;
            }

            if(msg.message == WM_KEYDOWN){
                keyInputs[msg.wParam] = true;
            }else if(msg.message == WM_KEYUP){
                keyInputs[msg.wParam] = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(keyInputs[KEY_W]){
            modelMatrix.m2[3][2] += 0.1;
        }
        if(keyInputs[KEY_S]){
            modelMatrix.m2[3][2] -= 0.1;
        }
        if(keyInputs[KEY_A]){
            modelMatrix.m2[3][0] += 0.1;
        }
        if(keyInputs[KEY_D]){
            modelMatrix.m2[3][0] -= 0.1;
        }

        
        transformMatrix = multiply(projectionMatrix, modelMatrix);
        d3d11Context->UpdateSubresource(vertexConstBuffer, 0, 0, &transformMatrix, 0, sizeof(transformMatrix));

        d3d11Context->ClearRenderTargetView(renderTargetView, color);
        d3d11Context->DrawIndexed(3, 0, 0);
        swapChain->Present(1, 0);
    }
}