#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <cassert>

template<typename Data>
class RWStructuredBuffer
{
public:
   RWStructuredBuffer(ID3D11Device *pDevice, UINT maxElements)
   {
      D3D11_BUFFER_DESC bufDesc;
      
      memset(&bufDesc, 0, sizeof(bufDesc));
      bufDesc.Usage = D3D11_USAGE_DEFAULT;
      bufDesc.ByteWidth = sizeof(Data) * maxElements;
      bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
      bufDesc.CPUAccessFlags = 0;
      bufDesc.StructureByteStride = sizeof(Data);
      bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

      pDevice->CreateBuffer(&bufDesc, NULL, &pResource);

      D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
      memset(&uavDesc, 0, sizeof(uavDesc));
      uavDesc.Format = DXGI_FORMAT_UNKNOWN;
      uavDesc.Buffer.FirstElement = 0;
      uavDesc.Buffer.NumElements = maxElements;
      uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
      uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
      
      // TODO: Check the results and error handle
      pDevice->CreateUnorderedAccessView(pResource, &uavDesc, &pUav);
   
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
      memset(&srvDesc, 0, sizeof(srvDesc));
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
      srvDesc.Format = DXGI_FORMAT_UNKNOWN;
      srvDesc.Buffer.ElementWidth = maxElements;
      
      pDevice->CreateShaderResourceView(pResource, &srvDesc, &pSrv);
   }

   ~RWStructuredBuffer()
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
   ID3D11Buffer *pResource;
   ID3D11UnorderedAccessView *pUav;
   ID3D11ShaderResourceView *pSrv;
};