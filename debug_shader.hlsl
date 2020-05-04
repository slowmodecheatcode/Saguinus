struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_POSITION;
};

cbuffer VertexConstants {
    float4x4 projectionMatrix;
};

cbuffer PixelConstants {
    float4 pixelColor;
};

struct PixelInput {
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.position = mul(projectionMatrix, float4(input.position, 1));
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    //return float4(1, 0, 0, 1);
    return pixelColor;
}

