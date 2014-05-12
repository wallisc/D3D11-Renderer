#include <d3d11.h>

class Mesh
{
public:
   ID3D11Buffer* m_vertexBuffer;
   ID3D11Buffer* m_indexBuffer;
   UINT m_MaterialIndex;

   unsigned int  m_numIndices;
};