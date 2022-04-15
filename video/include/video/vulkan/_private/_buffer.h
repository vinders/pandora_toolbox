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
# include "_memory.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        // -- buffer params -- -------------------------------------------------
        
        // Set buffer sharing config
        void __setBufferSharingMode(VkBufferCreateInfo& outBufferInfo,
                                   uint32_t* concurrentQueueFamilies, uint32_t queueCount) noexcept;
        

        // -- buffer builder/container -- --------------------------------------
        
        // Create buffer descriptor (vertex/index/uniform/...) + read memory requirements
        // (used for first creation in buffer builder -> more verifications + read requirements)
        // -> throws invalid_argument if buffer size is 0
        // -> throws runtime_error on failure
        VkBuffer __createBuffer(DeviceContext context, const VkBufferCreateInfo& params,
                                VkMemoryRequirements2& outRequirements);
        // Create buffer descriptor (vertex/index/uniform/...)
        // -> throws runtime_error on failure
        VkBuffer __createBuffer(DeviceContext context, const VkBufferCreateInfo& params);
        
        // Destroy/release buffer instance (only alloc verified)
        void __destroyBuffer(DeviceContext context, BufferHandle handle, VkDeviceMemory allocation) noexcept;
        

        // -- buffer read/write -- ---------------------------------------------
        
        // Write dynamic/staging buffer via memory mapping
        bool __writeMappedBuffer(DeviceContext context, VkMemoryPropertyFlags memoryUsage, VkDeviceMemory allocation,
                                 size_t allocOffset, size_t bufferSize, const void* sourceData) noexcept;
        // Write local/static buffer by filling and copying a staging buffer
        bool __writeLocalBuffer(DeviceContext context, const MemoryProps& memoryProps,
                                VkCommandPool commandPool, VkQueue copyCommandQueue,
                                BufferHandle buffer, size_t bufferSize, const void* sourceData) noexcept;
        
        // Copy mappable buffer content to other mappable buffer
        bool __copyMappedBuffer(DeviceContext context, VkDeviceMemory source, size_t sourceOffset, VkDeviceMemory dest,
                                size_t destOffset, size_t bufferSize, bool isHostCoherent) noexcept;
        // Copy buffer content to other buffer (command list)
        bool __copyLocalBuffer(DeviceContext context, VkCommandPool commandPool, VkQueue commandQueue,
                               BufferHandle source, BufferHandle dest,
                               const VkBufferCopy* regions, uint32_t regionCount) noexcept;
      
        void __throwWriteError(); // Throw runtime_error exception if required buffer/texture writing failed
      }
    }
  }
  
#endif
