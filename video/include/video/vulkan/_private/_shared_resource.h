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

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cassert>
# include <cstdint>
# include <memory>
# include "video/vulkan/api/types.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        class Renderer;
        
        // Throw native error message (or default if no message available)
        void throwError(VkResult result, const char* messagePrefix);
        
        /// @brief RAII container for native device rendering context
        class ScopedDeviceContext final {
        public:
          ScopedDeviceContext() noexcept = default; ///< Empty container
          ScopedDeviceContext(const ScopedDeviceContext&) = delete;
          ScopedDeviceContext& operator=(const ScopedDeviceContext&) = delete;
          inline ScopedDeviceContext(ScopedDeviceContext&& rhs) noexcept
            : _deviceContext(rhs._deviceContext), _physicalDevice(rhs._physicalDevice),
              _graphicsQueuesPerFamily(std::move(rhs._graphicsQueuesPerFamily)),
              _transientCommandPool(rhs._transientCommandPool) {
            rhs._deviceContext = VK_NULL_HANDLE;
            rhs._physicalDevice = VK_NULL_HANDLE;
            rhs._transientCommandPool = VK_NULL_HANDLE;
          }
          inline ScopedDeviceContext& operator=(ScopedDeviceContext&& rhs) noexcept {
            release();
            _deviceContext=rhs._deviceContext; _physicalDevice=rhs._physicalDevice;
            _graphicsQueuesPerFamily=std::move(rhs._graphicsQueuesPerFamily);
            _transientCommandPool=rhs._transientCommandPool;
            rhs._deviceContext = VK_NULL_HANDLE;
            rhs._physicalDevice = VK_NULL_HANDLE;
            rhs._transientCommandPool = VK_NULL_HANDLE;
            return *this;
          }
          inline ~ScopedDeviceContext() noexcept { release(); }
          
          ScopedDeviceContext(VkDevice deviceContext, VkPhysicalDevice physicalDevice) noexcept ///< Initialize device rendering context
            : _deviceContext(deviceContext), _physicalDevice(physicalDevice) {}
          void release() noexcept; ///< Destroy device rendering context
          
          inline VkPhysicalDevice device() const noexcept { return this->_physicalDevice; } ///< Get physical rendering device (VkPhysicalDevice)
          inline VkDevice context() const noexcept { return this->_deviceContext; }         ///< Get logical device context (VkDevice)
          inline const DynamicArray<CommandQueues>& commandQueues() const noexcept { return this->_graphicsQueuesPerFamily; } ///< Vulkan command queues (per family)
          inline VkCommandPool transientCommandPool() const noexcept { return this->_transientCommandPool; } ///< Get command pool for short-lived operations
          inline const CommandQueues& transientCommandQueues() const noexcept { ///< Get queue family for transient command pool
            return _graphicsQueuesPerFamily.value[_transientQueuesArrayIndex];
          }
          
        private:
          inline void _setGraphicsQueues(DynamicArray<CommandQueues>&& commandQueues) noexcept {
            this->_graphicsQueuesPerFamily = std::move(commandQueues);
          }
          inline void _setTransientCommandPool(VkCommandPool commandPool, uint32_t queuesArrayIndex) noexcept {
            this->_transientCommandPool = commandPool;
            this->_transientQueuesArrayIndex = queuesArrayIndex;
          }
          friend class Renderer;
          
        private:
          VkDevice _deviceContext = VK_NULL_HANDLE;
          VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
          DynamicArray<CommandQueues> _graphicsQueuesPerFamily; // command queues
          VkCommandPool _transientCommandPool = VK_NULL_HANDLE; // for short-lived operations
          uint32_t _transientQueuesArrayIndex = 0;
        };
        using DeviceResourceManager = std::shared_ptr<ScopedDeviceContext>; ///< Device resource manager
        
        // ---
        
        /// @brief RAII container for native vulkan resources
        template <typename T>
        class ScopedResource final {
        public:
          inline ScopedResource(T handle, DeviceResourceManager context,
                                void (*destroyFunc)(VkDevice,T,const VkAllocationCallbacks*)) noexcept
            : _handle(handle), _context(std::move(context)), _destroy(destroyFunc) {
            assert(this->_context != nullptr && this->_destroy != nullptr);
          }
                                
          ScopedResource() noexcept = default; ///< Empty resource
          inline ~ScopedResource() noexcept { release(); } ///< Safe destruction (if scope exit or exception)
          
          // -- assign/move/release --
          
          ScopedResource(const ScopedResource<T>&) = delete;
          inline ScopedResource(ScopedResource<T>&& rhs) noexcept
            : _handle(rhs._handle), _context(std::move(rhs._context)), _destroy(rhs._destroy) {
            rhs._handle = VK_NULL_HANDLE; rhs._context = nullptr;
          }
          
          ScopedResource<T>& operator=(const ScopedResource<T>&) = delete;
          ScopedResource<T>& operator=(ScopedResource<T>&& rhs) noexcept {
            release();
            _handle = rhs._handle; _context = std::move(rhs._context); _destroy = rhs._destroy;
            rhs._handle = VK_NULL_HANDLE; rhs._context = nullptr;
            return *this;
          }
          
          inline T extract() noexcept { ///< Return resource and release instance ownership
            T val = this->_handle;
            this->_handle = VK_NULL_HANDLE;
            return val;
          }
          void release() noexcept { ///< Destroy resource instance
            if (this->_handle != VK_NULL_HANDLE) {
              try { _destroy(this->_context->context(), this->_handle, nullptr); } catch (...) {}
              this->_handle = VK_NULL_HANDLE;
            }
          }
          
          // -- accessors --
          
          inline T operator*() const { return _handle; }
          inline T* operator->() const noexcept { return &_handle; }
          inline operator bool() const noexcept { return (_handle != VK_NULL_HANDLE); } ///< Verify validity
          inline bool hasValue() const noexcept { return (_handle != VK_NULL_HANDLE); } ///< Verify validity
          inline T value() const noexcept { return _handle; } ///< Read value (no verification -> call hasValue() first!)

          inline bool operator==(const ScopedResource<T>& rhs) const noexcept { return (_handle == rhs._handle); }
          inline bool operator!=(const ScopedResource<T>& rhs) const noexcept { return (_handle != rhs._handle); }
          inline VkDevice context() const noexcept { return _context->context(); } ///< Get associated context (call hasValue() first!)
          
        private:
          T _handle = VK_NULL_HANDLE;
          DeviceResourceManager _context = nullptr;
          void (*_destroy)(VkDevice,T,const VkAllocationCallbacks*) = nullptr;
        };

        template <typename T>
        using SharedResource = std::shared_ptr<ScopedResource<T> >; ///< Shared RAII container for native vulkan resources
        
        // ---
        
        struct RenderPass final {
          ScopedResource<VkRenderPass> handle;
          uint32_t maxColorAttachmentCount = 0;
        };
        using SharedRenderPass = std::shared_ptr<RenderPass>;             ///< Render-pass description object

        using InputLayout = std::shared_ptr<InputLayoutDescription>; ///< Input layout representation, for shader input stage
        using GlobalLayout = SharedResource<VkPipelineLayout>;       ///< Description object for globals: uniform buffers, storage buffers, samplers
        
        using GraphicsPipelineHandle = VkPipeline; ///< Native handle of GraphicsPipeline object (GraphicsPipeline.handle())
      }
    }
  }
#endif
