#include "saguinus.cpp"

#include "win_saguinus.h"

static u32 windowWidth = 1200;
static u32 windowHeight = 675;
static s32 halfWindowWidth = windowWidth * 0.5;
static s32 halfWindowHeight = windowHeight * 0.5;

static void checkError(HRESULT err, LPCSTR msg){
    if(err != S_OK){
        MessageBox(0, msg, "ERROR", 0);
    }
}

static f32 cubicInterpolate (f32 p[4], f32 x) {
	return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

static f32 bicubicInterpolate (f32 p[4][4], f32 x, f32 y) {
	f32 arr[4];
	arr[0] = cubicInterpolate(p[0], y);
	arr[1] = cubicInterpolate(p[1], y);
	arr[2] = cubicInterpolate(p[2], y);
	arr[3] = cubicInterpolate(p[3], y);
	return cubicInterpolate(arr, x);
}

static void createCubiclyInterpolatedPlane(f32* buffer, u32 width, u32 height, u32 freq){
    u32 dw = width / freq;
    u32 dh = height / freq;
    u32 totalSize = width * height * 4;
    u32 rAmt = freq * 4;
    f32** ranNums = (f32**)malloc(sizeof(f32*) * rAmt);
    for(u32 i = 0; i < rAmt; i++){
        ranNums[i] = (f32*)malloc(sizeof(f32) * rAmt);
    }
    for(u32 i = 0; i < rAmt; i++){
        for(u32 j = 0; j < rAmt; j++){
            ranNums[i][j] = (f32)randomU32() / (f32)MAX_U32;
        }
    }

    u32 ctr = 0;
    for(u32 i = 0; i < height; i++){
        for(u32 j = 0; j < width; j++){
            u32 row = i / dh;
            u32 col = j / dw;
            f32 rnum[4][4] = {
                {ranNums[row + 0][col], ranNums[row + 0][col + 1], ranNums[row + 0][col + 2], ranNums[row + 0][col + 3]},
                {ranNums[row + 1][col], ranNums[row + 1][col + 1], ranNums[row + 1][col + 2], ranNums[row + 1][col + 3]},
                {ranNums[row + 2][col], ranNums[row + 2][col + 1], ranNums[row + 2][col + 2], ranNums[row + 2][col + 3]},
                {ranNums[row + 3][col], ranNums[row + 3][col + 1], ranNums[row + 3][col + 2], ranNums[row + 3][col + 3]},
            };

            f32 x = (f32)(i % dh) / dh;
            f32 y = (f32)(j % dw) / dw;
            f32 v = bicubicInterpolate(rnum, x, y);
            buffer[ctr++] = v;
            buffer[ctr++] = v;
            buffer[ctr++] = 1;
            buffer[ctr++] = 1;
        }
    }

    for(u32 i = 0; i < rAmt; i++){
        free(ranNums[i]);
    }
    free(ranNums);
}

static void generatePerlinNoiseCubePixels(u8* buffer, u32 width = 256, u32 height = 256){
    u32 totalSize = width * height * 4;
    f32* tbuf = (f32*)malloc(totalSize * sizeof(f32));
    f32* tbuf2 = (f32*)malloc(totalSize * sizeof(f32));

    createCubiclyInterpolatedPlane(tbuf2, width, height, 1);
    for(u32 i = 0; i < totalSize; i++){
        tbuf[i] = tbuf2[i];
    }
    createCubiclyInterpolatedPlane(tbuf2, width, height, 2);
    for(u32 i = 0; i < totalSize; i++){
        tbuf[i] += tbuf2[i];
    }
    createCubiclyInterpolatedPlane(tbuf2, width, height, 4);
    for(u32 i = 0; i < totalSize; i++){
        tbuf[i] += tbuf2[i];
    }
    createCubiclyInterpolatedPlane(tbuf2, width, height, 8);
    for(u32 i = 0; i < totalSize; i++){
        tbuf[i] += tbuf2[i];
    }
    for(u32 i = 0; i < totalSize; i++){
        tbuf[i] /= 4;
    }

    u32 ctr = 0;
    for(u32 ct = 0; ct < 6; ct++){
        for(u32 i = 0; i < totalSize; i++){
            buffer[ctr++] = tbuf[i] * 255;
        }
    }

    free(tbuf);
}

static void* allocateMemory(u32 amount){
    return VirtualAlloc(0, amount, MEM_COMMIT, PAGE_READWRITE);
}

static bool readFileIntoBuffer(const s8* fileName, void* data, u32* fileLength){
    HANDLE file = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(file == INVALID_HANDLE_VALUE){
        return false;
    }        
    DWORD fileSize;
    fileSize = GetFileSize(file, 0);
    bool res = ReadFile(file, data, fileSize, 0, 0);
    if(!res){
        MessageBox(0, "Could not read file", "readFileIntoBuffer", 0);
        return false;
    }
    *fileLength = fileSize;
    CloseHandle(file);
    return true;
}

static bool writeToFile(const s8* fileName, void* data, u32 dataSize){
    HANDLE file = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    DWORD bytesWritten;
    bool res = WriteFile(file, data, dataSize, &bytesWritten, 0);

    if(!res){
        MessageBox(0, "Could not write to file", "writeToFile", 0);
        return false;
    }
    CloseHandle(file);
    return true;
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

    D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc = {};
    resViewDesc.Format = texDesc.Format;
    resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resViewDesc.Texture2D.MipLevels = 1;
    resViewDesc.Texture2D.MostDetailedMip = 0;

    hr = d3d11Device->CreateShaderResourceView(texture, &resViewDesc, (ID3D11ShaderResourceView**)&tex.texture);
    checkError(hr, "Error creating shader resource view");
    texture->Release();

    return tex;
}

static TextureCube createTextureCube(u8* data, u32 width, u32 height, u32 bytesPerPixel){
    TextureCube tex;

    D3D11_SUBRESOURCE_DATA texData[6] = {};

    u32 pitch = bytesPerPixel * width;
    u32 bytesPerTexture = pitch * height;
    for(u32 i = 0; i < 6; i++){
        texData[i].pSysMem = data + (i * bytesPerTexture);
        texData[i].SysMemPitch = pitch;
    }
    
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height; 
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
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
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* texture;
    HRESULT hr = d3d11Device->CreateTexture2D(&texDesc, texData, &texture);
    checkError(hr, "Error creating texture");

    D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc = {};
    resViewDesc.Format = texDesc.Format;
    resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    resViewDesc.Texture2D.MipLevels = 1;
    resViewDesc.Texture2D.MostDetailedMip = 0;

    hr = d3d11Device->CreateShaderResourceView(texture, &resViewDesc, (ID3D11ShaderResourceView**)&tex.texture);
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
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &texturedMeshRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    texturedMeshRenderer.vertexStride = sizeof(float) * 8;
    texturedMeshRenderer.vertexOffset = 0;

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
 

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &texturedMeshRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");


    bufferDesc.ByteWidth = sizeof(texturedMeshRenderer.vertexConstants);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &texturedMeshRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &constBufData, &texturedMeshRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");


    bufferDesc.ByteWidth = sizeof(texturedMeshRenderer.pixelConstants);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &texturedMeshRenderer.pixelConstants;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &pixConstBufData, &texturedMeshRenderer.pixelConstBuffer);
    checkError(hr, "Error creating pixel constant buffer");

    u8 tex[] = {
        200, 200, 200, 255,     100, 100, 100, 255,
        100, 100, 100, 255,     200, 200, 200, 255
    };
    texturedMeshRenderer.defaultTexture = createTexture2D(tex, 2, 2, 4);
}

