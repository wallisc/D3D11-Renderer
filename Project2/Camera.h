#include <Windows.h>
#include <xnamath.h>

class Camera
{
public:
   Camera(XMVECTOR position, XMVECTOR lookAt, XMVECTOR up);
   ~Camera();

   const XMMATRIX *GetViewMatrix() const;
   void MoveCamera(XMVECTOR delta);
   void RotateCameraHorizontally(float radians);
   void RotateCameraVertically(float radians);

private:
   XMVECTOR m_pos;
   XMVECTOR m_lookAt;
   XMVECTOR m_up;
   XMVECTOR m_left;

   mutable XMMATRIX m_viewMat;
   mutable BOOL m_CameraDirty;
};