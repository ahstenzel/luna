struct Input 
{
    float3 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

struct Output 
{
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

cbuffer UniformBlock : register(b0, space1) 
{
    float4x4 ViewProjectionMatrix : packoffset(c0);
};

Output main(in Input input)
{
    Output output;
    output.Color = input.Color;
    output.Position = mul(ViewProjectionMatrix, float4(input.Position, 1.0f));
    return output;
}