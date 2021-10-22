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

#if defined(_VIDEO_VULKAN_SUPPORT) && defined(_P_VIDEO_SHADER_COMPILERS)
# include <cstdint>
# include <SPIRV/GlslangToSpv.h>
# include <StandAlone/ResourceLimits.h>
# include "../../shader.h"

  namespace pandora {
    namespace video {
      namespace vulkan {
        inline EShLanguage _toShaderStage(ShaderType type) noexcept {
          switch (type) {
            case ShaderType::vertex:   return EShLanguage::EShLangVertex;
            case ShaderType::fragment: return EShLanguage::EShLangFragment;
            case ShaderType::geometry: return EShLanguage::EShLangGeometry;
            case ShaderType::compute:  return EShLanguage::EShLangCompute;
            case ShaderType::tessCtrl: return EShLanguage::EShLangTessControl;
            case ShaderType::tessEval: return EShLanguage::EShLangTessEvaluation;
            default: return EShLanguage::EShLangCompute;
          }
        }
        inline glslang::EShTargetClientVersion _toClientTarget(uint32_t apiVer) noexcept {
          uint32_t minor = VK_VERSION_MINOR(apiVer);
          switch (minor) {
            case 0: return glslang::EShTargetClientVersion::EShTargetVulkan_1_0;
            case 1: return glslang::EShTargetClientVersion::EShTargetVulkan_1_1;
            case 2: return glslang::EShTargetClientVersion::EShTargetVulkan_1_2;
            default: return static_cast<glslang::EShTargetClientVersion>(
                              (uint32_t)glslang::EShTargetClientVersion::EShTargetVulkan_1_0 | (minor<<12) );
          }
        }
        inline glslang::EShTargetLanguageVersion _toSpirvLanguage(uint32_t targetVer) noexcept {
          switch (targetVer) {
            case 10: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_0;
            case 11: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_1;
            case 12: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_2;
            case 13: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_3;
            case 14: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_4;
            case 15: return glslang::EShTargetLanguageVersion::EShTargetSpv_1_5;
            default: return static_cast<glslang::EShTargetLanguageVersion>(
                              (uint32_t)glslang::EShTargetLanguageVersion::EShTargetSpv_1_0 | ((targetVer%10)<<8) );
          }
        }
      }
    }
  }
# endif
