Texture3D<float4> m_colorBuffer : register(t1);
RWTexture2D<uint> m_colorBufferCount : register(u1);

// TODO: Pass this in via a constant buffer
#define MAX_DEPTH 8

struct VertexOutput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

float4 main(VertexOutput input) : SV_TARGET
{
   int3 coord = int3(input.tex.x * 1024, input.tex.y * 768, 0);
   uint pixelDepth = m_colorBufferCount[coord.xy].r;
   float4 color = float4(0, 0, 0, 0);
   
   for (uint i = 0; i < MAX_DEPTH; i++)
   {
      // Used to avoid branching. Makes sure entries above pixelDepth are ignored
      int validationMultiplier = i < pixelDepth; 

      coord.z = i;
      color += validationMultiplier * m_colorBuffer[coord] / float(pixelDepth);
   }

   color.w = 1.0f;
	return color;
}