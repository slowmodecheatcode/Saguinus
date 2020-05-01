#include "debug_font.h"

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

static void readFileIntoBuffer(const s8* fileName, void* data, u32* fileLength){
    HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file == INVALID_HANDLE_VALUE){
        MessageBox(0, "Could not open file", "readFileIntoBuffer", 0);
        exit(1);
    }        
    DWORD fileSize;
    fileSize = GetFileSize(file, 0);
    bool res = ReadFile(file, data, fileSize, 0, 0);
    if(!res){
        MessageBox(0, "Could not read file", "readFileIntoBuffer", 0);
        exit(1);
    }
    *fileLength = fileSize;
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
        default: {
            MessageBox(0, "Bits per pixel when creating texture must be 1, 2, or 4", "ERROR", 0);
            exit(1);
        }
    }
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DYNAMIC;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Texture2D* texture;
    HRESULT hr = d3d11Device->CreateTexture2D(&texDesc, &texData, &texture);
    checkError(hr, "Error creating texture");

    CD3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc(texture, D3D11_SRV_DIMENSION_TEXTURE2D, texDesc.Format, 0, 1, 0, 0);

    hr = d3d11Device->CreateShaderResourceView(texture, &resViewDesc, &tex.resourceView);
    checkError(hr, "Error creating shader resource view");
    texture->Release();

    return tex;
}

static Texture2D createTexture2D(const s8* fileName, u32 bytesPerPixel){
    u32 fileSize;
    readFileIntoBuffer(fileName, tempStorageBuffer, &fileSize);
    u8* fileData = tempStorageBuffer;
    u32 width = *(u32*)fileData;
    fileData += 4;
    u32 height = *(u32*)fileData;
    fileData += 4;
    Texture2D tex = createTexture2D(fileData, width, height, bytesPerPixel);
    return tex;
}

