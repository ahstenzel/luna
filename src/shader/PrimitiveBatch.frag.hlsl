struct Input 
{
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

struct Output
{
    float4 Color : SV_Target0;
    float Depth : SV_Depth;
};

Output main(Input input) {
    Output result;
    result.Color = input.Color;
    if (result.Color.a == 0.0f)
        discard;
    result.Depth = input.Position.z;
    return result;
}