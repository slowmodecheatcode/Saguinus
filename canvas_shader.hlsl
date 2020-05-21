struct VertexInput {
    float2 position : POSITION;
    float2 uvCoordinates : UVCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
    float text : TEXT;
};

cbuffer VertexConstants {
    float4x4 projectionMatrix;
    float depth;
    float isText;
};

cbuffer PixelConstants {
    float4 quadColor;
};

struct PixelInput {
    float4 position : SV_POSITION;
    float2 uvCoordinates : UVCOORD;
    float text : TEXT;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

VertexOutput vertexMain(VertexInput input){
    VertexOutput output;
    output.text = isText;
    output.uvCoordinates = input.uvCoordinates;
    output.position = mul(projectionMatrix, float4(input.position, depth, 1));
    return output;
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    float4 color = tex.Sample(texSampler, input.uvCoordinates);

    float m = input.text;
    if(m > 0.5){
        return float4(quadColor.r, quadColor.g, quadColor.b, quadColor.a * color.r);
    }else{
        return color * quadColor;
    }
}

