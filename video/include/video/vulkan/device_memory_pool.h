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
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cassert>
# include <cstdint>
# include "video/vulkan/_private/_memory.h" // includes vulkan
# include "video/vulkan/_private/_shared_resource.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        /// @class DeviceMemoryPool
        /// @brief Common/shared device memory allocation for buffer/texture/image suballocation:
        ///          * not required and mainly useful for big projects or advanced optimizations.
        ///          * used to create Buffer/Texture objects with suballocation.
        /// @remarks Suballocation in same memory block (Vulkan/D3D12 specific, mocked with higher-level APIs):
        ///            * many GPU drivers limit the total number of allocations (usually 4096 or more allocations).
        ///            * to work around this limit & reduce time spent on allocations, the same memory allocation
        ///              (large allocation) may be bound with various buffer/texture objects.
        ///            * vertex/index buffers, uniform buffers, and textures require different memory alignments:
        ///              - vertex/index: usually 16-byte blocks (verify with Buffer<>::Builder.requiredAlignment());
        ///              - uniform: at least 256-byte blocks (verify with Buffer<>::Builder.requiredAlignment());
        ///              - texture: at least 16-byte blocks (verify with Texture<>::Builder.requiredAlignment()).
        ///            * mixing too many buffer/image types in the same memory pool is not recommended:
        ///              - may require lots of padding for alignments -> waste of space;
        ///              - if too many types are requested in constructor, the API may not find a suitable memory type;
        ///              - mixing buffers and texture may result in inefficient data access.
        ///            * do not mix linear resources and optimal/non-linear resources in the same memory pool
        ///              (to avoid aliasing, reduce memory fragmentation and avoid waste of memory).
        ///              If mixed, additional padding is required between adjacent resources of different linearity.
        ///              (required padding = max(Renderer.deviceLimits().bufferImageGranularity, alignment).
        ///            * to use suballocation:
        ///              - create large DeviceMemoryPool instance (256MB for example);
        ///              - create Buffer/Texture builder with compatible usage/transfer/concurrency;
        ///              - build Buffer/Texture objects with suballocation: build(memoryPool, byteOffset);
        ///              - align byteOffset: DeviceMemoryPool::align(desiredOffset, builder.requiredAlignment());
        ///              - make sure the DeviceMemoryPool won't be destroyed before any Buffer/Texture using it.
        /// @warning Attached Buffer/Texture objects must be destroyed BEFORE the DeviceMemoryPool instance.
        class DeviceMemoryPool final {
        public:
          /// @brief Create memory pool allocation (for suballocations)
          /// @param device        Rendering device instance hosting the memory.
          /// @param poolByteSize  Total byte size of the allocation (must be a multiple of 256 bytes).
          /// @param builders      Array of Buffer<>::Builder or Texture<>::Builder instances (can't be NULL).
          ///                      All builders must have the same usage (Buffer/Texture<_Usage> and 'specialMemUsage').
          ///                      This list doesn't need to contain every single buffer/texture that will be bound.
          ///                      It should contain (at least) one buffer/texture builder of each of the configurations
          ///                      to support (type/transferMode/specialFlags/sharingMode/tiling).
          /// @param count         Array length of 'builders' (can't be 0).
          /// @remarks Typical use case:
          ///            - create array of builders with appropriate params, but do not build resources yet;
          ///            - compute total size of memory pool (sum of builder.requiredMemorySize(), each item aligned):
          ///              * size_t totalSize = 0;
          ///              * for each builder {
          ///              *   alignedOffsetUsedLater = DeviceMemoryPool::align(totalSize, builder.requiredAlignment());
          ///              *   totalSize = alignedOffsetUsedLater + builder.requiredMemorySize();
          ///              * }
          ///              * totalSize = DeviceMemoryPool::alignPoolSize(totalSize); // adjust to multiple of 256 bytes
          ///            - create memory pool: provide total size and array of builders;
          ///            - for each builder: builder.build(memoryPool, alignedOffset).
          /// @throws - out_of_range: if GPU doesn't support all buffer/image types at once;
          ///         - runtime_error: on creation failure.
          template <typename _Builder>
          DeviceMemoryPool(DeviceResourceManager device, size_t poolByteSize,
                           const _Builder* builders, uint32_t count)
            : _context(device->context()), _poolSize(poolByteSize) {
            uint32_t memoryTypeBits = 0xFFFFFFFFu;
            VkMemoryPropertyFlags preferred = (VkMemoryPropertyFlags)0xFFFFFFFFu;
            const _Builder* endIt = builders + (intptr_t)count;
            for (const _Builder* it = builders; it < endIt; ++it) { // find common memory type bits
              preferred &= it->_preferredUsage();
              memoryTypeBits &= it->memoryTypeBits();
            }
            // if missing some "preferred" bits: incompatible "preferred" types mixed -> use "required" type instead
            if (preferred != builders->_preferredUsage() && builders->_requiredUsage() != (VkMemoryPropertyFlags)0)
              preferred = builders->_requiredUsage(); // "required": same for all builders (based on class template)

            // memory allocation
            _memoryTypeIndex = device->memoryProps().findMemoryTypeIndex(memoryTypeBits,
                                                                         builders->_requiredUsage(), preferred);
            if (_memoryTypeIndex == MemoryProps::indexNotFound())
              __throwMemoryTypeNotFound();
            _allocation = _allocMemoryPool(_context, poolByteSize, _memoryTypeIndex); // throws
          }
          
          /// @brief Create memory pool allocation (for suballocations)
          /// @param device          Rendering device instance hosting the memory.
          /// @param poolByteSize    Total byte size of the allocation (must be a multiple of 256 bytes).
          /// @param memoryTypeIndex Target memory type (must be compatible will all resources that will be bound).
          inline DeviceMemoryPool(DeviceResourceManager device, size_t poolByteSize, uint32_t memoryTypeIndex)
            : _allocation(_allocMemoryPool(device->context(), poolByteSize, memoryTypeIndex)),
              _context(device->context()), _poolSize(poolByteSize), _memoryTypeIndex(memoryTypeIndex) {}
          
          DeviceMemoryPool() noexcept = default; ///< Empty -- not usable (only useful for variables with deferred init)
          DeviceMemoryPool(const DeviceMemoryPool&) = delete;
          DeviceMemoryPool(DeviceMemoryPool&&) noexcept;
          DeviceMemoryPool& operator=(const DeviceMemoryPool&) = delete;
          DeviceMemoryPool& operator=(DeviceMemoryPool&&) noexcept;
          inline ~DeviceMemoryPool() noexcept { release(); }
          
          void release() noexcept; ///< Destroy/release memory (do not call before destroying all bound resources!)

          // -- accessors --

          /// @brief Get native vulkan memory allocation handle
          inline VkDeviceMemory handle() const noexcept { return _allocation; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (_allocation == VK_NULL_HANDLE); }
          
          /// @brief Get total memory pool byte size
          inline size_t size() const noexcept { return _poolSize; }
          /// @brief Get memory type index used by the allocation
          inline uint32_t memoryTypeIndex() const noexcept { return _memoryTypeIndex; }

          // -- helpers --
          
          /// @brief Compute a pool size that is a multiple of 256 bytes
          /// @remarks Should be called to adjust the total pool size before creating a memory pool.
          static constexpr inline size_t alignPoolSize(size_t totalSize) noexcept {
            return (totalSize & 0xFFu) ? (totalSize & ~(size_t)0xFFu) + 256u : totalSize;
          }
          /// @brief Compute aligned byte offset
          /// @remarks Should be used to call Buffer/Texture::Builder.build for suballocations.
          /// @warning Alignment value can't be 0 (crash for division by 0).
          static constexpr inline size_t align(size_t byteOffset, size_t alignment) noexcept {
            assert(alignment != 0);
            return ((byteOffset + alignment - 1u)/alignment)*alignment;
          }

        private:
          static VkDeviceMemory _allocMemoryPool(DeviceContext context, size_t poolSize,
                                                 uint32_t memoryTypeIndex); // throws
        private:
          VkDeviceMemory _allocation = VK_NULL_HANDLE;
          DeviceContext _context = VK_NULL_HANDLE;
          size_t _poolSize = 0;
          uint32_t _memoryTypeIndex = 0;
        };
      }
    }
  }

#endif
