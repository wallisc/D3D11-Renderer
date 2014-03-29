Texture2D<float4> m_Source : register(t1);
Texture2D<float4> m_BlurLayer : register(t2);

#define BLUR_BLOCK_LENGTH 4
#define NUM_SAMPLES BLUR_BLOCK_LENGTH * BLUR_BLOCK_LENGTH

struct VertexOutput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

float4 main(VertexOutput input) : SV_TARGET
{
   int2 coord = int2(input.tex.x * 1024, input.tex.y * 741);
   float4 blurredColor = float4(0.0, 0.0, 0.0, 0.0);

   for(int x = -(BLUR_BLOCK_LENGTH / 2); x < BLUR_BLOCK_LENGTH / 2 + 1; x++)
   {
      for(int y = -(BLUR_BLOCK_LENGTH / 2); y < BLUR_BLOCK_LENGTH / 2 + 1; y++)
      {
         // TODO: Use a sampler
         int2 blurCoord = coord + int2(x, y);
         if (blurCoord.x > 0 && blurCoord.y > 0 && blurCoord.x < 1024 && blurCoord.y < 768)
         {
            blurredColor += m_BlurLayer[blurCoord];
         }
      }
   }
   blurredColor /= float(NUM_SAMPLES);
   float occlusionValue = blurredColor.w;

	return occlusionValue * (m_Source[coord] + blurredColor);
}