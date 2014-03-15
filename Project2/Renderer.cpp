#include "Renderer.h"
#include "D3DUtils.h"

#include <cassert>
#include <string>

using std::vector;
using std::string;

const XMFLOAT4 LIGHT_POSITION(0.0f, 1.9f, 0.0f, 1.0f);

struct VertexPos 
{
   XMFLOAT4 pos;
   XMFLOAT2 tex0;
   XMFLOAT4 norm;
};

Renderer::Renderer() : D3DBase()
{
   m_camX = 0.0f;
   m_camY = -1.0f;
   m_camZ = -3.5f;
   m_camYAngle = 3.14f;
}

bool Renderer::CreateD3DMesh(ObjReader::Mesh *mesh, ObjReader::Material *material, Mesh *d3dMesh)
{
   vector<ObjReader::Vertices> *pVerts = &mesh->verts;
   vector<ObjReader::Face> *pFaces = &mesh->faces;
   VertexPos *vertices = new VertexPos[pVerts->size()]();
   UINT *indices = new UINT[pFaces->size() * 3]();

   memset(d3dMesh, 0, sizeof(Mesh));

   for (int vertIdx = 0; vertIdx < pVerts->size(); vertIdx++)
   {
      ObjReader::Vertices vert = (*pVerts)[vertIdx];
      vertices[vertIdx].pos = XMFLOAT4(vert.x, vert.y, vert.z, 1.0f);
      vertices[vertIdx].tex0 = XMFLOAT2(vert.uv.u, vert.uv.v);
      vertices[vertIdx].norm = XMFLOAT4(vert.norm.x, vert.norm.y, vert.norm.z, 0.0f);
   }

   d3dMesh->m_numIndices = static_cast<UINT>(pFaces->size() * 3);
   for (int i = 0; i < pFaces->size(); i++)
   {
      ObjReader::Face face = (*pFaces)[i];
      indices[i * 3] = face.v1;
      indices[i * 3 + 1] = face.v2;
      indices[i * 3 + 2] = face.v3;
   }

   // Fill in a buffer description.
   D3D11_BUFFER_DESC bufferDesc;
   bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
   bufferDesc.ByteWidth       = static_cast<UINT>(sizeof( unsigned int ) * 3 * pFaces->size());
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
   if( FAILED( d3dResult ) ) return false;

   PS_Material_Constant_Buffer psConstBuf;
   psConstBuf.ambient = XMFLOAT4(material->ambient.r, material->ambient.g, material->ambient.b, 1.0f);
   psConstBuf.diffuse = XMFLOAT4(material->diffuse.r, material->diffuse.g, material->diffuse.b, 1.0f);
   psConstBuf.specular = XMFLOAT4(material->specular.r, material->specular.g, material->specular.b, 1.0f);
   psConstBuf.shininess = material->shininess;

   D3D11_BUFFER_DESC constBufDesc;
   ZeroMemory(&constBufDesc, sizeof( constBufDesc ));
   constBufDesc.Usage = D3D11_USAGE_DEFAULT;
   constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   constBufDesc.ByteWidth = sizeof( PS_Material_Constant_Buffer );

   D3D11_SUBRESOURCE_DATA constResourceData;
   ZeroMemory(&constResourceData, sizeof( constResourceData ));
   constResourceData.pSysMem = &psConstBuf;

   d3dResult = m_d3dDevice->CreateBuffer( &constBufDesc, &constResourceData, &d3dMesh->m_materialConstantBuffer);

   if ( FAILED(d3dResult) ) return false;

   D3D11_BUFFER_DESC vertexDesc;
   ZeroMemory(&vertexDesc, sizeof( vertexDesc ));
   vertexDesc.Usage = D3D11_USAGE_DEFAULT;
   vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   vertexDesc.ByteWidth = static_cast<UINT>(sizeof( VertexPos ) * pVerts->size());

   D3D11_SUBRESOURCE_DATA resourceData;
   ZeroMemory(&resourceData, sizeof( resourceData ));
   resourceData.pSysMem = vertices;

   d3dResult = m_d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &d3dMesh->m_vertexBuffer);

   if ( FAILED(d3dResult) ) return false;

   if (material->hasTexture)
   {
      d3dResult = D3DX11CreateShaderResourceViewFromFile(m_d3dDevice, 
                                                         material->textureName.c_str(),
                                                         0, 
                                                         0, 
                                                         &d3dMesh->m_texture, 
                                                         0);
          //"Failed to load the texture image!");
   }
   return true;
}

void Renderer::DestroyD3DMesh(Mesh *mesh) 
{
   if( mesh->m_vertexBuffer ) mesh->m_vertexBuffer->Release();
   if( mesh->m_indexBuffer ) mesh->m_indexBuffer->Release();
   if( mesh->m_materialConstantBuffer) mesh->m_materialConstantBuffer->Release();
}

