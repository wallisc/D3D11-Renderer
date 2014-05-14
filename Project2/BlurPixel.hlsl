Texture2D<float4> m_Source : register(t0);

SamplerState m_colorSampler : register(s0);
SamplerState m_shadowSampler : register(s1);

#define BLUR_BLOCK_LENGTH 1
#define NUM_SAMPLES BLUR_BLOCK_LENGTH * BLUR_BLOCK_LENGTH
#define WIDTH 1024 * 2
#define HEIGHT 768 * 2

struct VertexOutput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

float4 main(VertexOutput input) : SV_TARGET
{
   float4 blurredColor = float4(0.0, 0.0, 0.0, 0.0);

   for(int x = -(BLUR_BLOCK_LENGTH / 2); x < BLUR_BLOCK_LENGTH / 2 + 1; x++)
   {
      for(int y = -(BLUR_BLOCK_LENGTH / 2); y < BLUR_BLOCK_LENGTH / 2 + 1; y++)
      {
         float2 blurCoord = input.tex + float2(float(x) / float(WIDTH), float(y) / float(HEIGHT));
         blurredColor += m_Source.Sample(m_shadowSampler, blurCoord) / float(NUM_SAMPLES);
      }
   }

	return blurredColor;
}