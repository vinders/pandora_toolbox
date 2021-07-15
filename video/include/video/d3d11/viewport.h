/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
#include <cstdint>
#include "./api/types.h"

namespace pandora {
  namespace video {
    namespace d3d11 {
      /// @class Viewport
      /// @brief Viewport - 2D window rectangle where 3D scene is projected
      /// @remarks Direct3D11 viewports are based on top-left corner.
      class Viewport final {
      public:
        Viewport() noexcept { ///< Empty viewport (size: 0; 0)
          _params.TopLeftX=_params.TopLeftY=_params.Width=_params.Height=_params.MinDepth=0.f; _params.MaxDepth=1.f;
        }
        Viewport(const Viewport&) = default;
        Viewport(Viewport&&) noexcept = default;
        Viewport& operator=(const Viewport&) = default;
        Viewport& operator=(Viewport&&) noexcept = default;
        ~Viewport() noexcept = default;
        
        /// @brief Create viewport (located at origin [0;0])
        Viewport(uint32_t width, uint32_t height, float nearClipping = 0.f, float farClipping = 1.f) noexcept {
          _params.TopLeftX=_params.TopLeftY=0.f; _params.Width=(FLOAT)width; _params.Height=(FLOAT)height;
          _params.MinDepth=(FLOAT)nearClipping; _params.MaxDepth=(FLOAT)farClipping;
        }
        /// @brief Create viewport at specific position - native
        /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
        ///          - For API-independant coords, prefer 'fromTopLeft'/'fromBottomLeft' builders.
        Viewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float nearClipping = 0.f, float farClipping = 1.f) noexcept {
          _params.TopLeftX=(FLOAT)x; _params.TopLeftY=(FLOAT)y; _params.Width=(FLOAT)width; _params.Height=(FLOAT)height;
          _params.MinDepth=(FLOAT)nearClipping; _params.MaxDepth=(FLOAT)farClipping;
        }

        /// @brief Build viewport from top-left coordinates (API-independant coords)
        static inline Viewport fromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height,
                                           float nearClipping = 0.f, float farClipping = 1.f) noexcept {
          return Viewport(topLeftX, topLeftY, width, height, nearClipping, farClipping);
        }
        /// @brief Build viewport from bottom-left coordinates (API-independant coords)
        static inline Viewport fromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height,
                                              float nearClipping = 0.f, float farClipping = 1.f) noexcept {
          return Viewport(bottomLeftX, (int32_t)windowHeight - bottomLeftY - (int32_t)height, width, height, nearClipping, farClipping);
        }

        // -- operations --

        /// @brief Change viewport size and position - native
        /// @warning - Native API x/y coords: from top-left corner on Direct3D/Vulkan, from bottom-left corner on OpenGL.
        ///          - For API-independant coords, prefer 'resizeFromTopLeft'/'resizeFromBottomLeft' builders.
        inline void resize(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept { 
          _params.TopLeftX = (FLOAT)x; _params.TopLeftY = (FLOAT)y;
          _params.Width = (FLOAT)width; _params.Height = (FLOAT)height; 
        }
        /// @brief Change viewport size and position - from top-left coordinates (API-independant coords)
        inline void resizeFromTopLeft(uint32_t /*windowHeight*/, int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height) noexcept { 
          resize(topLeftX, topLeftY, width, height);
        }
        /// @brief Change viewport size and position - from bottom-left coordinates (API-independant coords)
        inline void resizeFromBottomLeft(uint32_t windowHeight, int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height) noexcept { 
          resize(bottomLeftX, (int32_t)windowHeight - bottomLeftY - (int32_t)height, width, height);
        }
        

        /// @brief Change viewport depth-clipping range
        inline void setDepthRange(double nearClipping, double farClipping) noexcept { 
          _params.MinDepth=(FLOAT)nearClipping; _params.MaxDepth=(FLOAT)farClipping;
        }

        // -- accessors --
        
        /// @brief Verify if x()/y() coords are based on top-left corner (true: D3D/Vulkan) or bottom-left corner (false: OpenGL)
        constexpr inline bool isFromTopLeft() const noexcept { return true; }
        constexpr inline int32_t x() const noexcept { return (int32_t)this->_params.TopLeftX; }  ///< X coord
        constexpr inline int32_t y() const noexcept  { return (int32_t)this->_params.TopLeftY; } ///< Y coord
        constexpr inline uint32_t width() const noexcept  { return (uint32_t)this->_params.Width; }  ///< Viewport width
        constexpr inline uint32_t height() const noexcept { return (uint32_t)this->_params.Height; } ///< Viewport height
        
        constexpr inline float nearClipping() const noexcept { return (float)this->_params.MinDepth; }///< Near clipping plane (min depth)
        constexpr inline float farClipping() const noexcept  { return (float)this->_params.MaxDepth; } ///< Far clipping plane (max depth)
        
        constexpr inline bool operator==(const Viewport& rhs) const noexcept {
          return (_params.TopLeftX==rhs._params.TopLeftX && _params.TopLeftY==rhs._params.TopLeftY
               && _params.Width==rhs._params.Width       && _params.Height==rhs._params.Height 
               && _params.MinDepth==rhs._params.MinDepth && _params.MaxDepth==rhs._params.MaxDepth);
        }
        constexpr inline bool operator!=(const Viewport& rhs) const noexcept { return !(this->operator==(rhs)); }
      
        inline D3D11_VIEWPORT* descriptor() noexcept { return &(this->_params); } ///< Get native Direct3D descriptor
        inline const D3D11_VIEWPORT* descriptor() const noexcept { return &(this->_params); } ///< Get native Direct3D descriptor
        
      private:
        D3D11_VIEWPORT _params;
      };
    }
  }
}
#endif
