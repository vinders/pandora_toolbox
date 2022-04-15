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
# include <cstdint>
# include <cstring>
# include "./_private/_shared_resource.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        /// @class DepthStencilBufferParams
        /// @brief Depth/stencil verification buffer configuration
        /// @remarks The same DepthStencilBufferParams can be used to build multiple instances (if needed).
        class DepthStencilBufferParams final {
        public:
          /// @brief Initialize depth/stencil buffer params
          /// @param sampleCount  Sample count for multisampling (anti-aliasing). Use 1 to disable multisampling.
          ///                     Call Renderer.is{Color/Depth/Stencil}SampleCountAvailable to verify supported values.
          /// @warning Width/height should be the same as the associated render-target size (swap-chain/texture).
          DepthStencilBufferParams(DepthStencilFormat format, uint32_t width, uint32_t height,
                                   uint32_t sampleCount = 1u) noexcept;
          
          DepthStencilBufferParams() noexcept; ///< Empty params -- not usable as is
          DepthStencilBufferParams(const DepthStencilBufferParams& rhs) noexcept = default;
          DepthStencilBufferParams& operator=(const DepthStencilBufferParams& rhs) noexcept = default;
          ~DepthStencilBufferParams() noexcept = default;
          
          // -- buffer format and size --
          
          /// @brief Set depth/stencil data format
          inline DepthStencilBufferParams& format(DepthStencilFormat format) noexcept {
            _params.format = (VkFormat)format;
            return *this;
          }
          /// @brief Set depth/stencil buffer width/height (texels)
          /// @warning Should be the same size as the associated render-target (swap-chain/texture).
          inline DepthStencilBufferParams& size(uint32_t width, uint32_t height) noexcept {
            _params.extent.width = width;
            _params.extent.height = height;
            return *this;
          }
          /// @brief Set sample count for multisampling (anti-aliasing). Use 1 to disable multisampling.
          /// @warning - Should be the same sample count as the associated render-target (swap-chain/texture).
          ///          - Call Renderer.is{Color/Depth/Stencil}SampleCountAvailable to make sure the value is supported.
          inline DepthStencilBufferParams& sampleCount(uint32_t count) noexcept {
            _params.samples = (VkSampleCountFlagBits)count;
            return *this;
          }

          // -- advanced features --
          
          /// @brief Set tiling arrangement for image data (optimal/linear)
          /// @warning For cross-API projects, avoid this method.
          inline DepthStencilBufferParams& tiling(VkImageTiling mode) noexcept {
            _params.tiling = mode;
            return *this;
          }
          /// @brief Set context sharing mode (exclusive / concurrent)
          /// @param concurrentQueueFamilies Concurrent sharing mode: array of queue family indices with access to buffer.
          ///                                Set NULL to use exclusive mode (recommended and usually faster).
          /// @param queueCount              Array size of 'concurrentQueueFamilies'.
          /// @warning For cross-API projects, avoid this method.
          DepthStencilBufferParams& sharingMode(uint32_t* concurrentQueueFamilies, uint32_t queueCount) noexcept;

          inline VkImageCreateInfo& descriptor() noexcept { return this->_params; } ///< Get native Vulkan descriptor
          inline const VkImageCreateInfo& descriptor() const noexcept { return this->_params; }///< Vulkan descriptor
          inline const VkImageCreateInfo* descriptorPtr() const noexcept { return &_params; }  ///< Vulkan descriptor
        
        private:
          VkImageCreateInfo _params{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        };
        
        
        // ---------------------------------------------------------------------
        // depth/stencil buffer container/builder
        // ---------------------------------------------------------------------
        
        /// @class DepthStencilBuffer
        /// @brief Vulkan depth/stencil fragment verification buffer/view for output merger
        /// @warning - The depth/stencil buffer must be destroyed BEFORE the associated Renderer instance!
        ///          - If the render-target is resized, a new DepthStencilBuffer must be created.
        ///          - If the adapter changes (GPU switching, monitor change with multi-GPU), it must also be re-created.
        /// @remarks The same depth/stencil buffer may be used with multiple/different render-targets
        ///          with the same size and sample count.
        class DepthStencilBuffer final {
        public:
          /// @brief Create depth/stencil buffer object -- reserved for internal use
          /// @remarks Prefer DepthStencilBuffer::Builder for standard usage
          /// @warning DepthStencilBuffer objects must be destroyed BEFORE the associated Renderer instance!
          DepthStencilBuffer(DeviceContext context, TextureHandle2D bufferHandle,
                             DepthStencilView bufferView, DepthStencilFormat format, uint32_t width,
                             uint32_t height, VkDeviceMemory memory) noexcept
            : _bufferView(bufferView), _bufferHandle(bufferHandle), _allocation(memory),
              _context(context), _pixelSize(_toPixelSize(width, height)), _format(format) {}

          DepthStencilBuffer() = default; ///< Empty buffer -- not usable (only useful for variables with deferred init)
          DepthStencilBuffer(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept;
          DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;
          DepthStencilBuffer& operator=(DepthStencilBuffer&& rhs) noexcept;
          ~DepthStencilBuffer() noexcept { release(); }
          void release() noexcept; ///< Destroy depth/stencil buffer
        
          // -- accessors --
          
          /// @brief Get native vulkan compatible depth/stencil buffer handle (cast to VkImage)
          inline TextureHandle2D handle() const noexcept { return _bufferHandle; }
          /// @brief Verify if initialized (false) or empty/moved/released (true)
          inline bool isEmpty() const noexcept { return (_bufferHandle == VK_NULL_HANDLE); }
          
          inline uint32_t width() const noexcept  { return (_pixelSize & 0xFFFFu); }///< Get buffer width
          inline uint32_t height() const noexcept { return (_pixelSize >> 16); }    ///< Get buffer height
          inline DepthStencilFormat format() const noexcept { return _format; }     ///< Get buffer data format
          
          /// @brief Get depth/stencil view of current DepthStencilBuffer
          /// @remarks - This value should be used to call 'Renderer.setActiveRenderTargets'.
          ///          - Activating the depth/stencil view is necessary for depth/stencil testing.
          inline DepthStencilView bufferView() const noexcept { return _bufferView; }


          // -- create buffers --
        
          /// @class DepthStencilBuffer.Builder
          /// @brief Create depth/stencil buffer for existing render-target(s)
          class Builder final {
          public:
            /// @brief Initialize depth/stencil buffer builder
            /// @warning - The DepthStencilBufferParams object must be kept alive while the Builder exists.
            ///          - The DepthStencilBufferParams object shouldn't be modified while still used by the Builder.
            /// @throws - invalid_argument: if width/height is 0;
            ///         - runtime_error: on buffer descriptor creation failure.
            Builder(DeviceResourceManager device, DepthStencilBufferParams& params); // non-const ref to force ext var
            
            Builder(const Builder&) = delete;
            Builder(Builder&& rhs) noexcept
              : _device(rhs._device), _params(rhs._params), _bufferHandle(rhs._bufferHandle),
                _memoryTypeIndex(rhs._memoryTypeIndex), _allocReq(rhs._allocReq), _memoryReq(rhs._memoryReq) {
              rhs._bufferHandle = VK_NULL_HANDLE;
            }
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&& rhs) noexcept {
              _destroyBuffer();
              memcpy(this, &rhs, sizeof(Builder));
              rhs._bufferHandle = VK_NULL_HANDLE;
              return *this;
            }
            /// @brief Destroy buffer builder
            inline ~Builder() noexcept { _destroyBuffer(); }
            
            // -- memory requirements --
            
            /// @brief Get dedicated allocation requirements.
            ///        Must be called to verify if a buffer:
            ///        * must use a dedicated allocation (Requirement::required)
            ///        * should use dedicated allocation to improve performance (Requirement::preferred).
            ///        * should use default allocation (Requirement::none).
            inline Requirement dedicatedAllocRequirement() const noexcept {
              return _allocReq.requiresDedicatedAllocation
                    ? Requirement::required
                    : (_allocReq.prefersDedicatedAllocation
                      ? Requirement::preferred
                      : Requirement::none);
            }
            /// @brief Get memory allocation size requirement (bytes).
            inline size_t requiredMemorySize() const noexcept {
              return (size_t)_memoryReq.memoryRequirements.size;
            }
            
            // -- build --
            
            /// @brief Create DepthStencilBuffer object
            /// @param dedicatedAllocMode  Force dedicated allocation (::force) or use default driver alloc (::disable).
            ///                            Set to 'FeatureMode::autodetect' to use the preferred allocation type.
            /// @throws runtime_error on failure
            DepthStencilBuffer build(FeatureMode dedicatedAllocMode = FeatureMode::autodetect);
          
          private:
            void _destroyBuffer() noexcept;
          private:
            DeviceResourceManager _device = nullptr;
            const VkImageCreateInfo* _params = nullptr;
            TextureHandle2D _bufferHandle = VK_NULL_HANDLE;
            uint32_t _memoryTypeIndex = 0;
          
            VkMemoryDedicatedRequirementsKHR _allocReq {
              VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR
            };
            VkMemoryRequirements2 _memoryReq {
                VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
                &_allocReq
            };
          };
          
        private:
          static constexpr inline uint32_t _toPixelSize(uint32_t width, uint32_t height) noexcept {
            return (width | (height << 16));
          }
          
        private:
          DepthStencilView _bufferView = VK_NULL_HANDLE;  // VkImageView
          TextureHandle2D _bufferHandle = VK_NULL_HANDLE; // VkImage
          VkDeviceMemory _allocation = VK_NULL_HANDLE;
          DeviceContext _context = nullptr;
          uint32_t _pixelSize = 0; // width / height
          DepthStencilFormat _format = DepthStencilFormat::d32_f;
        };
      }
    }
  }
  
#endif
