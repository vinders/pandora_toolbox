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
Vulkan - DepthStencilBuffer
       - Buffer<ResourceUsage:: immutable/staticGpu/dynamicCpu/staging>
       - MappedBufferIO
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cassert>
# include <cstdint>
# include "./_private/_shared_resource.h" // includes vulkan
# include "./_private/_resource_io.h" // includes vulkan
# include "./renderer.h" // includes vulkan
# if !defined(_CPP_REVISION) || _CPP_REVISION != 14
#   define __if_constexpr if constexpr
# else
#   define __if_constexpr if
# endif

  namespace pandora {
    namespace video {
      namespace vulkan {
        class BufferMemory;
        
        /// @class DepthStencilBuffer
        /// @brief Vulkan depth/stencil fragment verification buffer/view for output merger
        /// @warning - If the render-target is resized, a new DepthStencilBuffer must be created.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new DepthStencilBuffer must be created.
        /// @remarks - To use a depth/stencil buffer, call setActiveRenderTarget on Renderer with getDepthStencilView() and a target render view (same size).
        ///          - The same depth/stencil buffer may only be used with multiple/different render-targets if they have the same size.
        class DepthStencilBuffer final {
        public:
          /// @brief Create depth/stencil buffer for existing renderer/render-target
          /// @param sampleCount  Sample count for multisampling (anti-aliasing). Use 1 to disable multisampling.
          ///                     Call Renderer.is{Color/Depth/Stencil}SampleCountAvailable to make sure the value is supported.
          /// @warning - Params 'width'/'height' should be the same as the associated render-target size (swap-chain/texture buffer).
          ///          - Param 'tiling' only exists for advanced use of Vulkan API. Keep default value for cross-API projects.
          /// @throws - invalid_argument: if width/height is 0.
          ///         - runtime_error: creation failure.
          DepthStencilBuffer(Renderer& renderer, DepthStencilFormat format, uint32_t width, uint32_t height,
                             uint32_t sampleCount = 1u, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);
          ~DepthStencilBuffer() noexcept { release(); }
          /// @brief Destroy depth/stencil buffer
          void release() noexcept;
          
          DepthStencilBuffer() = default; ///< Empty depth/stencil buffer -- not usable (only useful to store variable not immediately initialized)
          DepthStencilBuffer(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept;
          DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer& operator=(DepthStencilBuffer&& rhs) noexcept;
        
          // -- accessors --
          
          /// @brief Get native vulkan compatible depth/stencil buffer handle (cast to VkImage)
          inline TextureHandle2D handle() const noexcept { return this->_depthStencilBuffer; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (this->_depthStencilBuffer == VK_NULL_HANDLE); }
          
          inline uint32_t width() const noexcept  { return _width(); } ///< Get depth/stencil buffer width
          inline uint32_t height() const noexcept { return _height(); }///< Get depth/stencil buffer height
          inline DepthStencilFormat getFormat() const noexcept { return this->_format; } ///< Get depth/stencil buffer data format
          
          /// @brief Get depth/stencil view of current DepthStencilBuffer
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the depth/stencil view is necessary for depth/stencil testing.
          inline DepthStencilView getDepthStencilView() const noexcept { return this->_depthStencilView; }


        private:
          inline uint32_t _width() const noexcept { return (this->_pixelSize & 0xFFFFu); }
          inline uint32_t _height() const noexcept { return (this->_pixelSize >> 16); }
          static constexpr inline uint32_t _toPixelSize(uint32_t width, uint32_t height) noexcept { return (width | (height << 16)); }
          
        private:
          DepthStencilView _depthStencilView = VK_NULL_HANDLE;  // VkImageView
          TextureHandle2D _depthStencilBuffer = VK_NULL_HANDLE; // VkImage
          VkDeviceMemory _depthStencilMemory = VK_NULL_HANDLE;
          DeviceResourceManager _context = nullptr;
          uint32_t _pixelSize = 0; // width / height
          DepthStencilFormat _format = DepthStencilFormat::d32_f;
        };
        
        // ---
        
        /// @class Buffer
        /// @brief Vulkan data buffer for shader stage(s): constant/uniform buffer, vertex array buffer, vertex index buffer...
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
        ///            - Suballocation in same memory block (Vulkan and D3D12 specific):
        ///                * many GPU drivers limit the total number of allocations (usually 4096 or more allocations).
        ///                * to work around this limit, and to reduce time spent on allocations, a large allocation can contain multiple buffer.
        ///                * to use this type of suballocation:
        ///                  - create Buffer objects with Buffer::createUnallocatedBuffer, to avoid individual allocation;
        ///                  - call BufferMemory::create to create a large VkDeviceMemory instance (256MB for example);
        ///                  - call BufferMemory.bind(VkBuffer, offset) on each Buffer object.
        ///                * for example, the same memory allocation may be bound with various vertex/index/uniform buffers.
        ///                * vertex/index buffers should be aligned in memory (4*32bits blocks).
        ///                * constant/uniform buffers should be aligned in memory (16*4*32bits blocks).
        ///                * more efficient with AMD GPUs.
        ///                * WARNING: this feature is not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
        ///            - Suballocation within buffer (cross-API):
        ///                * total number of buffers may also be limited, and using too many buffers can cause overhead and memory fragmentation.
        ///                * to reduce those side effects, large buffers containing multiple data sets can be used.
        ///                * this can be done instead of creating many buffers for an allocation (both techniques can also be combined).
        ///                * a single buffer can use multiple binding types (example: BufferType::vertex | BufferType::vertexIndex).
        ///                  Note that most drivers do not support mixing vertices/indices with uniform data.
        ///                * for example, the same large buffer could contain vertices + indices for the same mesh.
        ///                * a single buffer can contain data for multiple meshes or multiple lights...
        ///                  (a sub-set of the buffer can be bound to the renderer, using 'byteOffset' and 'strideByteSize' params).
        ///                * when using suballocation for vertices/indices, align memory for every sub-component (aligned at 4*32bits blocks).
        ///                * when using suballocation for constants/uniforms, align memory for every sub-component (aligned at 16*4*32bits blocks).
        ///                * very efficient with Nvidia GPUs.
        /// @warning Buffers do not guarantee the lifetime of the associated Renderer. They must be destroyed BEFORE destroying the Renderer!
        template <ResourceUsage _Usage>
        class Buffer final {
        public:
          using Type = Buffer<_Usage>;
        
          /// @brief Create buffer (to store data for shader stages) - undefined value
          ///        Individual memory allocation is made in constructor (automatic memory management, cross-API).
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
          /// @param concurrentQueueFamilies  Only for concurrent sharing mode: array of queue family indices that can access the buffer.
          ///                                 Set to NULL to use exclusive mode (recommended and usually faster).
          /// @param queueCount               Array size of 'concurrentQueueFamilies'.
          /// @warning - Static buffers: init/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          ///          - For cross-API projects, keep implicit/default values for params 'isBidirectional'/'concurrentQueueFamilies'/'queueCount'.
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - out_of_range: if current GPU doesn't support this combination of params (type/isBidirectional/sharing);
          ///         - runtime_error: on creation failure.
          inline Buffer(Renderer& renderer, BufferType type, size_t bufferByteSize, bool isBidirectional = false,
                 uint32_t* concurrentQueueFamilies = nullptr, uint32_t queueCount = 0)
            : _bufferSize(bufferByteSize), _device(renderer.resourceManager().get()), _type(type) {
            this->_handle = __createBufferContainer(renderer.context(), bufferByteSize,
                                                    _getTypeUsageFlags(type, isBidirectional),
                                                    concurrentQueueFamilies, queueCount);
            this->_allocation = __allocBufferContainer(renderer.context(), renderer.device(), this->_handle, _getMemoryUsageFlags());
          }
          
          /// @brief Create buffer (to store data for shader stages) with initial value
          ///        Individual memory allocation is made in constructor (automatic memory management, cross-API).
          /// @param renderer        The renderer for which the buffer is created: use the same renderer when binding it or when calling write.
          /// @param type            Type of buffer to create: constant/uniform buffer, vertex array buffer, vertex index buffer...
          ///                        Note: to use suballocation, the same buffer can use multiple binding types
          ///                        (example: BufferType::vertex | BufferType::vertexIndex).
          ///                        Warning: most drivers do not support mixing vertices/indices with constant/uniform data.
          /// @param bufferByteSize  The total number of bytes of the buffer (sizeof structure/array) -- must be a multiple of 16 bytes for constant/uniform buffers.
          /// @param initData        Buffer initial value -- structure or array of input values (must not be NULL if immutable).
          /// @param isBidirectional Copy operations can occur using this buffer both as a source and a destination.
          ///                        If 'isBidirectional'==false: * staticGpu buffers can only be destinations;
          ///                                                     * staging buffers can only be sources;
          ///                                                     * dynamicCpu and immutable buffers can't be any.
          /// @param concurrentQueueFamilies  Only for concurrent sharing mode: array of queue family indices that can access the buffer.
          ///                                 Set to NULL to use exclusive mode (recommended and usually faster).
          /// @param queueCount               Array size of 'concurrentQueueFamilies'.
          /// @warning - Static/immutable buffers: init/writing is a LOT more efficient when the source data type has a 16-byte alignment (see <system/align.h>).
          ///          - For cross-API projects, keep implicit/default values for params 'isBidirectional'/'concurrentQueueFamilies'/'queueCount'.
          /// @throws - invalid_argument: if 'bufferByteSize' is 0;
          ///         - out_of_range: if current GPU doesn't support this combination of params (type/isBidirectional/sharing);
          ///         - runtime_error: on creation failure.
          Buffer(Renderer& renderer, BufferType type, size_t bufferByteSize, const void* initData,
                 bool isBidirectional = false, uint32_t* concurrentQueueFamilies = nullptr, uint32_t queueCount = 0)
            : _bufferSize(bufferByteSize), _device(renderer.resourceManager().get()), _type(type) {
            this->_handle = __createBufferContainer(renderer.context(), bufferByteSize, _getTypeUsageFlags(type, isBidirectional),
                                                    concurrentQueueFamilies, queueCount);
            this->_allocation = __allocBufferContainer(renderer.context(), renderer.device(), this->_handle, _getMemoryUsageFlags());
            
            __if_constexpr (_Usage != ResourceUsage::immutable)
              write(initData);
            else {
              auto& transientCommandQueue = _device->transientCommandQueues().commandQueues;
              __writeWithStagingBuffer(_device->context(), _device->device(), _device->transientCommandPool(),
                                       transientCommandQueue.value[transientCommandQueue.length()-1u],
                                       this->_handle, bufferByteSize, initData);
            }
          }
          
          /// @brief Create buffer (to store data for shader stages) without any memory allocation, to allow suballocation.
          ///        A call to BufferMemory.bind must be performed after the creation.
          /// @warning Not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
          static inline Buffer createUnallocatedBuffer(Renderer& renderer, BufferType type,
                                                       size_t bufferByteSize, bool isBidirectional = false,
                                                       uint32_t* concurrentQueueFamilies = nullptr, uint32_t queueCount = 0) {
            return Buffer(__createBufferContainer(renderer.context(), bufferByteSize, _getTypeUsageFlags(type, isBidirectional),
                                                  concurrentQueueFamilies, queueCount), bufferByteSize, type, VK_NULL_HANDLE);
          }
          
          /// @brief Create from native buffer handle -- reserved for advanced usage
          /// @param individualMemory  Individual memory allocation for this buffer only
          ///                          (or VK_NULL_HANDLE to create unallocated buffer and use BufferMemory.bind for suballocation).
          /// @warning Native usage must be the same as template type! (or write operations won't work)
          ///          immutable/staticGpu: VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
          ///          dynamicCpu/staging: VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
          inline Buffer(Renderer& renderer, BufferHandle handle, size_t bufferSize, BufferType type,
                        VkDeviceMemory individualMemory = VK_NULL_HANDLE) noexcept
            : _handle(handle), _bufferSize(bufferSize), _device(renderer.resourceManager().get()), _allocation(individualMemory),
              _allocationOffset((individualMemory != VK_NULL_HANDLE) ? _individualAlloc() : 0), _type(type) {}
          
          Buffer() noexcept = default; ///< Empty buffer -- not usable (only useful to store variable not immediately initialized)
          Buffer(const Type&) = delete;
          Buffer(Type&& rhs) noexcept
            : _handle(rhs._handle), _bufferSize(rhs._bufferSize), _device(rhs._device),
              _allocation(rhs._allocation), _allocationOffset(rhs._allocationOffset), _type(rhs._type) {
            rhs._handle = VK_NULL_HANDLE;
            rhs._allocation = VK_NULL_HANDLE;
          }
          Type& operator=(const Type&) = delete;
          Type& operator=(Type&& rhs) noexcept {
            _handle=rhs._handle; _bufferSize=rhs._bufferSize; _device=rhs._device;
            _allocation=rhs._allocation; _allocationOffset=rhs._allocationOffset; _type=rhs._type;
            rhs._handle = VK_NULL_HANDLE;
            rhs._allocation = VK_NULL_HANDLE;
            return *this;
          }
          inline ~Buffer() noexcept { release(); }

          inline void release() noexcept { ///< Destroy/release buffer instance
            if (_handle != VK_NULL_HANDLE) {
              __destroyBufferContainer(_device->context(), _handle,
                                       (_allocationOffset == _individualAlloc()) ? _allocation : VK_NULL_HANDLE);
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
          inline BufferType type() const noexcept { return _type; }   ///< Get buffer type: uniform/vertex/index
          
          static constexpr inline ResourceUsage usage() noexcept { return _Usage; }    ///< Get buffer resource usage
          inline DeviceContext context() const noexcept { return _device->context(); } ///< Get parent context -- reserved for internal use
          inline VkDeviceMemory allocation() const noexcept { return _allocation; }    ///< Get memory alloc -- reserved for internal use
          inline size_t allocationOffset() const noexcept { ///< Get memory offset -- reserved for internal use
            return (this->_allocationOffset != _individualAlloc()) ? this->_allocationOffset : 0;
          }
          
          // -- operations --
          
          /// @brief Copy content of another buffer resource.
          ///        Recommended usage: to copy staging buffer data to a static buffer (GPU memory).
          ///                           -> allows using data as a shader resource after preparing it in a staging buffer.
          /// @param commandQueueIndex Choose command queue index (from family Renderer.resourceManage().transientCommandQueues()).
          ///                          Allows synchronization with rendering commands (same queue) or asynchronous copy (different).
          ///                          Only used when reading/writing in static/immutable buffer(s).
          ///                          Not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
          /// @warning - Both buffers must be the same size (bufferByteSize) and the same type (BufferType).
          ///          - Both buffers must use compatible internal formats.
          ///          - If current buffer usage is staging/dynamicCpu, the buffer must be "bidirectional".
          ///          - If 'source' buffer usage is immutable/staticGpu/dynamicCpu, the buffer must be "bidirectional".
          ///          - Buffers can't be currently mapped (with a MappedBufferIO instance or using vkMapMemory, for example).
          ///          - This command is not supported on immutable buffers.
          ///          - The renderer used with buffer constructor must still exist, or this may crash.
          template <ResourceUsage _RhsUsage>
          bool copy(Buffer<_RhsUsage>& source, uint32_t commandQueueIndex = 0) noexcept;
          
          /// @brief Discard previous data and write buffer data. Can only be used to write the entire buffer.
          ///        To only write sub-parts of it, use MappedBufferIO instead (only for dynamic and staging buffers).
          /// @param sourceData  Structure/array of the same byte size as 'bufferByteSize' in constructor. Can't be NULL.
          /// @param commandQueueIndex Choose command queue index (from family Renderer.resourceManage().transientCommandQueues()).
          ///                          Allows synchronization with rendering commands (same queue) or asynchronous copy (different).
          ///                          Only used when reading/writing in static/immutable buffer(s).
          ///                          Not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
          /// @returns Memory access/update success
          /// @warning - 'sourceData' must be the same size as the buffer ('bufferByteSize').
          ///          - The buffer can't be currently mapped (with a MappedBufferIO instance or using vkMapMemory, for example).
          ///          - The renderer used with buffer constructor must still exist, or this may crash.
          ///          - This command is not supported on immutable buffers.
          ///          - Make sure the 'sourceData' memory is properly aligned.
          bool write(const void* sourceData, uint32_t commandQueueIndex = 0) noexcept;

        private:
          static constexpr VkBufferUsageFlags _getTypeUsageFlags(BufferType type, bool isBidirectional) noexcept;
          static constexpr VkMemoryPropertyFlags _getMemoryUsageFlags() noexcept;
          static constexpr inline size_t _individualAlloc() noexcept { return (size_t)-1; }
          friend class BufferMemory;
          
        private:
          BufferHandle _handle = VK_NULL_HANDLE;
          size_t _bufferSize = 0;
          ScopedDeviceContext* _device = nullptr;
          VkDeviceMemory _allocation = VK_NULL_HANDLE;
          size_t _allocationOffset = _individualAlloc();
          BufferType _type = BufferType::uniform;
        };

        using ImmutableBuffer = Buffer<ResourceUsage::immutable>;
        using StaticBuffer = Buffer<ResourceUsage::staticGpu>;
        using DynamicBuffer = Buffer<ResourceUsage::dynamicCpu>;
        using StagingBuffer = Buffer<ResourceUsage::staging>;
        
        // ---
        
        /// @class BufferMemory
        /// @brief Common device memory allocation for buffer suballocation.
        ///        Only used with Buffer objects created with Buffer:createUnallocatedBuffer(...).
        ///        Must not be used with automatic/individual allocated buffers (other Buffer constructors).
        /// @warning - All attached Buffer objects must be destroyed BEFORE destroying the BufferMemory instance.
        ///          - Not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
        class BufferMemory final {
        public:
          /// @brief Initialize memory -- reserved for internal or advanced use -- call BufferMemory::create instead.
          BufferMemory(VkDeviceMemory deviceMemory, const DeviceResourceManager& context) noexcept;
          BufferMemory() noexcept = default; ///< Empty -- not usable (only useful to store variable not immediately initialized)
          BufferMemory(const BufferMemory&) = delete;
          BufferMemory(BufferMemory&&) noexcept = default;
          BufferMemory& operator=(const BufferMemory&) = delete;
          BufferMemory& operator=(BufferMemory&&) noexcept = default;
          ~BufferMemory() noexcept = default;
          
          inline void release() noexcept { _handle.release(); } ///< Destroy/release memory
          
          // -- accessors --

          inline VkDeviceMemory handle() const noexcept { return _handle.value(); } ///< Get native vulkan memory allocation handle
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return !_handle.hasValue(); }
          
          // -- operations --
          
          /// @brief Bind buffer to suballocation in existing memory allocation.
          ///        Must be called only once after buffer creation (only with Buffer:createUnallocatedBuffer(...)).
          /// @param allocation  Memory allocation created for the same Buffer type
          ///                    (ResourceUsage/BufferType/isBidirectional/concurrentQueueFamilies).
          /// @throws - logic_error: if 'buffer' has an individual allocation or is already bound.
          ///         - runtime_error: on failure.
          /// @warning - Only available if Buffer built with with Buffer:createUnallocatedBuffer(...).
          ///          - Not available for higher-level APIs (D3D11, OpenGL).
          template <ResourceUsage _Usage>
          inline void bind(Buffer<_Usage>& buffer, size_t byteOffset) {
            _bind(buffer.handle(), byteOffset, buffer._allocation);
            buffer._allocationOffset = byteOffset;
          }
          
          // -- builder --
        
          /// @brief Create memory allocation (for multiple buffer suballocations)
          /// @param renderer       Rendering device instance hosting the memory.
          /// @param usage          Resource memory usage mode (see Buffer class description).
          /// @param byteSize       Total byte size of the allocation (must be a multiple of 256 bytes).
          /// @param eachBufferType Array of typed buffers for which memory is allocated (with template _Usage=='mode').
          ///                       This is only used to determine the memory type to allocate (the buffers in the list won't be bound).
          ///                       This doesn't need to contain all buffers that should be bound to this allocation.
          ///                       But it should contain one of each type (BufferType/isBidirectional/concurrentQueueFamilies).
          ///                       Note: if too many types are mixed (different directions, different sharing modes...),
          ///                             the API may fail to find a suitable memory type (-> out_of_range exception).
          /// @throws out_of_range: if GPU doesn't support all the types specified at once;
          ///         runtime_error: on creation failure.
          /// @warning - The buffers listed in 'eachBufferType' will NOT be bound to the allocation.
          ///            Buffer.bind must be called for each buffer (even if it was in the list).
          ///          - Each buffer must have the same ResourceUsage as param 'mode'.
          ///          - This is not available for higher-level APIs (D3D11, OpenGL). For cross-API projects, use ifdefs or avoid it.
          static BufferMemory create(Renderer& renderer, ResourceUsage mode, size_t byteSize,
                                     const BufferHandle* eachBufferType, size_t bufferCount);

        private:
          void _bind(BufferHandle buffer, size_t byteOffset, VkDeviceMemory& outAllocation);
        private:
          ScopedResource<VkDeviceMemory> _handle;
        };
        
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
          /// @throws runtime_error on failure
          MappedBufferIO(Buffer<ResourceUsage::dynamicCpu>& buffer);
          /// @brief Open mapped data access for a staging buffer
          /// @params buffer  Staging buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Access mode: read / write / readWrite.
          /// @throws runtime_error on failure
          MappedBufferIO(Buffer<ResourceUsage::staging>& buffer, StagedMapping mode = StagedMapping::readWrite);
          
          MappedBufferIO() noexcept = default; ///< Empty -- not usable (only useful to store variable not immediately initialized)
          MappedBufferIO(const MappedBufferIO&) = delete;
          MappedBufferIO(MappedBufferIO&& rhs) noexcept
            : _mapped(rhs._mapped), _context(rhs._context), _buffer(rhs._buffer) { rhs._mapped = nullptr; }
          MappedBufferIO& operator=(const MappedBufferIO&) = delete;
          MappedBufferIO& operator=(MappedBufferIO&& rhs) noexcept {
            _mapped=rhs._mapped; _context=rhs._context; _buffer=rhs._buffer;
            rhs._mapped = nullptr;
            return *this;
          }
          ~MappedBufferIO() noexcept { close(); }
          
          
          // -- read/write access --
          
          inline bool isOpen() noexcept { return (this->_mapped != nullptr); } ///< Get current state
          
          /// @brief Get mapped data access for read/write operations (or NULL if no access is open)
          ///        Can be cast to the structure/class type to read/write, or used with memcpy/memset operations.
          /// @warning Write operations should never exceed the size of the open buffer!
          inline void* data() noexcept { return this->_mapped; }
          /// @brief Get mapped data access for read operations (or NULL if no access is open)
          ///        Can be cast to the structure/class type to read, or used with memcpy operations.
          /// @warning Only for staging buffers open with StagedMapping::read or StagedMapping::readWrite.
          inline const void* data() const noexcept { return this->_mapped; }
          
          
          // -- operations --
        
          /// @brief Open mapped data access for a dynamic buffer
          /// @params buffer  Dynamic buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Dynamic mapping mode:
          ///                 * discard: Discard previous data (as soon as GPU stops using it) -> content will be undefined.
          ///                            Recommended for each first write of a buffer for a new frame.
          ///                 * subsequent: Keep previous data (warning: data currently used by GPU must not be overwritten!).
          ///                               Recommended for subsequent writes of a buffer within the same frame.
          /// @returns Success
          inline bool open(Buffer<ResourceUsage::dynamicCpu>& buffer) noexcept {
            close();
            this->_context = buffer.context();
            this->_buffer = buffer.allocation();
            this->_mapped = __mapDataBuffer(buffer.context(), buffer.size(),
                                            buffer.allocation(), buffer.allocationOffset());
            return (this->_mapped != nullptr);
          }
          /// @brief Open mapped data access for a staging buffer
          /// @params buffer  Staging buffer to map (must contain a valid handle: undefined behavior otherwise).
          /// @params mode    Access mode: read / write / readWrite.
          /// @returns Success
          inline bool open(Buffer<ResourceUsage::staging>& buffer,
                           StagedMapping /*mode*/ = StagedMapping::readWrite) noexcept {
            close();
            this->_context = buffer.context();
            this->_buffer = buffer.allocation();
            this->_mapped = __mapDataBuffer(buffer.context(), buffer.size(),
                                            buffer.allocation(), buffer.allocationOffset());
            return (this->_mapped != nullptr);
          }
          /// @brief Close mapped buffer access.
          /// @warning No other read/write operations can be performed until 'open' is called again.
          inline void close() noexcept {
            if (_mapped != nullptr) {
              __unmapDataBuffer(_context, _buffer);
              _mapped = nullptr;
            }
          }

        private:
          void* _mapped = nullptr;
          DeviceContext _context = VK_NULL_HANDLE;
          VkDeviceMemory _buffer = VK_NULL_HANDLE;
        };
      }
    }
  }
  
  // ---
  
# define __P_BUFFER_STATIC_CLASS    pandora::video::vulkan::Buffer<pandora::video::vulkan::ResourceUsage::staticGpu>
# define __P_BUFFER_IMMUTABLE_CLASS pandora::video::vulkan::Buffer<pandora::video::vulkan::ResourceUsage::immutable>
# define __P_BUFFER_DYNAMIC_CLASS   pandora::video::vulkan::Buffer<pandora::video::vulkan::ResourceUsage::dynamicCpu>
# define __P_BUFFER_STAGING_CLASS   pandora::video::vulkan::Buffer<pandora::video::vulkan::ResourceUsage::staging>
  
  // VkBufferUsageFlags Buffer._getTypeUsageFlags(BufferType type, bool isBidirectional)
  template <> static constexpr inline
  VkBufferUsageFlags __P_BUFFER_STATIC_CLASS::_getTypeUsageFlags(pandora::video::vulkan::BufferType type,
                                                                 bool isBidirectional) noexcept {
    return isBidirectional
           ? ((VkBufferUsageFlags)type | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
           : ((VkBufferUsageFlags)type | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  }
  template <> static constexpr inline
  VkBufferUsageFlags __P_BUFFER_IMMUTABLE_CLASS::_getTypeUsageFlags(pandora::video::vulkan::BufferType type,
                                                                    bool isBidirectional) noexcept {
    return isBidirectional
           ? ((VkBufferUsageFlags)type | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
           : ((VkBufferUsageFlags)type | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  }
  template <> static constexpr inline
  VkBufferUsageFlags __P_BUFFER_DYNAMIC_CLASS::_getTypeUsageFlags(pandora::video::vulkan::BufferType type,
                                                                  bool isBidirectional) noexcept {
    return isBidirectional
           ? ((VkBufferUsageFlags)type | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
           : (VkBufferUsageFlags)type;
  }
  template <> static constexpr inline
  VkBufferUsageFlags __P_BUFFER_STAGING_CLASS::_getTypeUsageFlags(pandora::video::vulkan::BufferType,
                                                                  bool isBidirectional) noexcept {
    return isBidirectional
           ? (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
           : VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }
  
  // VkMemoryPropertyFlags Buffer._getMemoryUsageFlags()
  template <> static constexpr inline
  VkMemoryPropertyFlags __P_BUFFER_STATIC_CLASS::_getMemoryUsageFlags() noexcept {
    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }
  template <> static constexpr inline
  VkMemoryPropertyFlags __P_BUFFER_IMMUTABLE_CLASS::_getMemoryUsageFlags() noexcept {
    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  }
  template <> static constexpr inline
  VkMemoryPropertyFlags __P_BUFFER_DYNAMIC_CLASS::_getMemoryUsageFlags() noexcept {
    return (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
  template <> static constexpr inline
  VkMemoryPropertyFlags __P_BUFFER_STAGING_CLASS::_getMemoryUsageFlags() noexcept {
    return (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
  
  // bool Buffer.copy<ResourceUsage>(Buffer& source)
  template <> template <pandora::video::vulkan::ResourceUsage _RhsUsage> inline
  bool __P_BUFFER_STATIC_CLASS::copy(pandora::video::vulkan::Buffer<_RhsUsage>& source, uint32_t commandQueueIndex) noexcept {
    return __copyDataBuffer(_device->context(), _device->transientCommandPool(),
                            _device->transientCommandQueues().commandQueues.value[commandQueueIndex],
                            source.handle(), this->_handle, this->_bufferSize);
  }
  template <> template <pandora::video::vulkan::ResourceUsage _RhsUsage> inline
  bool __P_BUFFER_IMMUTABLE_CLASS::copy(pandora::video::vulkan::Buffer<_RhsUsage>& source, uint32_t commandQueueIndex) noexcept {
    return false;
  }
  template <> template <pandora::video::vulkan::ResourceUsage _RhsUsage> inline
  bool __P_BUFFER_DYNAMIC_CLASS::copy(pandora::video::vulkan::Buffer<_RhsUsage>& source, uint32_t commandQueueIndex) noexcept {
    __if_constexpr (_RhsUsage == pandora::video::vulkan::ResourceUsage::staging
                 || _RhsUsage == pandora::video::vulkan::ResourceUsage::dynamicCpu)
      return __copyMappedDataBuffer(this->_device->context(), this->_bufferSize,
                                    source.allocation(), source.allocationOffset(), allocation(), allocationOffset());
    else
      return __copyDataBuffer(_device->context(), _device->transientCommandPool(),
                              _device->transientCommandQueues().commandQueues.value[commandQueueIndex],
                              source.handle(), this->_handle, this->_bufferSize);
  }
  template <> template <pandora::video::vulkan::ResourceUsage _RhsUsage> inline
  bool __P_BUFFER_STAGING_CLASS::copy(pandora::video::vulkan::Buffer<_RhsUsage>& source, uint32_t commandQueueIndex) noexcept {
    __if_constexpr (_RhsUsage == pandora::video::vulkan::ResourceUsage::staging
                 || _RhsUsage == pandora::video::vulkan::ResourceUsage::dynamicCpu)
      return __copyMappedDataBuffer(this->_device->context(), this->_bufferSize,
                                    source.allocation(), source.allocationOffset(), allocation(), allocationOffset());
    else
      return __copyDataBuffer(_device->context(), _device->transientCommandPool(),
                              _device->transientCommandQueues().commandQueues.value[commandQueueIndex],
                              source.handle(), this->_handle, this->_bufferSize);
  }

  // bool Buffer.write(const void* sourceData)
  template <> inline bool __P_BUFFER_STATIC_CLASS::write(const void* sourceData, uint32_t commandQueueIndex) noexcept {
    return __writeWithStagingBuffer(_device->context(), _device->device(), _device->transientCommandPool(),
                                    _device->transientCommandQueues().commandQueues.value[commandQueueIndex],
                                    this->_handle, this->_bufferSize, sourceData);
  }
  template <> inline bool __P_BUFFER_IMMUTABLE_CLASS::write(const void*, uint32_t) noexcept {
    return false;
  }
  template <> inline bool __P_BUFFER_DYNAMIC_CLASS::write(const void* sourceData, uint32_t) noexcept {
    return __writeMappedDataBuffer(this->_device->context(), this->_bufferSize, this->_allocation,
                                   allocationOffset(), sourceData);
  }
  template <> inline bool __P_BUFFER_STAGING_CLASS::write(const void* sourceData, uint32_t) noexcept {
    return __writeMappedDataBuffer(this->_device->context(), this->_bufferSize, this->_allocation,
                                   allocationOffset(), sourceData);
  }

# undef __P_BUFFER_STATIC_CLASS
# undef __P_BUFFER_IMMUTABLE_CLASS
# undef __P_BUFFER_DYNAMIC_CLASS
# undef __P_BUFFER_STAGING_CLASS
# undef __if_constexpr
#endif
