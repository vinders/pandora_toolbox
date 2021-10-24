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
# include <cstddef>
# include <cstdint>
# include <memory/light_string.h>
# include "./api/types.h"      // includes vulkan

# ifdef _WINDOWS
#   define __SYSTEM_PATH_CHAR wchar_t
# else
#   define __SYSTEM_PATH_CHAR char
# endif

  namespace pandora {
    namespace video {
      namespace vulkan {
        class InputLayout;
        
        /// @class Shader
        /// @brief GPU shading program/effects for Vulkan renderer
        class Shader final {
        public:
          using Handle = VkShaderModule;
          
          /// @brief Create usable shader stage object -- reserved for internal use or advanced usage
          /// @remarks Prefer Shader::Builder for standard usage
          Shader(Handle handle, ShaderType type, DeviceResourceManager device, const char* entryPoint = "main");
          
          Shader() = default; ///< Empty shader -- not usable (only useful to store variable not immediately initialized)
          Shader(const Shader&) = delete;
          Shader(Shader&& rhs) noexcept
            : _stageInfo(rhs._stageInfo), _entryPoint(std::move(rhs._entryPoint)), _context(rhs._context) {
            rhs._stageInfo.module = VK_NULL_HANDLE;
          }
          Shader& operator=(const Shader&) = delete;
          Shader& operator=(Shader&& rhs) noexcept {
            this->_stageInfo=rhs._stageInfo; this->_entryPoint=std::move(rhs._entryPoint); this->_context=rhs._context;
            rhs._stageInfo.module = VK_NULL_HANDLE;
            return *this;
          }
          ~Shader() noexcept { release(); }
          void release() noexcept; ///< Destroy shader object
          
          
          // -- accessors --
          
          /// @brief Get native shader handle -- for internal use or advanced features
          inline Handle handle() const noexcept { return (Handle)this->_stageInfo.module; }
          inline ShaderType type() const noexcept { return (ShaderType)this->_stageInfo.stage; } ///< Get shader category/model type
          inline bool isEmpty() const noexcept { return (this->_stageInfo.module == VK_NULL_HANDLE); } ///< Verify if initialized (false) or empty/moved/released (true)
          /// @brief Get native shader stage info -- should be used to customize advanced settings: 'pSpecializationInfo', 'flags'...
          inline VkPipelineShaderStageCreateInfo& entryPoint() noexcept { return this->_stageInfo; }
          inline const VkPipelineShaderStageCreateInfo& entryPoint() const noexcept { return this->_stageInfo; }


          // -- create/compile shaders --
          
          /// @class Shader.Builder
          /// @brief Shader compiler and build (used to create shaders and input layouts).
          class Builder final {
          public:
            /// @brief Initialize shader builder with already compiled SPIR-V shader data.
            ///        To compile a shader from text, use 'Shader::Builder::compile' or 'Shader::Builder::compileFromFile' instead.
            ///        The Cmake option CWORK_SHADER_COMPILERS is required to compile shaders.
            /// @param binaryFileData  SPIR-V compiled binary data.
            /// @param length          Number of bytes in 'binaryFileData'.
            /// @param entryPoint      Name of the entry point function in the shader ("main"/"Main"/"VSMain"/"PsMain"/"MyShaderMain"/...).
            /// @remarks - Typically used when reading binary SPIR-V-compiled file.
            ///          - Can also be used with compiled shader data stored in memory.
            ///          - Will NOT work with GLSL text data: use 'Builder::compile' or 'Builder::compileFromFile' instead.
            /// @warning - 'binaryFileData' must remain available at least until the Builder is destroyed.
            ///          - 'binaryFileData' is NOT managed by the builder: if memory comes from a dynamic alloc, 
            ///            it won't be freed by the Builder -> needs to be freed manually (after destroying the Builder instance).
            Builder(ShaderType type, const uint8_t* binaryFileData, size_t length, const char* entryPoint = "main")
              : _data(binaryFileData), _length(length), _type(type), _entryPoint(entryPoint) {}
            
            Builder(const Builder&) = delete;
            Builder(Builder&& rhs) noexcept = default;
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&& rhs) noexcept = default;
            ~Builder() noexcept = default;

#           ifdef _P_VIDEO_SHADER_COMPILERS // option CWORK_SHADER_COMPILERS required
              /// @brief Compile SPIR-V shader from GLSL text content
              /// @param type         Shader category/model.
              /// @param textContent  ASCII uncompiled text data.
              /// @param length       Number of bytes in 'textContent'.
              /// @param entryPoint   Name of the entry point function in the shader ("main"/"Main"/"VSMain"/"PsMain"/"MyShaderMain"/...).
              /// @param spirvTarget  SPIR-V output target version (ex: 15 for 1.5).
              /// @param defaultVersion          Default language version to use (#version 450 / #define VULKAN 100 / #define VULKAN 110 / ...).
              /// @param customTBuiltInResources Pointer to a TBuiltInResources to use with parser (will use defaults otherwise)
              /// @throws runtime_error on failure
              static Builder compile(ShaderType type, const char* textContent, size_t length, 
                                     const char* entryPoint = "main", uint32_t clientTarget = VK_API_VERSION_1_2,
                                     uint32_t spirvTarget = 15, uint32_t defaultVersion = 100,
                                     void* customTBuiltInResource = nullptr);
              /// @brief Compile SPIR-V shader from GLSL text file
              /// @param type         Shader category/model.
              /// @param filePath     File to compile (absolute path / relative path to current directory).
              /// @param entryPoint   Name of the entry point function in the shader ("main"/"Main"/"VSMain"/"PsMain"/"MyShaderMain"/...).
              /// @param spirvTarget  SPIR-V output target version (ex: 15 for 1.5).
              /// @param defaultVersion          Default language version to use (#version 450 / #define VULKAN 100 / #define VULKAN 110 / ...).
              /// @param customTBuiltInResources Pointer to a TBuiltInResources to use with parser (will use defaults otherwise)
              /// @throws runtime_error on failure
              static Builder compileFromFile(ShaderType type, const __SYSTEM_PATH_CHAR* filePath, 
                                             const char* entryPoint = "main", uint32_t clientTarget = VK_API_VERSION_1_2,
                                             uint32_t spirvTarget = 15, uint32_t defaultVersion = 100,
                                             void* customTBuiltInResource = nullptr);
#           endif
            
            // -- create shader objects --
            
            /// @brief Create shader object
            /// @param deviceContext Device context handle from associated Renderer instance (Renderer.context())
            /// @throws runtime_error on failure
            Shader createShader(DeviceResourceManager device) const;
            
            /// @brief Create input layout for shader object
            /// @param device         Device handle from associated Renderer instance (Renderer.device())
            /// @param inputBindings     Array of input bindings (strides applied per vertex or per instance).
            /// @param bindingsLength    Number of elements in 'inputBindings'.
            /// @param layoutAttributes  Array of all input attributes for each binding.
            /// @param attributesLength  Number of elements in 'attributesLength'.
            /// @note  * very specific to each shading language -> currently no portable abstraction in toolbox;
            ///        * example:
            ///          VkVertexInputBindingDescription inputBindings[] = {
            ///            { 0, sizeof(MyVertexType), VK_VERTEX_INPUT_RATE_VERTEX }};
            ///          VkVertexInputAttributeDescription layoutAttributes[] = {
            ///            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MyVertexType, pos) },
            ///            { 1, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(MyVertexType, texcoord) },
            ///            { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(MyVertexType, normal) }};
            /// @remarks - The input layout is validated against shared input signature.
            ///          - The input layout may be bound with any other shader that has the same input signature.
            ///          - For engines with material shaders with different inputs, it's easier to verify shader params with reflection, instead of hardcoding them.
            /// @throws runtime_error if shader type isn't vertex or compute.
            InputLayout createInputLayout(VkVertexInputBindingDescription* inputBindings, size_t bindingsLength,
                                          VkVertexInputAttributeDescription* layoutAttributes, size_t attributesLength) const;
            
          private:
            Builder(ShaderType type, DynamicArray<char>&& shaderBuffer, const char* entryPoint)
              : _shaderBuffer(std::move(shaderBuffer)),
                _data(reinterpret_cast<const uint8_t*>(_shaderBuffer.value)),
                _length(static_cast<size_t>(_shaderBuffer.length())),
                _type(type),
                _entryPoint(entryPoint) {}
          private:
            DynamicArray<char> _shaderBuffer;
            const uint8_t* _data = nullptr;
            size_t _length = 0;
            ShaderType _type = ShaderType::vertex;
            pandora::memory::LightString _entryPoint;
          };
          
        private:
          VkPipelineShaderStageCreateInfo _stageInfo{};
          pandora::memory::LightString _entryPoint;
          DeviceResourceManager _context = VK_NULL_HANDLE;
        };
        
        // ---
        
        /// @class InputLayout
        /// @brief Data input layout for shader object(s)
        class InputLayout final {
        public:
          /// @brief Create usable input layout object -- reserved for internal use or advanced usage
          InputLayout(InputLayoutDescription&& description) : _description(std::move(description)) {}
          
          InputLayout() noexcept;
          InputLayout(const InputLayout&) = delete;
          InputLayout(InputLayout&& rhs) noexcept = default;
          InputLayout& operator=(const InputLayout&) = delete;
          InputLayout& operator=(InputLayout&& rhs) noexcept = default;
          ~InputLayout() noexcept { release(); }
          /// @brief Destroy input layout
          inline void release() noexcept {
            this->_description.bindings.clear();
            this->_description.attributes.clear();
          }
          
          inline InputLayoutHandle handle() const noexcept { return &(this->_description); } ///< Get native handle
          inline bool isEmpty() const noexcept { ///< Verify if initialized (false) or empty/moved/released (true)
            return (this->_description.bindings.length() == 0 && this->_description.attributes.length() == 0);
          } 
        
        private:
          InputLayoutDescription _description;
        };
      }
    }
  }
#endif
