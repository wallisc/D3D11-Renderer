Texture2D m_colorMap : register(t0);
Texture2D m_shadowMap : register(t1);

RWTexture3D<float4> m_colorBuffer : register(u3);
RWTexture2D<uint> m_colorBufferCounter : register(u4);

SamplerState m_colorSampler : register(s0);
SamplerState m_shadowSampler : register(s1);


struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float3 worldPos : POSITIONT;
  float2 tex0 : TEXCOORD0;
  float4 norm : NORMAL0;
  float4 lPos : TEXCOORD1;
};

struct PixelShaderOutput
{
  float4 color : COLOR0;
  float4 pos : COLOR1;
  float4 norm : COLOR3;

};

cbuffer Material : register(b0)
{
   float4 ambient;
   float4 diffuse;
   float4 specular;
   float shininess;
};

cbuffer Lights : register(b1)
{
   float4 lightDir;
   float4x4 lightMvp;
};

#define MAX_DEPTH 8
// TODO: Pass in via constant buffer
#define LIGHT_POWER 0.5
#define SHADOW_SAMPLES_SQRT 1
#define SHADOW_SAMPLES (SHADOW_SAMPLES_SQRT * SHADOW_SAMPLES_SQRT)


float3 phong( float3 norm, float3 eye, float3 lightDir, float3 amb, float3 dif, float3 spec, float shininess)
{
    float3 h = normalize(2.0 * norm - lightDir);
    float3 color = amb; 
    float nDotL = dot(norm, lightDir);

    if (nDotL > 0.0) 
    {
        //color += spec * pow(clamp(dot(eye, h), 0.0, 1.0) , shininess);
        color += dif * nDotL;
    }
    return color;
}

float4 main( PixelShaderInput input ) : SV_TARGET
{
    float3 n = normalize(input.norm.xyz);
    float3 e = -normalize(input.pos.xyz);

    float4 texColor = m_colorMap.Sample(m_colorSampler, input.tex0);
    float3 dif = texColor.xyz;
    float3 spec = specular.xyz;
    float3 amb = ambient.xyz;
    
    float3 color = phong(n, e, lightDir, amb, dif, spec, shininess);
 
    return float4(color, 1.0f);
#if 0
    PixelShaderOutput output;
    float3 n = normalize(float3(input.norm.xyz));
    float3 e = -normalize(float3(input.pos.xyz));

    float3 amb = float3(ambient.xyz);
    float3 dif = float3(diffuse.xyz);
    float3 spec = float3(specular.xyz);

    float3 color = float3(0, 0, 0);
    color = phong(n, e, lightDir, amb, dif, spec, shininess);

    color = color * LIGHT_POWER;

    // Save the color into the color buffer to be used later for SSGI
    int2 uavCoord = int2(int(input.pos.x), int(input.pos.y));
    // Race condition here
    int depth = m_colorBufferCounter[uavCoord] % MAX_DEPTH;
    m_colorBuffer[int3(uavCoord.xy, depth)] 
       = float4(color, input.pos.z);
    m_colorBufferCounter[uavCoord.xy].r++;

    output.color = float4(color, 1.0f);
    output.pos = float4(input.worldPos.xyz, 1.0f);
    float3 normalizedN = normalize(input.norm.xyz);
    output.norm = float4(normalizedN, 0.0f);
    
    return output;
#endif
}

float4 texMain( PixelShaderInput input ) : SV_TARGET
{
    float3 n = normalize(input.norm.xyz);
    float3 e = -normalize(input.pos.xyz);

    float4 texColor = m_colorMap.Sample(m_colorSampler, input.tex0);
    float3 dif = texColor.xyz;
    float3 spec = specular.xyz;
    float3 amb = dif.xyz * .2f;
    
    float depthValue = 1.0;
    input.lPos.xyz /= input.lPos.w;
    input.lPos.x = input.lPos.x / 2.0 + 0.5;
    input.lPos.y = input.lPos.y / -2.0 + 0.5;
    
    float3 color;
    int samplesShadowed = 0;
    for (int x = -(SHADOW_SAMPLES_SQRT / 2); x < SHADOW_SAMPLES_SQRT / 2 + 1; x++)
    {
      for (int y = -(SHADOW_SAMPLES_SQRT / 2); y < SHADOW_SAMPLES_SQRT / 2 + 1; y++)
      {
         float3 dx = float3( float(x) / 1024.0, float(y) / 768.0, 0.0);
         float3 samplePt = input.lPos.xyz + dx;

         // TODO: Replace this with a sampler
         if (samplePt.x > 0 && samplePt.x < 1 && samplePt.y > 0 && samplePt.y < 1
            && samplePt.z > 0.1 && samplePt.z < 1)
         {
            depthValue = m_shadowMap.Sample(m_colorSampler, samplePt.xy).r;
            if ( input.lPos.z - 0.000001 > depthValue )
            {
              samplesShadowed++;
            }
         }
      }
    }

    if (samplesShadowed < SHADOW_SAMPLES)
    {
       float shadowFactor = float(samplesShadowed) / float(SHADOW_SAMPLES);
       color = lerp(phong(n, e, lightDir, amb, dif, spec, shininess), amb, shadowFactor);
    }
    else
    {
       color = amb;
    }

    return float4(color, 1.0f);

}

