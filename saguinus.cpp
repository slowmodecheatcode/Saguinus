#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

static unsigned int width = 800;
static unsigned int height = 450;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
        case WM_DESTROY:{
            PostQuitMessage(0);
            return 0;
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);    
}

int main(int argc, char** argv){
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = "Saguinus";

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
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SampleDesc.Count = 1;      
    swapChainDesc.SampleDesc.Quality = 0;   
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
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


    ShowWindow(hwnd, SW_SHOW);
    bool isRunning = true;
    while(isRunning){
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if(msg.wParam == VK_ESCAPE){
                isRunning = false;
                break;
            }
            
        }
        d3d11Context->ClearRenderTargetView(renderTargetView, color);
        swapChain->Present(1, 0);
    }
}