Texture2D m_colorMap : register(t0);
SamplerState m_colorSampler : register(s0);

struct PixelShaderInput
{
  float4 pos : SV_POSITION;
  float2 tex0: TEXCOORD0;
  float4 norm : NORMAL0;
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

float4 main( PixelShaderInput input ) : SV_TARGET
{
    float3 lightDir = normalize(float3(lightPos.xyz) - float3(input.pos.xyz));
    float3 n = normalize(float3(input.norm.x, input.norm.y, input.norm.z));
    float3 e = -normalize(float3(input.pos.x, input.pos.y, input.pos.z));

    float3 amb = float3(ambient.x, ambient.y, ambient.z);
    float3 dif = float3(diffuse.x, diffuse.y, diffuse.z);
    float3 spec = float3(specular.x, specular.y, specular.z);

    float3 color = phong(n, e, lightDir, amb, dif, spec, shininess);

    return float4(color, 1.0f);
}

float4 texMain( PixelShaderInput input ) : SV_TARGET
{
    float3 lightDir = float3(0.0f, 0.0f, -1.0f);
    float3 n = normalize(float3(input.norm.x, input.norm.y, input.norm.z));
    float3 e = -normalize(float3(input.pos.x, input.pos.y, input.pos.z));

    float3 amb = float3(ambient.x, ambient.y, ambient.z);
    float4 texColor = m_colorMap.Sample(m_colorSampler, input.tex0);
    float3 dif = float3(texColor.x, texColor.y, texColor.z);
    float3 spec = float3(specular.x, specular.y, specular.z);

    float3 color = phong(n, e, lightDir, amb, dif, spec, shininess);

    return float4(color, 1.0f);
}