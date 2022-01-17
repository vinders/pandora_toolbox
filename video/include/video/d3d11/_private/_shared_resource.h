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
# include <stdexcept>
# include "video/d3d11/api/types.h" // includes D3D11

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Throw native error message (or default if no message available)
        void throwError(HRESULT result, const char* messageContent);
        // Throw shader-specific compilation error message
#       ifdef _P_VIDEO_SHADER_COMPILERS
          void throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo);
#       endif
        
        // ---
        
        /// @brief    RAII container for native Direct3D resources (lightweight version of ComPtr)
        /// @warning  Windows and D3D11 API headers must have been included!
        template <typename T>
        struct SharedResource final {
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
          inline T& value() const noexcept { return _value; } ///< Read value (no verification -> call hasValue() first!)
          inline T* get() const noexcept { return _value; }   ///< Get value pointer
          inline T** address() noexcept { release(); return &_value; } ///< Free previous value + get value address (for assignment)
          
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
          
          inline _Type** get() noexcept { return _values; } ///< Get entire collection, to bind all (or first N) resources to Renderer instance
          inline const _Type** get() const noexcept { return _values; } ///< Get entire collection (constant)

          inline _Type** getFrom(uint32_t index) const { ///< Get resource located at index (unsafe reference)
            if (index >= _length)
              throw std::out_of_range("SharedResourceArray.at: index out of range");
            return &_values[index];
          }
          inline SharedResource<_Type> at(uint32_t index) const { ///< Get resource located at index (shared object)
            if (index >= _length)
              throw std::out_of_range("SharedResourceArray.at: index out of range");
            if (_values[index] != nullptr)
              _values[index]->AddRef();
            return SharedResource<_Type>(_values[index]);
          }
          
          // -- operations --
          
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
        
        /// @brief Depth/stencil state driver resource - can be used with Renderer.setDepthStencilState
        using DepthStencilState = SharedResource<ID3D11DepthStencilState>;
        /// @brief Rasterizer state driver resource - can be used with Renderer.setRasterizerState
        using RasterizerState = SharedResource<ID3D11RasterizerState>;
        /// @brief Blend state driver resource - can be used with Renderer.setBlendState
        using BlendState = SharedResource<ID3D11BlendState>;
        
        /// @brief Sampler/filter state driver resource - can be used with Renderer.set<...>FilterState
        using FilterState = SharedResource<ID3D11SamplerState>;
        /// @brief Array of sampler/filter state driver resources - can be used with Renderer.set<...>FilterStates
        using FilterStateArray = SharedResourceArray<ID3D11SamplerState, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT>;
      }
    }
  }
#endif
