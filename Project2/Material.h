#include <d3d11.h>

class Material
{
public:
   ID3D11Buffer* m_materialConstantBuffer;
   ID3D11ShaderResourceView *m_texture;
};