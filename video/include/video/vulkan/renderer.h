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
# include <cstdint>
# include <memory>
# include <hardware/display_monitor.h>
# include "./api/_private/_dynamic_array.h"
# include "./api/types.h"      // includes vulkan
# include "./scissor.h"        // includes vulkan
# include "./viewport.h"       // includes vulkan
# include "./shader.h"         // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        class SwapChain;
        class DisplaySurface;
        class GraphicsPipeline;

        /// @class VulkanInstance
        /// @brief Vulkan driver client instance, used to initialize Renderer objects
        class VulkanInstance final {
        public:
          VulkanInstance(const VulkanInstance&) = delete;
          VulkanInstance& operator=(const VulkanInstance&) = delete;
          ~VulkanInstance() noexcept; ///< Destroy Vulkan instance

          /// @brief Create Vulkan instance for Renderer object(s)
          /// @param appName                Application title (instance identification) or NULL.
          /// @param appVersion             Application version, built with VK_MAKE_VERSION(major, minor, patch).
          /// @param featureLevel           Feature level supported by the application (ex: VK_API_VERSION_1_2).
          /// @param instanceAdditionalExts Array of additional instance extension to enable (or NULL).
          ///                               Note that 'surface', 'platform-specific surface', 'get physical device properties2', 'display'
          ///                               and 'driver properties' extensions are already enabled if supported (you should NOT specify them in your array).
          ///                               Before using specific extensions, make sure they're supported (VulkanLoader::findExtensions).
          /// @param additionalExtCount     Array size for 'instanceAdditionalExts'.
          /// @throws - runtime_error: creation failure (vulkan not supported, missing extensions, feature level too high...).
          ///         - bad_alloc: memory allocation failure.
          static std::shared_ptr<VulkanInstance> create(const char* appName = nullptr,
                                                        uint32_t appVersion = VK_MAKE_VERSION(1,0,0),
                                                        uint32_t featureLevel = VK_API_VERSION_1_2,
                                                        const char** instanceAdditionalExts = nullptr,
                                                        size_t additionalExtCount = 0);

          inline VkInstance vkInstance() const noexcept { return this->_instance; }     ///< Get vulkan instance
          inline uint32_t featureLevel() const noexcept { return this->_featureLevel; } ///< Get instance API level

        private:
          VulkanInstance(const VkInstanceCreateInfo& instanceInfo, uint32_t featureLevel); // private: call 'VulkanInstance::create' instead

          VkInstance _instance = VK_NULL_HANDLE;
          uint32_t _featureLevel = VK_API_VERSION_1_2;
#         if defined(_DEBUG) || !defined(NDEBUG)
            VkDebugUtilsMessengerEXT _messenger = VK_NULL_HANDLE;
#         endif
        };

        // ---
        
        /// @class Renderer
        /// @brief Vulkan rendering device and context (specific to adapter)
        /// @warning - Renderer and VulkanInstance are the main Vulkan resources, and should be kept alive while the program runs.
        ///          - If the adapter changes (GPU switching, different monitor on multi-GPU system...), a new Renderer must be created.
        ///          - Accessors to native Vulkan resources should be reserved for internal usage or for advanced features.
        /// @remarks - To render on display output, create a SwapChain + setActiveRenderTarget with it and a depth buffer.
        ///          - To render to a texture, create a TextureTarget2D + setActiveRenderTarget with it and a depth buffer.
        ///          - Multiple active targets can be used simultaneously, to render the same image on multiple outputs/textures.
        ///          - Multi-window rendering (same adapter): alternate between different SwapChain instances on the same Renderer.
        ///          - Multi-window rendering (different adapters): use different Render instances with their own SwapChain.
        ///          - Split-screen rendering (same window): alternate between different Viewport instances on the same SwapChain.
        ///          - Optimization: minimize shader changes, state changes, drawing calls: sort by shader, then texture/material, then other states.
        ///          - Optimization: order of meshes and polygons: front to back (if a depth buffer is enabled).
        ///          - Optimization: align vertex buffer entries to a multiple of 16-byte per entry (add padding if necessary).
        class Renderer final {
        public:
          /// @brief Create Vulkan rendering device and context
          /// @param monitor   Target display monitor for the renderer: used to determine the adapter to choose.
          /// @param instance  Vulkan driver instance (or NULL to create default instance).
          /// @param features  Features to support in device context (or 'defaultFeatures()' to enable standard features used by toolbox).
          ///                  If you choose advanced features, you'll probably need to specify 'deviceExtensions' as well.
          ///                  Warning: if some features are disabled, functionalities of the toolbox depending on them won't be usable anymore.
          /// @param areFeaturesRequired If some features are not supported by the GPU, throw error (true) or just disable them (false).
          ///                            If set to 'false', call 'enabledFeatures()' after creation to verify if something's missing.
          /// @param deviceExtensions  Custom array of device extension to enable
          ///                          (or NULL to enable all standard extensions used by the toolbox).
          ///                          Before using specific extensions, make sure they're supported (VulkanLoader::findExtensions).
          ///                          Warning: if the value is not NULL, no other extension than those specified here will be enabled
          ///                          -> functionalities of the toolbox depending on missing extensions won't be usable anymore.
          /// @param extensionCount    Array size for 'deviceExtensions'.
          /// @param commandQueueCount Number of parallel command queues created (usually one per swap-chain/target).
          /// @throws - runtime_error: creation failure.
          ///         - bad_alloc: memory allocation failure.
          Renderer(const pandora::hardware::DisplayMonitor& monitor, std::shared_ptr<VulkanInstance> instance = nullptr,
                   const VkPhysicalDeviceFeatures& features = defaultFeatures(), bool areFeaturesRequired = false,
                   const char** deviceExtensions = nullptr, size_t extensionCount = 0, size_t commandQueueCount = 1);
          /// @brief Destroy device and context resources
          ~Renderer() noexcept { _destroy(); }
          
          Renderer(const Renderer&) = delete;
          Renderer(Renderer&& rhs) noexcept;
          Renderer& operator=(const Renderer&) = delete;
          Renderer& operator=(Renderer&& rhs) noexcept;

          /// @brief Create default list of vulkan features (all basic standard features enabled)
          /// @remarks Used to create a Renderer object.
          ///          Should be fine-tuned to improve performance or to support special shader features.
          static VkPhysicalDeviceFeatures defaultFeatures() noexcept;
          /// @brief Get list of features enabled in vulkan renderer
          inline const VkPhysicalDeviceFeatures& enabledFeatures() const noexcept { return *(this->_features); }
          /// @brief Get limits of physical device associated with device context
          inline const VkPhysicalDeviceLimits& deviceLimits() const noexcept { return this->_physicalDeviceInfo->limits; }
          
          /// @brief Flush command buffers
          /// @remarks Should only be called: - just before a long CPU wait (ex: sleep)
          ///                                 - to wait for object destruction
          void flush() noexcept;
          

          // -- accessors --

          /// @brief Command queues from one family (with graphics support)
          struct CommandQueues final {
            DynamicArray<VkQueue> commandQueues = VK_NULL_HANDLE;
            uint32_t familyIndex = 0;
          };
          
          inline DeviceHandle device() const noexcept { return this->_physicalDevice; }  ///< Get physical rendering device (VkPhysicalDevice)
          inline DeviceContext context() const noexcept { return this->_deviceContext; } ///< Get logical device context (VkDevice)
          inline DeviceResourceManager resourceManager() const noexcept { return this->_deviceContext; } ///< Get resource manager (to build resources such as shaders)
          inline VkInstance vkInstance() const noexcept { return this->_instance->vkInstance(); }   ///< Get Vulkan instance
          inline uint32_t featureLevel() const noexcept { return this->_instance->featureLevel(); } ///< Get instance API level (VK_API_VERSION_1_2...)
          inline const DynamicArray<CommandQueues>& commandQueues() const noexcept { return this->_graphicsQueuesPerFamily; } ///< Get Vulkan command queues (per family)
          
          /// @brief Read device adapter VRAM size
          /// @returns Read success
          bool getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept;
          /// @brief Convert color/depth/component data format to native format (usable in input layout description) -- see "video/vulkan/shader.h".
          static constexpr inline VkFormat toLayoutFormat(DataFormat format) noexcept { return _getDataFormatComponents(format); }

          /// @brief Convert standard sRGB(A) color to gamma-correct linear RGB(A)
          /// @remarks Should be called to obtain colors to use with 'clearView(s)', 'setCleanActiveRenderTarget(s)', 'RendererStateFactory.create<...>Filter'
          static void sRgbToGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept;
          
          
          // -- feature support --
          
          /// @brief Detect current color space used by display monitor -- not supported with Vulkan
          /// @remarks Returns "unknown" with Vulkan (color space detection not supported)
          ///          (-> it's better to default to sRGB and let the user choose to optionally enable HDR).
          ColorSpace getMonitorColorSpace(const pandora::hardware::DisplayMonitor&) const noexcept { return ColorSpace::unknown; }

          /// @brief Screen tearing supported (variable refresh rate display)
          /// @remarks The variableMultisampleRate feature must have been enabled in constructor.
          inline bool isTearingAvailable() const noexcept { return static_cast<bool>(this->_features->variableMultisampleRate); }
          
          
          // -- render target operations --

          /// @brief Max number of simultaneous viewports/scissor-test rectangles per pipeline
          inline size_t maxViewports() noexcept { return this->_features->multiViewport ? this->_physicalDeviceInfo->limits.maxViewports : 1; }
          
          /// @brief Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
          /// @warning - With Vulkan, this viewport change is only supported if the GraphicsPipeline
          ///            was configured with dynamic viewports.
          ///            -> this command will fail if the pipeline viewport is static/fixed.
          ///          - Multiple viewports are only supported if 'multiViewport' feature was enabled in constructor
          ///            (enabled in defaultFeatures()).
          void setViewports(const Viewport* viewports, size_t numberViewports) noexcept;
          /// @brief Replace rasterizer viewport (3D -> 2D projection rectangle)
          /// @warning With Vulkan, this viewport change is only supported if the GraphicsPipeline
          ///          was configured with a dynamic viewport.
          ///          -> this command will fail if the pipeline viewport is static/fixed.
          void setViewport(const Viewport& viewport) noexcept;

          /// @brief Set rasterizer scissor-test rectangle(s)
          /// @warning - With Vulkan, this scissor-test change is only supported if the GraphicsPipeline
          ///            was configured with dynamic scissor-tests.
          ///            -> this command will fail if the pipeline scissor-test is static/fixed.
          void setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept;
          /// @brief Set rasterizer scissor-test rectangle
          /// @warning With Vulkan, this scissor-test change is only supported if the GraphicsPipeline
          ///          was configured with a dynamic scissor-test.
          ///          -> this command will fail if the pipeline scissor-test is static/fixed.
          void setScissorRectangle(const ScissorRectangle& rectangle) noexcept;
          
          // ---

          /// @brief Max number of simultaneous render-target views (swap-chains, texture targets...)
          inline size_t maxRenderTargets() noexcept { return this->_physicalDeviceInfo->limits.maxColorAttachments; }
          
          // -- pipeline status operations - shaders --

          inline void bindGraphicsPipeline(VkPipeline pipeline) noexcept {
            this->_boundPipeline = pipeline;
            //...
          }

          
        private:
          void _destroy() noexcept;
          inline bool _areColorSpacesAvailable() const noexcept { return (this->_instance->featureLevel() != VK_API_VERSION_1_0); }
          friend class pandora::video::vulkan::SwapChain;
          friend class pandora::video::vulkan::DisplaySurface;
          friend class pandora::video::vulkan::GraphicsPipeline;
          
        private:
          std::shared_ptr<VulkanInstance> _instance = nullptr;
          std::unique_ptr<VkPhysicalDeviceFeatures> _features = nullptr;
          std::unique_ptr<VkPhysicalDeviceProperties> _physicalDeviceInfo = nullptr;
          DeviceHandle _physicalDevice = VK_NULL_HANDLE;
          DeviceContext _deviceContext = VK_NULL_HANDLE;
          DynamicArray<CommandQueues> _graphicsQueuesPerFamily;

          VkPipeline _boundPipeline = VK_NULL_HANDLE;
        };

        // Throw native error message (or default if no message available)
        void throwError(VkResult result, const char* messagePrefix);
      }
    }
  }
# include "./swap_chain.h"        // includes vulkan
# include "./graphics_pipeline.h" // includes vulkan
#endif
