struct VertexInput {
    float2 position : POSITION;
    float2 uvCoordinates : UVCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 projectionMatrix;
};

cbuffer PixelConstants {
    float3 color;
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
    output.position = mul(projectionMatrix, float4(input.position, 0, 1));
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    float4 color = tex.Sample(texSampler, input.uvCoordinates);
    return color;
}