void Renderer::Update(FLOAT dt, BOOL *keyInputArray) 
{
   static const FLOAT MOVE_SPEED = 0.05f;
   static const FLOAT ROTATE_SPEED = 0.02f;

   float tx = 0.0f, ty = 0.0f, tz = 0.0f;
   float ry = 0.0f;

   if( keyInputArray['Q'])
   {
      tz += MOVE_SPEED; 
   }
   if( keyInputArray['E'])
   {
      tz -= MOVE_SPEED; 
   }
   if( keyInputArray['W'])
   {
      ty -= MOVE_SPEED; 
   }
   if( keyInputArray['S'])
   {
      ty += MOVE_SPEED; 
   }
   if( keyInputArray['A'])
   {
      tx += MOVE_SPEED; 
   }
   if( keyInputArray['D'])
   {
      tx -= MOVE_SPEED; 
   }
   if( keyInputArray['J'])
   {
      ry += ROTATE_SPEED; 
   }
   if( keyInputArray['L'])
   {
      ry -= ROTATE_SPEED; 
   }

   m_camX += tx;
   m_camY += ty;
   m_camZ += tz;
   m_camYAngle += ry;

   XMMATRIX perspective = XMMatrixPerspectiveFovLH(3.14f * 0.25f, (FLOAT)m_width / (FLOAT)m_height, 0.01f, 100.0f);
   XMVECTOR yAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
   XMVECTOR xAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

   XMMATRIX trans = XMMatrixTranslation(m_camX, m_camY, m_camZ);
   XMMATRIX rotation = XMMatrixRotationAxis(yAxis, m_camYAngle);
   XMMATRIX view = trans * rotation;
   m_vsTransConstBuf.mvp = view * perspective;

   XMMATRIX shadowTrans = XMMatrixTranslation(-m_lightPosition.x, -m_lightPosition.y, -m_lightPosition.z);
   XMMATRIX shadowRotation = XMMatrixRotationAxis(xAxis, 3.14f / 2.0f) * XMMatrixRotationAxis(yAxis, 3.14f);
   XMMATRIX shadowView = shadowTrans * shadowRotation;
   m_shadowMapTransform.mvp = shadowView * perspective;

   m_psLightConstBuf.position = m_lightPosition;
   m_psLightConstBuf.mvp = m_shadowMapTransform.mvp;
}

