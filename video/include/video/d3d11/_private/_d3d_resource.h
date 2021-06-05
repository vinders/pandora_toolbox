/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  namespace pandora {
    namespace video {
      namespace d3d11 {
        // Throw native error message (or default if no message available)
        void throwError(HRESULT result, const char* messageContent);
        
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
