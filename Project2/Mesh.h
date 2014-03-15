#include <d3d11.h>

class Mesh
{
public:
   ID3D11Buffer* m_vertexBuffer;
   ID3D11Buffer* m_indexBuffer;
   ID3D11Buffer* m_materialConstantBuffer;

   ID3D11ShaderResourceView *m_texture;

   unsigned int  m_numIndices;
};