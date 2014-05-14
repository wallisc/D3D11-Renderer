#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>
#include <cassert>

#include "D3DUtils.h"


struct PlaneVertex
{
   XMFLOAT4 pos;
   XMFLOAT2 tex0;
};

const static D3D11_INPUT_ELEMENT_DESC g_planeInputLayout[] =
{
   {"SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
   {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

const static PlaneVertex g_planeVertices[] =
{
   {XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
   {XMFLOAT4(3.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(2.0f, 0.0f)},
   {XMFLOAT4(-1.0f, -3.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 2.0f)}
};

class PlaneRenderer
{
public:
   PlaneRenderer(ID3D11Device *pDevice) 
   {
      ID3DBlob* vsBuffer = 0;
      BOOL compileResult = D3DUtils::CompileD3DShader("PlaneVertexShader2.hlsl", "main", "vs_5_0", &vsBuffer);
      assert(compileResult);
      HRESULT result = S_OK;
      result = pDevice->CreateVertexShader(
            vsBuffer->GetBufferPointer(), 
            vsBuffer->GetBufferSize(), 
            0, 
            &m_planeVS);
      assert(result == S_OK);


      D3D11_BUFFER_DESC vertexDesc;
      ZeroMemory(&vertexDesc, sizeof( vertexDesc ));
      vertexDesc.Usage = D3D11_USAGE_DEFAULT;
      vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      vertexDesc.ByteWidth = static_cast<UINT>(sizeof( PlaneVertex ) * 3);

      D3D11_SUBRESOURCE_DATA resourceData;
      ZeroMemory(&resourceData, sizeof( resourceData ));
      resourceData.pSysMem = g_planeVertices;

      result = pDevice->CreateBuffer( &vertexDesc, &resourceData, &m_planeBuffer);
      assert(result == S_OK);

      result = pDevice->CreateInputLayout( g_planeInputLayout, ARRAYSIZE(g_planeInputLayout),
         vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), &m_inputLayout );
      assert(result == S_OK);

   }

   ~PlaneRenderer() 
   {
      m_planeVS->Release();
      m_planeBuffer->Release();
      m_inputLayout->Release();
   }

   
   ID3D11VertexShader* m_planeVS;
   ID3D11Buffer* m_planeBuffer;
   ID3D11InputLayout* m_inputLayout;
};

