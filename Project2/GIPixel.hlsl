SamplerState m_colorSampler : register(s0);

Texture3D<float4> m_colorBuffer : register(t1);
Texture2D<float4> m_pFirstPassPositions : register(t2);
Texture2D<float4> m_firstPassNormals : register(t3);
Texture2D<float4> m_firstPassColors : register(t4);

RWTexture2D<uint> m_colorBufferCount : register(u1);

// TODO: Pass this in via a constant buffer
#define MAX_DEPTH 1
#define INDIRECT_LIGHT_CONTRIBUTION 1.f
#define OCCLUSION_CONTRIBUTION 1.0f
#define NUM_SAMPLES_SQRT 3
#define NUM_SAMPLES (NUM_SAMPLES_SQRT * NUM_SAMPLES_SQRT)
#define SAMPLE_DISTANCE 2.0

#define NUM_GI_SAMPLES_SQRT 3
#define NUM_GI_SAMPLES (NUM_SAMPLES_SQRT * NUM_SAMPLES_SQRT)
#define GI_SAMPLE_DISTANCE 15.0

#define TOTAL_GI_SAMPLES (NUM_GI_SAMPLES + NUM_SAMPLES)

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
   float4 origin =  m_pFirstPassPositions.Sample(m_colorSampler, input.tex);
   float4 color = float4(0, 0, 0,0);
   for( int xSample = -NUM_SAMPLES_SQRT / 2; xSample < NUM_SAMPLES_SQRT / 2 + 1; xSample++ )
   {
      for( int ySample = -NUM_SAMPLES_SQRT / 2; ySample < NUM_SAMPLES_SQRT / 2 + 1; ySample++ )
      {

         float2 intersectCoord = input.tex + SAMPLE_DISTANCE * float2(xSample, ySample) / float2(WIDTH, HEIGHT);
         if (xSample == ySample && xSample == 0) continue;

         // Ambient occlusion and GI work
         float4 testPoint = m_pFirstPassPositions.Sample(m_colorSampler, intersectCoord);

         float3 testPointDir = normalize(testPoint.xyz - origin.xyz);

         float nDotT = dot(norm.xyz, testPointDir);
         if (nDotT > 0.2 && abs(testPoint.z - origin.z) < .1)
         {
            // TODO: Move this math outside of the for loop
            color += m_firstPassColors.Sample(m_colorSampler, intersectCoord) * INDIRECT_LIGHT_CONTRIBUTION / float(TOTAL_GI_SAMPLES);
            numSamples++;
         }
      }
   }

   for( int xSample = -NUM_GI_SAMPLES_SQRT / 2; xSample < NUM_GI_SAMPLES_SQRT / 2 + 1; xSample++ )
   {
      for( int ySample = -NUM_GI_SAMPLES_SQRT / 2; ySample < NUM_GI_SAMPLES_SQRT / 2 + 1; ySample++ )
      {
         float r1 = rand((float2(xSample, ySample) / float2(WIDTH, HEIGHT)) + input.tex);
         float r2 = rand((float2(xSample, ySample) / float2(WIDTH, HEIGHT)) + input.tex);

         float2 intersectCoord = input.tex + GI_SAMPLE_DISTANCE * float2(xSample, ySample) * float2(r1, r2) / float2(WIDTH, HEIGHT);
         if (xSample == ySample && xSample == 0) continue;

         // Ambient occlusion and GI work
         float4 testPoint = m_pFirstPassPositions.Sample(m_colorSampler, intersectCoord);

         float3 testPointDir = normalize(testPoint.xyz - origin.xyz);

         float nDotT = dot(norm.xyz, testPointDir);
         if (nDotT > 0.2 && abs(testPoint.z - origin.z) < .3)
         {
            // TODO: Move this math outside of the for loop
            color += m_firstPassColors.Sample(m_colorSampler, intersectCoord) * INDIRECT_LIGHT_CONTRIBUTION / float(TOTAL_GI_SAMPLES);
         }
      }
   }

   color.w = 1.0f - (float(numSamples) / float(NUM_SAMPLES)) * OCCLUSION_CONTRIBUTION;
   return color;
}