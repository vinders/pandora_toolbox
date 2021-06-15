/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include <cstddef>
# include "./renderer.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class ConstantBuffer
        /// @brief Direct3D constant buffer data for shader stage(s)
        /// @remarks - To use it, bind it with a shader stage (Renderer.bind<...>ConstantBuffers).
        ///          - Data type size must be a multiple of 16 byte: add padding in structure if necessary.
        ///          - Constant buffer data is meant to be persistent: shouldn't be changed at most once per frame.
        ///          - Constant buffers should contain data that is useful as a whole for shaders:
        ///            -> they're entirely copied in GPU cache during Draw calls, to be available for each vertex/pixel.
        ///            -> recommended for position/projection/transformation info and options.
        ///            -> not appropriate for big data blocks of which only a few bytes are read by each vertex/pixel.
        class ConstantBuffer final {
        public:
          /// @brief Create constant buffer (to store data for/from shader stages)
          /// @param renderer    The renderer for which the buffer is created: use the same renderer when binding it or when calling read/write.
          /// @param bufferSize  The total size of the buffer (usually sizeof of the source data type) -- must be a multiple of 16 bytes
          /// @warning Data writing is a LOT more efficient when the source data type (and each sub-type) has a 16-byte alignment (see <system/align.h>).
          /// @throws - invalid_argument if bufferSize is 0
          ///         - runtime_error on creation failure (example: if 'bufferSize' is not a multiple of 16)
          ConstantBuffer(Renderer& renderer, size_t bufferSize);
          /// @brief Create constant buffer (to store data for/from shader stages) with initial value
          /// @param renderer    The renderer for which the buffer is created: use the same renderer when binding it or when calling read/write.
          /// @param bufferSize  The total size of the buffer (usually sizeof of the data type of 'initData') -- must be a multiple of 16 bytes
          /// @param isImmutable Buffer data is immutable (true) or can be modified (false)
          /// @param initData    Buffer initial value (must not be NULL if isImmutable==true)
          /// @warning Initialization/writing is a LOT more efficient when the source data type (and each sub-type) has a 16-byte alignment (see <system/align.h>).
          /// @throws - invalid_argument if bufferSize is 0
          ///         - runtime_error on creation failure (example: if 'bufferSize' is not a multiple of 16)
          ConstantBuffer(Renderer& renderer, size_t bufferSize, const void* initData, bool isImmutable = false);
          ~ConstantBuffer() noexcept { release(); }
          /// @brief Destroy constant buffer
          void release() noexcept;
          
          ConstantBuffer() = default; ///< Empty constant buffer -- not usable (only useful to store variable not immediately initialized)
          ConstantBuffer(const ConstantBuffer&) = delete;
          ConstantBuffer(ConstantBuffer&& rhs) noexcept;
          ConstantBuffer& operator=(const ConstantBuffer&) = delete;
          ConstantBuffer& operator=(ConstantBuffer&& rhs) noexcept;
        
          // -- accessors --
          
          /// @brief Get native Direct3D 11 compatible constant buffer handle (cast to ID3D11Buffer*)
          inline Renderer::ConstantBufferHandle handle() const noexcept { return this->_constantBuffer; }
          /// @brief Get pointer to native Direct3D 11 compatible constant buffer handle (usable as array of 1 item)
          inline const Renderer::ConstantBufferHandle* handleArray() const noexcept { return &(this->_constantBuffer); }
          inline bool isEmpty() const noexcept { return (this->_constantBuffer == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)

          // -- operations --

          /// @brief Write buffer data (not supported with immutable mode)
          /// @warning - Param 'sourceData' must be an object/structure/array of the same byte size as 'bufferSize' in constructor.
          ///          - Writing is a LOT more efficient when the source data type (and each sub-type) has a 16-byte alignment (see <system/align.h>).
          void write(Renderer& renderer, const void* sourceData);

        private:
          Renderer::ConstantBufferHandle _constantBuffer = nullptr; // ID3D11Buffer*
        };
      }
    }
  }

#endif
