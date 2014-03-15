#include "D3DBase.h"
#include "D3DCompiler.h"

D3DBase::D3DBase() : m_driverType(D3D_DRIVER_TYPE_NULL), m_featureLevel(D3D_FEATURE_LEVEL_11_0), 
					 m_d3dDevice(0), m_d3dContext(0), m_swapChain(0), m_backBufferTarget(0) 
{
}

D3DBase::~D3DBase() {
	Shutdown();
}

bool D3DBase::CompileD3DShader( char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer) 
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

bool D3DBase::LoadContent() 
{
	return true;
}

void D3DBase::UnloadContent() 
{
}

void D3DBase::Shutdown()
{
	UnloadContent();

	if (m_backBufferTarget) m_backBufferTarget->Release();
	if (m_swapChain) m_swapChain->Release();
	if (m_d3dContext) m_d3dContext->Release();
	if (m_d3dDevice) m_d3dDevice->Release();

	m_backBufferTarget = 0;
	m_swapChain = 0;
	m_d3dDevice = 0;
	m_d3dContext = 0;
}

bool D3DBase::Initialize(HINSTANCE hInstance, HWND hwnd) 
{
	m_hInstance = hInstance;
	m_hwnd = hwnd;

	RECT dim;
	GetClientRect(hwnd, &dim);

	m_width = dim.right - dim.left;
	m_height = dim.bottom - dim.top;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
   swapChainDesc.BufferCount = NUM_SWAP_CHAIN_BUFFERS;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator= 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	UINT creationFlags = 0;

#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT result;
	UINT driver = 0;

	for (driver = 0; driver < numDriverTypes; ++driver)
	{
		result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0,
			creationFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
			&swapChainDesc, &m_swapChain, &m_d3dDevice, &m_featureLevel, &m_d3dContext);
		if (SUCCEEDED(result))
		{
			m_driverType = driverTypes[driver];
			break;
		}
	}

	if (FAILED(result)) {
		DXTRACE_MSG("Failed to create the Direct3D device!");
		return false;
	}

	ID3D11Texture2D *backBufferTexture;
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

	if (FAILED(result)) 
	{
		DXTRACE_MSG("Failed to get the swap chain back buffer!");
		return false;
	}

	result = m_d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &m_backBufferTarget);

	if (backBufferTexture) backBufferTexture->Release();
	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to create the render target view!");
		return false;
	}

	D3D11_VIEWPORT view;
	view.Width = static_cast<FLOAT>(m_width);
	view.Height = static_cast<FLOAT>(m_height);
	view.MinDepth = 0.0f;
	view.MaxDepth = 1.0f;
	view.TopLeftX = 0.0f;
	view.TopLeftY = 0.0f;

	m_d3dContext->RSSetViewports(1, &view);

   D3D11_TEXTURE2D_DESC depthStencilDesc;
   depthStencilDesc.Width     = m_width;
   depthStencilDesc.Height    = m_height;
   depthStencilDesc.MipLevels = 1;
   depthStencilDesc.ArraySize = 1;
   depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthStencilDesc.SampleDesc.Count = 1;
   depthStencilDesc.SampleDesc.Quality = 0;
   depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
   depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
   depthStencilDesc.CPUAccessFlags = 0;
   depthStencilDesc.MiscFlags      = 0;

   result = m_d3dDevice->CreateTexture2D(
      &depthStencilDesc,             // Description of texture to create.
      0,
      &m_DepthStencilBuffer);       // Return pointer to depth/stencil buffer.

	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to create the depth stencil buffer!");
		return false;
	}

   result = m_d3dDevice->CreateDepthStencilView(
      m_DepthStencilBuffer,                // Resource we want to create a view to.
      0,
      &m_DepthStencilView);           // Return depth/stencil view

	if (FAILED(result))
	{
		DXTRACE_MSG("Failed to create the depth stencil view!");
		return false;
	}

	m_d3dContext->OMSetRenderTargets(1, &m_backBufferTarget, m_DepthStencilView);

	return LoadContent();
}