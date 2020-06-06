struct VertexInput {
    float3 position : POSITION;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 cameraMatrix;
};

// cbuffer PixelConstants {
// };

struct PixelInput {
    float4 position : SV_POSITION;
    float3 uvCoordinates : UVCOORD;
};

TextureCube tex : register(t0);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.position = mul(cameraMatrix, float4(input.position, 1)).xyww;
    output.uvCoordinates = input.position;
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    return tex.Sample(texSampler, input.uvCoordinates);
}

