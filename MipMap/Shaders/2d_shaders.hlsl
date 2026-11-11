struct VSInput
{
    float2 position : POSITION;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

PSInput VSMain(VSInput input)
{
    PSInput output;

    output.position = float4(input.position, 0.0f, 1.0f);
    output.color = input.color;
    output.uv = input.uv;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color * gTexture.Sample(gSampler, input.uv);
}