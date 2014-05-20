#include "Renderer.h"
#include "D3DUtils.h"

#include <cassert>
#include <string>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/postprocess.h>     // Post processing flags

using std::vector;
using std::string;

const XMFLOAT4 LIGHT_DIRECTION(0.0f, 1.0f, 0.0f, 0.0f);
const XMFLOAT4 LIGHT_UP(0.0f, 0.0f, 1.0f, 0.0f);

struct VertexPos 
{
   XMFLOAT4 pos;
   XMFLOAT2 tex0;
   XMFLOAT4 norm;
};

Renderer::Renderer() : D3DBase()
{

}

bool Renderer::InitializeMatMap(const aiScene *pAssimpScene)
{
   m_matList.clear();
   for( UINT i = 0; i < pAssimpScene->mNumMaterials; i++ ) 
   {
      aiMaterial *pMat = pAssimpScene->mMaterials[i];
      aiColor3D ambient, diffuse, specular;
      float shininess;

      pMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
      pMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
      pMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
      pMat->Get(AI_MATKEY_SHININESS_STRENGTH, shininess);

      PS_Material_Constant_Buffer psConstBuf;
      psConstBuf.ambient = XMFLOAT4(ambient.r, ambient.g, ambient.b, 1.0f);
      psConstBuf.diffuse = XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, 1.0f);
      psConstBuf.specular = XMFLOAT4(specular.r, specular.g, specular.b, 1.0f);
      psConstBuf.shininess = shininess;

      D3D11_BUFFER_DESC constBufDesc;
      ZeroMemory(&constBufDesc, sizeof( constBufDesc ));
      constBufDesc.Usage = D3D11_USAGE_DEFAULT;
      constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      constBufDesc.ByteWidth = sizeof( PS_Material_Constant_Buffer );

      D3D11_SUBRESOURCE_DATA constResourceData;
      ZeroMemory(&constResourceData, sizeof( constResourceData ));
      constResourceData.pSysMem = &psConstBuf;

      Material matInfo;
      HRESULT d3dResult = m_d3dDevice->CreateBuffer( &constBufDesc, &constResourceData, &matInfo.m_materialConstantBuffer);

      if ( FAILED(d3dResult) ) return false;

      UINT texCount = pMat->GetTextureCount(aiTextureType_DIFFUSE);

      if ( texCount > 0 )
      {
         aiString path;
         assert(texCount == 1);
         pMat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
         // TODO: hack that only takes in .jpgs
         if( path.C_Str()[path.length - 1] == 'g' )
         {
            HRESULT result = D3DX11CreateShaderResourceViewFromFile(m_d3dDevice, 
                                                               path.C_Str(),
                                                               0, 
                                                               0, 
                                                               &matInfo.m_texture, 
                                                               0);
            HR(result);
         }
      }
      else
      {
         matInfo.m_texture = NULL;
      }
      m_matList.push_back(matInfo);
   }
   return true;
}

void Renderer::DestroyMatMap()
{
   for( UINT i = 0; i < m_matList.size(); i++ )
   {
      m_matList[i].m_materialConstantBuffer->Release();
      if( m_matList[i].m_texture )
      {
         m_matList[i].m_texture->Release();
      }
   }
}


