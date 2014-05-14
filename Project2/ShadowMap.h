#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <cassert>
#pragma once

class ShadowMap
{
public:
   ShadowMap(ID3D11Device *pDevice, UINT width, UINT height) :
      m_width(width), m_height(height), m_pSrv(NULL), m_pDsv(NULL)
   {
      m_viewport.TopLeftY = m_viewport.TopLeftX = 0;
      m_viewport.Height = (FLOAT)m_height;
      m_viewport.Width = (FLOAT)m_width;
      m_viewport.MinDepth = 0.0f;
      m_viewport.MaxDepth = 1.0f;

      D3D11_TEXTURE2D_DESC texDesc;
      texDesc.Width = m_width;
      texDesc.Height = m_height;
      texDesc.MipLevels = 1;
      texDesc.ArraySize = 1;
      texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
      texDesc.SampleDesc.Count = 1;
      texDesc.SampleDesc.Quality = 0;
      texDesc.Usage = D3D11_USAGE_DEFAULT;
      texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
      texDesc.CPUAccessFlags = 0;
      texDesc.MiscFlags = 0;

      ID3D11Texture2D *depthMap = 0;
      assert(pDevice->CreateTexture2D(&texDesc, 0, &depthMap) == S_OK);
      D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
      dsvDesc.Flags = 0;
      dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
      dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      dsvDesc.Texture2D.MipSlice = 0;
      assert(pDevice->CreateDepthStencilView(depthMap, &dsvDesc, &m_pDsv) == S_OK);

      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      assert(pDevice->CreateShaderResourceView(depthMap, &srvDesc, &m_pSrv) == S_OK);

      // View saves a reference to the texture so we can release our
      // reference.
      depthMap->Release();  
   }

   ~ShadowMap()
   {
      m_pSrv->Release();
      m_pDsv->Release();
   }

   ID3D11ShaderResourceView *GetShaderResourceView() const
   {
      return m_pSrv;
   }

   ID3D11DepthStencilView *GetDepthStencilView() const
   {
      return m_pDsv;
   }
   
   const D3D11_VIEWPORT *GetViewport() const
   {
      return &m_viewport;
   }

private:
   ID3D11ShaderResourceView *m_pSrv;
   ID3D11DepthStencilView *m_pDsv;
   D3D11_VIEWPORT m_viewport;
   UINT m_width, m_height;
};