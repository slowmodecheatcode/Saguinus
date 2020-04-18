struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uvCoordinates : UVCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 fragmentPostion : FRAG_POS;
    float3 fragmentNormal : FRAG_NORM;
    float2 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 modelMatrix;
    float4x4 cameraMatrix;
};

struct PixelInput {
    float4 position : SV_POSITION;
    float3 fragmentPostion : FRAG_POS;
    float3 fragmentNormal : FRAG_NORM;
    float2 uvCoordinates : UVCOORD;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.uvCoordinates = input.uvCoordinates;
    output.fragmentPostion = float4(mul(modelMatrix, float4(input.position, 1))).xyz;
    output.fragmentNormal = float4(mul(modelMatrix, float4(input.normal, 0))).xyz;
    output.position = mul(cameraMatrix, mul(modelMatrix, float4(input.position, 1)));
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    float3 ambient = float3(0.2, 0.2, 0.2);
    float3 lightPosition = float3(1, 5, 2);

    float3 lightDirection = normalize(lightPosition - input.fragmentPostion);
    float3 diffuse = max(dot(lightDirection, input.fragmentNormal), 0);

    float4 color = tex.Sample(texSampler, input.uvCoordinates);
    return float4(ambient + diffuse, 1.0) * color;
}

