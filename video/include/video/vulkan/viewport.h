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
        class Viewport final : private VkViewport {
        public:
          Viewport() noexcept { ///< Empty viewport (size: 0; 0)
            x=y=width=height=minDepth=0.f; maxDepth=1.f;
          }
          Viewport(const Viewport&) = default;
          Viewport(Viewport&&) noexcept = default;
          Viewport& operator=(const Viewport&) = default;
          Viewport& operator=(Viewport&&) noexcept = default;
          ~Viewport() noexcept = default;
        
          /// @brief Create viewport (located at origin [0;0])
          Viewport(uint32_t sizeX, uint32_t sizeY) noexcept {
            x=y=minDepth=0.f; maxDepth=1.f;
            width=(float)sizeX; height=(float)sizeY;
          }
          /// @brief Create viewport (or fractional viewport) at specific position - native
          /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
          ///          - For API-independant coords, prefer 'fromTopLeft'/'fromBottomLeft' builders.
          Viewport(float coordX, float coordY, float sizeX, float sizeY, float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            x=coordX; y=coordY; width=sizeX; height=sizeY;
            minDepth=nearClipping; maxDepth=farClipping;
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
          inline void resize(float coordX, float coordY, float sizeX, float sizeY) noexcept { 
            x = coordX; y = coordY;
            width = sizeX; height = sizeY; 
          }
          /// @brief Change viewport size and position - from top-left coordinates (API-independant coords)
          inline void resizeFromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY, uint32_t sizeX, uint32_t sizeY) noexcept { 
            resize((float)topLeftX, (float)topLeftY, (float)sizeX, (float)sizeY);
          }
          /// @brief Change fractional viewport size and position - from top-left coordinates (API-independant coords)
          inline void resizeFromTopLeft(float /*windowHeight*/, float topLeftX, float topLeftY, float sizeX, float sizeY) noexcept { 
            resize(topLeftX, topLeftY, sizeX, sizeY);
          }

          /// @brief Change viewport size and position - from bottom-left coordinates (API-independant coords)
          inline void resizeFromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY, uint32_t sizeX, uint32_t sizeY) noexcept { 
            int32_t topY = (int32_t)windowHeight - bottomLeftY - (int32_t)sizeY;
            resize((float)bottomLeftX, (float)topY, (float)sizeX, (float)sizeY);
          }
          /// @brief Change fractional viewport size and position - from bottom-left coordinates (API-independant coords)
          inline void resizeFromBottomLeft(float windowHeight, float bottomLeftX, float bottomLeftY, float sizeX, float sizeY) noexcept { 
            resize(bottomLeftX, windowHeight - bottomLeftY - sizeY, sizeX, sizeY);
          }

          /// @brief Change viewport depth-clipping range
          inline void setDepthRange(float nearClipping, float farClipping) noexcept { 
            minDepth=nearClipping; maxDepth=farClipping;
          }

          // -- accessors --
        
          /// @brief Verify if coordX()/coordY() are based on top-left corner (true: D3D/Vulkan) or bottom-left corner (false: OpenGL)
          constexpr inline bool isFromTopLeft() const noexcept { return true; }
          constexpr inline float coordX() const noexcept { return this->x; }  ///< X coord
          constexpr inline float coordY() const noexcept  { return this->y; } ///< Y coord
          constexpr inline float sizeX() const noexcept  { return this->width; } ///< Viewport width
          constexpr inline float sizeY() const noexcept { return this->height; } ///< Viewport height
        
          constexpr inline float nearClipping() const noexcept { return this->minDepth; }///< Near clipping plane (min depth)
          constexpr inline float farClipping() const noexcept  { return this->maxDepth; } ///< Far clipping plane (max depth)
        
          constexpr inline bool operator==(const Viewport& rhs) const noexcept {
            return (x==rhs.x               && y==rhs.y
                 && width==rhs.width       && height==rhs.height 
                 && minDepth==rhs.minDepth && maxDepth==rhs.maxDepth);
          }
          constexpr inline bool operator!=(const Viewport& rhs) const noexcept { return !(this->operator==(rhs)); }
        };
      }
    }
  }
#endif