static void initializeCanvasRenderer(){
    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"canvas_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"canvas_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    HRESULT hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &canvasRenderer.vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex shader", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &canvasRenderer.pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel shader", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UVCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = d3d11Device->CreateInputLayout(layoutDesc, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &canvasRenderer.inputLayout);
    checkError(hr, "Could not create input layout");

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &canvasRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    canvasRenderer.vertexStride = sizeof(float) * 4;
    canvasRenderer.vertexOffset = 0;

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &canvasRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");

    D3D11_BUFFER_DESC vertConstDesc = {};
    vertConstDesc.ByteWidth = sizeof(canvasRenderer.vertexConstants);
    vertConstDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    canvasRenderer.vertexConstants.projectionMatrix = createOrthogonalProjection(0, windowWidth, 0, windowHeight, -1, 1);

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &canvasRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&vertConstDesc, &constBufData, &canvasRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");

    D3D11_BUFFER_DESC pixConstBufDesc = {};
    pixConstBufDesc.ByteWidth = sizeof(canvasRenderer.pixelConstants);
    pixConstBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &canvasRenderer.pixelConstants;

    hr = d3d11Device->CreateBuffer(&pixConstBufDesc, &pixConstBufData, &canvasRenderer.pixelConstBuffer);
    checkError(hr, "Error creating pixel constant buffer");

    u8 pix[] = {255, 255, 255, 255};
    canvasRenderer.defaultTexture = createTexture2D(pix, 1, 1, 4);

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
}

static void initializeDebugRenderer(){
    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"debug_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"debug_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    HRESULT hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &debugRenderer.vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex shader", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &debugRenderer.pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel shader", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = d3d11Device->CreateInputLayout(layoutDesc, 1, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &debugRenderer.inputLayout);
    checkError(hr, "Could not create input layout");

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    f32* vdat = (f32*)tempStorageBuffer;
    u32 vctr = 0;
    for(u32 i = 0; i < 72; i++){
        vdat[vctr++] = cubeVertices[i];
    }

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &debugRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    debugRenderer.vertexStride = sizeof(float) * 3;
    debugRenderer.vertexOffset = 0;

    //TEMPORARY BUFFER SIZE --- FIX THIS!!
    bufferDesc.ByteWidth = MEGABYTE(32);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    u16* idat = (u16*)tempStorageBuffer;
    u32 ictr = 0;
    for(u32 i = 0; i < 36; i++){
        idat[ictr++] = cubeIndices[i];
    }
    bufferData.pSysMem = tempStorageBuffer;
    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &debugRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");

    D3D11_BUFFER_DESC vertConstBufDesc = {};
    vertConstBufDesc.ByteWidth = sizeof(debugRenderer.vertexConstants);
    vertConstBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &debugRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&vertConstBufDesc, &constBufData, &debugRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");

    D3D11_BUFFER_DESC pixConstBufDesc = {};
    pixConstBufDesc.ByteWidth = sizeof(debugRenderer.pixelConstants);
    pixConstBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA pixConstBufData = {};
    pixConstBufData.pSysMem = &debugRenderer.pixelConstants;

    hr = d3d11Device->CreateBuffer(&pixConstBufDesc, &pixConstBufData, &debugRenderer.pixelConstBuffer);
    checkError(hr, "Error creating pixel constant buffer");

    debugRenderer.currentBufferCount = 72;
    debugRenderer.currentIndexCount = 36;
}

