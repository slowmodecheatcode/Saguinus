struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uvCoordinates : UVCOORD;
    float4x4 instanceMatrix : INSTANCE_MATRIX;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float4 shadowPos: SHADOW_POS;
    float3 fragmentPostion : FRAG_POS;
    float3 fragmentNormal : FRAG_NORM;
    float2 uvCoordinates : UVCOORD;
};

cbuffer VertexConstants {
    float4x4 modelMatrix;
    float4x4 cameraMatrix;
    float4x4 shadowMatrix;
};

cbuffer PixelConstants {
    float3 cameraPosition;
    float3 lightPosition;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

Texture2D tex : register(t0);
Texture2D shadowmap : register(t1);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.uvCoordinates = input.uvCoordinates;
    output.fragmentPostion = float4(mul(modelMatrix, float4(input.position, 1))).xyz;
    output.fragmentNormal = float4(mul(modelMatrix, float4(input.normal, 0))).xyz;
    output.position = mul(cameraMatrix, mul(modelMatrix, float4(input.position, 1)));
    output.shadowPos = mul(shadowMatrix, mul(modelMatrix, float4(input.position, 1)));
    return output;
}

VertexOutput instanceVertexMain(VertexInput input){
    VertexOutput output;
    output.uvCoordinates = input.uvCoordinates;
    output.fragmentPostion = float4(mul(input.instanceMatrix, float4(input.position, 1))).xyz;
    output.fragmentNormal = float4(mul(input.instanceMatrix, float4(input.normal, 0))).xyz;
    output.position = mul(cameraMatrix, mul(input.instanceMatrix, float4(input.position, 1)));
    output.shadowPos = mul(shadowMatrix, mul(input.instanceMatrix, float4(input.position, 1)));
    return output;
}

float4 pixelMain(VertexOutput input) : SV_TARGET {
    float3 lightDirection = normalize(lightPosition - input.fragmentPostion);
    float3 diffuseAmount = max(dot(lightDirection, input.fragmentNormal), 0);
    diffuseAmount *= diffuse;
    float4 color = tex.Sample(texSampler, input.uvCoordinates);

    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.shadowPos.x / input.shadowPos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.shadowPos.y / input.shadowPos.w * 0.5f);
    float pixelDepth = input.shadowPos.z / input.shadowPos.w;

    if ((saturate(shadowTexCoords.x) == shadowTexCoords.x) &&
        (saturate(shadowTexCoords.y) == shadowTexCoords.y) &&
        (pixelDepth > 0)){
            float shadowmapDepth = shadowmap.Sample(texSampler, shadowTexCoords).r;
            if(pixelDepth > shadowmapDepth + 0.000001){
                return float4(ambient, 1.0) * color;
            }else{
                return float4(ambient + diffuseAmount, 1.0) * color;
            }
    }else{
        return float4(ambient + diffuseAmount, 1.0) * color;
    }
}

