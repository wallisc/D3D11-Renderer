#ifndef _D3DBASE_H_
#define _D3DBASE_H_

#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>


class D3DBase 
{
public:
	D3DBase();
	virtual ~D3DBase();

	bool Initialize(HINSTANCE hInstance, HWND hwnd);
	void Shutdown();

	virtual bool LoadContent();
	virtual void UnloadContent();

	virtual void Update(FLOAT dt, BOOL *keyInputArray) = 0;
	virtual void Render() = 0;

protected:
   const static UINT NUM_SWAP_CHAIN_BUFFERS = 2;
	HINSTANCE m_hInstance;
	HWND m_hwnd;

	D3D_DRIVER_TYPE m_driverType;
	D3D_FEATURE_LEVEL m_featureLevel;

	ID3D11Device *m_d3dDevice;
	ID3D11DeviceContext *m_d3dContext;
	IDXGISwapChain *m_swapChain;
	ID3D11RenderTargetView *m_backBufferTarget;
   ID3D11Texture2D* m_DepthStencilBuffer;
   ID3D11DepthStencilView* m_DepthStencilView;

   UINT m_width;
   UINT m_height;
};

#endif //_D3DBASE_H_