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

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cassert>
# include <cstdint>
# include <cstring>
# include <memory>
# include "video/d3d11/api/types.h" // includes D3D11
# include "./_error.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        /// @brief    RAII container for native Direct3D resources (lightweight version of ComPtr)
        template <typename T>
        class SharedResource final {
        public:
          using Reference = T*;
          SharedResource() noexcept = default; ///< Empty resource
          inline SharedResource(T* value) noexcept : _value(value) {} ///< Initialized resource
          inline ~SharedResource() noexcept { release(); }  ///< Safe destruction (if scope exit or exception)
          
          // -- assign/move/release --
          
          inline SharedResource(const SharedResource<T>& rhs) : _value(rhs._value) {
            if (_value != nullptr)
              _value->AddRef();
          }
          inline SharedResource(SharedResource<T>&& rhs) noexcept : _value(rhs._value) { rhs._value = nullptr; }
          
          inline SharedResource<T>& operator=(const SharedResource<T>& rhs) {
            if (_value != nullptr) { try { _value->Release(); } catch (...) {} }
            _value = rhs._value;
            if (rhs._value != nullptr)
              _value->AddRef();
            return *this;
          }
          inline SharedResource<T>& operator=(SharedResource<T>&& rhs) noexcept {
            if (_value != nullptr) { try { _value->Release(); } catch (...) {} }
            _value = rhs._value;
            rhs._value = nullptr;
            return *this;
          }
          inline SharedResource<T>& operator=(T* value) noexcept {
            if (_value != nullptr) { try { _value->Release(); } catch (...) {} }
            _value = value;
            return *this;
          }
          
          /// @brief Return resource and release instance ownership
          inline T* extract() noexcept {
            T* val = _value;
            _value = nullptr;
            return val;
          }
          /// @brief Destroy resource instance
          inline void release() noexcept {
            if (_value != nullptr) {
              try { _value->Release(); } catch (...) {}
              _value = nullptr;
            }
          }
          
          // -- accessors --
          
          inline T& operator*() const { return *_value; }
          inline T* operator->() const noexcept { return _value; }
          inline operator bool() const noexcept { return (_value != nullptr); } ///< Verify validity
          inline bool hasValue() const noexcept { return (_value != nullptr); } ///< Verify validity
          inline T& value() const noexcept { return *_value; } ///< Read value (no verification -> call hasValue() first!)
          inline T* get() const noexcept { return _value; }    ///< Get value pointer
          inline const Reference* ptr() const noexcept { return &_value; } ///< Get pointer to value pointer
          inline T** address() noexcept { release(); return &_value; } ///< Free previous value + get address for assignment

          inline bool operator==(const SharedResource<T>& rhs) const noexcept { return (_value == rhs._value); }
          inline bool operator!=(const SharedResource<T>& rhs) const noexcept { return (_value != rhs._value); }
          
          // -- cast helpers --
          
          /// @brief Dynamic cast from parent interface - exception on failure
          template <typename _Parent>
          static inline SharedResource<T> fromInterface(_Parent* parent, const char* defaultErrorMessage) { // throws
            T* castValue = nullptr;
            auto result = parent->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&castValue));
            if (FAILED(result) || castValue == nullptr)
              throwError(result, defaultErrorMessage);
            return SharedResource<T>(castValue);
          }
          /// @brief Dynamic cast from parent interface - NULL value on failure
          template <typename _Parent>
          static inline SharedResource<T> tryFromInterface(_Parent* parent) {
            T* castValue = nullptr;
            if (FAILED(parent->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&castValue))))
              castValue = nullptr;
            return SharedResource<T>(castValue);
          }
          
          /// @brief Dynamic cast from child class - exception on failure
          template <typename _Parent>
          static inline SharedResource<T> fromChild(_Parent* parent, const char* defaultErrorMessage) { // throws
            T* castValue = nullptr;
            auto result = parent->GetParent(__uuidof(T), reinterpret_cast<void**>(&castValue));
            if (FAILED(result) || castValue == nullptr)
              throwError(result, defaultErrorMessage);
            return SharedResource<T>(castValue);
          }
          /// @brief Dynamic cast from child class - NULL value on failure
          template <typename _Parent>
          static inline SharedResource<T> tryFromChild(_Parent* parent) {
            T* castValue = nullptr;
            if (FAILED(parent->GetParent(__uuidof(T), reinterpret_cast<void**>(&castValue))))
              castValue = nullptr;
            return SharedResource<T>(castValue);
          }

        private:
          T* _value = nullptr;
        };
        
        // ---
        
        /// @brief RAII array of native Direct3D resources - store multi-slot rendering states
        template <typename _Type, size_t _MaxSize>
        class SharedResourceArray final {
        public:
          using Resource = _Type*;
        
          /// @brief Create empty container for multi-slot resources
          inline SharedResourceArray() noexcept : _length(0) { memset(_values, 0, _MaxSize*sizeof(_Type*)); }
          /// @brief Destroy state resources and container
          inline ~SharedResourceArray() noexcept { _release(); }
          
          // -- assign/move --

          inline SharedResourceArray(const SharedResourceArray<_Type,_MaxSize>& rhs) : _length(0) { _copy(rhs); }
          inline SharedResourceArray(SharedResourceArray<_Type,_MaxSize>&& rhs) noexcept : _length(rhs._length) {
            memcpy(_values, rhs._values, _MaxSize*sizeof(_Type*));
            memset(rhs._values, 0, _MaxSize*sizeof(_Type*));
            _length = rhs._length;
            rhs._length = 0;
          }
  
          inline SharedResourceArray<_Type,_MaxSize>& operator=(const SharedResourceArray<_Type,_MaxSize>& rhs) {
            _release();
            _copy(rhs);
            return *this; 
          }
          inline SharedResourceArray<_Type,_MaxSize>& operator=(SharedResourceArray<_Type,_MaxSize>&& rhs) noexcept {
            _release();
            memcpy(_values, rhs._values, _MaxSize*sizeof(_Type*));
            memset(rhs._values, 0, _MaxSize*sizeof(_Type*));
            _length = rhs._length;
            rhs._length = 0;
            return *this; 
          }
          
          
          // -- accessors --
          
          inline operator bool() const noexcept { return (_length != 0); } ///< Container is not empty
          inline bool empty() const noexcept { return (_length == 0); }    ///< Container is empty
          inline size_t size() const noexcept { return _length; } ///< Number of slots currently filled in container
          static size_t maxSize() noexcept { return _MaxSize; }   ///< Max number of resources in container
          
          inline _Type** get() noexcept { return _values; } ///< Get collection, to bind all (or first N) resources
          inline const _Type** get() const noexcept { return _values; } ///< Get collection (constant)

          inline const _Type** getFrom(uint32_t index) const { ///< Get resource located at index (unsafe reference)
            assert(index < _length);
            return &_values[index];
          }
          inline _Type** getFrom(uint32_t index) { ///< Get resource located at index (unsafe reference)
            assert(index < _length);
            return &_values[index];
          }
          inline SharedResource<_Type> at(uint32_t index) const { ///< Get resource located at index (shared object)
            assert(index < _length);
            if (_values[index] != nullptr)
              _values[index]->AddRef();
            return SharedResource<_Type>(_values[index]);
          }
          
          // -- operations --
          
          /// @brief Append new shared resource (if max size not reached)
          inline bool append(const SharedResource<_Type>& res) noexcept { return append(SharedResource<_Type>(res)); }
          /// @brief Append new shared resource (if max size not reached)
          /// @warning Source object is invalidated (even on failure)
          inline bool append(SharedResource<_Type>&& res) noexcept { return append(res.extract()); }
          /// @brief Append new unmanaged resource (if max size not reached)
          /// @warning Source pointer is invalidated (even on failure)
          bool append(_Type* res) noexcept {
            if (_length < maxSize()) {
              _values[_length] = res; 
              ++_length;
              return true;
            }
            if (res != nullptr) { try { res->Release(); } catch (...) {} } // failure
            return false;
          }
          
          /// @brief Insert new shared resource at index (and shift following indices)
          ///        (if max size not reached && index <= length)
          inline bool insert(uint32_t index, const SharedResource<_Type>& res) noexcept { return insert(index, SharedResource<_Type>(res)); }
          /// @brief Insert new shared resource at index (and shift following indices)
          ///        (if max size not reached && index <= length)
          /// @warning Source object is invalidated (even on failure)
          inline bool insert(uint32_t index, SharedResource<_Type>&& res) noexcept { return insert(index, res.extract()); }
          /// @brief Insert new unmanaged resource at index (and shift following indices)
          ///        (if max size not reached && index <= length)
          /// @warning Source pointer is invalidated (even on failure)
          bool insert(uint32_t index, _Type* res) noexcept {
            if (_length >= maxSize() || index > static_cast<uint32_t>(_length)) { // failure
              if (res != nullptr) { try { res->Release(); } catch (...) {} }
              return false;
            }
            
            _Type** target = &_values[index];
            if (index < static_cast<uint32_t>(_length))
              memmove(target + 1, target, (_length-(size_t)index)*sizeof(_Type*));
            *target = res;
            ++_length;
            return true;
          }
          
          /// @brief Replace existing resource with shared resource at index (if index < length)
          inline bool replace(uint32_t index, const SharedResource<_Type>& res) noexcept { return replace(index, SharedResource<_Type>(res)); }
          /// @brief Replace existing resource with shared resource at index (if index < length)
          /// @warning Source object is invalidated (even on failure)
          inline bool replace(uint32_t index, SharedResource<_Type>&& res) noexcept { return replace(index, res.extract()); }
          /// @brief Replace existing resource with unmanaged resource at index (if index < length)
          /// @warning Source pointer is invalidated (even on failure)
          bool replace(uint32_t index, _Type* res) noexcept {
            if (index >= static_cast<uint32_t>(_length)) { // failure
              if (res != nullptr) { try { res->Release(); } catch (...) {} }
              return false;
            }
            
            _Type** target = &_values[index];
            if (*target != nullptr) { try { (*target)->Release(); } catch (...) {} } // free old entry
            *target = res;
            return true;
          }
          
          /// @brief Destroy resource at index (and shift greater indices)
          void erase(uint32_t index) noexcept {
            if (index < _length) {
              _Type** target = &(_values[index]);
              if (*target != nullptr) { try { (*target)->Release(); } catch (...) {} }
              
              --_length;
              if (index < (uint32_t)_length)
                memmove(target, target + 1, (_length-(size_t)index)*sizeof(_Type*));
              _values[_length] = nullptr;
            }
          }
          /// @brief Destroy last resource (if any)
          void pop_back() noexcept {
            if (_length > 0) {
              --_length;
              _Type** target = &_values[_length];
              if (*target != nullptr) {
                try { (*target)->Release(); } catch (...) {}
                *target = nullptr;
              }
            }
          }
          /// @brief Destroy all resources (container can still be used)
          inline void clear() noexcept {
            _release();
            memset(_values, 0, _MaxSize*sizeof(_Type*));
          }
          
        private:
          void _copy(const SharedResourceArray<_Type,_MaxSize>& rhs) {
            assert(_length == 0);
            memcpy(_values, rhs._values, _MaxSize*sizeof(_Type*)); // set values (even if empty -> set 0)
            for (Resource* it = &_values[0]; _length < rhs.length; ++it, ++_length) {
              if (*it != nullptr)
                (*it)->AddRef();
            }
          }
          void _release() noexcept {
            if (_length > 0) {
              for (Resource* it = &_values[_length - 1]; it >= _values; --it) {
                if (*it != nullptr) { try { (*it)->Release(); } catch (...) {} }
              }
              _length = 0;
            }
          }

        private:
          Resource _values[_MaxSize];
          size_t _length = 0;
        };
        
        // ---

        /// @brief Native Direct3D resource identifier for state manager -- reserved for internal use
        template <size_t _DwordCount>
        class SharedResourceId final {
        public:
          SharedResourceId() noexcept = default;
          inline SharedResourceId(const SharedResourceId<_DwordCount>& rhs) noexcept { memcpy(_id, rhs._id, _DwordCount*sizeof(uint32_t)); }
          inline SharedResourceId(SharedResourceId<_DwordCount>&& rhs) noexcept { memcpy(_id, rhs._id, _DwordCount*sizeof(uint32_t)); }
          inline SharedResourceId<_DwordCount>& operator=(const SharedResourceId<_DwordCount>& rhs) noexcept {
            memcpy(_id, rhs._id, _DwordCount * sizeof(uint32_t));
            return *this;
          }
          inline SharedResourceId<_DwordCount>& operator=(SharedResourceId<_DwordCount>&& rhs) noexcept {
            memcpy(_id, rhs._id, _DwordCount * sizeof(uint32_t));
            return *this;
          }
          ~SharedResourceId() noexcept = default;

          inline bool operator==(const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) == 0); }
          inline bool operator!=(const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) != 0); }
          inline bool operator< (const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) < 0); }
          inline bool operator<=(const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) <= 0); }
          inline bool operator> (const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) > 0); }
          inline bool operator>=(const SharedResourceId<_DwordCount>& rhs) const noexcept { return (_compare(_id, rhs._id) >= 0); }

          inline uint32_t* id() noexcept { return this->_id; }             ///< Get writable identifier
          inline const uint32_t* id() const noexcept { return this->_id; } ///< Get constant identifier
          inline static size_t length() noexcept { return _DwordCount; }   ///< Get array size

        private:
          template <size_t N = _DwordCount>
          static inline int32_t _compare(typename std::enable_if<N == (size_t)1, const uint32_t*>::type lhs, const uint32_t* rhs) noexcept {
            return (*(int32_t*)lhs - *(int32_t*)rhs);
          }
          template <size_t N = _DwordCount>
          static inline int64_t _compare(typename std::enable_if<N == (size_t)2, const uint32_t*>::type lhs, const uint32_t* rhs) noexcept {
            return (*(int64_t*)lhs - *(int64_t*)rhs);
          }
          template <size_t N = _DwordCount>
          static inline int _compare(typename std::enable_if<N != (size_t)1 && N != (size_t)2, const uint32_t*>::type lhs, const uint32_t* rhs) noexcept {
            return memcmp(lhs, rhs, N*sizeof(uint32_t));
          }

        private:
          uint32_t _id[_DwordCount]{ 0 };
        };

        
        template <typename _Resource, size_t _IdDwordCount>
        struct SharedResourceCache final { ///< Native Direct3D resource cache entry -- reserved for internal use
          SharedResourceId<_IdDwordCount> id;
          _Resource handle;
          int32_t instances = 0;
        };

        // ---

        /// @brief Rasterizer state driver resource - usage: GraphicsPipeline::Builder / Renderer.setRasterizerState
        using RasterizerState = SharedResource<ID3D11RasterizerState>;
        /// @brief Depth/stencil state driver resource -usage: GraphicsPipeline::Builder / Renderer.setDepthStencilState
        using DepthStencilState = SharedResource<ID3D11DepthStencilState>;
        /// @brief Blend state driver resource - usage: GraphicsPipeline::Builder / Renderer.setBlendState
        using BlendState = SharedResource<ID3D11BlendState>;
        
        /// @brief Sampler/filter state driver resource - usage: Renderer.set<...>SamplerState
        using SamplerState = SharedResource<ID3D11SamplerState>;
        /// @brief Array of sampler/filter state driver resources - usage: Renderer.set<...>SamplerStates
        using SamplerStateArray = SharedResourceArray<ID3D11SamplerState, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT>;
        /// @brief Data input layout for shader object(s) - usage: GraphicsPipeline::Builder
        using InputLayout = SharedResource<ID3D11InputLayout>;

        // ---

        using RasterizerStateId   = SharedResourceId<4>; ///< Rasterizer state driver resource ID
        using DepthStencilStateId = SharedResourceId<2>; ///< Depth/stencil state driver resource ID
        using BlendStateId = SharedResourceId<D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT>; ///< Blend state driver resource ID
        using BlendFactorId = uint32_t; ///< Blend constant factor ID

        using RasterizerStateCache   = SharedResourceCache<RasterizerState,4>;   ///< Rasterizer state cached resource entry
        using DepthStencilStateCache = SharedResourceCache<DepthStencilState,2>; ///< Depth/stencil state cached resource entry
        using BlendStateCache = SharedResourceCache<BlendState,D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT>; ///< Blend state cached resource entry

        // ---

        /// @brief Graphics pipeline content -- reserved for internal use
        struct _DxPipelineStages final {
          // shader stages -- only display shaders allowed (no compute!)
          SharedResource<ID3D11DeviceChild> shaderStages[__P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX + 1]{};

          // pipeline state identifiers for cache
          ColorChannel blendConstant[4]{ 1.f,1.f,1.f,1.f };
          BlendStateId blendCacheId;
          RasterizerStateId rasterizerCacheId;
          DepthStencilStateId depthStencilCacheId;
          // pipeline state driver resources
          RasterizerState rasterizerState;
          DepthStencilState depthStencilState;
          BlendState blendState;
          BlendFactorId blendFactorId = 0;
          uint32_t stencilRef = 1u;
          
          // input format
          InputLayout inputLayout;
          VertexTopology topology = VertexTopology::triangles;
          // output constraints
          DynamicArray<D3D11_VIEWPORT> viewports;
          DynamicArray<D3D11_RECT> scissorTests;
          uint64_t viewportScissorId = 0;
        };
        /// @brief Attached graphics pipeline cache -- reserved for internal use
        struct _DxPipelineCache final {
          ID3D11DeviceChild* shaderStages[__P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX + 1]{};
          uint64_t viewportScissorId = 0;
          ID3D11RasterizerState* rasterizerState = nullptr;
          ID3D11DepthStencilState* depthStencilState = nullptr;
          ID3D11BlendState* blendState = nullptr;
          BlendFactorId blendFactorId = 0;
          uint32_t stencilRef = 0;
          VertexTopology topology = (VertexTopology)-1;

          _DxPipelineStages* lastPipeline = nullptr; // only used to unbind active GraphicsPipeline when destroying it
                                                     // (can't be used to know if values are the same because of dynamic changes)
        };
        using GraphicsPipelineHandle = _DxPipelineStages*; ///< Native handle of GraphicsPipeline object (GraphicsPipeline.handle())
      }
    }
  }
#endif
