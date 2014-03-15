#include "Renderer.h"
#include <cassert>
#include <string>

using std::vector;
using std::string;

const XMFLOAT4 LIGHT_POSITION(0.0f, 1.0f, -3.0f, 1.0f);

struct VertexPos 
{
   XMFLOAT4 pos;
   XMFLOAT2 tex0;
   XMFLOAT4 norm;
};

Renderer::Renderer() : D3DBase()
{
   m_camX = 0.0f;
   m_camY = -0.2f;
   m_camZ = 0.5f;
   m_camYAngle = 0.0f;
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

      if ( FAILED(d3dResult) ) 
      {
         DXTRACE_MSG( "Failed to load the texture image!" );
         return false;
      }
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
   static const FLOAT MOVE_SPEED = 0.01f;
   static const FLOAT ROTATE_SPEED = 0.05f;

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

   m_vsTransConstBuf.perspective = XMMatrixPerspectiveFovLH(3.14f * 0.25f, (FLOAT)m_width / (FLOAT)m_height, 0.01f, 100.0f);
   XMVECTOR yAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
   XMMATRIX trans = XMMatrixTranslation(m_camX, m_camY, m_camZ);
   XMMATRIX rotation = XMMatrixRotationAxis(yAxis, m_camYAngle);
   m_normalWorldTrans = XMMatrixIdentity();

   m_vsTransConstBuf.view = trans * rotation;
   m_vsTransConstBuf.world = XMMatrixIdentity();
   XMVECTOR transformedLight = XMVector4Transform(XMLoadFloat4(&m_lightPosition), m_vsTransConstBuf.perspective * m_vsTransConstBuf.view);
   XMStoreFloat4(&m_psLightConstBuf.position, transformedLight);
}

