SamplerState m_colorSampler : register(s0);

Texture3D<float4> m_colorBuffer : register(t1);
Texture2D<float4> m_firstPassColor : register(t2);
Texture2D<float4> m_firstPassNormals : register(t3);

RWTexture2D<uint> m_colorBufferCount : register(u1);

// TODO: Pass this in via a constant buffer
#define MAX_DEPTH 8
#define INDIRECT_LIGHT_CONTRIBUTION 0.5f
#define OCCLUSION_CONTRIBUTION 1.0f
#define NUM_SAMPLES_SQRT 4
#define NUM_SAMPLES (NUM_SAMPLES_SQRT * NUM_SAMPLES_SQRT)
#define SAMPLE_DISTANCE 4.0
#define WIDTH 1024
#define HEIGHT 768
struct VertexOutput
{
   float4 pos : SV_POSITION;
   float2 tex : TEXCOORD0;
};

float rand(in float2 uv)
{
    float2 noise = (frac(sin(dot(uv ,float2(12.9898,78.233)*2.0)) * 43758.5453));
    return abs(noise.x + noise.y) * 0.5f;
}

float4 main(VertexOutput input) : SV_TARGET
{
   float3 coord = float3(input.tex.x * WIDTH, input.tex.y * HEIGHT, 0);
   float4 norm = m_firstPassNormals.Sample(m_colorSampler, input.tex);
   
   uint numSamples = 0;
   float4 origin =  m_firstPassColor.Sample(m_colorSampler, input.tex);

   for( int xSample = -NUM_SAMPLES_SQRT / 2; xSample < NUM_SAMPLES_SQRT / 2 + 1; xSample++ )
   {
      for( int ySample = -NUM_SAMPLES_SQRT / 2; ySample < NUM_SAMPLES_SQRT / 2 + 1; ySample++ )
      {
         float2 intersectCoord = input.tex + SAMPLE_DISTANCE * float2(xSample, ySample) / float2(WIDTH, HEIGHT);
         if (xSample == ySample && xSample == 0) continue;
         // Ambient occlusion work
         float4 testPoint = m_firstPassColor.Sample(m_colorSampler, intersectCoord);

         float3 testPointDir = normalize(testPoint.xyz - origin.xyz);

         float nDotT = dot(norm.xyz, testPointDir);
         if (nDotT > 0.2 && abs(testPoint.z - origin.z) < .1)
         {
            numSamples++;
         }
      }
   }

   float4 color;
   color.w = 1.0f - (float(numSamples) / float(NUM_SAMPLES)) * OCCLUSION_CONTRIBUTION;
   color.xyz = float3(0, 0, 0);
   return color;
}

//float4 main(VertexOutput input) : SV_TARGET
//{
//   float3 coord = float3(input.tex.x * WIDTH, input.tex.y * HEIGHT, 0);
//   uint pixelDepth = m_colorBufferCount[coord.xy].r;
//   //for (uint i = 0; i < MAX_DEPTH; i++)
//   //{
//   //   // Used to avoid branching. Makes sure entries above pixelDepth are ignored
//   //   int validationMultiplier = i < pixelDepth; 
//   //
//   //   coord.z = i;
//   //   color += validationMultiplier * m_colorBuffer[coord] / float(pixelDepth);
//   //}
//   //
//   //color.w = 1.0f;
//
//	float4 color = float4(0.0, 0.0, 0.0, 1.0);
//   float4 norm = decompressNormal(m_firstPassNormals[coord.xy]);
//   
//   uint numCloseBySamples = 0;
//   float3 origin = float3(input.tex.x, input.tex.y, m_firstPassColor[coord.xy].w);
//   float3 testPointDir;
//#if 0
//   for( int xSample = -NUM_SAMPLES_SQRT / 2; xSample < NUM_SAMPLES_SQRT / 2 + 1; xSample++ )
//   {
//      for( int ySample = -NUM_SAMPLES_SQRT / 2; ySample < NUM_SAMPLES_SQRT / 2 + 1; ySample++ )
//      {
//         if (xSample == ySample && xSample == 0) continue;
//#endif
//         int xSample = 3;
//         int ySample = 0;
//
//         // Use a sampler
//         float2 intersectCoord = coord.xy + float2(xSample, ySample);
//         if(intersectCoord.x > 0 && intersectCoord.y > 0 && 
//            intersectCoord.x < WIDTH && intersectCoord.y < HEIGHT)
//         {
//            // Global illumination work
//            color += INDIRECT_LIGHT_CONTRIBUTION * m_firstPassColor[intersectCoord.xy] / float(NUM_SAMPLES);
//
//            // Ambient occlusion work
//            float zValue = m_firstPassColor[intersectCoord.xy].w;
//            float3 testPoint = float3(intersectCoord.x / float(WIDTH), intersectCoord.y / float(HEIGHT), zValue);
//            
//            testPointDir = normalize(testPoint - origin);
//
//            if( dot(norm.xyz, testPointDir) > 0.2f )
//            {
//               numCloseBySamples++;
//            }
//            
//         }
//#if 0
//      }
//   }
//#endif
//   
//   float occlusionValue = 1.0f - (float(numCloseBySamples) / float(NUM_SAMPLES)) * OCCLUSION_CONTRIBUTION;
//
//   color.w = 1.0f;
//   color.xyz = occlusionValue * float3(1, 0, 0);
//   return color;
//}