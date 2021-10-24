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
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_VIDEO_VULKAN_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- create/compile shaders -- ------------------------------------------------

# ifdef _P_VIDEO_SHADER_COMPILERS
    // GLGL shader compiler (glslang.lib)
    class __GlslangCompiler final {
    public:
      __GlslangCompiler() = default;
      ~__GlslangCompiler() noexcept {
        if (_isInitialized)
          glslang::FinalizeProcess();
      }

      DynamicArray<char> glslToSpirv(ShaderType type, const char* textContent, size_t length,
                                     const char* entryPoint, uint32_t clientTarget, uint32_t spirvTarget,
                                     uint32_t defaultVersion, void* customTBuiltInResource) { // throws
        if (!_isInitialized) {
          glslang::InitializeProcess();
          _isInitialized = true;
        }
        EShLanguage stage = _toShaderStage(type);

        glslang::TShader shader(stage);
        shader.setStrings(&textContent, 1);
        shader.setEnvInput(glslang::EShSource::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, defaultVersion);
        shader.setEnvClient(glslang::EShClient::EShClientVulkan, _toClientTarget(clientTarget));
        shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, _toSpirvLanguage(spirvTarget));

        std::string glslCode; // preprocessed code
        const TBuiltInResource* resources = (customTBuiltInResource != nullptr)
                                          ? (TBuiltInResource*)customTBuiltInResource
                                          : &glslang::DefaultTBuiltInResource;
        glslang::TShader::ForbidIncluder noIncluder;
        if (!shader.preprocess(resources, 450, ENoProfile, false, false, (EShMessages)(EShMsgSpvRules|EShMsgVulkanRules), &glslCode, noIncluder))
          throw std::runtime_error("Shader: GLSL preprocessing failed");

        const char* glslCode_cstr = glslCode.c_str();
        shader.setStrings(&glslCode_cstr, 1);
        shader.setEntryPoint(entryPoint);
        shader.setSourceEntryPoint(entryPoint);
        if (!shader.parse(resources, defaultVersion, false, (EShMessages)(EShMsgSpvRules|EShMsgVulkanRules)))
          throw std::runtime_error("Shader: GLSL parsing error");

        glslang::TProgram shaderProgram;
        shaderProgram.addShader(&shader);
        if (!shaderProgram.link((EShMessages)(EShMsgSpvRules|EShMsgVulkanRules)) || !shaderProgram.mapIO())
          throw std::runtime_error("Shader: GLSL link error");

        std::vector<unsigned int> spirvStorage;
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        glslang::GlslangToSpv(*shader.getIntermediate(), spirvStorage, &logger, &spvOptions);

        DynamicArray<char> output(spirvStorage.size()*sizeof(unsigned int));
        memcpy(output.value, spirvStorage.data(), spirvStorage.size()*sizeof(unsigned int));
#       if !defined(_CPP_REVISION) || _CPP_REVISION != 14
          return output;
#       else
          return std::move(output);
#       endif
      }

    private:
      bool _isInitialized = false;
    };
    static __GlslangCompiler g_compiler; // global instance (destroyed at the end of the program)

    // ---

    // Compile shader from text content
    Shader::Builder Shader::Builder::compile(ShaderType type, const char* textContent, size_t length,
                                             const char* entryPoint, uint32_t clientTarget, uint32_t spirvTarget,
                                             uint32_t defaultVersion, void* customTBuiltInResource) {
      return Shader::Builder(type, g_compiler.glslToSpirv(type, textContent, length, entryPoint,
                                                          clientTarget, spirvTarget, defaultVersion,
                                                          customTBuiltInResource), entryPoint);
    }

    // Compile shader from text file
    Shader::Builder Shader::Builder::compileFromFile(ShaderType type, const __SYSTEM_PATH_CHAR* filePath,
                                                     const char* entryPoint, uint32_t clientTarget, uint32_t spirvTarget,
                                                     uint32_t defaultVersion, void* customTBuiltInResource) {
      FILE* shaderFile = nullptr;
      DynamicArray<char> shaderData;
      try {
        errno = 0;
#       ifdef _WINDOWS
          if (_wfopen_s(&shaderFile, filePath, L"rb") != 0)
            throw std::runtime_error("Shader: file not found / not readable");
#       else
          shaderFile = fopen(path.c_str(), "rb");
          if (shaderFile == nullptr)
            throw std::runtime_error("Shader: file not found / not readable");
#       endif

        // read entire file
        fseek(shaderFile, 0, SEEK_END);
        size_t fileSize = ftell(shaderFile);
        fseek(shaderFile, 0, SEEK_SET);
        shaderData = DynamicArray<char>(fileSize + 1);
        memset(shaderData.value, 0, (fileSize + 1) * sizeof(char));
        fread(shaderData.value, 1, fileSize, shaderFile);

        fclose(shaderFile);
        shaderFile = nullptr;
      }
      catch (const std::exception&) {
        if (shaderFile != nullptr)
          fclose(shaderFile);
        throw;
      }
      return Shader::Builder(type, g_compiler.glslToSpirv(type, shaderData.value, shaderData.length()-1u,
                                                          entryPoint, clientTarget, spirvTarget, defaultVersion,
                                                          customTBuiltInResource), entryPoint);
    }