void Renderer::Render() 
{

   if (!m_d3dContext) return;

   float clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
   unsigned int zeroUints[4] = { 0, 0, 0, 0};

   m_d3dContext->ClearRenderTargetView(m_pFirstPassColors->GetRenderTargetView(), clearColor);
   m_d3dContext->ClearUnorderedAccessViewFloat(m_uav, clearColor);
   m_d3dContext->ClearUnorderedAccessViewUint(m_colorBufferDepthUAV, zeroUints);
   // Clear the depth buffer to 1.0f and the stencil buffer to 0.
   m_d3dContext->ClearDepthStencilView(m_DepthStencilView,
     D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

   m_d3dContext->ClearDepthStencilView(m_pShadowMap->GetDepthStencilView(),
     D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

   unsigned int stride = sizeof(VertexPos);
   unsigned int offset= 0;

   m_d3dContext->IASetInputLayout( m_inputLayout );
   m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   m_d3dContext->RSSetState(m_rasterState);

   m_d3dContext->VSSetShader(m_solidColorVS, 0, 0);

   m_pLightConstants->SetData(m_d3dContext, &m_psLightConstBuf);
   ID3D11Buffer *pCbs[] = { m_pLightConstants->GetConstantBuffer() };
   m_d3dContext->PSSetConstantBuffers(1 , 1, pCbs);
   m_d3dContext->VSSetConstantBuffers(1 , 1, pCbs);

   m_d3dContext->PSSetSamplers(0 , 1, &m_colorMapSampler);

   // Make sure the first SRV is cleared out
   ID3D11ShaderResourceView *pSrvs = { NULL };
   m_d3dContext->PSSetShaderResources(0 , 1, &pSrvs);

   for (UINT i = 0; i < 2; i++)
   {
      if (i == 0)
      {
         ID3D11RenderTargetView *pRtvs = { NULL };
         ID3D11ShaderResourceView *pSrvs = { NULL };

         m_pTransformConstants->SetData(m_d3dContext, &m_shadowMapTransform);


         m_d3dContext->PSSetShader(m_solidColorPSNoShadow, 0, 0);
         m_d3dContext->PSSetShaderResources(1 , 1, &pSrvs);
         m_d3dContext->OMSetRenderTargets(1, &pRtvs, m_pShadowMap->GetDepthStencilView());
      }
      else 
      {
         ID3D11ShaderResourceView *pSrv = m_pShadowMap->GetShaderResourceView();
         ID3D11UnorderedAccessView *pUav[] = { m_uav, m_colorBufferDepthUAV };
         ID3D11RenderTargetView *pRtv[] = { 
            m_pFirstPassColors->GetRenderTargetView(),
            m_pFirstPassNormals->GetRenderTargetView() };
         
         m_pTransformConstants->SetData(m_d3dContext, &m_vsTransConstBuf);

         m_d3dContext->PSSetShader(m_solidColorPS, 0, 0);
         m_d3dContext->OMSetRenderTargetsAndUnorderedAccessViews(1, pRtv, m_DepthStencilView, 2, 2, pUav, NULL);
         m_d3dContext->PSSetShaderResources(1 , 1, &pSrv);
      }
      ID3D11Buffer *pCbs[] = { m_pTransformConstants->GetConstantBuffer() };
      m_d3dContext->VSSetConstantBuffers(0 , 1, pCbs);

      for (UINT i = 0; i < scene.size(); i++)
      {
#if 0
         if( scene[i].m_texture )
         {
            m_d3dContext->PSSetShaderResources(0 , 1, &scene[i].m_texture);
            m_d3dContext->PSSetShader(m_texturePS, 0, 0);
         }
         else
         {
            m_d3dContext->PSSetShader(m_solidColorPS, 0, 0);
         }
#endif

         m_d3dContext->IASetVertexBuffers( 0, 1, &scene[i].m_vertexBuffer, &stride,  &offset);
         m_d3dContext->IASetIndexBuffer(scene[i].m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
         m_d3dContext->PSSetConstantBuffers(0 , 1, &scene[i].m_materialConstantBuffer);
         m_d3dContext->DrawIndexed(scene[i].m_numIndices, 0, 0);
      }
   }
   
   // Post processing effect
   ID3D11UnorderedAccessView *pUav[] = { m_colorBufferDepthUAV };
   ID3D11ShaderResourceView *pSrv[] = {
      m_colorBufferSrv,
      m_pFirstPassColors->GetShaderResourceView(),
      m_pFirstPassNormals->GetShaderResourceView()};


   m_d3dContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_backBufferTarget, m_DepthStencilView, 1, 1, pUav, NULL);
   m_d3dContext->PSSetShaderResources(1 , 3, pSrv);

   m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   m_d3dContext->VSSetShader(m_planeVS, 0, 0);
   m_d3dContext->PSSetShader(m_redPS, 0, 0);
   m_d3dContext->Draw(3, 0);

   m_swapChain->Present(0, 0);
}

bool Renderer::LoadContent() 
{
   m_pShadowMap = new ShadowMap(m_d3dDevice, m_width, m_height);
   m_pFirstPassColors = new RWRenderTarget(m_d3dDevice, m_width, m_height);
   m_pFirstPassNormals = new RWRenderTarget(m_d3dDevice, m_width, m_height);


   m_lightPosition = LIGHT_POSITION;

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

   ObjReader::ObjData data;
   string filename("cornell.obj");
   int result = ObjReader::ObjReader::ConvertFromFile(filename, &data);
   if (result != ObjReader::RESULT_SUCCESS) 
   {
      DXTRACE_MSG("Parsing for file failed");
      return false;
   }

   for (UINT i = 0; i < data.meshes.size(); i++)
   {
      Mesh d3dMesh;
      assert(data.matMap.count(data.meshes[i].materialName) > 0); 
      if (!CreateD3DMesh(&data.meshes[i], &data.matMap[data.meshes[i].materialName], &d3dMesh)) return false;
      scene.push_back(d3dMesh);
   }

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
     "RedShader.hlsl", 
     "main", 
     "ps_5_0", 
     &m_redPS));
   
   HR(D3DUtils::CreatePixelShader(
      m_d3dDevice,
      "PlainPixel.hlsl", 
      "texMain", 
      "ps_5_0", 
      &m_texturePS ));

   HR(D3DUtils::CreatePixelShader(
      m_d3dDevice,
      "PlainPixel.hlsl", 
      "mainNoShadow", 
      "ps_5_0", 
      &m_solidColorPSNoShadow));

   VS_Transformation_Constant_Buffer vsConstBuf;
   vsConstBuf.mvp = XMMatrixIdentity();

   m_pTransformConstants = new ConstantBuffer<VS_Transformation_Constant_Buffer>(m_d3dDevice);

   PS_Light_Constant_Buffer psConstBuf;
   psConstBuf.position = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

   m_pLightConstants = new ConstantBuffer<PS_Light_Constant_Buffer>(m_d3dDevice);

   D3D11_SAMPLER_DESC colorMapDesc;
   ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ));
   colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

   HR(m_d3dDevice->CreateSamplerState( &colorMapDesc, &m_colorMapSampler));

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
   delete m_pFirstPassColors;
   delete m_pFirstPassNormals;
   delete m_pShadowMap;
   delete m_pTransformConstants;
   delete m_pLightConstants;

   for(UINT i = 0; i < scene.size(); i++)
   {
      DestroyD3DMesh(&scene[i]);
   }

   if( m_solidColorPS ) m_solidColorPS->Release();
   if( m_solidColorPSNoShadow ) m_solidColorPSNoShadow->Release();
   if( m_solidColorVS ) m_solidColorVS->Release();
   if( m_inputLayout ) m_inputLayout->Release();
   if( m_colorMapSampler ) m_colorMapSampler->Release();
   if( m_uav ) m_uav->Release();
}