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

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cstdint>
# include "./api/types.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        /// @class Viewport
        /// @brief Viewport - 2D window rectangle where 3D scene is projected
        /// @remarks Vulkan viewports are based on top-left corner.
        class Viewport final {
        public:
          Viewport() noexcept { ///< Empty viewport (size: 0; 0)
            _params.x=_params.y=_params.width=_params.height=_params.minDepth=0.f; _params.maxDepth=1.f;
          }
          Viewport(const Viewport&) = default;
          Viewport(Viewport&&) noexcept = default;
          Viewport& operator=(const Viewport&) = default;
          Viewport& operator=(Viewport&&) noexcept = default;
          ~Viewport() noexcept = default;
        
          /// @brief Create viewport (located at origin [0;0])
          Viewport(uint32_t width, uint32_t height) noexcept {
            _params.x=_params.y=_params.minDepth=0.f; _params.maxDepth=1.f;
            _params.width=(float)width; _params.height=(float)height;
          }
          /// @brief Create viewport (or fractional viewport) at specific position - native
          /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
          ///          - For API-independant coords, prefer 'fromTopLeft'/'fromBottomLeft' builders.
          Viewport(float x, float y, float width, float height, float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            _params.x=x; _params.y=y; _params.width=width; _params.height=height;
            _params.minDepth=nearClipping; _params.maxDepth=farClipping;
          }

          // ---

          /// @brief Build viewport from top-left coordinates (API-independant coords)
          static inline Viewport fromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height,
                                             float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            return Viewport((float)topLeftX, (float)topLeftY, (float)width, (float)height, nearClipping, farClipping);
          }
          /// @brief Build fractional viewport from top-left coordinates (API-independant coords)
          static inline Viewport fromTopLeft(float /*windowHeight*/, float topLeftX, float topLeftY, float width, float height,
                                             float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            return Viewport(topLeftX, topLeftY, width, height, nearClipping, farClipping);
          }

          /// @brief Build viewport from bottom-left coordinates (API-independant coords)
          static inline Viewport fromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height,
                                                float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            int32_t topY = (int32_t)windowHeight - bottomLeftY - (int32_t)height;
            return Viewport((float)bottomLeftX, (float)topY, (float)width, (float)height, nearClipping, farClipping);
          }
          /// @brief Build fractional viewport from bottom-left coordinates (API-independant coords)
          static inline Viewport fromBottomLeft(float windowHeight, float bottomLeftX, float bottomLeftY, float width, float height,
                                                float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            return Viewport(bottomLeftX, windowHeight - bottomLeftY - height, width, height, nearClipping, farClipping);
          }

          // -- operations --

          /// @brief Change viewport size (or fractional size) and position - native
          /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
          ///          - For API-independant coords, prefer 'resizeFromTopLeft'/'resizeFromBottomLeft' builders.
          inline void resize(float x, float y, float width, float height) noexcept { 
            _params.x = x; _params.y = y;
            _params.width = width; _params.height = height; 
          }
          /// @brief Change viewport size and position - from top-left coordinates (API-independant coords)
          inline void resizeFromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height) noexcept { 
            resize((float)topLeftX, (float)topLeftY, (float)width, (float)height);
          }
          /// @brief Change fractional viewport size and position - from top-left coordinates (API-independant coords)
          inline void resizeFromTopLeft(float /*windowHeight*/, float topLeftX, float topLeftY, float width, float height) noexcept { 
            resize(topLeftX, topLeftY, width, height);
          }

          /// @brief Change viewport size and position - from bottom-left coordinates (API-independant coords)
          inline void resizeFromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height) noexcept { 
            int32_t topY = (int32_t)windowHeight - bottomLeftY - (int32_t)height;
            resize((float)bottomLeftX, (float)topY, (float)width, (float)height);
          }
          /// @brief Change fractional viewport size and position - from bottom-left coordinates (API-independant coords)
          inline void resizeFromBottomLeft(float windowHeight, float bottomLeftX, float bottomLeftY, float width, float height) noexcept { 
            resize(bottomLeftX, windowHeight - bottomLeftY - height, width, height);
          }

          /// @brief Change viewport depth-clipping range
          inline void setDepthRange(float nearClipping, float farClipping) noexcept { 
            _params.minDepth=nearClipping; _params.maxDepth=farClipping;
          }

          // -- accessors --
        
          /// @brief Verify if x()/y() coords are based on top-left corner (true: D3D/Vulkan) or bottom-left corner (false: OpenGL)
          constexpr inline bool isFromTopLeft() const noexcept { return true; }
          constexpr inline float x() const noexcept { return this->_params.x; }  ///< X coord
          constexpr inline float y() const noexcept  { return this->_params.y; } ///< Y coord
          constexpr inline float width() const noexcept  { return this->_params.width; }  ///< Viewport width
          constexpr inline float height() const noexcept { return this->_params.height; } ///< Viewport height
        
          constexpr inline float nearClipping() const noexcept { return this->_params.minDepth; }///< Near clipping plane (min depth)
          constexpr inline float farClipping() const noexcept  { return this->_params.maxDepth; } ///< Far clipping plane (max depth)
        
          constexpr inline bool operator==(const Viewport& rhs) const noexcept {
            return (_params.x==rhs._params.x && _params.y==rhs._params.y
                 && _params.width==rhs._params.width       && _params.height==rhs._params.height 
                 && _params.minDepth==rhs._params.minDepth && _params.maxDepth==rhs._params.maxDepth);
          }
          constexpr inline bool operator!=(const Viewport& rhs) const noexcept { return !(this->operator==(rhs)); }
      
          inline VkViewport* descriptor() noexcept { return &(this->_params); } ///< Get native Vulkan descriptor
          inline const VkViewport* descriptor() const noexcept { return &(this->_params); } ///< Get native Vulkan descriptor
        
        private:
          VkViewport _params;
        };
      }
    }
  }
#endif
