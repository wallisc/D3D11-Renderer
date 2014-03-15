struct VertexShaderInput
{
  float4 pos : POSITION;
  float2 tex0 : TEXCOORD0;
  float4 norm : NORMAL0;
};

cbuffer ConstBuffer
{
   float4x4 worldMat;
   float4x4 viewMat;
   float4x4 perspectiveMat;
};

struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float2 tex0: TEXCOORD0;
  float4 norm : NORMAL0;
};

PixelShaderInput main( VertexShaderInput input )
{
    PixelShaderInput output;
    output.pos = mul(perspectiveMat, mul(worldMat, mul(viewMat, input.pos)));
    output.norm = mul(perspectiveMat, mul(worldMat, mul(viewMat, input.norm)));
    output.tex0 = input.tex0;
    return output;
}