static void initializeSkyboxRenderer(){
    ID3DBlob* vertexBlob; 
    ID3DBlob* pixelBlob;
    ID3DBlob* errBlob = 0;               
    D3DCompileFromFile(L"skybox_shader.hlsl", 0, 0, "vertexMain", "vs_5_0", 0, 0, &vertexBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);
    D3DCompileFromFile(L"skybox_shader.hlsl", 0, 0, "pixelMain", "ps_5_0", 0, 0, &pixelBlob, &errBlob);
    if(errBlob != 0) MessageBox(0, (LPCSTR)errBlob->GetBufferPointer(), "ERROR", 0);

    HRESULT hr = d3d11Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), 0, &skyboxRenderer.vertexShader);
    if(errBlob != 0) MessageBox(0, "Error creating vertex shader", "ERROR", 0);
    hr = d3d11Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), 0, &skyboxRenderer.pixelShader);
    if(errBlob != 0) MessageBox(0, "Error creating pixel shader", "ERROR", 0);

    D3D11_INPUT_ELEMENT_DESC layoutDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = d3d11Device->CreateInputLayout(layoutDesc, 1, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &skyboxRenderer.inputLayout);
    checkError(hr, "Could not create input layout");

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(f32) * 5 * 24;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferData.pSysMem = tempStorageBuffer;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &skyboxRenderer.vertexBuffer);
    if(hr != S_OK)  MessageBox(0, "Error creating vertex buffer", "ERROR", 0);

    skyboxRenderer.vertexStride = sizeof(float) * 3;
    skyboxRenderer.vertexOffset = 0;

    bufferDesc.ByteWidth = sizeof(u16) * 36;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
 

    hr = d3d11Device->CreateBuffer(&bufferDesc, &bufferData, &skyboxRenderer.indexBuffer);
    checkError(hr, "Error creating index buffer");

    d3d11Context->UpdateSubresource(skyboxRenderer.vertexBuffer, 0, 0, skyboxVertices, 0, 0);
    d3d11Context->UpdateSubresource(skyboxRenderer.indexBuffer, 0, 0, cubeIndices, 0, 0);

    bufferDesc.ByteWidth = sizeof(skyboxRenderer.vertexConstants);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA constBufData = {};
    constBufData.pSysMem = &skyboxRenderer.vertexConstants;

    hr = d3d11Device->CreateBuffer(&bufferDesc, &constBufData, &skyboxRenderer.vertexConstBuffer);
    checkError(hr, "Error creating vertex constant buffer");


    u8 sbTex[] = {
        255, 0, 0, 255,
        200, 150, 0, 255,
        255, 255, 0, 255,
        0, 255, 0, 255,
        0, 0, 255, 255,
        200, 50, 150, 255
    };

    generatePerlinNoiseCubePixels(tempStorageBuffer);

    skyboxRenderer.defaultTexture = createTextureCube(tempStorageBuffer, 256, 256, 4);
    skyboxRenderer.currentTexture = skyboxRenderer.defaultTexture;
}

