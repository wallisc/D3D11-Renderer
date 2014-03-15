#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>

#include "D3DCompiler.h"

#define HM( e, m )                \
{                                 \
   HRESULT d3dResult = e;         \
   if( FAILED( d3dResult ))       \
   {                              \
      DXTRACE_MSG( m );           \
      return false;               \
   }                              \
}

#define HR( e ) \
   HM( e, "Error has occured: ##e" );

class D3DUtils
{
public:

   static FORCEINLINE HRESULT CreatePixelShader(
      ID3D11Device *pDevice,
      char* shaderFile, 
      char* shaderFunctionName,
      char* shaderCompiler,
      ID3D11PixelShader **ppPixelShader)
   {
      ID3DBlob* psBuffer = 0;
      BOOL compileResult = CompileD3DShader( shaderFile, shaderFunctionName, shaderCompiler, &psBuffer);

      if( compileResult == false )
      {
         MessageBox(0, "Error loading pixel shader!", "Compile Error", MB_OK);
         return false;
      }

      HRESULT result = pDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, ppPixelShader);

      psBuffer->Release();
      return result;
   }

   static FORCEINLINE BOOL CompileD3DShader( char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer) 
   {
       DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
   #if defined( DEBUG ) || defined( _DEBUG )
       shaderFlags |= D3DCOMPILE_DEBUG;
   #endif

       ID3DBlob* errorBuffer = 0;
      HRESULT result;
   
      result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, 0, 0, buffer, &errorBuffer, 0);
   
      if( FAILED( result ) )
      {
          if(errorBuffer != 0 )
          {
              OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
              errorBuffer->Release();
          }
          return false;
      }

       if(errorBuffer != 0 ) errorBuffer->Release();
       return true;
   }
};