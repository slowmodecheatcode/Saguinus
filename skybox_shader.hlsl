struct VertexInput {
    float3 position : POSITION;
    float2 uvCoordinates : UVCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 cameraMatrix;
};

// cbuffer PixelConstants {
// };

struct PixelInput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.position = mul(cameraMatrix, float4(input.position, 1));
    output.uvCoordinates = input.uvCoordinates;
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    return tex.Sample(texSampler, input.uvCoordinates);
}

