#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <cassert>

class RWRenderTarget
{
public:
   RWRenderTarget(ID3D11Device *pDevice, UINT width, UINT height)
   {
      D3D11_TEXTURE2D_DESC texDesc;
      texDesc.ArraySize = 1;
      texDesc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
      texDesc.CPUAccessFlags = 0;
      texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      texDesc.Height = height;
      texDesc.MipLevels = 1;
      texDesc.MiscFlags = 0;
      texDesc.SampleDesc.Count = 1;
      texDesc.SampleDesc.Quality = 0;
      texDesc.Usage = D3D11_USAGE_DEFAULT;
      texDesc.Width = width;
      
      // TODO: Check the results and error handle
      pDevice->CreateTexture2D(&texDesc, NULL, &pResource);

      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      srvDesc.Format = texDesc.Format;
      srvDesc.Texture2D.MipLevels = 1;
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;

      // TODO: Check the results and error handle
      pDevice->CreateShaderResourceView(pResource, &srvDesc, &pSrv);

      D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
      rtvDesc.Format = texDesc.Format;
      rtvDesc.Texture2D.MipSlice = 0;
      rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      
      // TODO: Check the results and error handle
      pDevice->CreateRenderTargetView(pResource, &rtvDesc, &pRtv);
   }

   ~RWRenderTarget()
   {
      pResource->Release();
      pRtv->Release();
      pSrv->Release();
   }

   ID3D11ShaderResourceView *GetShaderResourceView()
   {
      return pSrv;
   }

   ID3D11RenderTargetView *GetRenderTargetView()
   {
      return pRtv;
   }

private:
   ID3D11Texture2D *pResource;
   ID3D11RenderTargetView *pRtv;
   ID3D11ShaderResourceView *pSrv;
};