struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uvCoordinates : UVCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 transformMatrix;
};

struct PixelInput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.uvCoordinates = input.uvCoordinates;
    output.position = mul(transformMatrix, float4(input.position, 1));
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    float4 color = tex.Sample(texSampler, input.uvCoordinates);
    return color;//float4(0.8, 0.5, 0.2, 1.0);
}

