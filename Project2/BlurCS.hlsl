struct PointLight
{
   float4 pos;
   float4 col;
};

Texture2D m_ShadowMap : register(t0);
Texture2D m_ColorMap : register(t1);

RWTexture2D<float4>  m_BlurredMap;
AppendStructuredBuffer<PointLight> m_LightBuffer;


#define BLUR_BLOCK_LENGTH 1
#define NUM_SAMPLES BLUR_BLOCK_LENGTH * BLUR_BLOCK_LENGTH

#define WIDTH 2048
#define HEIGHT (768 * 2)

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{

   if(DTid.x % 64 == 0 && DTid.y % 64 == 0)
   {
      PointLight p;
      p.pos = float4(
         float(DTid.x) * 2 / float(WIDTH), 
         float(DTid.y) * 2 / float(HEIGHT), 
         m_ShadowMap[DTid.xy].r, 
         1);
      p.col = m_ColorMap[DTid.xy] * .2;
      m_LightBuffer.Append(p);
   }

   float4 blurColor = float4(0, 0, 0, 0);
   for( int x = -BLUR_BLOCK_LENGTH / 2; x < BLUR_BLOCK_LENGTH / 2 + 1; x++ )
   {
      for( int y = -BLUR_BLOCK_LENGTH / 2; y < BLUR_BLOCK_LENGTH / 2 + 1; y++ )
      {
         x = clamp(x, 0, WIDTH - 1);
         y = clamp(y, 0, HEIGHT - 1);
         blurColor += m_ShadowMap[DTid.xy + int2(x, y)] / float(NUM_SAMPLES);
      }
   }
   m_BlurredMap[DTid.xy] = blurColor;
}