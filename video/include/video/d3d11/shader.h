/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstdint>
# include "../common_types.h"
# include "./api/types.h"      // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        class InputLayout;
        
        /// @class Shader
        /// @brief GPU shading program/effects for Direct3D renderer
        class Shader final {
        public:
          using Handle = void*; // ID3D11VertexShader*/ID3D11PixelShader*/ID3D11GeometryShader*
                                // ID3D11ComputeShader*/ID3D11HullShader*/ID3D11DomainShader*
          
          /// @brief Create usable shader object -- reserved for internal use or advanced usage
          /// @remarks Prefer Shader::Builder for standard usage
          Shader(Handle handle, ShaderType type) : _handle(handle), _type(type) {}
          
          Shader() = default; ///< Empty shader -- not usable (only useful to store variable not immediately initialized)
          Shader(const Shader&) = delete;
          Shader(Shader&& rhs) noexcept : _handle(rhs._handle), _type(rhs._type) { rhs._handle = nullptr; }
          Shader& operator=(const Shader&) = delete;
          Shader& operator=(Shader&& rhs) noexcept { this->_handle=rhs._handle; this->_type=rhs._type; rhs._handle=nullptr; return *this; }
          ~Shader() noexcept { release(); }
          void release() noexcept; ///< Destroy shader object
          
          
          // -- accessors --
          
          /// @brief Get native shader handle -- for internal use or advanced features
          /// @remarks Depending on shader type, cast to ID3D11VertexShader*/ID3D11PixelShader*/ID3D11GeometryShader*/
          ///                                            ID3D11ComputeShader*/ID3D11HullShader*/ID3D11DomainShader*.
          inline Handle handle() const noexcept { return this->_handle; }
          inline ShaderType type() const noexcept { return this->_type; }///< Get shader category/model type
          inline bool isEmpty() const noexcept { return (this->_handle == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)


          // -- create/compile shaders --
          
          /// @class Shader.Builder
          /// @brief Shader compiler and build (used to create shaders and input layouts).
          class Builder final {
          public:
            /// @brief Initialize shader builder with already compiled shader data.
            ///        To compile a shader from text, use 'Shader::Builder::compile' or 'Shader::Builder::compileFromFile' instead.
            /// @remarks - Typically used when reading binary FXC-compiled "*.cso" file.
            ///          - Can also be used with compiled shader data stored in memory.
            ///          - Will NOT work with HLSL text data: use 'Builder::compile' or 'Builder::compileFromFile' instead.
            /// @warning - 'binaryFileData' must remain available at least until the Builder is destroyed.
            ///          - 'binaryFileData' is NOT managed by the builder: if memory comes from a dynamic alloc, 
            ///            it won't be freed by the Builder -> needs to be freed manually (after destroying the Builder instance).
            Builder(ShaderType type, const uint8_t* binaryFileData, size_t length)
              : _shaderBuffer(nullptr), _data(binaryFileData), _length(length), _type(type) {}
            
            Builder(const Builder&) = delete;
            Builder(Builder&& rhs) noexcept
              : _shaderBuffer(rhs._shaderBuffer), _data(rhs._data), _length(rhs._length), _type(rhs._type) { rhs._shaderBuffer = nullptr; }
            Builder& operator=(const Builder&) = delete;
            Builder& operator=(Builder&& rhs) noexcept { 
              this->_shaderBuffer=rhs._shaderBuffer; this->_data=rhs._data; this->_length=rhs._length; this->_type=rhs._type;
              rhs._shaderBuffer = nullptr; 
              return *this; 
            }
            ~Builder() noexcept {
              if (_shaderBuffer != nullptr)
                _shaderBuffer->Release();
            }

            /// @brief Compile shader from HLSL text content
            /// @param type         Shader category/model.
            /// @param textContent  ASCII uncompiled text data.
            /// @param length       Number of bytes in 'textContent'.
            /// @param entryPoint   Name of the entry point function in the shader ("main"/"Main"/"VSMain"/"PsMain"/"MyShaderMain"/...)
            /// @param isStrict     Force strict compilation rules (deprecated/legacy/incomplete syntax refused)
            /// @throws runtime_error on failure
            static Builder compile(ShaderType type, const char* textContent, size_t length, 
                                   const char* entryPoint = "main", bool isStrict = true);
            /// @brief Compile shader from HLSL text file ("*.hlsl")
            /// @param type         Shader category/model.
            /// @param filePath     File to compile (absolute path / relative path to current directory).
            /// @param entryPoint   Name of the entry point function in the shader ("main"/"Main"/"VSMain"/"PsMain"/"MyShaderMain"/...)
            /// @param isStrict     Force strict compilation rules (deprecated/legacy/incomplete syntax refused)
            /// @throws runtime_error on failure
            static Builder compileFromFile(ShaderType type, const wchar_t* filePath, 
                                           const char* entryPoint = "main", bool isStrict = true);
            
            // -- create shader objects --
            
            /// @brief Create shader object
            /// @param device Device handle from associated Renderer instance (Renderer.device())
            /// @throws runtime_error on failure
            Shader createShader(DeviceHandle device) const;
            
            /// @brief Create input layout for shader object
            /// @param device         Device handle from associated Renderer instance (Renderer.device())
            /// @param length         Number of elements in 'layoutElements'
            /// @param layoutElements Array of input element descriptions:
            ///        * very specific to each shading language -> currently no portable abstraction in toolbox;
            ///        * example:
            ///          D3D11_INPUT_ELEMENT_DESC layout[] = {
            ///            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            ///            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            ///            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }};
            /// @remarks - The input layout is validated against shared input signature.
            ///          - The input layout may be bound with any other shader that has the same input signature.
            ///          - For engines with material shaders with different inputs, it's easier to verify shader params with D3DReflect, instead of hardcoding them.
            /// @throws runtime_error on failure (or if layout doesn't match shader input signature)
            InputLayout createInputLayout(DeviceHandle device, D3D11_INPUT_ELEMENT_DESC* layoutElements, size_t length) const;
            
          private:
            Builder(ShaderType type, ID3DBlob* shaderBuffer)
              : _shaderBuffer(shaderBuffer),
                _data(static_cast<const uint8_t*>(_shaderBuffer->GetBufferPointer())),
                _length(static_cast<size_t>(_shaderBuffer->GetBufferSize())),
                _type(type) {}
          private:
            ID3DBlob* _shaderBuffer = nullptr;
            const uint8_t* _data = nullptr;
            size_t _length = 0;
            ShaderType _type = ShaderType::vertex;
          };
          
        private:
          Handle _handle = nullptr;
          ShaderType _type = ShaderType::vertex;
        };
        
        // ---
        
        /// @class InputLayout
        /// @brief Data input layout for shader object(s)
        class InputLayout final {
        public:
          /// @brief Create usable input layout object -- reserved for internal use or advanced usage
          InputLayout(InputLayoutHandle handle) : _handle(handle) {}
          
          InputLayout() = default;
          InputLayout(const InputLayout&) = delete;
          InputLayout(InputLayout&& rhs) noexcept : _handle(rhs._handle) { rhs._handle = nullptr; }
          InputLayout& operator=(const InputLayout&) = delete;
          InputLayout& operator=(InputLayout&& rhs) noexcept { 
            this->_handle = rhs._handle; rhs._handle = nullptr; return *this; 
          }
          ~InputLayout() noexcept { release(); }
          /// @brief Destroy input layout
          void release() noexcept {
            if (this->_handle)
              this->_handle->Release();
          }
          
          inline InputLayoutHandle handle() const noexcept { return this->_handle; }  ///< Get native handle
          inline bool isEmpty() const noexcept { return (this->_handle == nullptr); } ///< Verify if initialized (false) or empty/moved/released (true)
        
        private:
          InputLayoutHandle _handle = nullptr;
        };
      }
    }
  }
#endif
