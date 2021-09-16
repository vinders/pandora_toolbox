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
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./api/types.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class ScissorRectangle
        /// @brief Boundaries for scissor test: pixels located out of the rectangle are cut.
        ///        The scissor-test occurs after the fragment shader, to remove pixels out of bounds.
        class ScissorRectangle final {
        public:
          ScissorRectangle() noexcept { ///< Empty rectangle (size: 0; 0)
            _params.left = _params.top = _params.right = _params.bottom = 0;
          }
          ScissorRectangle(const ScissorRectangle&) = default;
          ScissorRectangle(ScissorRectangle&&) noexcept = default;
          ScissorRectangle& operator=(const ScissorRectangle&) = default;
          ScissorRectangle& operator=(ScissorRectangle&&) noexcept = default;
          ~ScissorRectangle() noexcept = default;

          /// @brief Create scissor rectangle - native
          /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
          ///          - For API-independant coords, prefer 'fromTopLeft'/'fromBottomLeft' builders.
          ScissorRectangle(int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height) noexcept {
            _params.left = (LONG)offsetX;
            _params.top = (LONG)offsetY;
            _params.right = (LONG)offsetX + (LONG)width;
            _params.bottom = (LONG)offsetY + (LONG)height;
          }

          // ---

          /// @brief Build viewport from top-left coordinates (API-independant coords)
          static inline ScissorRectangle fromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY,
                                                     uint32_t width, uint32_t height) noexcept {
            return ScissorRectangle(topLeftX, topLeftY, width, height);
          }
          /// @brief Build viewport from bottom-left coordinates (API-independant coords)
          static inline ScissorRectangle fromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY,
                                                        uint32_t width, uint32_t height) noexcept {
            return ScissorRectangle(bottomLeftX, (int32_t)windowHeight - bottomLeftY - (int32_t)height, width, height);
          }

          // -- accessors --

          constexpr inline int32_t x() const noexcept { return (int32_t)this->_params.left; } ///< X coord
          constexpr inline int32_t y() const noexcept  { return (int32_t)this->_params.top; } ///< Y coord
          constexpr inline uint32_t width() const noexcept  { return (uint32_t)this->_params.right - (uint32_t)this->_params.left; } ///< Rectangle width
          constexpr inline uint32_t height() const noexcept { return (uint32_t)this->_params.bottom - (uint32_t)this->_params.top; } ///< Rectangle height

          constexpr inline bool operator==(const ScissorRectangle& rhs) const noexcept {
            return (_params.left==rhs._params.left   && _params.top==rhs._params.top
                 && _params.right==rhs._params.right && _params.bottom==rhs._params.bottom);
          }
          constexpr inline bool operator!=(const ScissorRectangle& rhs) const noexcept { return !(this->operator==(rhs)); }

          inline D3D11_RECT* descriptor() noexcept { return &(this->_params); } ///< Get native Direct3D descriptor
          inline const D3D11_RECT* descriptor() const noexcept { return &(this->_params); } ///< Get native Direct3D descriptor

        private:
          D3D11_RECT _params;
        };
      }
    }
  }
#endif
