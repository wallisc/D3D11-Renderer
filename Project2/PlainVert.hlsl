struct VertexShaderInput
{
  float4 pos : POSITION;
  float2 tex0 : TEXCOORD0;
  float4 norm : NORMAL0;
};

cbuffer ConstBuffer
{
   float4x4 mvpMat;
};

cbuffer Lights 
{
   float4 lightPos;
};

// TODO: (msft-Chris) Revisit this struct and slim out variables that aren't needed
struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float3 worldPos : POSITIONT;
  float2 tex0: TEXCOORD0;
  float4 norm : NORMAL0;
};

PixelShaderInput main( VertexShaderInput input )
{
    PixelShaderInput output;
    output.pos = mul(mvpMat, input.pos);
    output.worldPos = input.pos.xyz;
    output.norm = input.norm;

    output.tex0 = input.tex0;
    return output;
}