bool Renderer::CreateD3DMesh(const aiMesh *pMesh, const aiScene *pAssimpScene, Mesh *d3dMesh)
{
   UINT numVerts = pMesh->mNumVertices;
   UINT numFaces = pMesh->mNumFaces;
   UINT numIndices = numFaces * 3;
   VertexPos *vertices = new VertexPos[numVerts]();
   UINT *indices = new UINT[numIndices]();

   assert(*pMesh->mNumUVComponents == 2 || *pMesh->mNumUVComponents == 0 );
   memset(d3dMesh, 0, sizeof(Mesh));
   for (UINT vertIdx = 0; vertIdx < numVerts; vertIdx++)
   {
      auto pVert = &pMesh->mVertices[vertIdx];
      vertices[vertIdx].pos = XMFLOAT4(pVert->x, pVert->y, pVert->z, 1);

      auto pNorm = &pMesh->mNormals[vertIdx];
      vertices[vertIdx].norm = XMFLOAT4(pNorm->x, pNorm->y, pNorm->z, 0);
      
      if (*pMesh->mNumUVComponents > 0)
      {
         auto pUV = &pMesh->mTextureCoords[0][vertIdx];
         vertices[vertIdx].tex0 = XMFLOAT2(pUV->x, pUV->y);
      }
   }

   d3dMesh->m_numIndices = numIndices;
   for (UINT i = 0; i < numFaces; i++)
   {
      auto pFace = &pMesh->mFaces[i];
      assert(pFace->mNumIndices == 3);
      indices[i * 3] = pFace->mIndices[0];
      indices[i * 3 + 1] = pFace->mIndices[1];
      indices[i * 3 + 2] = pFace->mIndices[2];
   }

   auto pMat = pAssimpScene->mMaterials[pMesh->mMaterialIndex];
   aiColor3D ambient, diffuse, specular;
   float shininess;
   pMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
   pMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
   pMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
   pMat->Get(AI_MATKEY_SHININESS_STRENGTH, shininess);

   // Fill in a buffer description.
   D3D11_BUFFER_DESC bufferDesc;
   bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
   bufferDesc.ByteWidth       = static_cast<UINT>(sizeof( unsigned int ) * numIndices);
   bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
   bufferDesc.CPUAccessFlags  = 0;
   bufferDesc.MiscFlags       = 0;

   // Define the resource data.
   D3D11_SUBRESOURCE_DATA InitData;
   InitData.pSysMem = indices;
   InitData.SysMemPitch = 0;
   InitData.SysMemSlicePitch = 0;

   // Create the buffer with the device.
   HRESULT d3dResult = m_d3dDevice->CreateBuffer( &bufferDesc, &InitData, &d3dMesh->m_indexBuffer );
   if( FAILED( d3dResult ) ) 
   {
    	MessageBox(NULL, "CreateBuffer failed", "Error", MB_OK);
      return false;
   }
   
   D3D11_BUFFER_DESC vertexDesc;
   ZeroMemory(&vertexDesc, sizeof( vertexDesc ));
   vertexDesc.Usage = D3D11_USAGE_DEFAULT;
   vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   vertexDesc.ByteWidth = static_cast<UINT>(sizeof( VertexPos ) * numVerts);

   D3D11_SUBRESOURCE_DATA resourceData;
   ZeroMemory(&resourceData, sizeof( resourceData ));
   resourceData.pSysMem = vertices;

   d3dResult = m_d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &d3dMesh->m_vertexBuffer);

   if ( FAILED(d3dResult) ) return false;
   
   d3dMesh->m_MaterialIndex = pMesh->mMaterialIndex;

   return true;
}


void Renderer::DestroyD3DMesh(Mesh *mesh) 
{
   if( mesh->m_vertexBuffer ) mesh->m_vertexBuffer->Release();
   if( mesh->m_indexBuffer ) mesh->m_indexBuffer->Release();
}

