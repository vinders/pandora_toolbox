/*******************************************************************************
Description : Example - camera with focal-point
Note : this example is fine for third-person views and fixed/rotating cameras,
       but not appropriate for first-person views.
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/camera.h>
  using CamProjection = pandora::video::d3d11::CameraProjection;
  using MatrixFloat4 = DirectX::XMMATRIX;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/camera.h>
# include <glm/glm.hpp>
  using CamProjection = pandora::video::vulkan::CameraProjection;
  using MatrixFloat4 = glm::mat4;
#else
# include <video/openGL4/camera.h>
  using CamProjection = pandora::video::openGL4::CameraProjection;
  using MatrixFloat4 = glm::mat4;
#endif


// Simple camera rotating around a focal-point
class Camera final {
public:
  Camera() noexcept { _computeView(); }
  Camera(const Camera&) = default;
  ~Camera() = default;

  // Initialize camera aspect-ratio + sensitivity
  Camera(uint32_t clientWidth, uint32_t clientHeight, float sensitivity) noexcept
	: _projection(clientWidth, clientHeight, 60.f), _sensitivity(sensitivity) {
	_computeView();
  }
  // Update settings
  void configure(uint32_t clientWidth, uint32_t clientHeight, float sensitivity) noexcept {
	_projection.setRatio(clientWidth, clientHeight);
	_sensitivity = sensitivity;
  }

  // Rotate camera view (raw X/Y motion)
  void move(float deltaX, float deltaY) noexcept {
	_pitch += deltaY * _sensitivity;
	_yaw += deltaX * _sensitivity;
	_computeView();
  }

  // -- accessors --

  MatrixFloat4 projectionMatrix() const noexcept { return _projection.projectionMatrix(); }
  MatrixFloat4 viewMatrix() const noexcept { return _viewMatrix; }
  const float* position() const noexcept { return _position; }


private:
  void _computeView() noexcept {
	if (_pitch > 89.f)
	  _pitch = 89.f;
	else if (_pitch < 1.f)
	  _pitch = 1.f;

#   if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT) // TMP (CameraView not yet implemented in toolbox)
	  DirectX::XMFLOAT3 basePosition(0.0f, 0.0f, -1.2f);
	  DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(_pitch), DirectX::XMConvertToRadians(_yaw), 0);
	  DirectX::XMVECTOR eyePos = XMVector3TransformCoord(DirectX::XMLoadFloat3(&basePosition), rotationMatrix);
	  
	  DirectX::XMFLOAT4 eyePosVec;
	  DirectX::XMStoreFloat4(&eyePosVec, eyePos);
	  _position[0] = eyePosVec.x; _position[1] = eyePosVec.y; _position[2] = eyePosVec.z; _position[3] = eyePosVec.w;
	  _viewMatrix = DirectX::XMMatrixLookAtLH(eyePos, DirectX::XMVectorSet(0.0f, 0.2f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
#   endif
  }

private:
  CamProjection _projection;
  MatrixFloat4 _viewMatrix;
  float _position[4];
  float _pitch = 32.f;
  float _yaw = 0.f;
  float _sensitivity = 0.25f;
};