cbuffer PerObject : register(b0)
{
    row_major float4x4 world;
    row_major float4x4 worldViewProj;
    float3 lightDir;
    float  ambient;
    float3 color;
    float  useTexture;
};

Texture2D    tex : register(t0);
SamplerState samp : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float2 uv       : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    float4 p = float4(input.position, 1.0f);

    PSInput output;
    output.position = mul(p, worldViewProj);
    output.worldPos = mul(p, world).xyz;
    output.uv       = input.uv;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // 隣のピクセルとのワールド座標の差から、面の法線を求める
    float3 dx = ddx(input.worldPos);
    float3 dy = ddy(input.worldPos);
    float3 normal = normalize(cross(dx, dy));

    // ソフトウェア版と同じLambert + 環境光
    float ndotl = saturate(dot(normal, -lightDir));
    float intensity = ambient + (1.0f - ambient) * ndotl;

    // テクスチャがなければ単色
    float3 basecolor = color;
    if (useTexture > 0.5f) {
        float2 uv = float2(input.uv.x, 1.0f - input.uv.y);
        basecolor = tex.Sample(samp, uv).rgb;
    }

    return float4(basecolor* intensity, 1.0f);
}