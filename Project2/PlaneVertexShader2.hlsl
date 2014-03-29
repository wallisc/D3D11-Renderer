struct VertexInput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

struct VertexOutput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

VertexOutput main( VertexInput input ) 
{
   VertexOutput output;
   output.tex = input.tex;
   output.pos = input.pos;
   return output;
}