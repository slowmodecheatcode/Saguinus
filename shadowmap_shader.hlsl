struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uvCoordinates : UVCOORD;
    float4x4 instanceMatrix : INSTANCE_MATRIX;
};

struct VertexOutput {
    float4 position : SV_POSITION;
};

cbuffer VertexConstants {
    float4x4 modelMatrix;
    float4x4 cameraMatrix;
};

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    float4 pos = float4(input.position, 1.0f);

    output.position = mul(cameraMatrix, mul(modelMatrix, float4(input.position, 1)));

    return output;
}

VertexOutput vertexInstanceMain(VertexInput input){
    VertexOutput output;
    float4 pos = float4(input.position, 1.0f);

    output.position = mul(cameraMatrix, mul(input.instanceMatrix, float4(input.position, 1)));

    return output;
}

void pixelMain(VertexOutput input){}