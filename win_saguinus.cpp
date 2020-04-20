#include "graphics_utilities.h"
#include "win_saguinus.h"

static u32 windowWidth = 800;
static u32 windowHeight = 450;
static s32 halfWindowWidth = windowWidth * 0.5;
static s32 halfWindowHeight = windowHeight * 0.5;
static bool updateCamera = false;
Vector4 clearColor(0.5, 0.2, 0.8, 1);

static void checkError(HRESULT err, LPCSTR msg){
    if(err != S_OK){
        MessageBox(0, msg, "ERROR", 0);
    }
}

static Texture2D createTexture2D(u8* data, u32 width, u32 height, u32 bytesPerPixel){
    Texture2D tex;

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = data;
    texData.SysMemPitch = bytesPerPixel * width;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height; 
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    switch(bytesPerPixel){
        case 1 :{
            texDesc.Format = DXGI_FORMAT_R8_UNORM;
            break;
        }
        case 2 :{
            texDesc.Format = DXGI_FORMAT_R8G8_UNORM;
            break;
        }
        case 4 :{
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }
    }
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = d3d11Device->CreateTexture2D(&texDesc, &texData, &tex.texture);
    checkError(hr, "Error creating texture");

    CD3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc(tex.texture, D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 1, 0, 0);

    hr = d3d11Device->CreateShaderResourceView(tex.texture, &resViewDesc, &tex.resourceView);
    checkError(hr, "Error shader resource view");

    return tex;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
        case WM_QUIT:
        case WM_CLOSE:{
            exit(0);
            break;
        }
        case WM_MOUSEMOVE:{
            GetCursorPos(&mousePosition);
            ScreenToClient(hwnd, &mousePosition);
            updateCamera = true;
            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);    
}

int WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR argv, int argc){
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = "Saguinus";
    wc.hCursor = LoadCursorA(0, IDC_ARROW);

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
                               100, 100, windowWidth, windowHeight, 0, 0, GetModuleHandle(0), 0);

    if (hwnd == 0){
        MessageBox(0, "Error creating window", "ERROR", 0);
        return 0;
    }

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
    HRESULT hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, d3dFeatureLevels,
                                               1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, 
                                               &d3d11Device, 0, &d3d11Context);

    checkError(hr, "Error initializing device and swapchain");

    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backBuffer);
    checkError(hr, "Error initializing back buffer");
    D3D11_TEXTURE2D_DESC backBufferDescriptor;
    backBuffer->GetDesc(&backBufferDescriptor);

    D3D11_VIEWPORT viewPort = {};
    viewPort.Height = windowHeight;
    viewPort.Width = windowWidth;
    viewPort.MinDepth = 0;
    viewPort.MaxDepth = 1;

    d3d11Context->RSSetViewports(1, &viewPort);

    ID3D11RenderTargetView* renderTargetView;
    hr = d3d11Device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
    checkError(hr, "Error creating render target view");
    
    ID3D11Texture2D* depthTexture = 0;
    D3D11_TEXTURE2D_DESC depthTexDesc = {};
    depthTexDesc.Width = windowWidth;
    depthTexDesc.Height = windowHeight;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;
    hr = d3d11Device->CreateTexture2D(&depthTexDesc, 0, &depthTexture);
    checkError(hr, "Error creating depth texture");

    D3D11_DEPTH_STENCIL_DESC depthStenDesc = {};
    depthStenDesc.DepthEnable = true;
    depthStenDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStenDesc.DepthFunc = D3D11_COMPARISON_LESS;

    ID3D11DepthStencilState * depthStencilState;
    d3d11Device->CreateDepthStencilState(&depthStenDesc, &depthStencilState);

    d3d11Context->OMSetDepthStencilState(depthStencilState, 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStenViewDesc = {};
    depthStenViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStenViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStenViewDesc.Texture2D.MipSlice = 0;

    ID3D11DepthStencilView* depthStencilView;
    hr = d3d11Device->CreateDepthStencilView(depthTexture,  &depthStenViewDesc, &depthStencilView);
    checkError(hr, "Error creating depth stencil view texture");

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias = false;
    rasterizerDesc.DepthBiasClamp = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.AntialiasedLineEnable = false;
    ID3D11RasterizerState* rasterizerState;
    d3d11Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    d3d11Context->RSSetState(rasterizerState);

    d3d11Context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

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
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UVCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    ID3D11InputLayout* inputLayout;
    hr = d3d11Device->CreateInputLayout(layoutDesc, 3, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &inputLayout);
    d3d11Context->IASetInputLayout(inputLayout);

    d3d11Context->VSSetShader(vertexShader, 0, 0);
    d3d11Context->PSSetShader(pixelShader, 0, 0);

    f32 vertices[] = {
        -0.5, -0.5, 0.5,        0.0, 0.0, 1.0,      0.0, 1.0,
        -0.5,  0.5, 0.5,        0.0, 0.0, 1.0,      0.0, 0.0,
         0.5,  0.5, 0.5,        0.0, 0.0, 1.0,      1.0, 0.0,
         0.5, -0.5, 0.5,        0.0, 0.0, 1.0,      1.0, 1.0,

        -0.5,  0.5, -0.5,       0.0, 0.0, -1.0,     0.0, 1.0,
        -0.5, -0.5, -0.5,       0.0, 0.0, -1.0,     0.0, 0.0,
         0.5, -0.5, -0.5,       0.0, 0.0, -1.0,     1.0, 0.0,
         0.5,  0.5, -0.5,       0.0, 0.0, -1.0,     1.0, 1.0,

        -0.5,  0.5,  0.5,       0.0,  1.0,  0.0,    0.0, 1.0,
        -0.5,  0.5, -0.5,       0.0,  1.0,  0.0,    0.0, 0.0,
         0.5,  0.5, -0.5,       0.0,  1.0,  0.0,    1.0, 0.0,
         0.5,  0.5,  0.5,       0.0,  1.0,  0.0,    1.0, 1.0,

        -0.5, -0.5, -0.5,       0.0,  -1.0,  0.0,    0.0, 1.0,
        -0.5, -0.5,  0.5,       0.0,  -1.0,  0.0,    0.0, 0.0,
         0.5, -0.5,  0.5,       0.0,  -1.0,  0.0,    1.0, 0.0,
         0.5, -0.5, -0.5,       0.0,  -1.0,  0.0,    1.0, 1.0,

        -0.5, -0.5, -0.5,       -1.0,  0.0,  0.0,    0.0, 1.0,
        -0.5,  0.5, -0.5,       -1.0,  0.0,  0.0,    0.0, 0.0,
        -0.5,  0.5,  0.5,       -1.0,  0.0,  0.0,    1.0, 0.0,
        -0.5, -0.5,  0.5,       -1.0,  0.0,  0.0,    1.0, 1.0,

         0.5, -0.5,  0.5,        1.0,  0.0,  0.0,    0.0, 1.0,
         0.5,  0.5,  0.5,        1.0,  0.0,  0.0,    0.0, 0.0,
         0.5,  0.5, -0.5,        1.0,  0.0,  0.0,    1.0, 0.0,
         0.5, -0.5, -0.5,        1.0,  0.0,  0.0,    1.0, 1.0,
    };

    CD3D11_BUFFER_DESC vertexDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    ID3D11Buffer* vertexBuffer;
    hr = d3d11Device->CreateBuffer(&vertexDesc, &vertexData, &vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    u32 stride = sizeof(float) * 8;
    u32 offset = 0;
    d3d11Context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    u16 indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    CD3D11_BUFFER_DESC indexDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;

    ID3D11Buffer* indexBuffer;
    hr = d3d11Device->CreateBuffer(&indexDesc, &indexData, &indexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating index buffer", "ERROR", 0);

    d3d11Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    CD3D11_BUFFER_DESC constBufDesc(sizeof(VertexConstants), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &vertexConstants;

    ID3D11Buffer* vertexConstBuffer;
    hr = d3d11Device->CreateBuffer(&constBufDesc, &constBufData, &vertexConstBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex constant buffer", "ERROR", 0);

    d3d11Context->VSSetConstantBuffers(0, 1, &vertexConstBuffer);

    CD3D11_BUFFER_DESC pixConstBufDesc(sizeof(pixelConstants), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &pixelConstants;

    ID3D11Buffer* pixelConstBuffer;
    hr = d3d11Device->CreateBuffer(&pixConstBufDesc, &pixConstBufData, &pixelConstBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating pixel constant buffer", "ERROR", 0);
    d3d11Context->PSSetConstantBuffers(0, 1, &pixelConstBuffer);

    u8 tex[] = {
        0, 255, 0, 255,     0, 0, 255, 255,
        0, 0, 255, 255,     0, 255, 0, 255
    };
    Texture2D texture = createTexture2D(tex, 2, 2, 4);

    u8 tex2[] {255, 255, 255, 255};
    Texture2D lightTexture = createTexture2D(tex2, 1, 1, 4);


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

    D3D11_SUBRESOURCE_DATA texData = {};
    texData.pSysMem = tex;
    texData.SysMemPitch = sizeof(char) * 8;

    Camera camera;
    camera.position.z -= 5;
    camera.moveSpeed = 3;
    camera.rotateSpeed = 1;
    camera.mouseSensitivity = 0.1;
    Matrix4 modelMatrix(1);
    modelMatrix.m2[3][2] = 0;
    camera.projection = createPerspectiveProjection(70.0, (f32)windowWidth / (f32)windowHeight, 0.001, 1000.0);
    Matrix4 transformMatrix = multiply(camera.projection, modelMatrix);

    pixelConstants.lightPosition = Vector3(-10, 0, 3);
    pixelConstants.ambient = Vector3(0.2);
    pixelConstants.diffuse = Vector3(1);

    Matrix4 lightModel(1);
    scale(&lightModel, 0.25);

    screenCenter.x = halfWindowWidth;
    screenCenter.y = halfWindowHeight;
    ClientToScreen(hwnd, &screenCenter);
    SetCursorPos(screenCenter.x, screenCenter.y);
    ShowCursor(false);
    ShowWindow(hwnd, SW_SHOW);
    bool isRunning = true;

    f32 deltaTime = 0;
    u64 endTime = 0;
    u64 startTime = GetTickCount64();
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

        if(updateCamera){
            f32 xDif = mousePosition.x - halfWindowWidth;
            f32 yDif = mousePosition.y - halfWindowHeight;
            rotate(&camera.orientation, camera.up, deltaTime * xDif * camera.mouseSensitivity);
            rotate(&camera.orientation, camera.right, deltaTime * yDif * camera.mouseSensitivity);
            SetCursorPos(screenCenter.x, screenCenter.y);
            updateCamera = false;
        }

        if(keyInputs[KEY_W]){
            camera.position -= camera.forward * deltaTime * camera.moveSpeed;
        }
        if(keyInputs[KEY_S]){
            camera.position += camera.forward * deltaTime * camera.moveSpeed;
        }
        if(keyInputs[KEY_A]){
           camera.position += camera.right * deltaTime * camera.moveSpeed;
        }
        if(keyInputs[KEY_D]){
            camera.position -= camera.right * deltaTime * camera.moveSpeed;
        }
        if(keyInputs[KEY_R]){
           camera.position -= camera.up * deltaTime * camera.moveSpeed;
        }
        if(keyInputs[KEY_F]){
            camera.position += camera.up * deltaTime * camera.moveSpeed;
        }

        if(keyInputs[KEY_UP]){
            rotate(&camera.orientation, camera.right, -deltaTime * camera.rotateSpeed);
        }
        if(keyInputs[KEY_DOWN]){
            rotate(&camera.orientation, camera.right, deltaTime * camera.rotateSpeed);
        }
        if(keyInputs[KEY_LEFT]){
            rotate(&camera.orientation, camera.up, -deltaTime * camera.rotateSpeed);
        }
        if(keyInputs[KEY_RIGHT]){
            rotate(&camera.orientation, camera.up, deltaTime * camera.rotateSpeed);
        }
        if(keyInputs[KEY_Q]){
            rotate(&camera.orientation, camera.forward, deltaTime * camera.rotateSpeed);
        }
        if(keyInputs[KEY_E]){
            rotate(&camera.orientation, camera.forward, -deltaTime * camera.rotateSpeed);
        }

        d3d11Context->ClearRenderTargetView(renderTargetView, clearColor.v);
        d3d11Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);

        updateCameraView(&camera);

        vertexConstants.cameraMatrix = camera.projection * camera.view;
        vertexConstants.modelMatrix = modelMatrix;

        pixelConstants.cameraPosition = camera.position;
        pixelConstants.lightPosition.x += deltaTime;

        d3d11Context->PSSetShaderResources(0, 1, &texture.resourceView);
        d3d11Context->UpdateSubresource(vertexConstBuffer, 0, 0, &vertexConstants, 0, sizeof(vertexConstants));
        d3d11Context->UpdateSubresource(pixelConstBuffer, 0, 0, &pixelConstants, 0, sizeof(pixelConstants));
        d3d11Context->DrawIndexed(sizeof(indices) / sizeof(u16), 0, 0);

        vertexConstants.modelMatrix = lightModel;
        vertexConstants.modelMatrix.m2[3][0] = pixelConstants.lightPosition.x;
        vertexConstants.modelMatrix.m2[3][1] = pixelConstants.lightPosition.y;
        vertexConstants.modelMatrix.m2[3][2] = pixelConstants.lightPosition.z;
        d3d11Context->PSSetShaderResources(0, 1, &lightTexture.resourceView);
        d3d11Context->UpdateSubresource(vertexConstBuffer, 0, 0, &vertexConstants, 0, sizeof(vertexConstants));
        d3d11Context->DrawIndexed(sizeof(indices) / sizeof(u16), 0, 0);

        swapChain->Present(1, 0);

        endTime = GetTickCount64();
        deltaTime = (f32)(endTime - startTime) / 1000.0f;
        startTime = endTime;
    }

    return 0;
}