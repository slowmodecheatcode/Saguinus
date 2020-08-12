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

float linearize(float f, float np, float fp){
    float d = f * 2.0 - 1.0;
    return (2.0 * np * fp) / (fp + np - d * (fp - np));
}

float4 pixelMain(PixelInput input) : SV_TARGET {
    float4 color = tex.Sample(texSampler, input.uvCoordinates);

    float m = input.text;
    float mm = 1 - m;

    return float4(quadColor.rgb * max(m, color.rgb), quadColor.a * color[(int)(mm * 3)]);
}