void Renderer::Update(FLOAT dt, BOOL *keyInputArray) 
{
   static const FLOAT MOVE_SPEED = 0.05f;
   static const FLOAT ROTATE_SPEED = 0.02f;
   static const FLOAT LIGHT_ROTATE_SPEED = 0.01f;


   float tx = 0.0f, ty = 0.0f, tz = 0.0f;
   float ry = 0.0f, rx = 0.0f;
   float lightRotation = 0.0f;
   const float MoveUnit = m_cameraUnit * MOVE_SPEED;

   if( keyInputArray['Q'])
   {
      tz += m_cameraUnit;
   }
   if( keyInputArray['E'])
   {
      tz -= m_cameraUnit; 
   }
   if( keyInputArray['W'])
   {
      ty += m_cameraUnit; 
   }
   if( keyInputArray['S'])
   {
      ty -= m_cameraUnit; 
   }
   if( keyInputArray['A'])
   {
      tx -= m_cameraUnit; 
   }
   if( keyInputArray['D'])
   {
      tx += m_cameraUnit; 
   }
   if( keyInputArray['J'])
   {
      ry -= ROTATE_SPEED; 
   }
   if( keyInputArray['L'])
   {
      ry += ROTATE_SPEED; 
   }
   if( keyInputArray['K'])
   {
      rx -= ROTATE_SPEED; 
   }
   if( keyInputArray['I'])
   {
      rx += ROTATE_SPEED; 
   }
   
   if( keyInputArray['Z'])
   {
      lightRotation += LIGHT_ROTATE_SPEED;
   }
   if( keyInputArray['C'])
   {
      lightRotation -= LIGHT_ROTATE_SPEED;
   }

   XMVECTOR xAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
   XMFLOAT3 pos(tx, ty, tz);

   m_pCamera->MoveCamera(XMLoadFloat3(&pos));
   m_pCamera->RotateCameraHorizontally(ry);
   m_pCamera->RotateCameraVertically(rx);

   XMMATRIX perspective = XMMatrixPerspectiveFovLH(m_fieldOfView, (FLOAT)m_width / (FLOAT)m_height, m_nearPlane, m_farPlane);

   XMMATRIX view = *m_pCamera->GetViewMatrix();
   
   XMMATRIX lightRotationMatrix = XMMatrixRotationAxis(xAxis, lightRotation);
   XMVECTOR lightDirVector = XMVector4Normalize(XMVector4Transform(XMLoadFloat4(&m_lightDirection), lightRotationMatrix));
   XMVECTOR lightUpVector =  XMVector4Normalize(XMVector4Transform(XMLoadFloat4(&m_lightUp), lightRotationMatrix));
   
   XMFLOAT4 lookAtPoint = XMFLOAT4(0, 0, 0, 0);
   XMVECTOR lookAtPointVec = XMLoadFloat4(&lookAtPoint);
   
   XMStoreFloat4(&m_lightDirection, lightDirVector);
   XMStoreFloat4(&m_lightUp, lightUpVector);


   XMVECTOR shadowEye = XMVectorSetW(lightDirVector * 2000.0f, 1.0f);
   XMMATRIX lightView = XMMatrixLookAtLH(shadowEye, lookAtPointVec, lightUpVector);

   m_vsTransConstBuf.mvp = view * perspective;
   m_vsLightTransConstBuf.mvp = lightView * perspective;
   
   m_psLightConstBuf.direction = m_lightDirection;
   m_psLightConstBuf.mvp = m_vsLightTransConstBuf.mvp;
}

