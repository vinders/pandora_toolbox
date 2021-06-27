/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "../shader_types.h"
# include "./renderer.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class StaticBuffer
        /// @brief Direct3D data buffer for shader stage(s): constant buffer, vertex array buffer, vertex index buffer...
        ///        * Static buffer: GPU read-write (very fast) / indirect CPU write-only (slow).
        ///           -> recommended for writable buffers rarely updated by the CPU, or with small data;
        ///           -> used for static meshes with skinning/skeletal animation in shaders.
        ///        * Immutable buffer (isImmutable==true): initialized at creation, then GPU read-only (fastest).
        ///           -> best option for buffers that never change.
        ///        * For buffers rewritten at least once per frame or with huge data transfers, prefer DynamicBuffer.
        ///        * For more details about buffer types, see 'DataBufferType' in <video/shader_types.h>.
        /// @remarks - To use it, bind it to the associated Renderer object (must be the same as the one used in constructor).
        ///          - Constant buffer data type size must be a multiple of 16 byte: add padding in structure/array-item if necessary.
        ///          - Static buffers are meant to be persistent: should be updated at most once per frame (and less than that if the buffer size is big).
        ///          - Common practice: * geometry centered around (0;0;0) -> vertex buffers;
        ///                             * world matrix to offset the entire model in the environment -> combined with camera view into constant buffer;
        ///                             * vertices repositioned in vertex shader by world/view matrix and projection matrix.
        class StaticBuffer final {
        public:
          /// @brief Create data buffer (to store data for shader stages)
          /// @param renderer       The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type           Type of buffer to create: constant buffer, vertex array buffer, vertex index buffer...
          /// @param bufferByteSize The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant buffers.
          /// @warning Initialization/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - runtime_error: on creation failure.
          StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize);
          /// @brief Create data buffer (to store data for shader stages) with initial value
          /// @param renderer       The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type           Type of buffer to create: constant buffer, vertex array buffer, vertex index buffer...
          /// @param bufferByteSize The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant buffers.
          /// @param initData       Buffer initial value -- structure or array of input values (must not be NULL if immutable)
          /// @param isImmutable    Buffer data is immutable (true) or can be modified (false)
          /// @warning Initialization/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          /// @throws - invalid_argument if 'bufferByteSize' is 0 / if immutable and 'initData' is NULL;
          ///         - runtime_error on creation failure.
          StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                       size_t bufferByteSize, const void* initData, bool isImmutable = false);
          
          /// @brief Create from native buffer handle - must use D3D11_USAGE_DEFAULT for 'write' to work!
          StaticBuffer(Renderer::DataBufferHandle handle, size_t bufferSize, pandora::video::DataBufferType type) noexcept 
            : _buffer(handle), _bufferSize(bufferSize), _type(type) {}
          ~StaticBuffer() noexcept { release(); }
          void release() noexcept { ///< Destroy/release static buffer instance
            if (this->_buffer) {
              try { this->_buffer->Release(); } catch (...) {}
              this->_buffer = nullptr;
            }
          }
          
          StaticBuffer() = default; ///< Empty buffer -- not usable (only useful to store variable not immediately initialized)
          StaticBuffer(const StaticBuffer&) = delete;
          StaticBuffer(StaticBuffer&& rhs) noexcept : _buffer(rhs._buffer), _bufferSize(rhs._bufferSize), _type(rhs._type) { rhs._buffer = nullptr; }
          StaticBuffer& operator=(const StaticBuffer&) = delete;
          StaticBuffer& operator=(StaticBuffer&& rhs) noexcept {
            release();
            this->_buffer = rhs._buffer; this->_bufferSize = rhs._bufferSize; this->_type = rhs._type;
            rhs._buffer = nullptr;
            return *this;
          }
        
          // -- accessors --
          
          /// @brief Get native Direct3D 11 compatible buffer handle
          inline Renderer::DataBufferHandle handle() const noexcept { return this->_buffer; }
          /// @brief Get pointer to native Direct3D 11 compatible buffer handle (usable as array of 1 item)
          inline const Renderer::DataBufferHandle* handleArray() const noexcept { return &(this->_buffer); }
          
          inline bool isEmpty() const noexcept { return (this->_buffer == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
          inline size_t size() const noexcept { return this->_bufferSize; } ///< Get buffer byte size
          pandora::video::DataBufferType type() const noexcept { return this->_type; } ///< Get buffer type: constant/vertex/index
          
          // -- operations --

          /// @brief Write buffer data (has no effect if buffer is immutable)
          /// @param renderer    Renderer used in constructor.
          /// @param sourceData  Structure/array of the same byte size as 'bufferByteSize' in constructor.
          inline void write(Renderer& renderer, const void* sourceData) {
            renderer.context()->UpdateSubresource(this->_buffer, 0, nullptr, sourceData, 0, 0);
          }

        private:
          Renderer::DataBufferHandle _buffer = nullptr;
          size_t _bufferSize = 0;
          pandora::video::DataBufferType _type = pandora::video::DataBufferType::constant;
        };
      }
    }
  }

#endif