# endif


// -- create shader objects -- -------------------------------------------------

  // Create shader object
  Shader Shader::Builder::createShader(DeviceResourceManager device) const {
    if (device == VK_NULL_HANDLE)
      throw std::invalid_argument("Shader: device context is NULL");

    VkShaderModuleCreateInfo shaderInfo;
    memset(&shaderInfo, 0, sizeof(VkShaderModuleCreateInfo));
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = this->_length;
    shaderInfo.pCode = reinterpret_cast<const uint32_t*>(this->_data);

    VkShaderModule shaderModule;
    auto result = vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS)
      throwError(result, "Shader: creation error");
    return Shader(shaderModule, this->_type, device, this->_entryPoint.c_str());
  }

  // Create usable shader stage object
  Shader::Shader(Shader::Handle handle, ShaderType type, DeviceResourceManager device, const char* entryPoint)
    : _context(device) {
    memset(&_stageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    this->_entryPoint = entryPoint; // throws
    _stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _stageInfo.stage = (VkShaderStageFlagBits)type;
    _stageInfo.module = (VkShaderModule)handle;
    _stageInfo.pName = this->_entryPoint.c_str();
  }
  
  // Destroy shader object
  void Shader::release() noexcept {
    if (this->_stageInfo.module != VK_NULL_HANDLE) {
      try {
        vkDestroyShaderModule(this->_context, this->_stageInfo.module, nullptr);
        this->_stageInfo.module = VK_NULL_HANDLE;
      }
      catch (...) {}
    }
  }
  
  // ---

  // Create input layout for shader object
  InputLayout Shader::Builder::createInputLayout(VkVertexInputBindingDescription* inputBindings, size_t bindingsLength,
                                                 VkVertexInputAttributeDescription* layoutAttributes, size_t attributesLength) const {
    if (this->_type != ShaderType::vertex && this->_type != ShaderType::compute)
      throw std::runtime_error("Shader.createInputLayout: incorrect shader type");

    InputLayoutDescription layout;
    layout.bindings = DynamicArray<VkVertexInputBindingDescription>(bindingsLength);
    if (bindingsLength)
      memcpy(layout.bindings.value, inputBindings, bindingsLength*sizeof(VkVertexInputBindingDescription));
    layout.attributes = DynamicArray<VkVertexInputAttributeDescription>(attributesLength);
    if (attributesLength)
      memcpy(layout.attributes.value, layoutAttributes, attributesLength*sizeof(VkVertexInputAttributeDescription));

    return InputLayout(std::move(layout));
  }

#endif
