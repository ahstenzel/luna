Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

struct Input 
{
    float2 TexCoord : TEXCOORD0;
    float4 Color : TEXCOORD1;
    float4 Position : SV_Position;
};

struct Output
{
    float4 Color : SV_Target0;
    float Depth : SV_Depth;
};

Output main(Input input) {
    Output result;
    result.Color = input.Color * Texture.Sample(Sampler, input.TexCoord);
    if (result.Color.a == 0.0f)
        discard;
    result.Depth = input.Position.z;
    return result;
}