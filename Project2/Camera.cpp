#include <cassert>
#include "Camera.h"

Camera::Camera(XMVECTOR position, XMVECTOR lookAt, XMVECTOR up) :
   m_pos(position), m_lookAt(lookAt), m_up(up), m_CameraDirty(TRUE) 
{
   assert(XMVectorGetX(XMVector3Length(up)) > .9 && XMVectorGetX(XMVector3Length(up)) < 1.1 );
   assert(XMVectorGetX(XMVector3Length(lookAt)) > .9 && XMVectorGetX(XMVector3Length(lookAt)) < 1.1 );

   m_left = XMVector3Normalize(XMVector3Cross(lookAt, up));
}

Camera::~Camera() {}

const XMMATRIX *Camera::GetViewMatrix() const
{
   if( m_CameraDirty )
   {
      m_viewMat = XMMatrixLookAtLH(m_pos, m_pos + m_lookAt, m_up);
   }

   m_CameraDirty = FALSE;
   return &m_viewMat;
}
   
void Camera::MoveCamera(XMVECTOR delta)
{
   m_pos = XMVectorAdd(XMVectorScale(m_lookAt, XMVectorGetZ(delta)), m_pos);
   m_pos = XMVectorAdd(XMVectorScale(m_left, XMVectorGetX(delta)), m_pos);
   m_pos = XMVectorAdd(XMVectorScale(m_up, XMVectorGetY(delta)), m_pos);
   m_CameraDirty = true;
}

void Camera::RotateCameraHorizontally(float radians)
{
   XMMATRIX rotateMat = XMMatrixRotationAxis(m_up, radians);

   // TODO: Is all this work necessary?
   m_lookAt = XMVector3Transform(m_lookAt, rotateMat);
   m_left = XMVector3Transform(m_left, rotateMat);
   m_CameraDirty = true;
}

void Camera::RotateCameraVertically(float radians)
{
   XMMATRIX rotateMat = XMMatrixRotationAxis(m_left, radians);

   // TODO: Is all this work necessary?
   m_lookAt = XMVector3Transform(m_lookAt, rotateMat);
   m_up = XMVector3Transform(m_up, rotateMat);
   m_CameraDirty = true;
}