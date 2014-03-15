#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <cassert>

template<typename DataType>
class ConstantBuffer
{
public:
   ConstantBuffer(ID3D11Device *pDevice)
   {
      D3D11_BUFFER_DESC constBufDesc;
      ZeroMemory(&constBufDesc, sizeof( constBufDesc ));
      constBufDesc.Usage = D3D11_USAGE_DEFAULT;
      constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      constBufDesc.ByteWidth = sizeof(DataType);

      // TODO: If there's an error here, the best thing to do is probably throw an exception
      HRESULT d3dresult = pDevice->CreateBuffer( &constBufDesc, NULL, &m_pConstantBuffer);
      
#ifdef _DEBUG
      ZeroMemory(&data, sizeof(data));
#endif
   }

   ~ConstantBuffer()
   {
      m_pConstantBuffer->Release();
   }

   const DataType *GetData() const
   {
      return &data;
   }

   void SetData(ID3D11DeviceContext* pContext, DataType *pNewData)
   {
      data = *pNewData;
      pContext->UpdateSubresource( m_pConstantBuffer, 0, 0, &data, 0, 0 );
   }

   ID3D11Buffer *GetConstantBuffer() const
   {
      return m_pConstantBuffer;
   }

private:
   ID3D11Buffer* m_pConstantBuffer;
   DataType data;
};