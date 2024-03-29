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
# include "../api/types.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @brief Texel coordinates (position)
        class TexelPosition final {
        public:
          TexelPosition() noexcept = default;
          TexelPosition(uint32_t x) noexcept { _x=x; } ///< Create Texture1D texel position
          TexelPosition(uint32_t x, uint32_t y) noexcept { _x=x; _y=y; } ///< Create Texture2D texel position
          TexelPosition(uint32_t x, uint32_t y, uint32_t z) noexcept { _x=x; _y=y; _z=z; } ///< Create Texture3D texel position
          
          TexelPosition(const TexelPosition&) = default;
          TexelPosition& operator=(const TexelPosition&) = default;
          ~TexelPosition() noexcept = default;
          
          inline void setX(uint32_t x) noexcept { _x=x; } ///< set X position
          inline void setY(uint32_t y) noexcept { _y=y; } ///< set Y position (0 if not a 2D/3D texture)
          inline void setZ(uint32_t z) noexcept { _z=z; } ///< set Z position (0 if not a 3D texture)
          inline uint32_t getX() const noexcept { return _x; } ///< get X position
          inline uint32_t getY() const noexcept { return _y; } ///< get Y position
          inline uint32_t getZ() const noexcept { return _z; } ///< get Z position
          
        private:
          uint32_t _x = 0;
          uint32_t _y = 0;
          uint32_t _z = 0;
        };
        
        /// @brief Texture coordinates (region location)
        class TextureCoords final {
        public:
          TextureCoords() noexcept = default;
          TextureCoords(uint32_t x, uint32_t width) noexcept { _coords.left=x; _coords.right=x+width; } ///< Create Texture1D coords
          TextureCoords(uint32_t x, uint32_t y, uint32_t width, uint32_t height) noexcept { ///< Create Texture2D coords
            _coords.left=x; _coords.top=y; _coords.right=x+width; _coords.bottom=y+height;
          }
          TextureCoords(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth) noexcept { ///< Create Texture3D coords
            _coords.left=x; _coords.top=y; _coords.front=z;
            _coords.right=x+width; _coords.bottom=y+height; _coords.back=z+depth;
          }
          
          TextureCoords(const TextureCoords&) = default;
          TextureCoords& operator=(const TextureCoords&) = default;
          ~TextureCoords() noexcept = default;
          
          inline void setX(uint32_t x, uint32_t width) noexcept { _coords.left=x; _coords.right=x+width; }  ///< set X dimension
          inline void setY(uint32_t y, uint32_t height) noexcept { _coords.top=y; _coords.bottom=y+height; }///< set Y dimension
          inline void setZ(uint32_t z, uint32_t depth) noexcept { _coords.front=z; _coords.back=z+depth; }  ///< set Z dimension
          const D3D11_BOX& coords() const noexcept { return _coords; } ///< native Direct3D box coords
          
        private:
          D3D11_BOX _coords{ 0,0,0,1,1,1 };
        };
      }
    }
  }

#endif