void Renderer::Render() 
{

   if (!m_d3dContext) return;

   float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
   float clearNormals[4] = { 0.5f, 0.5f, 0.5f, 0.0f };
   float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   float clearDepth[4] = { 1.0f, 1.0f, 1.0f, 1.0f };


   unsigned int zeroUints[4] = { 0, 0, 0, 0};

   m_d3dContext->ClearRenderTargetView(m_backBufferTarget, clearColor);
   
   m_d3dContext->ClearUnorderedAccessViewFloat(m_pBlurredShadowSurface->GetUnorderedAccessView(), clearDepth);
   m_d3dContext->ClearUnorderedAccessViewFloat(m_uav, clearColor);
   m_d3dContext->ClearUnorderedAccessViewUint(m_colorBufferDepthUAV, zeroUints);

   // Clear the depth buffer to 1.0f and the stencil buffer to 0.
   m_d3dContext->ClearDepthStencilView(m_DepthStencilView,
     D3D11_CLEAR_DEPTH, 1.0f, 0);

   m_d3dContext->ClearDepthStencilView(m_pShadowMap->GetDepthStencilView(),
     D3D11_CLEAR_DEPTH, 1.0f, 0);

   unsigned int stride = sizeof(VertexPos);
   unsigned int offset= 0;


   m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   m_d3dContext->RSSetState(m_rasterState);

   m_pLightConstants->SetData(m_d3dContext, &m_psLightConstBuf);
   ID3D11Buffer *pFirstPassCbs[] = { m_pLightConstants->GetConstantBuffer() };
   m_d3dContext->PSSetConstantBuffers(1 , 1, pFirstPassCbs);
   m_d3dContext->VSSetConstantBuffers(1 , 1, pFirstPassCbs);

   ID3D11SamplerState *samplers[] = { m_colorMapSampler, m_shadowSampler };
   m_d3dContext->PSSetSamplers(0 , 2, samplers);
         
   m_d3dContext->PSSetShader(m_solidColorPS, 0, 0);
   ID3D11Buffer *pCbs[] = { m_pTransformConstants->GetConstantBuffer() };
   m_d3dContext->VSSetConstantBuffers(0 , 1, pCbs);

   for (UINT draw = 0; draw < 2; draw++)
   {
      if (draw == 0)
      {
         m_d3dContext->IASetInputLayout( m_inputLayout );
         m_d3dContext->RSSetViewports(1, m_pShadowMap->GetViewport());
         m_d3dContext->VSSetShader(m_solidColorVS, 0, 0);
       
         m_pTransformConstants->SetData(m_d3dContext, &m_vsLightTransConstBuf);
         
         ID3D11RenderTargetView *pLightMapRtv[] = { m_pLightMap->GetRenderTargetView() };
         ID3D11ShaderResourceView *pNullSrv[] = { NULL };

         m_d3dContext->PSSetShaderResources(1 , 1, pNullSrv);
         m_d3dContext->OMSetRenderTargets(1, pLightMapRtv, m_pShadowMap->GetDepthStencilView());
      }
      else
      {
         ID3D11RenderTargetView *pNullRtv[] = { NULL, NULL };

         m_d3dContext->OMSetRenderTargets(2, pNullRtv, NULL);

         ID3D11ShaderResourceView *pSrv[] = { 
            m_pShadowMap->GetShaderResourceView(),
            m_pLightMap->GetShaderResourceView() };
         ID3D11UnorderedAccessView *pUav[] = { 
            m_pBlurredShadowSurface->GetUnorderedAccessView(),
            m_pLightBuffer->GetUnorderedAccessView() };

         UINT initialCounts[] = {
            0,
            0 };

         m_d3dContext->CSSetShader(m_blurCS, NULL, 0);
         m_d3dContext->CSSetShaderResources(0, 2, pSrv);
         m_d3dContext->CSSetUnorderedAccessViews(0, 2, pUav, initialCounts);
         m_d3dContext->Dispatch(m_shadowMapWidth, m_shadowMapHeight, 1);

         ID3D11UnorderedAccessView *pNullUav[] = { NULL, NULL };
         ID3D11ShaderResourceView *pNullSrv[] = { NULL, NULL };

         m_d3dContext->CSSetShaderResources(0, 2, pNullSrv);
         m_d3dContext->CSSetUnorderedAccessViews(0, 2, pNullUav, NULL);

         // Prepare the setup for actual rendering
         ID3D11UnorderedAccessView *pFirstPassUav[] = { m_uav, m_colorBufferDepthUAV };
         ID3D11RenderTargetView *pFirstPassRtv[] = { 
            m_backBufferTarget };
         
         ID3D11ShaderResourceView *pShadowSrv[] = {
            m_pBlurredShadowSurface->GetShaderResourceView(),
            m_pLightBuffer->GetShaderResourceView()
         };
         
         m_d3dContext->RSSetViewports(1, &m_viewport);
         m_d3dContext->IASetInputLayout( m_inputLayout );
         m_d3dContext->VSSetShader(m_solidColorVS, 0, 0);
         m_d3dContext->OMSetRenderTargetsAndUnorderedAccessViews(1, pFirstPassRtv, m_DepthStencilView, 3, 2, pFirstPassUav, NULL);
         m_d3dContext->PSSetShaderResources(1 , 2, pShadowSrv);
         m_pTransformConstants->SetData(m_d3dContext, &m_vsTransConstBuf);
      }

      for (UINT i = 0; i < scene.size(); i++)
      {
         auto pMat = &m_matList[scene[i].m_MaterialIndex];
         if( pMat->m_texture )
         {
            m_d3dContext->PSSetShaderResources(0 , 1, &pMat->m_texture);
            if( draw == 0)
            {
               m_d3dContext->PSSetShader(m_textureNoShadingPS, 0, 0);
            }
            else
            {
               m_d3dContext->PSSetShader(m_texturePS, 0, 0);
            }
         }
         else
         {
            m_d3dContext->PSSetShaderResources(0 , 1, &pMat->m_texture);
            m_d3dContext->PSSetShader(m_solidColorPS, 0, 0);
         }

         m_d3dContext->IASetVertexBuffers( 0, 1, &scene[i].m_vertexBuffer, &stride,  &offset);
         m_d3dContext->IASetIndexBuffer(scene[i].m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
         m_d3dContext->PSSetConstantBuffers(0 , 1, &pMat->m_materialConstantBuffer);
         m_d3dContext->DrawIndexed(scene[i].m_numIndices, 0, 0);
      }
   }
   
   // Clear our the SRVs
   ID3D11ShaderResourceView *pNullSrv[] = { NULL, NULL, NULL, NULL };
   m_d3dContext->PSSetShaderResources(1 , 4, pNullSrv);
   m_swapChain->Present(0, 0);
}

bool Renderer::LoadContent() 
{
	m_viewport.Width = static_cast<FLOAT>(m_width);
	m_viewport.Height = static_cast<FLOAT>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

   m_shadowMapHeight = m_height;
   m_shadowMapWidth = m_width;
   m_pShadowMap = new ShadowMap(m_d3dDevice, m_shadowMapWidth, m_shadowMapHeight);
   m_pBlurredShadowMap = new RWRenderTarget(m_d3dDevice, m_shadowMapWidth, m_shadowMapHeight);
   m_pBlurredShadowSurface = new RWComputeSurface(m_d3dDevice,  m_shadowMapWidth, m_shadowMapHeight);
   m_pLightMap = new RWRenderTarget(m_d3dDevice, m_shadowMapWidth, m_shadowMapHeight);
   m_pLightBuffer = new RWStructuredBuffer<PS_Point_Light>(m_d3dDevice, 24 * 32);

   m_pPlaneRenderer = new PlaneRenderer(m_d3dDevice);

   D3D11_RASTERIZER_DESC rasterizerDesc;
   rasterizerDesc.FillMode = D3D11_FILL_SOLID;
   rasterizerDesc.CullMode = D3D11_CULL_BACK;
   rasterizerDesc.FrontCounterClockwise = FALSE;
   rasterizerDesc.DepthBias = 0;
   rasterizerDesc.SlopeScaledDepthBias = 0.0f;
   rasterizerDesc.DepthBiasClamp = 0.0;
   rasterizerDesc.DepthClipEnable = TRUE;
   rasterizerDesc.ScissorEnable = FALSE;
   rasterizerDesc.MultisampleEnable = FALSE;
   rasterizerDesc.AntialiasedLineEnable = FALSE;

   HRESULT rasterResult = m_d3dDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterState);
	if (FAILED(rasterResult))
	{
		DXTRACE_MSG("Failed to create the rasterizer state");
		return false;
	}
   
   // Create an instance of the Importer class
  Assimp::Importer importer;
  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll 
  // propably to request more postprocessing than we do in this example.
  const aiScene* AssimpScene = importer.ReadFile( "sponza.obj", 
        aiProcess_CalcTangentSpace       | 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_MakeLeftHanded         | 
        aiProcess_FlipUVs                | 
        aiProcess_FlipWindingOrder       | 
        aiProcess_GenSmoothNormals       |
        aiProcess_PreTransformVertices   |
        aiProcess_SortByPType);

  InitializeMatMap( AssimpScene );
  for (UINT i = 0; i < AssimpScene->mNumMeshes; i++)
  {
     Mesh d3dMesh;
     const aiMesh *pMesh = AssimpScene->mMeshes[i];
     bool result = CreateD3DMesh(pMesh, AssimpScene, &d3dMesh);
     if ( result != true ) return false;
     scene.push_back(d3dMesh);
  }

  if (AssimpScene->HasCameras())
  {
     assert(AssimpScene->mNumCameras == 1);
     auto pCam = AssimpScene->mCameras[0];
     auto pos = pCam->mPosition;
     auto lookAt = pCam->mLookAt;
     auto up = pCam->mUp;
     
     XMFLOAT3 fPos(pos.x, pos.y, pos.z);
     XMFLOAT3 fLookAt(lookAt.x, lookAt.y, lookAt.z);
     XMFLOAT3 fUp(up.x, up.y, up.z);

     m_pCamera = new Camera(XMLoadFloat3(&fPos), XMLoadFloat3(&fLookAt), XMLoadFloat3(&fUp));
     m_nearPlane = pCam->mClipPlaneNear;
     m_farPlane = pCam->mClipPlaneFar;
     m_cameraUnit = 50.0f;
     m_fieldOfView = pCam->mHorizontalFOV * 2.0f / pCam->mAspect;
  }
  else
  {
     XMFLOAT3 pos(0.0f, 1.0f, 0.0f);
     XMFLOAT3 lookAt(1.0f, 0.0f, 0.0f);
     XMFLOAT3 up(0.0f, 1.0f, 0.0f);
     m_pCamera = new Camera(XMLoadFloat3(&pos), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));

     m_nearPlane = 1.0;
     m_farPlane = 20000.0f;
     m_cameraUnit = 50.0f;
     m_fieldOfView = 3.14f / 2.0f;
  }

    m_pLightConstants = new ConstantBuffer<PS_Light_Constant_Buffer>(m_d3dDevice);

  if (AssimpScene->HasLights())
  {
#if 0
     assert(AssimpScene->mNumLights == 1);
     auto pLight = AssimpScene->mLights[0];
     
     assert(pLight->mType == aiLightSource_POINT);
     auto lightPos = pLight->mPosition;
     m_lightPosition = XMFLOAT4(lightPos.x, lightPos.y, lightPos.z, 1.0f);
#endif
  }
  else
  {
      m_lightDirection = LIGHT_DIRECTION;
      m_lightUp = LIGHT_UP;
  }

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
  flags |= D3DCOMPILE_DEBUG;
