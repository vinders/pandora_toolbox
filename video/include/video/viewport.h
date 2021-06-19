/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>

namespace pandora {
  namespace video {
    /// @class Viewport
    /// @brief Viewport - 2D window rectangle to project 3D scene
    class Viewport final {
    public:
      /// @brief Create viewport (located at origin [0;0], default depth/clipping range)
      constexpr Viewport(uint32_t width, uint32_t height) : _width(width), _height(height) {}
      /// @brief Create viewport at position (default depth/clipping range)
      /// @warning Position from top-left corner on Direct3D/Vulkan and from bottom-left corner on OpenGL.
      constexpr Viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) 
        : _x(x), _y(y), _width(width), _height(height) {}
      /// @brief Create viewport at position - custom depth/clipping range
      /// @warning Position from top-left corner on Direct3D/Vulkan and from bottom-left corner on OpenGL.
      constexpr Viewport(int32_t x, int32_t y, uint32_t width, uint32_t height,
                         double nearClipping, double farClipping)
        : _x(x), _y(y), _width(width), _height(height), 
          _nearClipping(nearClipping), _farClipping(farClipping) {}
          
      Viewport() = default;
      Viewport(const Viewport&) = default;
      Viewport(Viewport&&) noexcept = default;
      Viewport& operator=(const Viewport&) = default;
      Viewport& operator=(Viewport&&) noexcept = default;
      ~Viewport() noexcept = default;
      
      // -- accessors --
      
      constexpr inline int32_t x() const noexcept { return this->_x; }  ///< X coord
      constexpr inline int32_t y() const noexcept  { return this->_y; } ///< Y coord
      constexpr inline uint32_t width() const noexcept  { return this->_width; }  ///< Viewport width
      constexpr inline uint32_t height() const noexcept { return this->_height; } ///< Viewport height
      
      constexpr inline double nearClipping() const noexcept { return this->_nearClipping; }///< Near clipping plane (min depth)
      constexpr inline double farClipping() const noexcept  { return this->_farClipping; } ///< Far clipping plane (max depth)
      
      constexpr inline bool operator==(const Viewport& rhs) const noexcept {
        return (_x==rhs._x && _y==rhs._y && _width==rhs._width && _height==rhs._height 
             && _nearClipping==rhs._nearClipping && _farClipping==rhs._farClipping);
      }
      constexpr inline bool operator!=(const Viewport& rhs) const noexcept { return !(this->operator==(rhs)); }
      
      // -- operations --

      /// @brief Change viewport top-left position
      inline void move(int32_t x, int32_t y) noexcept { 
        this->_x = x; this->_y = y;
      }          
      /// @brief Change viewport size
      inline void resize(uint32_t width, uint32_t height) noexcept { 
        this->_width = width; this->_height = height; 
      }
      /// @brief Change viewport size and top-left position
      inline void resize(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept { 
        this->_x = x; this->_y = y;
        this->_width = width; this->_height = height; 
      }

      /// @brief Change viewport depth/clipping range
      inline void setDepthRange(double nearClipping, double farClipping) noexcept { 
        this->_nearClipping = nearClipping;
        this->_farClipping = farClipping;
      }
      
    private:
      int32_t _x = 0;
      int32_t _y = 0;
      uint32_t _width = 640u;
      uint32_t _height = 480u;
      double _nearClipping = 0.0;
      double _farClipping = 1.0;
    };
    
    
    // -- API-specific viewport builders --
    
    /// @brief Viewport builder for Direct3D/Vulkan (top-left base) -- for template API abstraction, use Renderer::ViewportBuilder alias
    class TopBasedViewportBuilder final {
    public:
      /// @brief Build viewport from top-left coordinates
      static inline Viewport fromTopLeft(int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height,
                                         uint32_t /*windowHeight*/, double nearClipping = 0., double farClipping = 1.) {
        return Viewport(topLeftX, topLeftY, width, height, nearClipping, farClipping);
      }
      /// @brief Build viewport from bottom-left coordinates
      static inline Viewport fromBottomLeft(int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height,
                                            uint32_t windowHeight, double nearClipping = 0., double farClipping = 1.) {
        return Viewport(bottomLeftX, (int32_t)windowHeight - bottomLeftY - (int32_t)height, width, height, nearClipping, farClipping);
      }
    };
    /// @brief Viewport builder for OpenGL/OpenGLES (bottom-left base) -- for template API abstraction, use Renderer::ViewportBuilder alias
    class BottomBasedViewportBuilder final {
    public:
      /// @brief Build viewport from top-left coordinates
      static inline Viewport fromTopLeft(int32_t topLeftX, int32_t topLeftY, uint32_t width, uint32_t height,
                                         uint32_t windowHeight, double nearClipping = 0., double farClipping = 1.) {
        return Viewport(topLeftX, (int32_t)windowHeight - topLeftY - (int32_t)height, width, height, nearClipping, farClipping);
      }
      /// @brief Build viewport from bottom-left coordinates
      static inline Viewport fromBottomLeft(int32_t bottomLeftX, int32_t bottomLeftY, uint32_t width, uint32_t height,
                                            uint32_t /*windowHeight*/, double nearClipping = 0., double farClipping = 1.) {
        return Viewport(bottomLeftX, bottomLeftY, width, height, nearClipping, farClipping);
      }
    };
  }
}
