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
        /// @class Viewport
        /// @brief Viewport - 2D window rectangle where 3D scene is projected
        /// @remarks Direct3D11 viewports are based on top-left corner.
        class Viewport final : private D3D11_VIEWPORT {
        public:
          Viewport() noexcept { ///< Empty viewport (size: 0; 0)
            TopLeftX=TopLeftY=Width=Height=MinDepth=0.f; MaxDepth=1.f;
          }
          Viewport(const Viewport&) = default;
          Viewport(Viewport&&) noexcept = default;
          Viewport& operator=(const Viewport&) = default;
          Viewport& operator=(Viewport&&) noexcept = default;
          ~Viewport() noexcept = default;
        
          /// @brief Create viewport (located at origin [0;0])
          Viewport(uint32_t width, uint32_t height) noexcept {
            TopLeftX=TopLeftY=MinDepth=0.f; MaxDepth=1.f;
            Width=(FLOAT)width; Height=(FLOAT)height;
          }
          /// @brief Create viewport (or fractional viewport) at specific position - native
          /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
          ///          - For API-independant coords, prefer 'fromTopLeft'/'fromBottomLeft' builders.
          Viewport(float x, float y, float width, float height, float nearClipping = 0.f, float farClipping = 1.f) noexcept {
            TopLeftX=(FLOAT)x; TopLeftY=(FLOAT)y; Width=(FLOAT)width; Height=(FLOAT)height;
            MinDepth=(FLOAT)nearClipping; MaxDepth=(FLOAT)farClipping;
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
            TopLeftX = (FLOAT)x; TopLeftY = (FLOAT)y;
            Width = (FLOAT)width; Height = (FLOAT)height; 
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
            MinDepth=(FLOAT)nearClipping; MaxDepth=(FLOAT)farClipping;
          }

          // -- accessors --
        
          /// @brief Verify if coordX()/coordY() coords are based on top-left corner (true: D3D/Vulkan) or bottom-left corner (false: OpenGL)
          constexpr inline bool isFromTopLeft() const noexcept { return true; }
          constexpr inline float coordX() const noexcept { return (float)this->TopLeftX; }  ///< X coord
          constexpr inline float coordY() const noexcept  { return (float)this->TopLeftY; } ///< Y coord
          constexpr inline float sizeX() const noexcept  { return (float)this->Width; } ///< Viewport width
          constexpr inline float sizeY() const noexcept { return (float)this->Height; } ///< Viewport height
        
          constexpr inline float nearClipping() const noexcept { return (float)this->MinDepth; }///< Near clipping plane (min depth)
          constexpr inline float farClipping() const noexcept  { return (float)this->MaxDepth; } ///< Far clipping plane (max depth)
        
          constexpr inline bool operator==(const Viewport& rhs) const noexcept {
            return (TopLeftX==rhs.TopLeftX && TopLeftY==rhs.TopLeftY
                 && Width==rhs.Width       && Height==rhs.Height 
                 && MinDepth==rhs.MinDepth && MaxDepth==rhs.MaxDepth);
          }
          constexpr inline bool operator!=(const Viewport& rhs) const noexcept { return !(this->operator==(rhs)); }
        };
      }
    }
  }
#endif
