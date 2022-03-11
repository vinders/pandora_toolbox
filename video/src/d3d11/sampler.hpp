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


// -- sampler builder -- -------------------------------------------------------

  SamplerParams::SamplerParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    _params.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    _params.AddressU = _params.AddressV = _params.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    _params.MaxAnisotropy = 1;
    _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    _params.MinLOD = 1.f;
  }
  
  void SamplerParams::_init(const TextureWrap textureWrapUVW[3], float lodMin, float lodMax) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    textureWrap(textureWrapUVW);
    _params.MinLOD = lodMin;
    _params.MaxLOD = lodMax;
  }
  
  SamplerParams& SamplerParams::borderColor(const ColorChannel rgba[4]) noexcept {
    if (rgba)
      memcpy(&_params.BorderColor[0], &rgba[0], 4u*sizeof(ColorChannel));
    else
      memset(&_params.BorderColor[0], 0, 4u*sizeof(ColorChannel));
    return *this;
  }

  // ---

  // Create sampler filter state - can be used to change sampler filter state when needed (setSamplerState)
  SamplerState SamplerBuilder::create(const SamplerParams& params) {
    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler error");
    return SamplerState(stateData);
  }

#endif
