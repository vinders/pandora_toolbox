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
        /// @class DynamicBuffer
        /// @brief Direct3D dynamic data buffer for shader stage(s): constant buffer, vertex array buffer, vertex index buffer...
        ///        * Dynamic buffer: GPU read-write (slow) / CPU write-only (very fast).
        ///           -> recommended for writable buffers updated for each frame (or more) by the CPU;
        ///           -> recommended for writable buffers with huge data transfers;
        ///           -> used for meshes with variable vertex count (usually terrains) and for per-vertex animation.
        ///        * For small buffers that aren't updated too often, prefer StaticBuffer.
        ///        * For more details about buffer types, see 'DataBufferType' in <video/shader_types.h>.
        /// @remarks - To use it, bind it to the associated Renderer object (must be the same as the one used in constructor).
        ///          - Constant buffer data type size must be a multiple of 16 byte: add padding in structure/array-item if necessary.
        ///          - Dynamic buffers are meant to be updated often: at least once per frame (or less than that if the buffer size is big).
        ///          - Common practice: * geometry centered around (0;0;0) -> vertex buffers;
        ///                             * world matrix to offset the entire model in the environment -> combined with camera view into constant buffer;
        ///                             * vertices repositioned in vertex shader by world/view matrix and projection matrix.
        class DynamicBuffer final {
        public:
          /// @brief Create data buffer (to store data for shader stages)
          /// @param renderer       The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type           Type of buffer to create: constant buffer, vertex array buffer, vertex index buffer...
          /// @param bufferByteSize The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant buffers.
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - runtime_error: on creation failure.
          DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize);
          /// @brief Create data buffer (to store data for shader stages) with initial value
          /// @param renderer       The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type           Type of buffer to create: constant buffer, vertex array buffer, vertex index buffer...
          /// @param bufferByteSize The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes  for constant buffers.
          /// @param initData       Buffer initial value -- structure or array of input values.
          /// @throws - invalid_argument if 'bufferByteSize' is 0;
          ///         - runtime_error on creation failure.
          DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                       size_t bufferByteSize, const void* initData);
          
          /// @brief Create from native buffer handle - must use D3D11_USAGE_DYNAMIC for 'writeDiscard'/'write' to work!
          DynamicBuffer(Renderer::DataBufferHandle handle, size_t bufferSize, pandora::video::DataBufferType type) noexcept 
            : _buffer(handle), _bufferSize(bufferSize), _type(type) {}
          ~DynamicBuffer() noexcept { release(); }
          void release() noexcept { ///< Destroy/release dynamic buffer instance
            if (this->_buffer) {
              try { this->_buffer->Release(); } catch (...) {}
              this->_buffer = nullptr;
            }
          }
          
          DynamicBuffer() = default; ///< Empty buffer -- not usable (only useful to store variable not immediately initialized)
          DynamicBuffer(const DynamicBuffer&) = delete;
          DynamicBuffer(DynamicBuffer&& rhs) noexcept : _buffer(rhs._buffer), _bufferSize(rhs._bufferSize), _type(rhs._type) { rhs._buffer = nullptr; }
          DynamicBuffer& operator=(const DynamicBuffer&) = delete;
          DynamicBuffer& operator=(DynamicBuffer&& rhs) noexcept {
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

          /// @brief Write buffer data and discard previous data - recommended for first write of the buffer for a frame
          /// @param renderer    Renderer used in constructor.
          /// @param sourceData  Structure/array of the same byte size as 'bufferByteSize' in constructor.
          /// @returns Memory access/update success
          bool writeDiscard(Renderer& renderer, const void* sourceData);
          /// @brief Vertex/index buffers: write buffer data with no overwrite - recommended for subsequent writes of the buffer within same frame.
          ///        Constant buffers: same as 'writeDiscard'.
          /// @param renderer    Renderer used in constructor.
          /// @param sourceData  Structure/array of the same byte size as 'bufferByteSize' in constructor.
          /// @returns Memory access/update success
          bool write(Renderer& renderer, const void* sourceData);

        private:
          Renderer::DataBufferHandle _buffer = nullptr;
          size_t _bufferSize = 0;
          pandora::video::DataBufferType _type = pandora::video::DataBufferType::constant;
        };
      }
    }
  }

#endif
