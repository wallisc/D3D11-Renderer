#ifndef _BLANK_DEMO_H_
#define _BLANK_DEMO_H_

#include "D3DBase.h"
#include "Mesh.h"
#include "ObjReader.h"
#include <xnamath.h>
#include <vector>

struct VS_Transformation_Constant_Buffer 
{
   XMMATRIX world;
   XMMATRIX view;
   XMMATRIX perspective;
};

__declspec(align(16))
struct PS_Light_Constant_Buffer
{
   XMFLOAT4 position;
};

__declspec(align(16))
struct PS_Material_Constant_Buffer
{
   XMFLOAT4 ambient;
   XMFLOAT4 diffuse;
   XMFLOAT4 specular;
   FLOAT shininess;
};

class Renderer : public D3DBase
{
public:
   Renderer();
   void Update(FLOAT dt, BOOL *keyInputArray);
   void Render();
   bool LoadContent();
   void UnloadContent();

private:
   bool CreateD3DMesh(ObjReader::Mesh *mesh, ObjReader::Material *mat, Mesh *d3dMesh);
   void DestroyD3DMesh(Mesh *d3dMesh);

   ID3D11VertexShader* m_solidColorVS;
   ID3D11PixelShader* m_solidColorPS;
   ID3D11PixelShader* m_texturePS;
   ID3D11InputLayout* m_inputLayout;
   ID3D11Buffer* m_vsConstantBuffer;
   ID3D11Buffer* m_psConstantBuffer;
   ID3D11SamplerState* m_colorMapSampler;
   ID3D11RasterizerState* m_rasterState;

   std::vector<Mesh> scene;

   // At some point these should be encapsulated into a Mesh Object 
   VS_Transformation_Constant_Buffer m_vsTransConstBuf;
   PS_Material_Constant_Buffer m_psMaterialConstBuf;
   PS_Light_Constant_Buffer m_psLightConstBuf;

   XMMATRIX m_normalWorldTrans;
   XMFLOAT4 m_lightPosition;
   float m_camX, m_camY, m_camZ;
   float m_camYAngle;
   UINT m_numIndices;
};

#endif //_BLANK_DEMO_H_