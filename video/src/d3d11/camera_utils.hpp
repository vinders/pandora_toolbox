/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- camera projection -- -----------------------------------------------------

  // Compute shader projection matrix
  void CameraProjection::_computeProjection() noexcept {
    _constrainFieldOfView();
    float fovRad = DirectX::XMConvertToRadians(this->_fieldOfView);
    this->_projection = DirectX::XMMatrixPerspectiveFovLH(fovRad, this->_displayRatio, this->_nearPlane, this->_farPlane);
  }


// -- camera world/view helpers -- ---------------------------------------------

  MatrixFloat4x4 CameraUtils::computeWorldMatrix(float modelPosition[3], float yaw) noexcept {
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(modelPosition[0], modelPosition[1], modelPosition[2]);
    if (yaw) {
      DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f), DirectX::XMConvertToRadians(yaw));
      return translation * rotation;
    }
    return translation;
  }


//https://docs.microsoft.com/en-us/archive/msdn-magazine/2014/june/directx-factor-the-canvas-and-the-camera
//https://www.gamedev.net/tutorials/programming/graphics/directx-11-c-game-camera-r2978/
//https://github.com/Pindrought/DirectX-11-Engine-VS2017/tree/Tutorial_31/DirectX%2011%20Engine%20VS2017/DirectX%2011%20Engine%20VS2017/Graphics
//https://learnopengl.com/Getting-started/Camera

#endif
