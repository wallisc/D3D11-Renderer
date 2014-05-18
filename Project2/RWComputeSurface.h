#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <cassert>

class RWComputeSurface
{
public:
   RWComputeSurface(ID3D11Device *pDevice, UINT width, UINT height)
   {
      D3D11_TEXTURE2D_DESC texDesc;
      texDesc.ArraySize = 1;
      texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D10_BIND_SHADER_RESOURCE;
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

      D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
      memset(&uavDesc, 0, sizeof(uavDesc));

      uavDesc.Format = texDesc.Format;
      uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
      
      // TODO: Check the results and error handle
      pDevice->CreateUnorderedAccessView(pResource, &uavDesc, &pUav);
   }

   ~RWComputeSurface()
   {
      pResource->Release();
      pUav->Release();
      pSrv->Release();
   }

   ID3D11ShaderResourceView *GetShaderResourceView()
   {
      return pSrv;
   }

   ID3D11UnorderedAccessView *GetUnorderedAccessView()
   {
      return pUav;
   }

private:
   ID3D11Texture2D *pResource;
   ID3D11UnorderedAccessView *pUav;
   ID3D11ShaderResourceView *pSrv;
};