#endif
   // Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
  LPCSTR profile = ( m_d3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";
   const D3D_SHADER_MACRO defines[] = 
   {
       "EXAMPLE_DEFINE", "1",
       NULL, NULL
   };
   ID3DBlob* csBuffer = nullptr;
   HRESULT hr = D3DUtils::CompileD3DShader( "BlurCS.hlsl", "main", "cs_5_0", &csBuffer);

   HR(m_d3dDevice->CreateComputeShader( csBuffer->GetBufferPointer(), 
		                               csBuffer->GetBufferSize(), 
		                               NULL, &m_blurCS));

   ID3DBlob* vsBuffer = 0;
   BOOL compileResult = D3DUtils::CompileD3DShader("PlainVert.hlsl", "main", "vs_5_0", &vsBuffer);
   if( compileResult == false )
   {
      MessageBox(0, "Error loading vertex shader!", "Compile Error", MB_OK);
      return false;
   }
   HR(m_d3dDevice->CreateVertexShader(
         vsBuffer->GetBufferPointer(), 
         vsBuffer->GetBufferSize(), 
         0, 
         &m_solidColorVS));
   
   ID3DBlob* vsPlaneBuffer = 0;
   compileResult = D3DUtils::CompileD3DShader("PlaneVertexShader.hlsl", "main", "vs_5_0", &vsPlaneBuffer);
   if( compileResult == false )
   {
      MessageBox(0, "Error loading vertex shader!", "Compile Error", MB_OK);
      return false;
   }

   HR(m_d3dDevice->CreateVertexShader(
         vsPlaneBuffer->GetBufferPointer(), 
         vsPlaneBuffer->GetBufferSize(), 
         0,
         &m_planeVS));

   vsPlaneBuffer->Release();

   D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
   {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };

   unsigned int totalLayoutElements = ARRAYSIZE( solidColorLayout );

   HR(m_d3dDevice->CreateInputLayout( solidColorLayout, totalLayoutElements,
      vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_inputLayout ));

   vsBuffer->Release();

   HR(D3DUtils::CreatePixelShader(
      m_d3dDevice,
      "PlainPixel.hlsl", 
      "main", 
      "ps_5_0", 
      &m_solidColorPS));

   HR(D3DUtils::CreatePixelShader(
     m_d3dDevice,
     "GIPixel.hlsl", 
     "main", 
     "ps_5_0", 
     &m_globalIlluminationPS));

   HR(D3DUtils::CreatePixelShader(
     m_d3dDevice,
     "BlurPixel.hlsl", 
     "main", 
     "ps_5_0", 
     &m_blurPS));
   
   HR(D3DUtils::CreatePixelShader(
      m_d3dDevice,
      "PlainPixel.hlsl", 
      "texMain", 
      "ps_5_0", 
      &m_texturePS ));

   HR(D3DUtils::CreatePixelShader(
      m_d3dDevice,
      "TextureShader.hlsl", 
      "main", 
      "ps_5_0", 
      &m_textureNoShadingPS ));

   VS_Transformation_Constant_Buffer vsConstBuf;
   vsConstBuf.mvp = XMMatrixIdentity();

   m_pTransformConstants = new ConstantBuffer<VS_Transformation_Constant_Buffer>(m_d3dDevice);

   D3D11_SAMPLER_DESC colorMapDesc;
   ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ));
   colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

   HR(m_d3dDevice->CreateSamplerState( &colorMapDesc, &m_colorMapSampler));

   D3D11_SAMPLER_DESC shadowSamplerDesc;
   ZeroMemory( &shadowSamplerDesc, sizeof( shadowSamplerDesc ));
   shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
   shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
   shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
   shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

   HR(m_d3dDevice->CreateSamplerState( &shadowSamplerDesc, &m_shadowSampler));


   ID3D11Texture2D *colorBufferDepth;

   D3D11_TEXTURE2D_DESC cbdDesc;
   memset(&cbdDesc, 0, sizeof(cbdDesc));
   cbdDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
   cbdDesc.Format = DXGI_FORMAT_R32_UINT;
   cbdDesc.Height = m_height;
   cbdDesc.Width = m_width;
   cbdDesc.MipLevels = 1;
   cbdDesc.Usage = D3D11_USAGE_DEFAULT;
   cbdDesc.SampleDesc.Count = 1;
   cbdDesc.ArraySize = 1;
   HR(m_d3dDevice->CreateTexture2D(&cbdDesc, NULL,  &colorBufferDepth));


   D3D11_UNORDERED_ACCESS_VIEW_DESC cbdUavDesc;
   cbdUavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
   cbdUavDesc.Format = cbdDesc.Format;
   cbdUavDesc.Texture2D.MipSlice = 0;
   HM(m_d3dDevice->CreateUnorderedAccessView(colorBufferDepth, &cbdUavDesc, &m_colorBufferDepthUAV),
      "Failed to create the UAV for the color buffer depth");
   ID3D11Texture3D *recordDepth;

   D3D11_TEXTURE3D_DESC ctDesc;
   memset(&ctDesc, 0, sizeof(ctDesc));
   ctDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
   ctDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   ctDesc.Depth = MAX_COLOR_BUFFER_DEPTH;
   ctDesc.Height = m_height;
   ctDesc.Width = m_width;
   ctDesc.MipLevels = 1;
   ctDesc.Usage = D3D11_USAGE_DEFAULT;
   HR(m_d3dDevice->CreateTexture3D(&ctDesc, NULL,  &recordDepth));

   D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
   uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
   uavDesc.Texture3D.MipSlice = 0;
   uavDesc.Texture3D.FirstWSlice = 0;
   uavDesc.Texture3D.WSize = -1;
   uavDesc.Format = ctDesc.Format;
   HR(m_d3dDevice->CreateUnorderedAccessView(recordDepth, &uavDesc, &m_uav));

   D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
   memset(&srvDesc, 0, sizeof(srvDesc));
   srvDesc.Format = ctDesc.Format;
   srvDesc.Texture3D.MipLevels = 1;
   srvDesc.Texture3D.MostDetailedMip = 0;
   srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D;
   HR(m_d3dDevice->CreateShaderResourceView(recordDepth, &srvDesc, &m_colorBufferSrv));

   return true;
}

void Renderer::UnloadContent() 
{
   delete m_pShadowMap;
   delete m_pLightMap;

   delete m_pLightBuffer;
   
   delete m_pBlurredShadowMap;

   delete m_pBlurredShadowSurface;

   delete m_pTransformConstants;
   delete m_pLightConstants;
   delete m_pPlaneRenderer;
   delete m_pCamera;

   DestroyMatMap();
   for(UINT i = 0; i < scene.size(); i++)
   {
      DestroyD3DMesh(&scene[i]);
   }

   if( m_solidColorPS ) m_solidColorPS->Release();
   if( m_solidColorVS ) m_solidColorVS->Release();
   if( m_inputLayout ) m_inputLayout->Release();
   if( m_colorMapSampler ) m_colorMapSampler->Release();
   if( m_uav ) m_uav->Release();
}