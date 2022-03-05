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
--------------------------------------------------------------------------------
Direct3D11 - Buffer<ResourceUsage:: immutable/staticGpu/dynamicCpu/staging>
           - MappedBufferIO
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstdint>
# include "./_private/_resource_io.h" // includes D3D11
# include "./renderer.h" // includes D3D11
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
#   define __if_constexpr if constexpr
# else
#   define __if_constexpr if
# endif

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @class Buffer
        /// @brief Direct3D data buffer for shader stage(s): constant/uniform buffer, vertex array buffer, vertex index buffer...
        ///        * Immutable buffer (isImmutable==true): initialized at creation, then GPU read-only (fastest).
        ///           -> best option for buffers that never change.
        ///        * Static buffer (staticGpu): GPU read-write (very fast) / indirect CPU write-only (slow).
        ///           -> recommended for writable buffers rarely updated by the CPU, or with small data;
        ///           -> used for static meshes with skinning/skeletal animation in shaders.
        ///        * Dynamic buffer (dynamicCpu): GPU read-write (slow) / CPU write-only (very fast).
        ///           -> recommended for writable buffers updated for each frame (or more) by the CPU;
        ///           -> recommended for writable buffers with huge data transfers;
        ///           -> used for meshes with variable vertex count (usually terrains) and for per-vertex animation.
        ///        * Staging buffer: no GPU access, CPU read-write (fastest).
        ///           -> recommended for writable buffers accessed many times per frame by the CPU;
        ///           -> recommended for buffers with content that must be read by the CPU;
        ///           -> usually used with a static buffer: data is copied from one buffer to another, to allow CPU read/write operations (staging) and GPU usage (static).
        ///        * For more details about buffer types, see 'BufferType' in <video/api/types.h>.
        ///        * For more details about buffer usages, see 'ResourceUsage' in <video/api/types.h>.
        /// @remarks - To use it, bind it to the associated Renderer object (must be the same as the one used in constructor).
        ///          - Constant/uniform buffer data type size must be a multiple of 16 byte: add padding in structure/array-item if necessary.
        ///          - Static buffers are meant to be persistent: should be updated at most once per frame (and less than that if the buffer size is big).
        ///          - Dynamic buffers are meant to be updated often: at least once per frame (or close to it if the buffer size is big).
        ///          - Common practice: * geometry centered around (0;0;0) -> vertex buffers;
        ///                             * world matrix to offset the entire model in the environment -> combined with camera view into constant/uniform buffer;
        ///                             * vertices repositioned in vertex shader by world/view matrix and projection matrix.
        ///          - The following topics are only for big projects using hundreds of buffers:
        ///            - Suballocation in same memory block: not supported with Direct3D11.
        ///                * many GPU drivers limit the total number of allocations (usually 4096 or more allocations).
        ///                * managed automatically by Direct3D11 (but useful with lower-level APIs, such as Vulkan).
        ///            - Suballocation within buffer (cross-API):
        ///                * total number of buffers may also be limited, and using too many buffers can cause overhead and memory fragmentation.
        ///                * to reduce time spent on allocations and side effects, large buffers containing multiple data sets can be used.
        ///                * this can be done instead of creating many buffers for an allocation (both techniques can also be combined with lower-level APIs).
        ///                * a single buffer can use multiple binding types (example: BufferType::vertex | BufferType::vertexIndex).
        ///                  Note that most drivers do not support mixing vertices/indices with uniform data.
        ///                * for example, the same large buffer could contain vertices + indices for the same mesh.
        ///                * a single buffer can contain data for multiple meshes or multiple lights...
        ///                  (a sub-set of the buffer can be bound to the renderer, using 'byteOffset' and 'strideByteSize' params).
        ///                * when using suballocation for vertices/indices, align memory for every sub-component (aligned at 4*32bits blocks).
        ///                * when using suballocation for constants/uniforms, align memory for every sub-component (aligned at 16*4*32bits blocks).
        /// @warning Buffers do not guarantee the lifetime of the associated Renderer. They must be destroyed BEFORE destroying the Renderer!
        template <ResourceUsage _Usage>
        class Buffer final {
        public:
          using Type = Buffer<_Usage>;
        
          /// @brief Create buffer (to store data for shader stages) - undefined value
          /// @param renderer        The renderer for which the buffer is created: use the same renderer when binding it or writing in it.
          /// @param type            Type of buffer to create: constant/uniform buffer, vertex array buffer, vertex index buffer...
          ///                        Note: to use suballocation, the same buffer can use multiple binding types
          ///                        (example: BufferType::vertex | BufferType::vertexIndex).
          ///                        Warning: most drivers do not support mixing vertices/indices with constant/uniform data.
          /// @param bufferByteSize  The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant/uniform buffers.
          /// @param isBidirectional Copy operations can occur using this buffer both as a source and a destination.
          ///                        If 'isBidirectional'==false: * staticGpu buffers can only be destinations;
          ///                                                     * staging buffers can only be sources;
          ///                                                     * dynamicCpu buffers can't be any.
          ///                        Note: this flag is ignored with Direct3D11 (always bidirectional), but exists for cross-API projects.
          /// @warning - Static buffers: init/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          ///          - Immutable buffers: not allowed (initial value required).
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - runtime_error: on creation failure.
          inline Buffer(Renderer& renderer, BufferType type, size_t bufferByteSize, bool /*isBidirectional*/ = true)
            : _bufferSize(bufferByteSize), _context(renderer.context()), _type(type) {
            this->_handle = __createDataBuffer(renderer.device(), _getBindFlag(type), bufferByteSize,
                                               _getResourceUsageType(_Usage), _getResourceUsageCpuAccess(_Usage), nullptr);
          }
          /// @brief Create buffer (to store data for shader stages) with initial value
          /// @param renderer       The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type           Type of buffer to create: constant/uniform buffer, vertex array buffer, vertex index buffer...
          ///                       Note: to use suballocation, the same buffer can use multiple binding types
          ///                       (example: BufferType::vertex | BufferType::vertexIndex | BufferType::uniform).
          ///                       Warning: most drivers do not support mixing vertices/indices with constant/uniform data.
          /// @param bufferByteSize The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant/uniform buffers.
          /// @param initData       Buffer initial value -- structure or array of input values (must not be NULL if immutable).
          /// @param isBidirectional Copy operations can occur using this buffer both as a source and a destination.
          ///                        If 'isBidirectional'==false: * staticGpu buffers can only be destinations;
          ///                                                     * staging buffers can only be sources;
          ///                                                     * dynamicCpu buffers can't be any.
          ///                        Note: this flag is ignored with Direct3D11 (always bidirectional), but exists for cross-API projects.
          /// @warning Static/immutable buffers: init/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - runtime_error: on creation failure.
          inline Buffer(Renderer& renderer, BufferType type, size_t bufferByteSize, const void* initData, bool /*isBidirectional*/ = true)
            : _bufferSize(bufferByteSize), _context(renderer.context()), _type(type) {
            D3D11_SUBRESOURCE_DATA resourceData{ initData, 0, 0 };
            this->_handle = __createDataBuffer(renderer.device(), _getBindFlag(type), bufferByteSize,
                                               _getResourceUsageType(_Usage), _getResourceUsageCpuAccess(_Usage),
                                               initData ? &resourceData : nullptr);
          }
          
          /// @brief Create from native buffer handle -- reserved for advanced usage
          /// @warning Native usage must be the same as template type! (or write operations won't work)
          ///          immutable: D3D11_USAGE_IMMUTABLE; staticGpu: D3D11_USAGE_DEFAULT;
          ///          dynamicCpu: D3D11_USAGE_DYNAMIC; staging: D3D11_USAGE_STAGING;
          inline Buffer(Renderer& renderer, BufferHandle handle, size_t bufferSize, BufferType type) noexcept 
            : _handle(handle), _bufferSize(bufferSize), _context(renderer.context()), _type(type) {}
          
          Buffer() noexcept = default; ///< Empty buffer -- not usable (only useful to store variable not immediately initialized)
          Buffer(const Type&) = delete;
          Buffer(Type&& rhs) noexcept = default;
          Type& operator=(const Type&) = delete;
          Type& operator=(Type&& rhs) noexcept = default;
          ~Buffer() noexcept = default;

          inline void release() noexcept { _handle.release(); } ///< Destroy/release buffer instance
          
          // -- accessors --
          
          /// @brief Get native Direct3D 11 compatible buffer handle
          inline BufferHandle handle() const noexcept { return _handle.get(); }
          /// @brief Get pointer to native Direct3D 11 compatible buffer handle (usable as array of 1 item)
          inline const BufferHandle* handlePtr() const noexcept { return _handle.ptr(); }
          
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return !_handle.hasValue(); }
          inline size_t size() const noexcept { return _bufferSize; } ///< Get buffer byte size
          inline BufferType type() const noexcept { return _type; }   ///< Get buffer type: uniform/vertex/index
          
          static constexpr inline ResourceUsage usage() noexcept { return _Usage; } ///< Get buffer resource usage
          inline DeviceContext context() const noexcept { return _context; }        ///< Get parent context -- reserved for internal use
          
          // -- operations --
          
          /// @brief Copy content of another buffer resource.
          ///        Recommended usage: to copy staging buffer data to a static buffer (GPU memory).
          ///                           -> allows using data as a shader resource after preparing it in a staging buffer.
          /// @warning - Both buffers must be the same size (bufferByteSize) and the same type (BufferType).
          ///          - Both buffers must use compatible internal formats.
          ///          - Buffers can't be currently mapped (with a MappedBufferIO instance or using D3D11_MAPPED_SUBRESOURCE, for example).
          ///          - This command is not supported on immutable buffers.
          ///          - The renderer used with buffer constructor must still exist, or this may crash.
          template <ResourceUsage _RhsUsage>
          inline bool copy(Buffer<_RhsUsage>& source) noexcept {
            __if_constexpr (_Usage != ResourceUsage::immutable) {
              try {
                this->_context->CopyResource((ID3D11Resource*)_handle.get(), (ID3D11Resource*)source.handle());
                return true;
              }
              catch (...) {}
            }
            return false;
          }
          
          /// @brief Discard previous data and write buffer data. Can only be used to write the entire buffer.
          ///        To only write sub-parts of it, use MappedBufferIO instead (only for dynamic and staging buffers).
          /// @param sourceData  Structure/array of the same byte size as 'bufferByteSize' in constructor. Can't be NULL.
          /// @returns Memory access/update success
          /// @warning - 'sourceData' must be the same size as the buffer ('bufferByteSize').
          ///          - The buffer can't be currently mapped (with a MappedBufferIO instance or using D3D11_MAPPED_SUBRESOURCE, for example).
          ///          - The renderer used with buffer constructor must still exist, or this may crash.
          ///          - This command is not supported on immutable buffers.
          ///          - Make sure the 'sourceData' memory is properly aligned.
          bool write(const void* sourceData) noexcept;

        private:
          static constexpr inline UINT _getBindFlag(BufferType type) noexcept {
            return (_Usage != ResourceUsage::staging) ? (UINT)type : 0;
          }
        private:
          SharedResource<ID3D11Buffer> _handle;
          size_t _bufferSize = 0;
          DeviceContext _context = nullptr;
          BufferType _type = BufferType::uniform;
        };

        using ImmutableBuffer = Buffer<ResourceUsage::immutable>;
        using StaticBuffer = Buffer<ResourceUsage::staticGpu>;
        using DynamicBuffer = Buffer<ResourceUsage::dynamicCpu>;
        using StagingBuffer = Buffer<ResourceUsage::staging>;
        
        // ---
        
        /// @class MappedBufferIO
        /// @brief Mapped read/write data buffer access (constant/uniform, vertex, index)
        ///        Supports dynamic (write) and staging (read/write) buffers only.
        /// @warning The Buffer object must be kept alive while this instance is open.
        class MappedBufferIO final {
        public:
          /// @brief Open mapped data access for a dynamic buffer
          /// @params buffer  Dynamic buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Dynamic mapping mode:
          ///                 * discard: Discard previous data (as soon as GPU stops using it) -> content will be undefined.
          ///                            Recommended for each first write of a buffer for a new frame.
          ///                 * subsequent: Keep previous data (warning: data currently used by GPU must not be overwritten!).
          ///                               Recommended for subsequent writes of a buffer within the same frame.
          ///                 Not available in lower-level APIs (Vulkan...). Use ifdefs or ignore for cross-API projects.
          /// @throws runtime_error on failure
          inline MappedBufferIO(Buffer<ResourceUsage::dynamicCpu>& buffer, DynamicMapping mode = DynamicMapping::discard)
            : _context(buffer.context()), _buffer(buffer.handle()) {
            _open_construct(buffer.context(), buffer.handle(), (D3D11_MAP)mode);
          }
          /// @brief Open mapped data access for a staging buffer
          /// @params buffer  Staging buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Access mode: read / write / readWrite.
          /// @throws runtime_error on failure
          inline MappedBufferIO(Buffer<ResourceUsage::staging>& buffer, StagedMapping mode = StagedMapping::readWrite)
            : _context(buffer.context()), _buffer(buffer.handle()) {
            _open_construct(buffer.context(), buffer.handle(), (D3D11_MAP)mode);
          }
          
          MappedBufferIO() noexcept = default; ///< Empty -- not usable (only useful to store variable not immediately initialized)
          MappedBufferIO(const MappedBufferIO&) = delete;
          MappedBufferIO(MappedBufferIO&& rhs) noexcept
            : _mapped(rhs._mapped), _context(rhs._context), _buffer(rhs._buffer) { rhs._mapped.pData = nullptr; }
          MappedBufferIO& operator=(const MappedBufferIO&) = delete;
          MappedBufferIO& operator=(MappedBufferIO&& rhs) noexcept {
            _mapped=rhs._mapped; _context=rhs._context; _buffer=rhs._buffer;
            rhs._mapped.pData = nullptr;
            return *this;
          }
          ~MappedBufferIO() noexcept { close(); }
          
          
          // -- read/write access --
          
          inline bool isOpen() noexcept { return (this->_mapped.pData != nullptr); } ///< Get current state
          
          /// @brief Get mapped data access for read/write operations (or NULL if no access is open)
          ///        Can be cast to the structure/class type to read/write, or used with memcpy/memset operations.
          /// @warning Write operations should never exceed the size of the open buffer!
          inline void* data() noexcept { return this->_mapped.pData; }
          /// @brief Get mapped data access for read operations (or NULL if no access is open)
          ///        Can be cast to the structure/class type to read, or used with memcpy operations.
          /// @warning Only for staging buffers open with StagedMapping::read or StagedMapping::readWrite.
          inline const void* data() const noexcept { return this->_mapped.pData; }
          
          
          // -- operations --
        
          /// @brief Open mapped data access for a dynamic buffer
          /// @params buffer  Dynamic buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Dynamic mapping mode:
          ///                 * discard: Discard previous data (as soon as GPU stops using it) -> content will be undefined.
          ///                            Recommended for each first write of a buffer for a new frame.
          ///                 * subsequent: Keep previous data (warning: data currently used by GPU must not be overwritten!).
          ///                               Recommended for subsequent writes of a buffer within the same frame.
          ///                 Not available in lower-level APIs (Vulkan...). Use ifdefs or ignore for cross-API projects.
          /// @returns Success
          inline bool open(Buffer<ResourceUsage::dynamicCpu>& buffer, DynamicMapping mode = DynamicMapping::discard) noexcept {
            return _reopen(buffer.context(), buffer.handle(), (D3D11_MAP)mode);
          }
          /// @brief Open mapped data access for a staging buffer
          /// @params buffer  Staging buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Access mode: read / write / readWrite.
          /// @returns Success
          inline bool open(Buffer<ResourceUsage::staging>& buffer, StagedMapping mode = StagedMapping::readWrite) noexcept {
            return _reopen(buffer.context(), buffer.handle(), (D3D11_MAP)mode);
          }
          /// @brief Close mapped buffer access.
          /// @warning No other read/write operations can be performed until 'open' is called again.
          inline void close() noexcept {
            if (_mapped.pData != nullptr) {
              _context->Unmap((ID3D11Resource*)_buffer, 0);
              _mapped.pData = nullptr;
            }
          }
        
        private:
          void _open_construct(DeviceContext context, BufferHandle buffer, D3D11_MAP mode);
          bool _reopen(DeviceContext context, BufferHandle buffer, D3D11_MAP mode) noexcept;
        
        private:
          D3D11_MAPPED_SUBRESOURCE _mapped{ nullptr, 0, 0 };
          DeviceContext _context = nullptr;
          BufferHandle _buffer = nullptr;
        };
      }
    }
  }
  
  // ---
  
  template <> inline bool
  pandora::video::d3d11::Buffer<pandora::video::d3d11::ResourceUsage::staticGpu>::write(const void* sourceData) noexcept {
    try { _context->UpdateSubresource((ID3D11Resource*)_handle.get(), 0, nullptr, sourceData, 0, 0); }
    catch (...) { return false; }
    return true;
  }
  template <> inline bool
  pandora::video::d3d11::Buffer<pandora::video::d3d11::ResourceUsage::immutable>::write(const void*) noexcept {
    return false;
  }
  template <> inline bool
  pandora::video::d3d11::Buffer<pandora::video::d3d11::ResourceUsage::dynamicCpu>::write(const void* sourceData) noexcept {
    return __writeMappedDataBuffer(_context, _handle.get(), _bufferSize,
                                   D3D11_MAP_WRITE_DISCARD, sourceData);
  }
  template <> inline bool
  pandora::video::d3d11::Buffer<pandora::video::d3d11::ResourceUsage::staging>::write(const void* sourceData) noexcept {
    return __writeMappedDataBuffer(_context, _handle.get(), _bufferSize, D3D11_MAP_WRITE, sourceData);
  }

# undef __if_constexpr
#endif
