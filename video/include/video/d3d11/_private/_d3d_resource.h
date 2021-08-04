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
# include "video/d3d11/api/types.h"

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Throw native error message (or default if no message available)
        void throwError(HRESULT result, const char* messageContent);
        // Throw shader-specific error message
        void throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo);
        
        // ---
        
        // RAII container for native Direct3D resources (lightweight version of ComPtr)
        // -> Windows and D3D11 API headers must have been included!
        template <typename T>
        struct D3dResource final {
          D3dResource() = default; ///< Empty resource
          D3dResource(T* value) : _value(value) {} ///< Initialized resource
          ~D3dResource() noexcept { destroy(); }  ///< Safe destruction (if scope exit or exception)
          
          // -- assignment/move --
          
          D3dResource(const D3dResource&) = delete;
          D3dResource& operator=(const D3dResource&) = delete;
          
          inline D3dResource(D3dResource&& rhs) noexcept : _value(rhs._value) { rhs._value = nullptr; }
          inline D3dResource& operator=(D3dResource&& rhs) noexcept {
            if (_value != nullptr)
              _value->Release();
            _value = rhs._value;
            rhs._value = nullptr;
            return *this;
          }
          inline D3dResource& operator=(T* value) noexcept {
            if (_value != nullptr)
              _value->Release();
            _value = value;
            return *this;
          }
          
          /// @brief Abandon resource ownership
          inline T* extract() noexcept {
            T* val = _value;
            _value = nullptr;
            return val;
          }
          /// @brief Free resource
          inline void destroy() noexcept {
            if (_value != nullptr) {
              _value->Release(); 
              _value = nullptr;
            }
          }
          
          // -- accessors --
          
          inline T& operator*() const { return *_value; }
          inline T* operator->() const noexcept { return _value; }
          inline operator bool() const noexcept { return (_value != nullptr); } ///< Verify validity
          inline bool hasValue() const noexcept { return (_value != nullptr); } ///< Verify validity
          inline T& value() const noexcept { return _value; } ///< Read value (no verification -> call hasValue() first!)
          inline T* get() noexcept { return _value; }         ///< Get value pointer
          inline T** address() noexcept { destroy(); return &_value; } ///< Free previous value + get value address (for assignment)
          
          // -- builders --
          
          /// @brief Dynamic cast from parent interface - exception on failure
          /// @throws Exception on failure
          template <typename _Parent>
          static inline D3dResource<T> fromInterface(_Parent* parent, const char* defaultErrorMessage) { // throws
            T* castValue = nullptr;
            auto result = parent->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&castValue));
            if (FAILED(result) || castValue == nullptr)
              throwError(result, defaultErrorMessage);
            return D3dResource<T>(castValue);
          }
          /// @brief Dynamic cast from parent interface - NULL value on failure
          template <typename _Parent>
          static inline D3dResource<T> tryFromInterface(_Parent* parent) {
            T* castValue = nullptr;
            if (FAILED(parent->QueryInterface(__uuidof(T), reinterpret_cast<void**>(&castValue))))
              castValue = nullptr;
            return D3dResource<T>(castValue);
          }
          
          /// @brief Dynamic cast from child class - exception on failure
          /// @throws Exception on failure
          template <typename _Parent>
          static inline D3dResource<T> fromChild(_Parent* parent, const char* defaultErrorMessage) { // throws
            T* castValue = nullptr;
            auto result = parent->GetParent(__uuidof(T), reinterpret_cast<void**>(&castValue));
            if (FAILED(result) || castValue == nullptr)
              throwError(result, defaultErrorMessage);
            return D3dResource<T>(castValue);
          }
          /// @brief Dynamic cast from child class - NULL value on failure
          template <typename _Parent>
          static inline D3dResource<T> tryFromChild(_Parent* parent) {
            T* castValue = nullptr;
            if (FAILED(parent->GetParent(__uuidof(T), reinterpret_cast<void**>(&castValue))))
              castValue = nullptr;
            return D3dResource<T>(castValue);
          }

        private:
          T* _value = nullptr;
        };
      }
    }
  }
#endif
