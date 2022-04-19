/*******************************************************************************
MIT License
Copyright (c) 2022 Romain Vinders

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
Vulkan - Buffer<ResourceUsage::local/dynamic/staging>
       - Buffer<ResourceUsage::local/dynamic/staging>::Builder
       - BufferParams<ResourceUsage::local/dynamic/staging>
       - MappedBufferIO
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cassert>
# include <cstdint>
# include <cstring>
# include <type_traits>
# include "video/vulkan/_private/_buffer.h" // includes vulkan
# include "video/vulkan/_private/_memory.h" // includes vulkan
# include "video/vulkan/_private/_shared_resource.h" // includes vulkan
# include "./device_memory_pool.h"
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
#   define __if_constexpr          if constexpr
#   define __move_cpp14_only(obj)  obj // no move to avoid copy ellision
# else
#   define __if_constexpr          if
#   define __move_cpp14_only(obj)  std::move(obj)
# endif

  namespace pandora {
    namespace video {
      namespace vulkan {
        // -- buffer params --
        
        /// @class BufferParams
        /// @brief Data/storage buffer configuration (vertex/index/uniform data for shader stages)
        /// @remarks The same BufferParams can be used to build multiple Buffer instances (if needed).
        /// @tparam _Usage Buffer memory usage:
        ///                * Local/static: GPU read-write (fastest) / indirect CPU write-only (slow).
        ///                   -> meant to be persistent: should be updated at most once per frame (or less if big size);
        ///                   -> best option for buffers that never change (fixed geometry...).
        ///                   -> can be copied from compatible staging buffer: preferable to update large buffers.
        ///                   -> used for static meshes with skinning/skeletal animation in shaders;
        ///                   -> 'specialMemUsage': set immutable buffer (read-only).
        ///                * Dynamic: GPU read-write (slow) / CPU write-only (fast).
        ///                   -> meant to be updated often by the CPU: at least once per frame;
        ///                   -> recommended for writable buffers with huge data transfers;
        ///                   -> used for meshes with variable vertex count (dynamic terrains) and for vertex animation;
        ///                   -> 'specialMemUsage': use GPU VRAM heap with direct CPU access (if available).
        ///                * Staging: CPU-only read-write (very fast).
        ///                   -> recommended for readable/writable buffers accessed many times per frame by the CPU;
        ///                   -> recommended for buffers with content that must be read by the CPU;
        ///                   -> usually used with a static buffer: data is copied from one buffer to another;
        ///                   -> 'specialMemUsage': use CPU cache (fastest linear access, slower for random access).
        ///                * For more details about memory usages, see 'ResourceUsage' in <video/api/types.h>.
        template <ResourceUsage _Usage>
        class BufferParams final {
        public:
          using Type = BufferParams<_Usage>;
        
          /// @brief Initialize buffer params
          /// @param type  Type of buffer to create:
          ///              * uniform buffer: global information shared between vertices (material, camera, lighting...).
          ///              * vertex array buffer: mesh data storage (vertex position, normal, color...).
          ///              * vertex index buffer: vertex ordering/indexing for associated vertex array buffer.
          ///              * For more details about buffer types, see 'BufferType' in <video/api/types.h>.
          ///              Note: - vertex/index binding types can usually be combined (logical OR operator);
          ///                    - most drivers do not support mixing vertices/indices with uniform data;
          ///                    - the buffer type will be ignored for staging buffers (not typed).
          /// @param bufferByteSize  Total number of bytes (sizeof structure/array)
          ///                        Must be a multiple of 16 bytes for uniform buffers!
          /// @param specialMemUsage Special memory usage (depending on template argument '_Usage'):
          ///                        * local: immutable buffer (read-only):
          ///                            - ideal for static resources that don't change (textures, geometry...);
          ///                            - ideal for large random-access resources (such as textures);
          ///                            - note: with Vulkan, the immutable flag has no effect.
          ///                        * dynamic: special VRAM heap with DMA/direct CPU access (if available):
          ///                            - fast access for small buffers from both CPU and GPU;
          ///                            - recommended for small uniform buffers updated often;
          ///                            - many GPUs provide up to 256MB of such memory.
          ///                        * staging: use CPU cache:
          ///                            - faster linear CPU read/write access: ideal for small data used with 'memcpy';
          ///                            - slow CPU random access: would cause cache misses and be much slower!
          /// @param transferMode    * bidirectional: copy operations can use this buffer as source and destination.
          ///                        * standard (recommended):
          ///                            - Static usage: only destination;
          ///                            - Staging usage: only source;
          ///                            - Dynamic usage: no copy allowed.
          /// @warning Writing is MUCH more efficient when source data type uses 16-byte alignment (see <system/align.h>).
          inline BufferParams(BufferType type, size_t bufferByteSize, bool specialMemUsage = false,
                              TransferMode transferMode = TransferMode::standard) noexcept
            : _preferredUsage(_toPreferredMemoryUsageFlags(_Usage, specialMemUsage)) {
            _params.size = (VkDeviceSize)bufferByteSize;
            _params.usage = _toBufferUsageFlags(type, _Usage, transferMode);
            _params.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
          }
          
          BufferParams() noexcept = default; ///< Empty buffer params -- not usable as is
          BufferParams(const Type& rhs) noexcept = default;
          Type& operator=(const Type& rhs) noexcept = default;
          ~BufferParams() noexcept = default;
          
          // -- buffer type and size --
          
          /// @brief Set buffer type, memory usage and transfer mode
          /// @param type            Type of buffer to create (see class constructor).
          /// @param specialMemUsage Local=>immutable / Dynamic=>DMA / Staging=>cached (see class constructor).
          /// @param transferMode    Bidirectional / standard (see class constructor).
          inline Type& type(BufferType type, bool specialMemUsage = false,
                                    TransferMode transferMode = TransferMode::standard) noexcept {
            _params.usage = _toBufferUsageFlags(type, _Usage, transferMode);
            _preferredUsage = _toPreferredMemoryUsageFlags(_Usage, specialMemUsage);
            return *this;
          }
          /// @brief Set buffer size (bytes)
          /// @param bufferByteSize  Total number of bytes (sizeof structure/array)
          ///                        Must be a multiple of 16 bytes for uniform buffers!
          /// @warning Writing is MUCH more efficient when source data type uses 16-byte alignment (see <system/align.h>).
          inline Type& size(size_t bufferByteSize) noexcept {
            _params.size = (VkDeviceSize)bufferByteSize;
            return *this;
          }
          
          // -- advanced features --
          
          /// @brief Set advanced API-specific flags
          /// @warning For cross-API projects, avoid this method.
          inline Type& specialFlags(VkBufferCreateFlags flags) noexcept {
            _params.flags = flags;
            return *this;
          }
          /// @brief Set context sharing mode (exclusive / concurrent)
          /// @param concurrentQueueFamilies Concurrent sharing mode: array of queue family indices with access to buffer.
          ///                                Set NULL to use exclusive mode (recommended and usually faster).
          /// @param queueCount              Array size of 'concurrentQueueFamilies'.
          /// @warning For cross-API projects, avoid this method.
          inline Type& sharingMode(uint32_t* concurrentQueueFamilies, uint32_t queueCount) noexcept {
            __setBufferSharingMode(_params, concurrentQueueFamilies, queueCount);
            return *this;
          }

          inline VkBufferCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native Vulkan descriptor
          inline const VkBufferCreateInfo& descriptor() const noexcept { return this->_params; }///< Vulkan descriptor
          inline const VkBufferCreateInfo* descriptorPtr() const noexcept { return &_params; }  ///< Vulkan descriptor
          inline VkMemoryPropertyFlags _descPreferredUsage() const noexcept { return _preferredUsage; }
        
        private:
          VkBufferCreateInfo _params{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
          VkMemoryPropertyFlags _preferredUsage = _toPreferredMemoryUsageFlags(_Usage,false);
        };
        
        using StaticBufferParams = BufferParams<ResourceUsage::local>;
        using DynamicBufferParams = BufferParams<ResourceUsage::dynamic>;
        using StagingBufferParams = BufferParams<ResourceUsage::staging>;
        
        
        // ---------------------------------------------------------------------
        // buffer container/builder
        // ---------------------------------------------------------------------
        
        /// @class Buffer
        /// @brief Vulkan data/storage buffer for shader stage(s): uniform, vertex array, vertex index...
        /// @tparam _Usage Buffer memory usage:
        ///                * Local/static: GPU read-write (fastest) / indirect CPU write-only (slow).
        ///                   -> meant to be persistent: should be updated at most once per frame (or less if big size);
        ///                   -> best option for buffers that never change (fixed geometry...).
        ///                   -> can be copied from compatible staging buffer: preferable to update large buffers.
        ///                   -> used for static meshes with skinning/skeletal animation in shaders.
        ///                * Dynamic: GPU read-write (slow) / CPU write-only (fast).
        ///                   -> meant to be updated often by the CPU: at least once per frame;
        ///                   -> recommended for writable buffers with huge data transfers;
        ///                   -> used for meshes with variable vertex count (dynamic terrains) and for vertex animation.
        ///                * Staging: CPU-only read-write (very fast).
        ///                   -> recommended for readable/writable buffers accessed many times per frame by the CPU;
        ///                   -> recommended for buffers with content that must be read by the CPU;
        ///                   -> usually used with a static buffer: data is copied from one buffer to another.
        ///                * For more details about memory usages, see 'ResourceUsage' in <video/api/types.h>.
        /// @remarks - To use a buffer, bind it to the associated Renderer object with a descriptor set.
        ///          - Uniform data type size must be a multiple of 16 byte: add padding in data structure if necessary.
        ///          - Common practice: * geometry centered around (0;0;0) -> vertex buffers;
        ///                             * world matrix to offset the entire model in the environment
        ///                               -> combined with camera view into uniform buffer;
        ///                             * vertices positioned in vertex shader by world/view matrix & projection matrix.
        ///          - The following advanced topics are meant for big projects using hundreds of buffers:
        ///            - Suballocation in same memory block:
        ///                * many GPU drivers limit the total number of allocations (usually 4096 or more allocations).
        ///                * to work around this limit & reduce time spent on allocations, the same memory allocation
        ///                  (usually a large allocation) may be bound with various buffer objects.
        ///                * to use this type of suballocation:
        ///                  - create large DeviceMemoryPool instance (256MB for example);
        ///                  - create Buffer/Texture builder with compatible usage/transfer/concurrency;
        ///                  - build Buffer/Texture objects with suballocation: build(memoryPool, byteOffset);
        ///                  - align byteOffset: DeviceMemoryPool::align(desiredOffset, builder.requiredAlignment());
        ///                  - make sure the DeviceMemoryPool won't be destroyed before any Buffer/Texture using it.
        ///                * vertex/index buffers should be aligned (usually 16-byte blocks, verify with Buffer::Builder).
        ///                * uniform buffers should be aligned (at least 256-byte blocks, verify with Buffer::Builder).
        ///                * NOTE: this feature is mocked by higher-level APIs (D3D11,OpenGL) -> always default allocs.
        ///            - Virtual sub-buffers within buffer:
        ///                * using too many buffers can cause overhead and memory fragmentation.
        ///                * to reduce those side effects, large buffers containing multiple data sets can be used.
        ///                * a single buffer can use multiple binding types (combined with logical OR operator).
        ///                  Note that most drivers only support mixing vertices and indices.
        ///                * for example, the same large buffer could contain vertices + indices for the same mesh.
        ///                * a single buffer can contain data for multiple meshes or multiple lights...
        ///                  (a sub-set of the buffer can be bound to the renderer, using offset and size params).
        ///                * vert./ind.: align memory for each entry (usually 16-byte blocks, verify with Buffer::Builder).
        ///                * uniforms: align memory for each entry (at least 256-byte blocks, verify with Buffer::Builder).
        /// @warning Buffers do not guarantee the lifetime of the associated Renderer.
        ///          They must be destroyed BEFORE destroying the Renderer!
        template <ResourceUsage _Usage>
        class Buffer final {
        public:
          using Type = Buffer<_Usage>;
        
          /// @brief Create data/storage buffer object -- reserved for internal use
          /// @remarks Prefer Buffer::Builder for standard usage
          /// @warning Buffer objects must be destroyed BEFORE the associated Renderer instance!
          Buffer(DeviceContext context, VkBuffer bufferHandle, size_t bufferByteSize,
                 VkBufferUsageFlags typeFlags, VkMemoryPropertyFlags memoryUsage,
                 VkDeviceMemory individualMemory) noexcept
            : _handle(bufferHandle), _allocation(individualMemory), _allocOffset(_individualAllocBit()),
              _context(context), _bufferSize(bufferByteSize), _typeFlags(typeFlags), _memoryUsage(memoryUsage) {}
          /// @brief Create data/storage buffer object -- reserved for internal use -- suballocation
          /// @remarks Prefer Buffer::Builder for standard usage
          /// @warning Buffer objects must be destroyed BEFORE the associated Renderer instance!
          Buffer(DeviceContext context, VkBuffer bufferHandle, size_t bufferByteSize,
                 VkBufferUsageFlags typeFlags, VkMemoryPropertyFlags memoryUsage,
                 VkDeviceMemory memoryPool, size_t memoryOffset) noexcept
            : _handle(bufferHandle), _allocation(memoryPool), _allocOffset(memoryOffset),
              _context(context), _bufferSize(bufferByteSize), _typeFlags(typeFlags), _memoryUsage(memoryUsage) {}
          
          Buffer() noexcept = default; ///< Empty buffer -- not usable (only useful for variables with deferred init)
          Buffer(const Type&) = delete;
          Buffer(Type&& rhs) noexcept
            : _handle(rhs._handle), _allocation(rhs._allocation), _allocOffset(rhs._allocOffset),
              _context(rhs._context), _bufferSize(rhs._bufferSize), _typeFlags(rhs._typeFlags),
              _memoryUsage(rhs._memoryUsage) {
            rhs._handle = VK_NULL_HANDLE;
          }
          Type& operator=(const Type&) = delete;
          Type& operator=(Type&& rhs) noexcept {
            release();
            memcpy(this, &rhs, sizeof(Type));
            rhs._handle = VK_NULL_HANDLE;
            return *this;
          }
          inline ~Buffer() noexcept { release(); }
          
          inline void release() noexcept { ///< Destroy/release buffer instance
            if (_handle != VK_NULL_HANDLE) {
              __destroyBuffer(_context, _handle, (_allocOffset==_individualAllocBit()) ? _allocation : VK_NULL_HANDLE);
              _handle = VK_NULL_HANDLE;
            }
          }
          
          // -- accessors --
          
          /// @brief Get native vulkan compatible buffer handle
          inline BufferHandle handle() const noexcept { return _handle; }
          /// @brief Get pointer to native vulkan compatible buffer handle (usable as array of 1 item)
          inline const BufferHandle* handlePtr() const noexcept { return &_handle; }
          
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (_handle == VK_NULL_HANDLE); }
          inline size_t size() const noexcept { return _bufferSize; } ///< Get buffer byte size
          
          /// @brief Get buffer type: uniform/vertex/index...
          /// @remarks Will return 0 for staging buffers (not typed)
          inline BufferType type() const noexcept { return _toBufferType(_typeFlags); }
          /// @brief Get transfer mode: standard/bidirectional
          inline TransferMode transferMode() const noexcept { return _toTransferMode(_typeFlags); }
          /// @brief Get buffer memory usage: local/dynamic/staging...
          constexpr inline ResourceUsage memoryUsage() const noexcept { return _Usage; }
          /// @brief Get buffer memory usage flags -- reserved for internal or advanced use
          inline VkMemoryPropertyFlags memoryUsageFlags() const noexcept { return _memoryUsage; }
          
          inline DeviceContext context() const noexcept { return _context; } ///< Vulkan context handle
          inline VkDeviceMemory allocation() const noexcept { return _allocation; } ///< Memory allocation
          inline size_t allocOffset() const noexcept { ///< Byte offset in memory allocation -- internal use
            return (this->_allocOffset & _allocValueMask());
          }
          
          // -- operations --
          
          /// @brief Copy content from another buffer resource (local source and/or destination).
          ///        Recommended usage: copy staging buffer data to static buffer (GPU memory).
          ///                           -> allows using data as shader resource after preparing it in a staging buffer.
          /// @warning - Both buffers must be the same size (bufferByteSize) and the same type (BufferType).
          ///          - Both buffers must use compatible internal formats.
          ///          - If destination buffer usage is staging/dynamic, the buffer must be "bidirectional".
          ///          - If source buffer usage is local/dynamic, the source buffer must be "bidirectional".
          ///          - Buffers must not already be mapped (MappedBufferIO instance, vkMapMemory...).
          template <ResourceUsage _RhsUsage, ResourceUsage _Priv = _Usage>
          inline typename std::enable_if<_RhsUsage==ResourceUsage::local || _Priv==ResourceUsage::local,
          bool>::type copyFrom(DeviceResourceManager device, const Buffer<_RhsUsage>& source) noexcept {
            auto& transientCommandQueue = device->transientCommandQueues().commandQueues;
            const VkBufferCopy copyRegion{ 0, 0, _bufferSize };
            return __copyLocalBuffer(_context, device->transientCommandPool(),
                                     transientCommandQueue[transientCommandQueue.length()-1u],
                                     source.handle(), _handle, &copyRegion, 1);
          }
          /// @brief Copy content from another buffer resource (mappable buffers).
          /// @warning - Both buffers must be the same size (bufferByteSize) and the same type (BufferType).
          ///          - Both buffers must use compatible internal formats.
          ///          - If destination buffer usage is staging/dynamic, the buffer must be "bidirectional".
          ///          - If source buffer usage is local/dynamic, the source buffer must be "bidirectional".
          ///          - Buffers must not already be mapped (MappedBufferIO instance, vkMapMemory...).
          template <ResourceUsage _RhsUsage, ResourceUsage _Priv = _Usage>
          inline typename std::enable_if<_RhsUsage!=ResourceUsage::local && _Priv!=ResourceUsage::local,
          bool>::type copyFrom(const Buffer<_RhsUsage>& source) noexcept {
            bool isHostCoherent = ((_memoryUsage & source.memoryUsageFlags()) & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            return __copyMappedBuffer(_context, source.allocation(), source.allocOffset(),
                                      _allocation, allocOffset(), _bufferSize, isHostCoherent);
          }
          
          /// @brief Discard previous data and write local buffer data. 
          ///        Only used to write the entire buffer.
          /// @param sourceData  Structure/array of the same byte size as 'buffer.size()'. Can't be NULL.
          /// @returns Memory access/update success
          /// @remarks Local buffer access: very inefficient (temporary staging buffer creation).
          ///          Prefer copyFrom with a staging buffer if written often.
          /// @warning - The buffer must not already be mapped (MappedBufferIO instance, vkMapMemory...).
          ///          - Make sure the memory of 'sourceData' is properly aligned.
          template <ResourceUsage _Private = _Usage>
          inline typename std::enable_if<_Private==ResourceUsage::local,
          bool>::type write(DeviceResourceManager device, const void* sourceData) noexcept {
            auto& transientCommandQueue = device->transientCommandQueues().commandQueues;
            return __writeLocalBuffer(_context, device->memoryProps(), device->transientCommandPool(),
                                     transientCommandQueue[transientCommandQueue.length()-1u],
                                     _handle, _bufferSize, sourceData);
          }
          /// @brief Discard previous data and write mappable buffer (dynamic/staging).
          ///        Only used to write the entire buffer. To write sub-parts of it, use MappedBufferIO instead.
          /// @param sourceData  Structure/array of the same byte size as 'buffer.size()'. Can't be NULL.
          /// @returns Memory access/update success
          /// @warning - The buffer must not already be mapped (MappedBufferIO instance, vkMapMemory...).
          ///          - Make sure the memory of 'sourceData' is properly aligned.
          template <ResourceUsage _Private = _Usage>
          inline typename std::enable_if<_Private!=ResourceUsage::local,
          bool>::type write(const void* sourceData) noexcept {
            return __writeMappedBuffer(_context, _memoryUsage, _allocation, allocOffset(), _bufferSize, sourceData);
          }


          // -- create buffers --
        
          /// @class Buffer.Builder
          /// @brief Data/storage buffer creator
          class Builder final {
          public:
            /// @brief Initialize buffer builder
            /// @warning - The BufferParams object must be kept alive while the Builder exists.
            ///          - The BufferParams object shouldn't be modified while still used by the Builder
            ///            (except potentially the buffer size, but AFTER each call to 'build' (for the next call),
            ///            to create similar buffers with various sizes).
            /// @throws - invalid_argument: if buffer size param is 0;
            ///         - runtime_error: on buffer descriptor creation failure.
            inline Builder(DeviceResourceManager device, BufferParams<_Usage>& params) // non-const ref to force ext var
              : _device(device), _params(params.descriptorPtr()), _preferredUsageFlags(params._descPreferredUsage()) {
              _bufferHandle = __createBuffer(device->context(), params.descriptor(), _memReq);
            }
            
            Builder() noexcept = default; ///< Empty builder -- not usable (only for variables with deferred init)
            Builder(const Builder&) = delete;
            Builder(Builder&& rhs) noexcept
              : _device(rhs._device), _params(rhs._params), _bufferHandle(rhs._bufferHandle),
                _memoryTypeIndex(rhs._memoryTypeIndex), _allocReq(rhs._allocReq), _memReq(rhs._memReq) {
              rhs._bufferHandle = VK_NULL_HANDLE;
            }
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&& rhs) noexcept {
              if (_bufferHandle != VK_NULL_HANDLE)
                __destroyBuffer(_device->context(), _bufferHandle, VK_NULL_HANDLE);
              memcpy(this, &rhs, sizeof(Builder));
              rhs._bufferHandle = VK_NULL_HANDLE;
              return *this;
            }
            /// @brief Destroy buffer builder
            inline ~Builder() noexcept {
              if (_bufferHandle != VK_NULL_HANDLE)
                __destroyBuffer(_device->context(), _bufferHandle, VK_NULL_HANDLE);
            }
            
            // -- memory requirements --
            
            /// @brief Get dedicated allocation requirements.
            ///        Must be called to verify if a buffer:
            ///        * must use a dedicated allocation (Requirement::required)
            ///        * should use dedicated allocation to improve performance (Requirement::preferred).
            ///        * should use default allocation or suballocation (Requirement::none).
            inline Requirement dedicatedAllocRequirement() const noexcept {
              return _allocReq.requiresDedicatedAllocation
                    ? Requirement::required
                    : (_allocReq.prefersDedicatedAllocation
                      ? Requirement::preferred
                      : Requirement::none);
            }
            /// @brief Get memory alignment requirement (bytes).
            inline size_t requiredAlignment() const noexcept { return (size_t)_memReq.memoryRequirements.alignment; }
            /// @brief Get memory allocation size requirement (bytes).
            inline size_t requiredMemorySize() const noexcept { return (size_t)_memReq.memoryRequirements.size; }
            
            /// @brief Get buffer memory usage: local/dynamic/staging...
            constexpr inline ResourceUsage memoryUsage() const noexcept { return _Usage; }
            /// @brief Get compatible vulkan memory type indices -- reserved for internal or advanced use
            inline uint32_t memoryTypeBits() const noexcept { return _memReq.memoryRequirements.memoryTypeBits; }
            
            // -- build --
            
            /// @brief Create Buffer object - default / dedicated allocation
            /// @param dedicatedAllocMode  Force dedicated allocation (::force) or use default driver alloc (::disable).
            ///                            Set to 'FeatureMode::autodetect' to use the preferred allocation type.
            /// @warning Immutable buffers must be built with 'build(initData,dedicatedAllocMode)' instead.
            /// @throws - out_of_range if no compatible memory type could be found;
            ///         - runtime_error on failure.
            Buffer<_Usage> build(FeatureMode dedicatedAllocMode = FeatureMode::autodetect) {
              if (_memoryTypeIndex == MemoryProps::indexNotFound()) { // first dedicated build -> find memory type
                _memoryTypeIndex = _device->memoryProps().findMemoryTypeIndex(_memReq.memoryRequirements.memoryTypeBits,
                                                             _toRequiredMemoryUsageFlags(_Usage), _preferredUsageFlags);
                if (_memoryTypeIndex == MemoryProps::indexNotFound())
                  __throwMemoryTypeNotFound();
              }

              DeviceContext context = _device->context();
              if (_bufferHandle == VK_NULL_HANDLE) // has been built before -> create new handle
                _bufferHandle = __createBuffer(context, *_params); // throws
              auto memory = __allocBuffer(context, _bufferHandle, _memReq.memoryRequirements.size, _memoryTypeIndex,
                                          __isDedicatedBuffer(dedicatedAllocMode, _allocReq.requiresDedicatedAllocation,
                                                              _allocReq.prefersDedicatedAllocation, _params->size));
              VkBuffer buffer = _bufferHandle;
              _bufferHandle = VK_NULL_HANDLE; // reset _bufferHandle in case 'build' is called again
              return Buffer(context, buffer, (size_t)_params->size, _params->usage,
                            _device->memoryProps().getPropertyFlags(_memoryTypeIndex), memory);
            }
            /// @brief Create Buffer object - suballocation in shared memory block
            ///          * many GPU drivers limit the total number of allocations (usually 4096 or more allocations).
            ///          * to work around this limit & reduce time spent on allocations, the same memory allocation
            ///            (usually a large allocation) may be bound with various buffer objects.
            ///          * vertex/index: should be aligned (usually 16-byte blocks, verify with requiredAlignment).
            ///          * uniform: should be aligned (at least 256-byte blocks, verify with requiredAlignment).
            /// @param byteOffset  Aligned byte offset in memory pool
            ///                    (call 'DeviceMemoryPool::align(desiredOffset, builder.requiredAlignment())').
            /// @warning * Make sure 'byteOffset' is aligned with 'requiredAlignment()'.
            ///            Using unaligned offsets may result in undefined behaviors (such as memory aliasing);
            ///          * Immutable buffers must be built with 'build(initData,memoryPool,byteOffset)' instead;
            ///          * Use a compatible memory pool (same memory usage, type, transfer mode, concurrency mode).
            /// @throws runtime_error on failure
            Buffer<_Usage> build(DeviceMemoryPool& memoryPool, size_t byteOffset) {
              DeviceContext context = _device->context();
              if (_bufferHandle == VK_NULL_HANDLE) // already built -> create new handle
                _bufferHandle = __createBuffer(context, *_params); // throws
              
              __bindBuffer(context, _bufferHandle, _memReq.memoryRequirements.size,
                           memoryPool.handle(), byteOffset); // throws
              VkBuffer buffer = _bufferHandle;
              _bufferHandle = VK_NULL_HANDLE; // reset _bufferHandle in case 'build' is called again
              return Buffer(context, buffer, (size_t)_params->size, _params->usage,
                            _device->memoryProps().getPropertyFlags(memoryPool.memoryTypeIndex()),
                            memoryPool.handle(), byteOffset);
            }
            
            // ---
            
            /// @brief Create initialized local Buffer - default / dedicated allocation
            /// @param dedicatedAllocMode  Force dedicated allocation (::force) or use default driver alloc (::disable).
            ///                            Set to 'FeatureMode::autodetect' to use the preferred allocation type.
            /// @param initData  Set initial buffer data (must be the same size as buffer, can't be NULL).
            /// @warning The initialization will create a temporary staging buffer to copy content.
            ///          If you intend to create a staging buffer for regular updates, consider using
            ///          the 'build' method without 'initData' instead (to avoid the extra cost),
            ///          then fill data with your own staging buffer.
            /// @throws runtime_error on failure
            template <ResourceUsage _Private = _Usage>
            inline Buffer<_Usage> build(typename std::enable_if<_Private==ResourceUsage::local, const void*>::type
                                        initData, FeatureMode dedicatedAllocMode = FeatureMode::autodetect) {
              Buffer<_Usage> buffer = build(dedicatedAllocMode);
              auto& transientCommandQueue = _device->transientCommandQueues().commandQueues;
              if (!__writeLocalBuffer(_device->context(), _device->memoryProps(), _device->transientCommandPool(),
                                      transientCommandQueue[transientCommandQueue.length()-1u],
                                      buffer.handle(), buffer.size(), initData))
                __throwWriteError();
              return __move_cpp14_only(buffer);
            }
            /// @brief Create initialized mappable Buffer (dynamic/staging) - default / dedicated allocation
            /// @param dedicatedAllocMode  Force dedicated allocation (::force) or use default driver alloc (::disable).
            ///                            Set to 'FeatureMode::autodetect' to use the preferred allocation type.
            /// @param initData  Set initial buffer data (must be the same size as buffer, can't be NULL).
            /// @throws runtime_error on failure
            template <ResourceUsage _Private = _Usage>
            inline Buffer<_Usage> build(typename std::enable_if<_Private!=ResourceUsage::local, const void*>::type
                                        initData, FeatureMode dedicatedAllocMode = FeatureMode::autodetect) {
              Buffer<_Usage> buffer = build(dedicatedAllocMode);
              if (!__writeMappedBuffer(buffer.context(), buffer.memoryUsageFlags(), buffer.allocation(),
                                       buffer.allocOffset(), buffer.size(), initData))
                __throwWriteError();
              return __move_cpp14_only(buffer);
            }
            
            // ---
            
            /// @brief Create initialized local Buffer - suballocation in shared memory block
            /// @param byteOffset Aligned byte offset in memory pool
            ///                   (call 'DeviceMemoryPool::align(desiredOffset, builder.requiredAlignment())').
            /// @param initData   Set initial buffer data (must be the same size as buffer, can't be NULL).
            /// @warning * Make sure 'byteOffset' is aligned with 'requiredAlignment()'.
            ///            Using unaligned offsets may result in undefined behaviors (such as memory aliasing).
            ///          * The initialization will create a temporary staging buffer to copy content.
            ///            If you intend to create a staging buffer for regular updates, consider using
            ///            the 'build' method without 'initData' instead (to avoid the extra cost),
            ///            then fill data with your own staging buffer.
            /// @throws runtime_error on failure
            template <ResourceUsage _Private = _Usage>
            inline Buffer<_Usage> build(typename std::enable_if<_Private==ResourceUsage::local, const void*>::type
                                        initData, DeviceMemoryPool& memoryPool, size_t byteOffset) {
              Buffer<_Usage> buffer = build(memoryPool, byteOffset);
              auto& transientCommandQueue = _device->transientCommandQueues().commandQueues;
              if (!__writeLocalBuffer(_device->context(), _device->memoryProps(), _device->transientCommandPool(),
                                      transientCommandQueue[transientCommandQueue.length()-1u],
                                      buffer.handle(), buffer.size(), initData))
                __throwWriteError();
              return __move_cpp14_only(buffer);
            }
            /// @brief Create initialized mappable Buffer (dynamic/staging) - suballocation in shared memory block
            /// @param byteOffset Aligned byte offset in memory pool
            ///                   (call 'DeviceMemoryPool::align(desiredOffset, builder.requiredAlignment())').
            /// @param initData   Set initial buffer data (must be the same size as buffer, can't be NULL).
            /// @warning Make sure 'byteOffset' is aligned with 'requiredAlignment()'.
            ///          Using unaligned offsets may result in undefined behaviors (such as memory aliasing).
            /// @throws runtime_error on failure
            template <ResourceUsage _Private = _Usage>
            inline Buffer<_Usage> build(typename std::enable_if<_Private!=ResourceUsage::local, const void*>::type
                                        initData, DeviceMemoryPool& memoryPool, size_t byteOffset) {
              Buffer<_Usage> buffer = build(memoryPool, byteOffset);
              if (!__writeMappedBuffer(buffer.context(), buffer.memoryUsageFlags(), buffer.allocation(),
                                       buffer.allocOffset(), buffer.size(), initData))
                __throwWriteError();
              return __move_cpp14_only(buffer);
            }
          
          private:
            constexpr inline VkMemoryPropertyFlags _requiredUsage() const noexcept {
              return _toRequiredMemoryUsageFlags(_Usage);
            }
            inline VkMemoryPropertyFlags _preferredUsage() const noexcept { return _preferredUsageFlags; }
            friend class DeviceMemoryPool;
            
          private:
            DeviceResourceManager _device = nullptr;
            const VkBufferCreateInfo* _params = nullptr;
            BufferHandle _bufferHandle = VK_NULL_HANDLE;
            uint32_t _memoryTypeIndex = MemoryProps::indexNotFound();
            VkMemoryPropertyFlags _preferredUsageFlags = (VkMemoryPropertyFlags)0;
          
            VkMemoryDedicatedRequirementsKHR _allocReq {
              VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR
            };
            VkMemoryRequirements2 _memReq {
                VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
                &_allocReq
            };
          };
          
        private:
          static constexpr inline size_t _individualAllocBit() noexcept { return ~((~(size_t)0uLL) >> 1); }
          static constexpr inline size_t _allocValueMask() noexcept { return ((~(size_t)0uLL) >> 1); }
          static_assert((sizeof(size_t)==8u && _individualAllocBit()==(size_t)0x8000000000000000uLL
                                            && _allocValueMask()==(size_t)0x7FFFFFFFFFFFFFFFuLL)
                     || (sizeof(size_t)==4u && _individualAllocBit()==(size_t)0x80000000uL
                                            && _allocValueMask()==(size_t)0x7FFFFFFFuL)
                     || (sizeof(size_t)!=8u && sizeof(size_t)!=4u), "vulkan::Buffer: non-standard 'size_t' type...");
        private:
          BufferHandle _handle = VK_NULL_HANDLE;
          VkDeviceMemory _allocation = VK_NULL_HANDLE;
          size_t _allocOffset = _individualAllocBit();
          DeviceContext _context = nullptr;
          
          size_t _bufferSize = 0;
          VkBufferUsageFlags _typeFlags = (VkBufferUsageFlags)0;
          VkMemoryPropertyFlags _memoryUsage = (VkMemoryPropertyFlags)0;
        };
        
        using StaticBuffer = Buffer<ResourceUsage::local>;
        using DynamicBuffer = Buffer<ResourceUsage::dynamic>;
        using StagingBuffer = Buffer<ResourceUsage::staging>;
        
        
        // ---------------------------------------------------------------------
        // buffer mapping
        // ---------------------------------------------------------------------

        /// @class MappedBufferIO
        /// @brief Mapped read/write data buffer access (uniform, vertex, index...)
        ///        Supports dynamic (write) and staging (read/write) buffers only.
        /// @warning The Buffer object must be kept alive while this instance is open.
        class MappedBufferIO final {
        public:
          MappedBufferIO() noexcept = default; ///< Create empty/closed instance (call 'open' to map a buffer).
          
          /// @brief Map dynamic buffer memory -- entire buffer (discard mode)
          ///        Will discard previous data. Recommended for each first write of a buffer for a new frame.
          ///        Before calling this, make sure the GPU is no longer reading the buffer.
          /// @params buffer  Dynamic buffer (must contain a valid handle).
          /// @warning No exception: verify success by calling 'isOpen()' after construction.
          inline MappedBufferIO(DynamicBuffer& buffer) noexcept
            : _context(buffer.context()), _memory(buffer.allocation()) {
            _openWriteMode(buffer.allocOffset(), buffer.size(),
                           (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          /// @brief Map dynamic buffer memory -- partial mapping (stream mode)
          ///        Only rewrite a subset of the buffer. Data currently used by GPU must not be overwritten!
          ///        Typical use: increase 'offset' for each subsequent mapping to avoid overlapping previous data.
          /// @params buffer  Dynamic buffer (must contain a valid handle).
          /// @params offset  Byte offset inside buffer memory (or 0 to map buffer from first byte).
          /// @params size    Byte size to map (or 0 to map buffer until last byte).
          /// @warning No exception: verify success by calling 'isOpen()' after construction.
          inline MappedBufferIO(DynamicBuffer& buffer, size_t offset, size_t size) noexcept
            : _context(buffer.context()), _memory(buffer.allocation()) {
            _openWriteMode(buffer.allocOffset() + offset, (size != 0) ? size : buffer.size() - offset,
                           (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          
          /// @brief Map staging buffer memory -- entire buffer
          /// @params buffer  Staging buffer (must contain a valid handle).
          /// @params mode    Mapping mode: read / write / readWrite.
          /// @warning No exception: verify success by calling 'isOpen()' after construction.
          inline MappedBufferIO(StagingBuffer& buffer, IOMode mode) noexcept
            : _context(buffer.context()), _memory(buffer.allocation()) {
            _open(buffer.allocOffset(), buffer.size(),
                  mode, (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          /// @brief Map staging buffer memory -- partial mapping
          /// @params buffer  Staging buffer (must contain a valid handle).
          /// @params mode    Mapping mode: read / write / readWrite.
          /// @params offset  Byte offset inside buffer memory (or 0 to map buffer from first byte).
          /// @params size    Byte size to map (or 0 to map buffer until last byte).
          /// @warning No exception: verify success by calling 'isOpen()' after construction.
          inline MappedBufferIO(StagingBuffer& buffer, IOMode mode, size_t offset, size_t size) noexcept
            : _context(buffer.context()), _memory(buffer.allocation()) {
            _open(buffer.allocOffset() + offset, (size != 0) ? size : buffer.size() - offset,
                  mode, (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          
          MappedBufferIO(const MappedBufferIO&) = delete;
          MappedBufferIO(MappedBufferIO&& rhs) noexcept
            : _mapped(rhs._mapped), _context(rhs._context), _memory(rhs._memory),
              _flushedOffset(rhs._flushedOffset), _flushedSize(rhs._flushedSize) { rhs._mapped = nullptr; }
          MappedBufferIO& operator=(const MappedBufferIO&) = delete;
          MappedBufferIO& operator=(MappedBufferIO&& rhs) noexcept {
            _mapped=rhs._mapped; _context=rhs._context; _memory=rhs._memory;
            _flushedOffset=rhs._flushedOffset; _flushedSize=rhs._flushedSize;
            rhs._mapped = nullptr;
            return *this;
          }
          inline ~MappedBufferIO() noexcept { close(); }
          
          
          // -- read/write access --
          
          inline bool isOpen() const noexcept { return (_mapped != nullptr); } ///< Get current state

          /// @brief Get mapped data access for read/write operations (or NULL if not open)
          ///        Can be cast to the structure/class type to read/write, or used with memcpy/memset operations.
          /// @warning Write operations should never exceed the size of the open buffer!
          inline void* data() noexcept { return _mapped; }
          /// @brief Get mapped data access for read operations (or NULL if not open)
          ///        Can be cast to the structure/class type to read, or used with memcpy operations.
          /// @warning Only for staging buffers open with StagedMapping::read or StagedMapping::readWrite.
          inline const void* data() const noexcept { return _mapped; }
          
          
          // -- operations --
          
          /// @brief Map dynamic buffer memory -- entire buffer (discard mode)
          ///        Will discard previous data. Recommended for each first write of a buffer for a new frame.
          ///        Before calling this, make sure the GPU is no longer reading the buffer.
          /// @params buffer  Dynamic buffer (must contain a valid handle).
          /// @returns Success
          /// @warning If already open, 'close' MUST be called before mapping another buffer,
          ///          or a leak will occur (in debug mode, an assertion will fail).
          inline bool open(DynamicBuffer& buffer) noexcept {
            _context = buffer.context();
            _memory = buffer.allocation();
            return _openWriteMode(buffer.allocOffset(), buffer.size(),
                                  (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          /// @brief Map dynamic buffer memory -- partial mapping (stream mode)
          ///        Only rewrite a subset of the buffer. Data currently used by GPU must not be overwritten!
          ///        Typical use: increase 'offset' for each subsequent mapping to avoid overlapping previous data.
          /// @params buffer  Dynamic buffer (must contain a valid handle).
          /// @params offset  Byte offset inside buffer memory (or 0 to map buffer from first byte).
          /// @params size    Byte size to map (or 0 to map buffer until last byte).
          /// @returns Success
          /// @warning If already open, 'close' MUST be called before mapping another buffer,
          ///          or a leak will occur (in debug mode, an assertion will fail).
          inline bool open(DynamicBuffer& buffer, size_t offset, size_t size) noexcept {
            _context = buffer.context();
            _memory = buffer.allocation();
            return _openWriteMode(buffer.allocOffset() + offset, (size != 0) ? size : buffer.size() - offset,
                                  (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          
          // ---
          
          /// @brief Map staging buffer memory -- entire buffer
          /// @params buffer  Staging buffer (must contain a valid handle).
          /// @params mode    Mapping mode: read / write / readWrite.
          /// @returns Success
          /// @warning If already open, 'close' MUST be called before mapping another buffer,
          ///          or a leak will occur (in debug mode, an assertion will fail).
          inline bool open(StagingBuffer& buffer, IOMode mode) noexcept {
            _context = buffer.context();
            _memory = buffer.allocation();
            return _open(buffer.allocOffset(), buffer.size(),
                         mode, (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          /// @brief Map staging buffer memory -- partial mapping
          /// @params buffer  Staging buffer (must contain a valid handle).
          /// @params mode    Mapping mode: read / write / readWrite.
          /// @params offset  Byte offset inside buffer memory (or 0 to map buffer from first byte).
          /// @params size    Byte size to map (or 0 to map buffer until last byte).
          /// @returns Success
          /// @warning If already open, 'close' MUST be called before mapping another buffer,
          ///          or a leak will occur (in debug mode, an assertion will fail).
          inline bool open(StagingBuffer& buffer, IOMode mode, size_t offset, size_t size) noexcept {
            _context = buffer.context();
            _memory = buffer.allocation();
            return _open(buffer.allocOffset() + offset, (size != 0) ? size : buffer.size() - offset,
                         mode, (buffer.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
          }
          
          // ---

          /// @brief Unmap data buffer (close access, if a buffer is mapped)
          /// @warning No read/write operations can be performed after it until 'open' is called again.
          void close() noexcept;

        private:
          bool _open(size_t offset, size_t size, IOMode mode, bool isHostCoherent) noexcept; // staging
          bool _openWriteMode(size_t offset, size_t size, bool isHostCoherent) noexcept; // dynamic
        private:
          void* _mapped = nullptr;
          DeviceContext _context = VK_NULL_HANDLE;
          VkDeviceMemory _memory = VK_NULL_HANDLE;
          size_t _flushedOffset = 0; // only used if a flush is required before unmapping
          size_t _flushedSize = 0;   // only used if a flush is required before unmapping
        };
      }
    }
  }
  
# undef __if_constexpr
# undef __move_cpp14_only
#endif