static void renderSkybox(Camera* camera){
    d3d11Context->PSSetSamplers(0, 1, &linearSampler);
    d3d11Context->VSSetShader(skyboxRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(skyboxRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(skyboxRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &skyboxRenderer.vertexBuffer, &skyboxRenderer.vertexStride, &skyboxRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(skyboxRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &skyboxRenderer.vertexConstBuffer);
    // d3d11Context->PSSetConstantBuffers(0, 1, &skyboxRenderer.pixelConstBuffer);
    d3d11Context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&skyboxRenderer.currentTexture);

    Matrix4 camMat = camera->projection * quaternionToMatrix4(camera->orientation);
    d3d11Context->UpdateSubresource(skyboxRenderer.vertexConstBuffer, 0, 0, &camMat, 0, 0);

    d3d11Context->DrawIndexed(36, 0, 0);
}

static void renderDebugBuffer(DebugBuffer* buffer, Camera* camera){
    d3d11Context->VSSetShader(debugRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(debugRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(debugRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &debugRenderer.vertexBuffer, &debugRenderer.vertexStride, &debugRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(debugRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &debugRenderer.vertexConstBuffer);
    d3d11Context->PSSetConstantBuffers(0, 1, &debugRenderer.pixelConstBuffer);

    u32 len = buffer->totalCubes;
    Matrix4 camMat = camera->projection * camera->view;
    for(u32 i = 0; i < len; i++){
        Matrix4 modelMatrix = buildModelMatrix(buffer->cubePositions[i], buffer->cubeScales[i], Quaternion());
        debugRenderer.vertexConstants.cameraMatrix = camMat * modelMatrix;
        debugRenderer.pixelConstants.color = buffer->cubeColors[i];

        d3d11Context->UpdateSubresource(debugRenderer.vertexConstBuffer, 0, 0, &debugRenderer.vertexConstants, 0, 0);
        d3d11Context->UpdateSubresource(debugRenderer.pixelConstBuffer, 0, 0, &debugRenderer.pixelConstants, 0, 0);

        d3d11Context->DrawIndexed(36, 0, 0);
    }

    buffer->totalCubes = 0;


    D3D11_MAPPED_SUBRESOURCE vertData;
    D3D11_MAPPED_SUBRESOURCE indData;                   
    d3d11Context->Map(debugRenderer.vertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &vertData);
    d3d11Context->Map(debugRenderer.indexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &indData);

    f32* vdat = (f32*)vertData.pData;
    u16* idat = (u16*)indData.pData;
    u32 vctr = debugRenderer.currentBufferCount;
    u32 ictr = debugRenderer.currentIndexCount;

    len = buffer->totalLines;
    for(u32 i = 0; i < len; i++){
        Vector3 startPos = buffer->lineStarts[i];
        Vector3 endPos = buffer->lineEnds[i];
        f32 lineWidth = buffer->lineWidths[i];

        Vector3 val = normalOf(cross(endPos - startPos, camera->position - startPos));
        Vector3 ang = val * lineWidth * 0.5;

        Vector3 p1 = startPos - ang;
        Vector3 p2 = endPos - ang;
        Vector3 p3 = endPos + ang;
        Vector3 p4 = startPos + ang;

        vdat[vctr++] = p1.x;    vdat[vctr++] = p1.y;    vdat[vctr++] = p1.z;
        vdat[vctr++] = p2.x;    vdat[vctr++] = p2.y;    vdat[vctr++] = p2.z;
        vdat[vctr++] = p3.x;    vdat[vctr++] = p3.y;    vdat[vctr++] = p3.z;
        vdat[vctr++] = p4.x;    vdat[vctr++] = p4.y;    vdat[vctr++] = p4.z;
        idat[ictr++] = 0; idat[ictr++] = 1; idat[ictr++] = 2;
        idat[ictr++] = 2; idat[ictr++] = 3; idat[ictr++] = 0;

        debugRenderer.pixelConstants.color = buffer->lineColors[i];
        debugRenderer.vertexConstants.cameraMatrix = camMat;
        d3d11Context->UpdateSubresource(debugRenderer.vertexConstBuffer, 0, 0, &debugRenderer.vertexConstants, 0, 0);
        d3d11Context->UpdateSubresource(debugRenderer.pixelConstBuffer, 0, 0, &debugRenderer.pixelConstants, 0, 0);

        d3d11Context->DrawIndexed(6, debugRenderer.currentIndexCount, debugRenderer.currentBufferCount / 3);
        debugRenderer.currentBufferCount += 12;
        debugRenderer.currentIndexCount += 6;
    }

    d3d11Context->Unmap(debugRenderer.vertexBuffer, 0);
    d3d11Context->Unmap(debugRenderer.indexBuffer, 0);

    buffer->totalLines = 0;
    debugRenderer.currentBufferCount = 72;
    debugRenderer.currentIndexCount = 36;
}

static TexturedMesh createTexturedMesh(f32* vertexData, u32 vertexDataSize, u16* indexData, u32 indexDataSize){
    u32 totalIndices = indexDataSize / sizeof(u16);
    u32 totalVertsInBuffer = texturedMeshRenderer.vertexDataUsed / (sizeof(f32) * 8);
    u32 indexOffset = texturedMeshRenderer.indexDataUsed / sizeof(u16);

    D3D11_MAPPED_SUBRESOURCE vertData;
    D3D11_MAPPED_SUBRESOURCE indData;                   
    d3d11Context->Map(texturedMeshRenderer.vertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &vertData);
    d3d11Context->Map(texturedMeshRenderer.indexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &indData);
    f32* vdat = (f32*)vertData.pData;
    u16* idat = (u16*)indData.pData;
    vdat += texturedMeshRenderer.vertexDataUsed / sizeof(f32);
    idat += indexOffset;

    u32 totalFloats = vertexDataSize / sizeof(f32);
    for(u32 i = 0; i < totalFloats; i++){
        vdat[i] = vertexData[i];
    }

    for(u32 i = 0; i < totalIndices; i++){
        idat[i] = indexData[i];
    }

    d3d11Context->Unmap(texturedMeshRenderer.vertexBuffer, 0);
    d3d11Context->Unmap(texturedMeshRenderer.indexBuffer, 0);


    TexturedMesh mesh;
    
    mesh.indexCount = totalIndices;
    mesh.indexOffset = indexOffset;
    mesh.indexAddon = totalVertsInBuffer;
    mesh.texture = texturedMeshRenderer.defaultTexture;
    mesh.totalVertices = vertexDataSize / (sizeof(f32) * 8);
    mesh.vertexBufferOffset = texturedMeshRenderer.vertexDataUsed;

    texturedMeshRenderer.indexDataUsed += indexDataSize;
    texturedMeshRenderer.vertexDataUsed += vertexDataSize;

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

static AnimatedMesh createAnimatedMesh(f32* vertexData, u32 vertexDataSize, u16* indexData, u32 indexDataSize){
    AnimatedMesh mesh = {};
    u32 totalVertices = vertexDataSize >> 6;

    mesh.positions = (f32*)allocateLongTermMemory(gameState, sizeof(f32) * totalVertices * 3);
    mesh.normals = (f32*)allocateLongTermMemory(gameState, sizeof(f32) * totalVertices * 3);
    mesh.weights = (f32*)allocateLongTermMemory(gameState, sizeof(f32) * totalVertices * 4);
    mesh.bones = (f32*)allocateLongTermMemory(gameState, sizeof(f32) * totalVertices * 4);

    f32* vptr = (f32*)tempStorageBuffer;
    f32* dptr = vertexData;
    f32* pptr = mesh.positions;
    f32* nptr = mesh.normals;
    f32* wptr = mesh.weights;
    f32* bptr = mesh.bones;
    for(u32 i = 0; i < totalVertices; i++){
        for(u32 j = 0; j < 3; j++){
            *vptr = *dptr;
            *pptr = *dptr;
            pptr++;
            vptr++;
            dptr++;
        }
        for(u32 j = 0; j < 3; j++){
            *vptr = *dptr;
            *nptr = *dptr;
            nptr++;
            vptr++;
            dptr++;
        }
        for(u32 j = 0; j < 2; j++){
            *vptr = *dptr;
            vptr++;
            dptr++;
        }
        for(u32 j = 0; j < 4; j++){
            *wptr = *dptr;
            wptr++;
            dptr++;
        }
        for(u32 j = 0; j < 4; j++){
            *bptr = *dptr;
            bptr++;
            dptr++;
        }
    }
    mesh.mesh = createTexturedMesh((f32*)tempStorageBuffer, totalVertices * 32, indexData, indexDataSize);
    mesh.totalVertices = totalVertices;

    return mesh;
}

static AnimatedMesh createAnimatedMesh(const s8* fileName){
    u32 fl;
    readFileIntoBuffer(fileName, tempStorageBuffer, &fl);
    u8* tsbPtr = tempStorageBuffer;
    u32 vSize = *(u32*)tsbPtr;
    tsbPtr += 4;
    u32 iSize = *(u32*)tsbPtr;
    tsbPtr += 4;
    f32* vData = (f32*)tsbPtr;
    tsbPtr += vSize;
    u16* iData = (u16*)tsbPtr;

    AnimatedMesh mesh = createAnimatedMesh(vData, vSize, iData, iSize);

    return mesh;
}

static MeshAnimation createMeshAnimation(const s8* fileName){
    MeshAnimation anim = {};
    u32 fl;
    readFileIntoBuffer(fileName, tempStorageBuffer, &fl);
    u8* tsbPtr = tempStorageBuffer;
    u32 totalBones = *(u32*)tsbPtr;
    tsbPtr += 4;
    u32 totalPoses = *(u32*)tsbPtr;
    tsbPtr += 4;

    u32 keyframesSize = totalPoses * sizeof(u32);
    u32 posesSize = totalBones * totalPoses * sizeof(Bone);
    u32 ibtSize = totalBones * sizeof(Matrix4);
    u32 totalSize = keyframesSize + posesSize + ibtSize;

    u8* wptr = allocateLongTermMemory(gameState, totalSize);
    anim.keyframes = (u32*)wptr;
    wptr += keyframesSize;
    anim.poses = (Bone*)wptr;
    wptr += posesSize;
    anim.inverseBindTransforms = (Matrix4*)wptr;

    u32* kf = anim.keyframes;
    for(u32 i = 0; i < totalPoses; i++){
        f32 f = *(f32*)tsbPtr;
        *kf = (u32)f;
        kf++;
        tsbPtr += 4;
    }

    Matrix4* ibt = anim.inverseBindTransforms;
    for(u32 i = 0; i < totalBones; i++){
        *ibt = *(Matrix4*)tsbPtr;
        ibt++;
        tsbPtr += sizeof(Matrix4);
    }
    
    Bone* bp = anim.poses;
    u32 totalBonesInAnimation = totalBones * totalPoses;
    for(u32 i = 0; i < totalBonesInAnimation; i++){
        *bp = *(Bone*)tsbPtr;
        bp++;
        tsbPtr += sizeof(Bone);
    }

    anim.currentKeyframe = 0;
    anim.nextKeyframe = 1;
    anim.totalBones = totalBones;
    anim.totalPoses = totalPoses;
    anim.frameRate = 24;
    anim.currentPoseTime = (anim.keyframes[1] - anim.keyframes[0]) / anim.frameRate;

    return anim;
}

static void renderAnimatedMesh(AnimatedMesh* mesh, Vector3 position, Vector3 scale, Quaternion orientation){
    MeshAnimation* anim = mesh->animation;

    f32 dt = anim->currentPoseElapsed / anim->currentPoseTime;
    u32 cfi = anim->currentKeyframe * anim->totalBones;
    u32 nfi = anim->nextKeyframe * anim->totalBones;

    Matrix4 poseMatrices[32];
    Matrix4 parentMatrices[32];
    Quaternion brot = slerp(anim->poses[cfi].orientaion, anim->poses[nfi].orientaion, dt);
    Vector3 bpos = linearInterpolation(anim->poses[cfi].position, anim->poses[nfi].position, dt);
    Matrix4 bmat = buildModelMatrix(bpos, Vector3(1), brot);
    parentMatrices[0] = bmat;
    poseMatrices[0] = bmat * anim->inverseBindTransforms[0];
    for(u32 i = 1; i < anim->totalBones; i++){
        brot = slerp(anim->poses[cfi + i].orientaion, anim->poses[nfi + i].orientaion, dt);
        bpos = linearInterpolation(anim->poses[cfi + i].position, anim->poses[nfi + i].position, dt);
        bmat = buildModelMatrix(bpos, Vector3(1), brot);
        bmat = parentMatrices[(int)anim->poses[cfi + i].parentIndex] * bmat;
        parentMatrices[i] = bmat;
        poseMatrices[i] = bmat * anim->inverseBindTransforms[i];
    }

    D3D11_MAPPED_SUBRESOURCE vertData;            
    d3d11Context->Map(texturedMeshRenderer.vertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &vertData);
    f32* vdat = (f32*)vertData.pData;
    vdat += mesh->mesh.vertexBufferOffset / sizeof(f32);

    for(u32 i = 0; i < mesh->totalVertices; i++){
        Vector3 vpos = ((Vector3*)mesh->positions)[i];
        Vector3 vnrm = ((Vector3*)mesh->normals)[i];
        Vector4 weight = ((Vector4*)mesh->weights)[i];
        Vector4 bones = ((Vector4*)mesh->bones)[i];

        Vector4 p4(vpos, 1.0);
        Vector4 n4(vnrm, 0.0);

        Vector4 npos = (poseMatrices[(int)bones.x] * p4) * weight.x;
        npos +=        (poseMatrices[(int)bones.y] * p4) * weight.y;
        npos +=        (poseMatrices[(int)bones.z] * p4) * weight.z;
        npos +=        (poseMatrices[(int)bones.w] * p4) * weight.w;

        Vector4 nnrm = (poseMatrices[(int)bones.x] * n4) * weight.x;
        nnrm +=        (poseMatrices[(int)bones.y] * n4) * weight.y;
        nnrm +=        (poseMatrices[(int)bones.z] * n4) * weight.z;
        nnrm +=        (poseMatrices[(int)bones.w] * n4) * weight.w;

        vdat[0] = npos.x;
        vdat[1] = npos.y;
        vdat[2] = npos.z;
        vdat[3] = nnrm.x;
        vdat[4] = nnrm.y;
        vdat[5] = nnrm.z;
        vdat += 8;
    }

    d3d11Context->Unmap(texturedMeshRenderer.vertexBuffer, 0);

    addTexturedMeshToBuffer(gameState, &mesh->mesh, position, scale, orientation);
}

static void renderCanvasBuffer(CanvasBuffer* buffer){
    d3d11Context->PSSetSamplers(0, 1, &pointSampler);
    d3d11Context->VSSetShader(canvasRenderer.vertexShader, 0, 0);
    d3d11Context->PSSetShader(canvasRenderer.pixelShader, 0, 0);
    d3d11Context->IASetInputLayout(canvasRenderer.inputLayout);
    d3d11Context->IASetVertexBuffers(0, 1, &canvasRenderer.vertexBuffer, &canvasRenderer.vertexStride, &canvasRenderer.vertexOffset);
    d3d11Context->IASetIndexBuffer(canvasRenderer.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    d3d11Context->VSSetConstantBuffers(0, 1, &canvasRenderer.vertexConstBuffer);
    d3d11Context->PSSetConstantBuffers(0, 1, &canvasRenderer.pixelConstBuffer);

    u32 tot = buffer->totalGUIItems;
    f32 depth = -0.9999;
    for(u32 i = 0; i < tot; i++){
        d3d11Context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&buffer->textures[i]);

        Vector2 position = buffer->positions[i];
        Vector2 scale = buffer->scales[i];
        Vector2 textureOffset = buffer->textureOffsets[i];
        Vector2 textureScale = buffer->textureScales[i];

        D3D11_MAPPED_SUBRESOURCE vertData;
        D3D11_MAPPED_SUBRESOURCE indData;
        d3d11Context->Map(canvasRenderer.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertData);
        d3d11Context->Map(canvasRenderer.indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indData);
        
        f32* vdat = (f32*)vertData.pData;
        u16* idat = (u16*)indData.pData;

        u32 vctr = 0;
        u32 ictr = 0;
        u32 polyCtr = 0;

        vdat[vctr++] = position.x; vdat[vctr++] = position.y; vdat[vctr++] = textureOffset.x; vdat[vctr++] = textureOffset.y + textureScale.y;
        vdat[vctr++] = position.x; vdat[vctr++] = position.y + scale.y; vdat[vctr++] = textureOffset.x; vdat[vctr++] = textureOffset.y;
        vdat[vctr++] = position.x + scale.x; vdat[vctr++] = position.y + scale.y; vdat[vctr++] = textureOffset.x + textureScale.x; vdat[vctr++] = textureOffset.y;
        vdat[vctr++] = position.x + scale.x; vdat[vctr++] = position.y; vdat[vctr++] = textureOffset.x + textureScale.x; vdat[vctr++] = textureOffset.y + textureScale.y;

        idat[ictr++] = polyCtr; idat[ictr++] = polyCtr + 1; idat[ictr++] = polyCtr + 2; 
        idat[ictr++] = polyCtr + 2; idat[ictr++] = polyCtr + 3; idat[ictr++] = polyCtr;
        polyCtr += 4;

        canvasRenderer.vertexConstants.depth = depth;
        canvasRenderer.vertexConstants.isText = buffer->isText[i];
        d3d11Context->UpdateSubresource(canvasRenderer.vertexConstBuffer, 0, 0, &canvasRenderer.vertexConstants, 0, 0);
        depth += 0.0001;
        canvasRenderer.pixelConstants.color = buffer->colors[i];
        d3d11Context->UpdateSubresource(canvasRenderer.pixelConstBuffer, 0, 0, &canvasRenderer.pixelConstants, 0, 0);
        d3d11Context->Unmap(canvasRenderer.vertexBuffer, 0);
        d3d11Context->Unmap(canvasRenderer.indexBuffer, 0);
        d3d11Context->DrawIndexed(ictr, 0, 0);
    }

    buffer->totalGUIItems = 0;
    buffer->debugPrinterY = buffer->debugPrinterStartY;
}

static void renderTexturedMeshBuffer(TexturedMeshBuffer* tmb, Camera* camera, PointLight* light){
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

    u32 totalMeshes = tmb->totalMeshes;
    for(u32 i = 0; i < totalMeshes; i++){
        d3d11Context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&tmb->textures[i].texture);
        texturedMeshRenderer.vertexConstants.modelMatrix = buildModelMatrix(tmb->positions[i], tmb->scales[i], tmb->orientations[i]);
        d3d11Context->UpdateSubresource(texturedMeshRenderer.vertexConstBuffer, 0, 0, &texturedMeshRenderer.vertexConstants, 0, 0);

        d3d11Context->DrawIndexed(tmb->indexCounts[i], tmb->indexOffsets[i], tmb->indexAddons[i]);
    }
    tmb->totalMeshes = 0;
}

static void setMasterAudioVolume(f32 v){
        checkError(XAudio2MasterVoice->SetVolume(0.002), "Could not set master volume");
}

static void updateAudioEmitterDynamics(AudioEmitter ae, Vector3 epos, Vector3 lpos, Vector3 lrgt){
    Vector3 etol = epos - lpos;
    f32 len = length(etol);
    len = maximumOf((10 - len) / 10, 0);
    f32 dp = dot(normalOf(etol), lrgt);
    dp = (dp + 1) * 0.5;
    Vector2 nc(1 - dp, dp);
    nc.x *= len;
    nc.y *= len;
    checkError(((IXAudio2SourceVoice*)ae.emitter)->SetChannelVolumes(2, nc.v), "Could not set channel volumes");
}

static void playAudioEmitter(AudioEmitter ae, s8* buffer, u32 bufferSize){
    XAUDIO2_BUFFER audioBuffer = {}; 
    audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
    audioBuffer.AudioBytes = bufferSize;
    audioBuffer.pAudioData = (const BYTE*)buffer;
    audioBuffer.PlayLength = bufferSize / 2;
    checkError(((IXAudio2SourceVoice*)ae.emitter)->Stop(0), "Could not stop sound");
    checkError(((IXAudio2SourceVoice*)ae.emitter)->FlushSourceBuffers(), "Could not flush source buffer");
    checkError(((IXAudio2SourceVoice*)ae.emitter)->SubmitSourceBuffer(&audioBuffer), "Could not submit audio buffer");
    checkError(((IXAudio2SourceVoice*)ae.emitter)->Start(0), "Could not play sound");
}

static AudioEmitter createAudioEmitter(){
    AudioEmitter ae;

    WAVEFORMATEX wft = {};         
    wft.wFormatTag = WAVE_FORMAT_PCM;
    wft.nChannels = 2;
    wft.nSamplesPerSec = 44100; //fix this
    wft.nAvgBytesPerSec = 44100 * 2; //fix this
    wft.nBlockAlign = 2;
    wft.wBitsPerSample = 8;

    IXAudio2SourceVoice* pSourceVoice;
    checkError(XAudio2Pointer->CreateSourceVoice(&((IXAudio2SourceVoice*)ae.emitter), (WAVEFORMATEX*)&wft), "Could not create source voice");

    return ae;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg){
        case WM_QUIT:
        case WM_CLOSE:{
            exit(0);
            break;
        }
        case WM_WINDOWPOSCHANGED:{
            screenCenter.x = halfWindowWidth;
            screenCenter.y = halfWindowHeight;
            ClientToScreen(hwnd, &screenCenter);
            break;
        }
        case WM_MOUSEMOVE:{
            GetCursorPos(&mousePosition);
            ScreenToClient(hwnd, &mousePosition);
            f32 nmy = windowHeight - mousePosition.y;
            if(mousePosition.x != gameState->mousePosition.x || nmy != gameState->mousePosition.y){
                gameState->mousePosition = Vector2(mousePosition.x, nmy);
                gameState->updateCamera = true;
                if(gameState->mode == GameMode::GAME_MODE_DEBUG){
                    SetCursorPos(screenCenter.x, screenCenter.y);
                }
            }
            
            break;
        }
        case WM_GETMINMAXINFO :{
            MINMAXINFO* info = (MINMAXINFO*)lParam;
            info->ptMinTrackSize.x = gameState->gameResolution.x;
            info->ptMinTrackSize.y = gameState->gameResolution.y;
            break;
        }
        case WM_KEYDOWN:{
            keyInputs[wParam] = true;
            break;
        }
        case WM_KEYUP:{
            keyInputs[wParam] = false;
            break;
        }
        case WM_LBUTTONDOWN :{
            mouseInputs[MOUSE_BUTTON_LEFT] = true;
            break;
        }     
        case WM_LBUTTONUP :{
            mouseInputs[MOUSE_BUTTON_LEFT] = false;
            break;  
        }
        case WM_MBUTTONDOWN :{
            mouseInputs[MOUSE_BUTTON_MIDDLE] = true;
            break;
        }     
        case WM_MBUTTONUP :{
            mouseInputs[MOUSE_BUTTON_MIDDLE] = false;
            break;  
        }       
        case WM_RBUTTONDOWN :{
            mouseInputs[MOUSE_BUTTON_RIGHT] = true;
            break;
        }     
        case WM_RBUTTONUP :{
            mouseInputs[MOUSE_BUTTON_RIGHT] = false;
            break;  
        }case WM_MOUSEWHEEL:{
            gameState->mouseScrollDelta = (s16)HIWORD(wParam) / WHEEL_DELTA;
            break;
        }         
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);    
}

int WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR argv, int argc){
    checkError(XAudio2Create(&XAudio2Pointer, 0, XAUDIO2_DEFAULT_PROCESSOR), "Could not initialize XAudio2");
    CoInitialize(0);
    checkError(XAudio2Pointer->CreateMasteringVoice(&XAudio2MasterVoice), "Could not create mastering voice");

    tempStorageBuffer = (u8*)VirtualAlloc(0, MEGABYTE(32), MEM_COMMIT, PAGE_READWRITE);
    longTermStorageBuffer = (u8*)VirtualAlloc(0, GIGABYTE(1), MEM_COMMIT, PAGE_READWRITE);
    gameState = (GameState*)VirtualAlloc(0, sizeof(GameState), MEM_COMMIT, PAGE_READWRITE);

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = "Saguinus";
    wc.hCursor = LoadCursorA(0, IDC_ARROW);

    s32 screenWidth = GetSystemMetrics(SM_CXSCREEN);
    s32 screenHeight = GetSystemMetrics(SM_CYSCREEN);
    s32 sx = (screenWidth >> 1) - halfWindowWidth;
    s32 sy = (screenHeight >> 1) - halfWindowHeight;

    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
                               sx, sy, windowWidth, windowHeight, 0, 0, GetModuleHandle(0), 0);

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
    depthStenDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

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

    initializeCanvasRenderer();
    initializeTexturedMeshRenderer();
    initializeDebugRenderer();
    initializeSkyboxRenderer();

    //ShowCursor(false);
    ShowWindow(hwnd, SW_SHOW);
    bool isRunning = true;
  
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++){
         if(XInputGetState(i, &gamepad1.state) == ERROR_SUCCESS){
            gamepad1.index = i;
            break;
        }
    }

    if(!CopyFile("libsaguinus.dll", "libsaguinus_copy.dll", false)) return 1;
    HMODULE dllPtr = LoadLibrary("libsaguinus_copy.dll");
    if(dllPtr == 0){
        MessageBox(0, "Could not load library libsaguinus.dll", "ERROR", 0);
        return 1;
    }
    typedef void (*ugsptr)(GameState*);
    ugsptr updateGS = 0;
    updateGS = (ugsptr)GetProcAddress(dllPtr, "updateGameState");
    if(updateGS == 0){
        MessageBox(0, "Could not get address to updateGameState", "ERROR", 0);
        return 1;
    }

    gameState->windowDimenstion = Vector2(windowWidth, windowHeight);
    gameState->osFunctions.readFileIntoBuffer = &readFileIntoBuffer;
    gameState->osFunctions.writeToFile = &writeToFile;
    gameState->osFunctions.createAnimatedMesh = &createAnimatedMesh;
    gameState->osFunctions.createTexture2DFromFile = &createTexture2D;
    gameState->osFunctions.createTexture2DFromData = &createTexture2D;
    gameState->osFunctions.createTexturedMesh = &createTexturedMesh;
    gameState->osFunctions.allocateMemory = &allocateMemory;
    gameState->osFunctions.createAudioEmitter = &createAudioEmitter;
    gameState->osFunctions.updateAudioEmitterDynamics = &updateAudioEmitterDynamics;
    gameState->osFunctions.playAudioEmitter = &playAudioEmitter;
    gameState->osFunctions.setMasterAudioVolume = &setMasterAudioVolume;
    gameState->osFunctions.createMeshAnimation = &createMeshAnimation;
    gameState->osFunctions.renderAnimatedMesh = &renderAnimatedMesh;
    gameState->keyInputs = keyInputs;
    gameState->mouseInputs = mouseInputs;
    gameState->currentFont = &debugFont;
    gameState->storage.tempMemoryBuffer = tempStorageBuffer;
    gameState->storage.longTermBuffer = longTermStorageBuffer;
    gameState->storage.longTermBufferPointer = gameState->storage.longTermBuffer;
    initializeKeyCodes(gameState);

    f32 deltaTime = 0;
    LARGE_INTEGER endTime;
    LARGE_INTEGER startTime;
    LARGE_INTEGER performanceFrequency;
    QueryPerformanceFrequency(&performanceFrequency);
    QueryPerformanceCounter(&startTime);
    while(isRunning){
        MSG msg = { };
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(keyInputs[VK_ESCAPE]){
            isRunning = false;
        }

        if(XInputGetState(gamepad1.index, &gamepad1.state) == ERROR_SUCCESS 
        && gamepad1.state.dwPacketNumber != gamepad1.lastPacket){
            gameState->gamepad1.leftStickX = (f32)gamepad1.state.Gamepad.sThumbLX / GAMEPAD_STICK_MAX;
            gameState->gamepad1.leftStickY = (f32)gamepad1.state.Gamepad.sThumbLY / GAMEPAD_STICK_MAX;
            gameState->gamepad1.rightStickX = (f32)gamepad1.state.Gamepad.sThumbRX / GAMEPAD_STICK_MAX;
            gameState->gamepad1.rightStickY = (f32)gamepad1.state.Gamepad.sThumbRY / GAMEPAD_STICK_MAX;
            gameState->gamepad1.leftTrigger = (f32)gamepad1.state.Gamepad.bLeftTrigger / GAMEPAD_TRIGGER_MAX;
            gameState->gamepad1.rightTrigger = (f32)gamepad1.state.Gamepad.bRightTrigger / GAMEPAD_TRIGGER_MAX;

            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_A] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_A;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_B] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_B;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_X] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_X;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_Y] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_LB] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_RB] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_L3] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_R3] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_D_UP] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_D_DONW] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_D_LEFT] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_D_RIGHT] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_START] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_START;
            gameState->gamepad1.buttons[gameState->inputCodes.GAMEPAD_BACK] = gamepad1.state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
        }

        if(keyPressedOnce(gameState, gameState->inputCodes.KEY_P)){
            if(!FreeLibrary(dllPtr)) return 1;
            if(!CopyFile("libsaguinus.dll", "libsaguinus_copy.dll", false)) return 1;
            dllPtr = LoadLibrary("libsaguinus_copy.dll");
            if(dllPtr == 0){
                MessageBox(0, "Could not load library libsaguinus.dll", "ERROR", 0);
                return 1;
            }
        
            updateGS = 0;
            updateGS = (ugsptr)GetProcAddress(dllPtr, "updateGameState");
            if(updateGS == 0){
                MessageBox(0, "Could not get address to updateGameState", "ERROR", 0);
                return 1;
            }
            gameState->isInitialized = false;
        }
        
        updateGS(gameState);

        d3d11Context->ClearRenderTargetView(renderTargetView, gameState->clearColor.v);
        d3d11Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
        
        //rendering is done here
        renderSkybox(&gameState->camera);
        renderTexturedMeshBuffer(&gameState->txtdMeshBuffer, &gameState->camera, &gameState->light);
        renderDebugBuffer(&gameState->debugBuffer, &gameState->camera);
        //renderTextBuffer(&gameState->textBuffer);
        renderCanvasBuffer(&gameState->canvasBuffer);
        
        swapChain->Present(1, 0);

        QueryPerformanceCounter(&endTime);
        gameState->deltaTime = (f32)(endTime.QuadPart - startTime.QuadPart) / (f32)performanceFrequency.QuadPart;
        startTime = endTime;
    }

    return 0;
}