void Renderer::Render() 
{

   if (!m_d3dContext) return;

   float clearColor[4] = { 0.2f, 0.2f, 0.2f, 0.0f };
   m_d3dContext->ClearRenderTargetView(m_backBufferTarget, clearColor);

   // Clear the depth buffer to 1.0f and the stencil buffer to 0.
   m_d3dContext->ClearDepthStencilView(m_DepthStencilView,
     D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

   unsigned int stride = sizeof(VertexPos);
   unsigned int offset= 0;

   m_d3dContext->IASetInputLayout( m_inputLayout );
   m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
   m_d3dContext->RSSetState(m_rasterState);

   m_d3dContext->VSSetShader(m_solidColorVS, 0, 0);

   m_vsTransConstBuf.world = m_normalWorldTrans;
   m_d3dContext->UpdateSubresource( m_vsConstantBuffer, 0, 0, &m_vsTransConstBuf, 0, 0 );
   m_d3dContext->VSSetConstantBuffers(0 , 1, &m_vsConstantBuffer);

   m_d3dContext->UpdateSubresource( m_psConstantBuffer, 0, 0, &m_psLightConstBuf, 0, 0 );
   m_d3dContext->PSSetConstantBuffers(1 , 1, &m_psConstantBuffer);
   m_d3dContext->PSSetSamplers(0 , 1, &m_colorMapSampler);

   for (UINT i = 0; i < scene.size(); i++)
   {
      if( scene[i].m_texture )
      {
         m_d3dContext->PSSetShaderResources(0 , 1, &scene[i].m_texture);
         m_d3dContext->PSSetShader(m_texturePS, 0, 0);
      }
      else
      {
         m_d3dContext->PSSetShader(m_solidColorPS, 0, 0);
      }

      m_d3dContext->IASetVertexBuffers( 0, 1, &scene[i].m_vertexBuffer, &stride,  &offset);
      m_d3dContext->IASetIndexBuffer(scene[i].m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
      m_d3dContext->PSSetConstantBuffers(0 , 1, &scene[i].m_materialConstantBuffer);
      m_d3dContext->DrawIndexed(scene[i].m_numIndices, 0, 0);
   }

   m_swapChain->Present(0, 0);
}

bool Renderer::LoadContent() 
{
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
   string filename("bunny.obj");
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
   BOOL compileResult = CompileD3DShader("PlainVert.hlsl", "main", "vs_5_0", &vsBuffer);
   if( compileResult == false )
   {
      MessageBox(0, "Error loading vertex shader!", "Compile Error", MB_OK);
      return false;
   }
   HRESULT d3dResult = m_d3dDevice->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &m_solidColorVS);
   if( FAILED(d3dResult) )
   {
      if(vsBuffer) vsBuffer->Release();
      return false;
   }

   D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
   {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
   };

   unsigned int totalLayoutElements = ARRAYSIZE( solidColorLayout );

   d3dResult = m_d3dDevice->CreateInputLayout( solidColorLayout, totalLayoutElements,
      vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_inputLayout );

   vsBuffer->Release();

   if( FAILED( d3dResult ) ) return false;

   ID3DBlob* psBuffer = 0;

   compileResult = CompileD3DShader( "PlainPixel.hlsl", "main", "ps_5_0", &psBuffer );

   if( compileResult == false )
   {
      MessageBox(0, "Error loading pixel shader!", "Compile Error", MB_OK);
      return false;
   }

   d3dResult = m_d3dDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &m_solidColorPS);

   psBuffer->Release();

   compileResult = CompileD3DShader( "PlainPixel.hlsl", "texMain", "ps_5_0", &psBuffer );

   if( compileResult == false )
   {
      MessageBox(0, "Error loading pixel shader!", "Compile Error", MB_OK);
      return false;
   }

   d3dResult = m_d3dDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &m_texturePS);

   psBuffer->Release();

   if( FAILED( d3dResult) ) return false;

   VS_Transformation_Constant_Buffer vsConstBuf;
   vsConstBuf.perspective = XMMatrixPerspectiveRH(static_cast<FLOAT>(m_width), static_cast<FLOAT>(m_height), 0.01f, 100.0f);
   vsConstBuf.world = XMMatrixIdentity();
   vsConstBuf.view = XMMatrixIdentity();

   D3D11_BUFFER_DESC constBufDesc;
   ZeroMemory(&constBufDesc, sizeof( constBufDesc ));
   constBufDesc.Usage = D3D11_USAGE_DEFAULT;
   constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   constBufDesc.ByteWidth = sizeof( VS_Transformation_Constant_Buffer );

   D3D11_SUBRESOURCE_DATA constResourceData;
   ZeroMemory(&constResourceData, sizeof( constResourceData ));
   constResourceData.pSysMem = &vsConstBuf;

   d3dResult = m_d3dDevice->CreateBuffer( &constBufDesc, &constResourceData, &m_vsConstantBuffer);

   if ( FAILED(d3dResult) ) return false;

   PS_Light_Constant_Buffer psConstBuf;
   psConstBuf.position = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

   ZeroMemory(&constBufDesc, sizeof( constBufDesc ));
   constBufDesc.Usage = D3D11_USAGE_DEFAULT;
   constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   constBufDesc.ByteWidth = sizeof( PS_Light_Constant_Buffer );

   ZeroMemory(&constResourceData, sizeof( constResourceData ));
   constResourceData.pSysMem = &psConstBuf;

   d3dResult = m_d3dDevice->CreateBuffer( &constBufDesc, &constResourceData, &m_psConstantBuffer);

   if ( FAILED(d3dResult) ) return false;

   D3D11_SAMPLER_DESC colorMapDesc;
   ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ));
   colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
   colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
   colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

   d3dResult = m_d3dDevice->CreateSamplerState( &colorMapDesc, &m_colorMapSampler);

   if( FAILED( d3dResult ))
   {
      DXTRACE_MSG( "Failed to create color map sampler state!" );
      return false;
   }

   return true;
}

void Renderer::UnloadContent() 
{
   for(UINT i = 0; i < scene.size(); i++)
   {
      DestroyD3DMesh(&scene[i]);
   }

   if( m_solidColorPS ) m_solidColorPS->Release();
   if( m_solidColorVS ) m_solidColorVS->Release();
   if( m_inputLayout ) m_inputLayout->Release();
   if( m_colorMapSampler ) m_colorMapSampler->Release();

   m_solidColorPS  = 0;
   m_solidColorVS = 0;
   m_inputLayout = 0;
   m_colorMapSampler = 0;

}