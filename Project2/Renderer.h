#ifndef _BLANK_DEMO_H_
#define _BLANK_DEMO_H_

#include "D3DBase.h"
#include "Mesh.h"
#include "Material.h"


#include "ShadowMap.h"
#include "ConstantBuffer.h"
#include "RWRenderTarget.h"
#include "RWComputeSurface.h"
#include "RWStructuredBuffer.h"
#include "PlaneRenderer.h"
#include "Camera.h"

#include <assimp/scene.h>           // Output data structure

#include <xnamath.h>
#include <vector>
#include <map>

#define MAX_COLOR_BUFFER_DEPTH 8

__declspec(align(16))
struct PS_Point_Light
{
   XMFLOAT4 position;
   XMFLOAT4 color;
};

struct VS_Transformation_Constant_Buffer 
{
   XMMATRIX mvp;
};

__declspec(align(16))
struct PS_Light_Constant_Buffer
{
   XMFLOAT4 direction;
   XMMATRIX mvp;
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
   bool InitializeMatMap(const aiScene *pAssimpScene);
   void DestroyMatMap();

   bool CreateD3DMesh(const aiMesh *pMesh, const aiScene *pAssimpScene, Mesh *d3dMesh);

   void DestroyD3DMesh(Mesh *d3dMesh);

   UINT m_shadowMapHeight;
   UINT m_shadowMapWidth;

   D3D11_VIEWPORT m_viewport;

   ID3D11VertexShader* m_solidColorVS;
   ID3D11VertexShader* m_planeVS;

   ID3D11PixelShader* m_globalIlluminationPS;
   ID3D11PixelShader* m_solidColorPS;
   ID3D11PixelShader* m_texturePS;
   ID3D11PixelShader* m_blurPS;
   ID3D11PixelShader* m_textureNoShadingPS;


   ID3D11InputLayout* m_inputLayout;

   ConstantBuffer<VS_Transformation_Constant_Buffer> *m_pTransformConstants;
   ConstantBuffer<PS_Light_Constant_Buffer> *m_pLightConstants;

   RWRenderTarget* m_pBlurredShadowMap;
   RWRenderTarget* m_pLightMap;

   RWStructuredBuffer<PS_Point_Light> *m_pLightBuffer;

   ID3D11ComputeShader* m_blurCS;
   RWComputeSurface* m_pBlurredShadowSurface;

   PlaneRenderer* m_pPlaneRenderer;

   ID3D11SamplerState* m_colorMapSampler;
   ID3D11SamplerState* m_shadowSampler;

   ID3D11RasterizerState* m_rasterState;
   ID3D11UnorderedAccessView* m_uav;
   ID3D11UnorderedAccessView* m_colorBufferDepthUAV;
   ID3D11ShaderResourceView *m_colorBufferSrv;

   std::vector<Mesh> scene;
   std::vector<Material> m_matList;

   ShadowMap *m_pShadowMap;
   VS_Transformation_Constant_Buffer m_shadowMapTransform;

   // At some point these should be encapsulated into a Mesh Object 
   VS_Transformation_Constant_Buffer m_vsTransConstBuf;
   VS_Transformation_Constant_Buffer m_vsLightTransConstBuf;

   PS_Material_Constant_Buffer m_psMaterialConstBuf;
   PS_Light_Constant_Buffer m_psLightConstBuf;

   XMMATRIX m_camViewTrans;

   XMFLOAT4 m_lightDirection;
   XMFLOAT4 m_lightUp;

   Camera *m_pCamera;


   float m_cameraUnit;

   // TODO: Encapsulate in a perspective object
   float m_nearPlane;
   float m_farPlane;
   float m_fieldOfView; // vertical FOV in radians

   UINT m_numIndices;
};

#endif //_BLANK_DEMO_H_