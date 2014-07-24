#include "ShaderDefines.h"

struct PointLight
{
   float4 pos;
   float4 posView;
   float4 col;
   float4 screenPos;
   float4  screenRad;
};

Texture2D m_ShadowMap : register(t0);
Texture2D m_ColorMap : register(t1);
Texture2D m_ShadowWorldPosMap : register(t2);


RWTexture2D<float4>  m_BlurredMap;
AppendStructuredBuffer<PointLight> m_LightBuffer;

cbuffer Lights : register(b0)
{
   float4 lightDir;
   float4x4 lightMvp;
   float4x4 lightInvMvp;
};

cbuffer ConstBuffer : register(b1)
{
   float4x4 mvpMat;
   float4x4 invProj;
   float4x4 mvMat;
   float4x4 proj;
};

#define BLUR_BLOCK_LENGTH 1
#define NUM_SAMPLES BLUR_BLOCK_LENGTH * BLUR_BLOCK_LENGTH

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
   if(DTid.x % TILE_WIDTH == 0 && DTid.y % TILE_HEIGHT == 0)
   {
      PointLight p;
      p.pos = m_ShadowWorldPosMap[DTid.xy];
      p.posView = mul(mvMat, p.pos);
      p.col = m_ColorMap[DTid.xy];
      
      p.screenPos = mul(mvpMat, p.pos);
      float4 offsetPos = p.posView + float4(MAX_LIGHT_RADIUS, 0, 0, 0);
      float4 screenOffset = mul(proj, offsetPos);
      p.screenRad = float4(0, 0, 0, 0);
      p.screenRad.x = abs(length(p.screenPos.xyz - screenOffset.xyz));
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