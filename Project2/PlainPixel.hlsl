Texture2D m_colorMap : register(t0);
Texture2D m_shadowMap : register(t1);
RWTexture3D<float4> m_colorBuffer : register(u3);
RWTexture2D<uint> m_colorBufferCounter : register(u4);
SamplerState m_colorSampler : register(s0);

#define MAX_DEPTH 8
// TODO: Pass in via constant buffer
#define LIGHT_POWER 0.7

struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float3 worldPos : POSITIONT;
  float2 tex0 : TEXCOORD0;
  float4 norm : NORMAL0;
  float4 transformedNorm : NORMAL1;
  float4 lpos: TEXCOORD1;
};

struct PixelShaderOutput
{
  float4 color : COLOR0;
  float4 pos : COLOR1;
  float4 norm : COLOR3;

};

cbuffer Material 
{
   float4 ambient;
   float4 diffuse;
   float4 specular;
   float shininess;
};

cbuffer Lights 
{
   float4 lightPos;
   float4x4 lightMvp;
};

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

PixelShaderOutput main( PixelShaderInput input ) : SV_TARGET
{
    PixelShaderOutput output;
    float3 lightDir = normalize(lightPos.xyz - float3(input.worldPos.xyz));
    float3 n = normalize(float3(input.norm.xyz));
    float3 e = -normalize(float3(input.pos.xyz));

    float3 amb = float3(ambient.xyz);
    float3 dif = float3(diffuse.xyz);
    float3 spec = float3(specular.xyz);

    input.lpos.xyz /= input.lpos.w;
    input.lpos.x = input.lpos.x / 2.0 + 0.5;
    input.lpos.y = input.lpos.y / -2.0 + 0.5;

    float depthValue = 1.0;
    // TODO: Replace this with a sampler
    if (input.lpos.x > 0 && input.lpos.x < 1 && input.lpos.y > 0 && input.lpos.y < 1
        && input.lpos.z > 0.1 && input.lpos.z < 1)
    {
       depthValue = m_shadowMap.Sample(m_colorSampler, input.lpos.xy).r;
    }

    float3 color = float3(0, 0, 0);
    if ( input.lpos.z - 0.001 <= depthValue )
    {
       color = phong(n, e, lightDir, amb, dif, spec, shininess);
    }
    else
    {
       color = amb;
    }
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
}

float4 mainNoShadow( PixelShaderInput input ) : SV_TARGET
{
    float3 lightDir = normalize(lightPos.xyz - float3(input.worldPos.xyz));
    float3 n = normalize(float3(input.norm.xyz));
    float3 e = -normalize(float3(input.pos.xyz));

    float3 amb = float3(ambient.xyz);
    float3 dif = float3(diffuse.xyz);
    float3 spec = float3(specular.xyz);

    float3 color = phong(n, e, lightDir, amb, dif, spec, shininess);

    return float4(color, 1.0f);
}

float4 texMain( PixelShaderInput input ) : SV_TARGET
{
    float3 lightDir = float3(0.0f, 0.0f, -1.0f);
    float3 n = normalize(input.norm.xyz);
    float3 e = -normalize(input.pos.xyz);

    float3 amb = float3(ambient.xyz);
    float4 texColor = m_colorMap.Sample(m_colorSampler, input.tex0);
    float3 dif = float3(texColor.xyz);
    float3 spec = float3(specular.xyz);

    float3 color = phong(n, e, lightDir, amb, dif, spec, shininess);

    return float4(color, 1.0f);
}