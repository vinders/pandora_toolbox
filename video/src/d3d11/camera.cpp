/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cmath>
# include "video/d3d11/camera.h"

  using namespace pandora::video::d3d11;
  

// -- camera projection -- -----------------------------------------------------

  // Compute shader projection matrix
  void CameraProjection::_computeProjection() noexcept {
    _constrainFieldOfView();
    float fovRad = DirectX::XMConvertToRadians(this->_fieldOfView);
    this->_projection = DirectX::XMMatrixPerspectiveFovLH(fovRad, this->_displayRatio, this->_nearPlane, this->_farPlane);
  }
  

// -- camera view helpers -- ---------------------------------------------------


#endif

//https://docs.microsoft.com/en-us/archive/msdn-magazine/2014/june/directx-factor-the-canvas-and-the-camera
//https://www.gamedev.net/tutorials/programming/graphics/directx-11-c-game-camera-r2978/
//https://github.com/Pindrought/DirectX-11-Engine-VS2017/tree/Tutorial_31/DirectX%2011%20Engine%20VS2017/DirectX%2011%20Engine%20VS2017/Graphics
//https://learnopengl.com/Getting-started/Camera