static void initializeTexturedMeshRenderer(){
    texturedMeshRenderer.vertexDataUsed = 0;
    texturedMeshRenderer.indexDataUsed = 0;

    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"textured_mesh_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"textured_mesh_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    HRESULT hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &texturedMeshRenderer.vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex shader", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &texturedMeshRenderer.pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel shader", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UVCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = d3d11Device->CreateInputLayout(layoutDesc, 3, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &texturedMeshRenderer.inputLayout);
    checkError(hr, "Could not create input layout");

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    CD3D11_BUFFER_DESC vertexDesc(MEGABYTE(32), D3D11_BIND_VERTEX_BUFFER);

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&vertexDesc, &bufferData, &texturedMeshRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    texturedMeshRenderer.vertexStride = sizeof(float) * 8;
    texturedMeshRenderer.vertexOffset = 0;

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    CD3D11_BUFFER_DESC indexDesc(MEGABYTE(32), D3D11_BIND_INDEX_BUFFER);
 

    hr = d3d11Device->CreateBuffer(&indexDesc, &bufferData, &texturedMeshRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");


    CD3D11_BUFFER_DESC constBufDesc(sizeof(texturedMeshRenderer.vertexConstants), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &texturedMeshRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&constBufDesc, &constBufData, &texturedMeshRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");


    CD3D11_BUFFER_DESC pixConstBufDesc(sizeof(texturedMeshRenderer.pixelConstants), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &texturedMeshRenderer.pixelConstants;

    hr = d3d11Device->CreateBuffer(&pixConstBufDesc, &pixConstBufData, &texturedMeshRenderer.pixelConstBuffer);
    checkError(hr, "Error creating pixel constant buffer");

    u8 tex[] = {
        0, 255, 0, 255,     0, 0, 255, 255,
        0, 0, 255, 255,     0, 255, 0, 255
    };
    texturedMeshRenderer.defaultTexture = createTexture2D(tex, 2, 2, 4);
}

static void initializeTextRenderer(){
    textRenderer.vertexDataUsed = 0;
    textRenderer.indexDataUsed = 0;

    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"text_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"text_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    HRESULT hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &textRenderer.vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex shader", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &textRenderer.pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel shader", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UVCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = d3d11Device->CreateInputLayout(layoutDesc, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &textRenderer.inputLayout);
    checkError(hr, "Could not create input layout");

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &textRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    textRenderer.vertexStride = sizeof(float) * 4;
    textRenderer.vertexOffset = 0;

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    //CD3D11_BUFFER_DESC indexDesc(MEGABYTE(32), D3D11_BIND_INDEX_BUFFER);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &textRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");


    CD3D11_BUFFER_DESC constBufDesc(sizeof(textRenderer.vertexConstants), D3D11_BIND_CONSTANT_BUFFER);

    textRenderer.vertexConstants.projectionMatrix = createOrthogonalProjection(0, windowWidth, 0, windowHeight, -1, 1);

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &textRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&constBufDesc, &constBufData, &textRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");


    CD3D11_BUFFER_DESC pixConstBufDesc(sizeof(textRenderer.pixelConstants), D3D11_BIND_CONSTANT_BUFFER);

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &textRenderer.pixelConstants;

    hr = d3d11Device->CreateBuffer(&pixConstBufDesc, &pixConstBufData, &textRenderer.pixelConstBuffer);
    checkError(hr, "Error creating pixel constant buffer");

    debugFont.bitmapWidth = debugFontBitmapWidth;
    debugFont.bitmapHeight = debugFontBitmapHeight;
    debugFont.totalCharacters = debugFontTotalCharacters;
    debugFont.missingCharacterCodeIndex = debugFontMissingCharacterCodeIndex;
    debugFont.characterCodes = debugFontCharacterCodes;
    debugFont.xOffset = debugFontCharacterXOffset;
    debugFont.yOffset = debugFontCharacterYOffset;
    debugFont.width = debugFontCharacterWidth;
    debugFont.height = debugFontCharacterHeight;
    debugFont.bitmapX = debugFontCharacterBitmapX;
    debugFont.bitmapY = debugFontCharacterBitmapY;
    debugFont.bitmapCharacterWidth = debugFontCharacterBitmapWidth;
    debugFont.bitmapCharacterHeight = debugFontCharacterBitmapHeight;
    debugFont.kerning = debugFontCharacterKernAmount;
    debugFont.bitmap = createTexture2D(debugFontBitmapPixels, 50, 50, 1);

    textRenderer.currentFont = &debugFont;
}

static TexturedMesh createTexturedMesh(f32* vertexData, u32 vertexDataSize, u16* indexData, u32 indexDataSize){
    u32 totalIndices = indexDataSize / sizeof(u16);
    u32 totalVertsInBuffer = texturedMeshRenderer.vertexDataUsed / (sizeof(f32) * 8);

    TexturedMesh mesh;
    D3D11_BOX box;
    box.left = texturedMeshRenderer.vertexDataUsed;
    box.top = 0;
    box.front = 0;
    box.right = box.left + vertexDataSize;
    box.bottom = 1;
    box.back = 1;
    d3d11Context->UpdateSubresource(texturedMeshRenderer.vertexBuffer, 0, &box, vertexData, 0, 0);

    box.left = texturedMeshRenderer.indexDataUsed;
    box.top = 0;
    box.front = 0;
    box.right = box.left + indexDataSize;
    box.bottom = 1;
    box.back = 1;
    d3d11Context->UpdateSubresource(texturedMeshRenderer.indexBuffer, 0, &box, indexData, 0, 0);

    u32 indexOffset = texturedMeshRenderer.indexDataUsed / sizeof(u16);
    mesh.indexCount = totalIndices;
    mesh.indexOffset = indexOffset;
    texturedMeshRenderer.indexDataUsed += indexDataSize;
    texturedMeshRenderer.vertexDataUsed += vertexDataSize;

    mesh.indexCount = totalIndices;
    mesh.indexOffset = indexOffset;
    mesh.indexAddon = totalVertsInBuffer;
    mesh.position = Vector3(0);
    mesh.scale = Vector3(1);
    mesh.texture = texturedMeshRenderer.defaultTexture;

    return mesh;
}

static TexturedMesh createTexturedMesh(const s8* fileName){
    u32 fileSize;
    readFileIntoBuffer(fileName, tempStorageBuffer, &fileSize);
    u8* fileData = tempStorageBuffer;
    u32 vsz = *(u32*)fileData; 
    fileData += 4;
    u32 isz = *(u32*)fileData;
    fileData += 4;
    f32* vts = (f32*)fileData;
    fileData += vsz;
    u16* ids = (u16*)fileData;
    TexturedMesh m = createTexturedMesh(vts, vsz, ids, isz);
    m.texture = texturedMeshRenderer.defaultTexture;
    return m;
}

static void renderText(const s8* text, f32 xpos, f32 ypos, f32 scale, Vector4 color){
    Font* f = textRenderer.currentFont;
    d3d11Context->PSSetSamplers(0, 1, &pointSampler);
    d3d11Context->VSSetShader(textRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(textRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(textRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &textRenderer.vertexBuffer, &textRenderer.vertexStride, &textRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(textRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &textRenderer.vertexConstBuffer);
    d3d11Context->PSSetConstantBuffers(0, 1, &textRenderer.pixelConstBuffer);
    d3d11Context->PSSetShaderResources(0, 1, &f->bitmap.resourceView);

    D3D11_MAPPED_SUBRESOURCE vertData;
    D3D11_MAPPED_SUBRESOURCE indData;
    d3d11Context->Map(textRenderer.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertData);
    d3d11Context->Map(textRenderer.indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indData);
    
    f32* vdat = (f32*)vertData.pData;
    u16* idat = (u16*)indData.pData;

    u32 vctr = 0;
    u32 ictr = 0;
    u32 polyCtr = 0;

    f32 xStart = xpos;
    f32 yStart = ypos;

    const s8* c = text;
    while(*c != '\0'){
        u32 charIndex = binarySearch(f->characterCodes, *c, 0, f->totalCharacters, f->missingCharacterCodeIndex);
        
        f32 bmX = f->bitmapX[charIndex];
        f32 bmY = f->bitmapY[charIndex];
        f32 bmW = f->bitmapCharacterWidth[charIndex];
        f32 bmH = f->bitmapCharacterHeight[charIndex];
        f32 cW = f->width[charIndex];
        f32 cH = f->height[charIndex];

        vdat[vctr++] = xStart; vdat[vctr++] = yStart; vdat[vctr++] = bmX; vdat[vctr++] = bmY + bmH ;
        vdat[vctr++] = xStart; vdat[vctr++] =  yStart + (cH * scale); vdat[vctr++] = bmX; vdat[vctr++] = bmY;
        vdat[vctr++] = xStart + (cW * scale); vdat[vctr++] = yStart + (cH * scale); vdat[vctr++] = bmX + bmW; vdat[vctr++] = bmY;
        vdat[vctr++] =  xStart + (cW * scale); vdat[vctr++] = yStart; vdat[vctr++] = bmX + bmW; vdat[vctr++] = bmY + bmH;

        xStart += (cW * scale) + f->kerning[charIndex];

        idat[ictr++] = polyCtr; idat[ictr++] = polyCtr + 1; idat[ictr++] = polyCtr + 2; 
        idat[ictr++] = polyCtr + 2; idat[ictr++] = polyCtr + 3; idat[ictr++] = polyCtr;
        polyCtr += 4;

        c++;
    }

    d3d11Context->Unmap(textRenderer.vertexBuffer, 0);
    d3d11Context->Unmap(textRenderer.indexBuffer, 0);
    textRenderer.pixelConstants.color = color;
    d3d11Context->UpdateSubresource(textRenderer.pixelConstBuffer, 0, 0, &textRenderer.pixelConstants, 0, 0);

    d3d11Context->DrawIndexed(ictr, 0, 0);
}

static void renderTexturedMesh(TexturedMesh* mesh, Camera* camera, PointLight* light){
    d3d11Context->PSSetSamplers(0, 1, &linearSampler);
    d3d11Context->VSSetShader(texturedMeshRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(texturedMeshRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(texturedMeshRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &texturedMeshRenderer.vertexBuffer, &texturedMeshRenderer.vertexStride, &texturedMeshRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(texturedMeshRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &texturedMeshRenderer.vertexConstBuffer);
    d3d11Context->PSSetConstantBuffers(0, 1, &texturedMeshRenderer.pixelConstBuffer);
    d3d11Context->PSSetShaderResources(0, 1, &mesh->texture.resourceView);

    texturedMeshRenderer.vertexConstants.modelMatrix = buildModelMatrix(mesh->position, mesh->scale, mesh->orientation);
    texturedMeshRenderer.vertexConstants.cameraMatrix = camera->projection * camera->view;

    texturedMeshRenderer.pixelConstants.cameraPosition = camera->position;

    texturedMeshRenderer.pixelConstants.lightPosition = light->position;
    texturedMeshRenderer.pixelConstants.ambient = light->ambient;
    texturedMeshRenderer.pixelConstants.diffuse = light->diffuse;
    texturedMeshRenderer.pixelConstants.specular = light->specular;

    d3d11Context->UpdateSubresource(texturedMeshRenderer.vertexConstBuffer, 0, 0, &texturedMeshRenderer.vertexConstants, 0, 0);
    d3d11Context->UpdateSubresource(texturedMeshRenderer.pixelConstBuffer, 0, 0, &texturedMeshRenderer.pixelConstants, 0, 0);

    d3d11Context->DrawIndexed(mesh->indexCount, mesh->indexOffset, mesh->indexAddon);
}

static void renderTexturedMeshes(TexturedMesh* meshes, u32 totalMeshes, Camera* camera, PointLight* light){
    d3d11Context->PSSetSamplers(0, 1, &linearSampler);
    d3d11Context->VSSetShader(texturedMeshRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(texturedMeshRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(texturedMeshRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &texturedMeshRenderer.vertexBuffer, &texturedMeshRenderer.vertexStride, &texturedMeshRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(texturedMeshRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &texturedMeshRenderer.vertexConstBuffer);
    d3d11Context->PSSetConstantBuffers(0, 1, &texturedMeshRenderer.pixelConstBuffer);

    texturedMeshRenderer.vertexConstants.cameraMatrix = camera->projection * camera->view;

    texturedMeshRenderer.pixelConstants.cameraPosition = camera->position;
    texturedMeshRenderer.pixelConstants.lightPosition = light->position;
    texturedMeshRenderer.pixelConstants.ambient = light->ambient;
    texturedMeshRenderer.pixelConstants.diffuse = light->diffuse;
    texturedMeshRenderer.pixelConstants.specular = light->specular;
    d3d11Context->UpdateSubresource(texturedMeshRenderer.pixelConstBuffer, 0, 0, &texturedMeshRenderer.pixelConstants, 0, 0);

    for(u32 i = 0; i < totalMeshes; i++){
        d3d11Context->PSSetShaderResources(0, 1, &meshes[i].texture.resourceView);
        texturedMeshRenderer.vertexConstants.modelMatrix = buildModelMatrix(meshes[i].position, meshes[i].scale, meshes[i].orientation);
        d3d11Context->UpdateSubresource(texturedMeshRenderer.vertexConstBuffer, 0, 0, &texturedMeshRenderer.vertexConstants, 0, 0);

        d3d11Context->DrawIndexed(meshes[i].indexCount, meshes[i].indexOffset, meshes[i].indexAddon);
    }
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
    tempStorageBuffer = (u8*)VirtualAlloc(0, MEGABYTE(32), MEM_COMMIT, PAGE_READWRITE);

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
    D3D11_SAMPLER_DESC samplerDescripter = {};
    samplerDescripter.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescripter.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescripter.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescripter.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; 
    samplerDescripter.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescripter.MinLOD = 0;
    samplerDescripter.MaxLOD = D3D11_FLOAT32_MAX;
    d3d11Device->CreateSamplerState(&samplerDescripter, &pointSampler);
    samplerDescripter.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    d3d11Device->CreateSamplerState(&samplerDescripter, &linearSampler);

    ID3D11BlendState* blendState;

    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = true;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    d3d11Device->CreateBlendState(&blendStateDesc, &blendState);

    d3d11Context->OMSetBlendState(blendState, 0, 0xffffffff);


    initializeTextRenderer();
    initializeTexturedMeshRenderer();

    u32 fileSize;
    readFileIntoBuffer("suzanne.texpix", tempStorageBuffer, &fileSize);
    u8* fileData = tempStorageBuffer;
    u32 imgWidth = *(u32*)fileData;
    fileData += 4;
    u32 imgHeight = *(u32*)fileData;
    fileData += 4;
    Texture2D suzanneTexture = createTexture2D(fileData, imgWidth, imgHeight, 4);

    TexturedMesh cube2 = createTexturedMesh("suzanne.texmesh");
    cube2.texture = suzanneTexture;

    const u32 MESH_COUNT = 1000;
    TexturedMesh meshes[MESH_COUNT];
    u32 seed = 1;
    for(u32 i = 0; i < MESH_COUNT; i++){
        meshes[i] = cube2;
        seed = xorshift(seed);
        meshes[i].position.x = seed % 100;
        meshes[i].position.x -= 50;
        seed = xorshift(seed);
        meshes[i].position.y = seed % 100;
        meshes[i].position.y -= 50;
        seed = xorshift(seed);
        meshes[i].position.z = seed % 100;
        meshes[i].position.z -= 50;
    }
    
    Camera camera;
    camera.position.z -= 5;
    camera.moveSpeed = 3;
    camera.rotateSpeed = 1;
    camera.mouseSensitivity = 0.1;

    camera.projection = createPerspectiveProjection(70.0, (f32)windowWidth / (f32)windowHeight, 0.001, 1000.0);

    PointLight light;
    light.position = Vector3(5, 5, -3);
    light.diffuse = Vector3(1, 1, 1);

    screenCenter.x = halfWindowWidth;
    screenCenter.y = halfWindowHeight;
    ClientToScreen(hwnd, &screenCenter);
    SetCursorPos(screenCenter.x, screenCenter.y);
    //ShowCursor(false);
    ShowWindow(hwnd, SW_SHOW);
    bool isRunning = true;

    DWORD dwResult;    
    XINPUT_STATE state = {};
    s64 gamepadIndex;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++){
        dwResult = XInputGetState(i, &state);
        if(dwResult == ERROR_SUCCESS){
            gamepadIndex = i;
            break;
        }else{
        }
    }
    f32 LX = state.Gamepad.sThumbLX;
    f32 LY = state.Gamepad.sThumbLY;

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

        updateCameraView(&camera);

        d3d11Context->ClearRenderTargetView(renderTargetView, clearColor.v);
        d3d11Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);

        light.position = -camera.position;

        
        dwResult = XInputGetState(gamepadIndex, &state);
         if(dwResult == ERROR_SUCCESS){
            LX = state.Gamepad.sThumbLX;
            LY = state.Gamepad.sThumbLY;
        }
        

        //rendering is done here
        renderTexturedMeshes(meshes, MESH_COUNT, &camera, &light);

        renderText("`~!@#$%^&*()-_=+[]{}|\\;:\'", 50, 400, 5, Vector4(1, 0, 0, 1));
        renderText("<>,./?0123456789abcdefghi", 50, 350, 5, Vector4(1, 1, 0, 1));
        renderText("jklmnopqrstuvwxyzABCDEF", 50, 300, 5, Vector4(1, 1, 1, 0.5));
        renderText("GHIJKLMNOPQRSTUVWXYZ", 50, 250, 5, Vector4(0.2, 0.5, 0.8, 1));
        s8 buf[512];
        createDebugString(buf, "Left X:%f  LeftY:%f", LX, LY);
        renderText(buf, 50, 200, 2, Vector4(0, 0, 0, 1));

        swapChain->Present(1, 0);

        endTime = GetTickCount64();
        deltaTime = (f32)(endTime - startTime) / 1000.0f;
        startTime = endTime;
    }

    return 0;
}