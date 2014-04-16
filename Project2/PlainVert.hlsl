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
   float4x4 lightMvp;
};

// TODO: (msft-Chris) Revisit this struct and slim out variables that aren't needed
struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float3 worldPos : POSITIONT;
  float2 tex0: TEXCOORD0;
  float4 norm : NORMAL0;
  float4 lpos: TEXCOORD1;
};

PixelShaderInput main( VertexShaderInput input )
{
    PixelShaderInput output;
    output.pos = mul(mvpMat, input.pos);

    // This code will need to change once the model matrix is used
    output.worldPos = input.pos.xyz;
    output.norm = input.norm;
    output.lpos = mul(lightMvp, input.pos);

    output.tex0 = input.tex0;
    return output;
}