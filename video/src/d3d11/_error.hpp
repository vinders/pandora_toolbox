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
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- error messages -- --------------------------------------------------------

  // Exception class with LightString
  class RuntimeException final : public std::runtime_error {
  public:
    RuntimeException(std::shared_ptr<pandora::memory::LightString>&& msg) noexcept : std::runtime_error(msg->c_str()), _message(std::move(msg)) {}
    RuntimeException(const RuntimeException& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException(RuntimeException&& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException& operator=(const RuntimeException& rhs) noexcept { std::runtime_error::operator=(rhs); _message = rhs._message; return *this; }
    virtual ~RuntimeException() noexcept { _message.reset(); }
  private:
    std::shared_ptr<pandora::memory::LightString> _message;
  };
  
  // ---
  
  static inline const char* __getDirect3DError(HRESULT result) {
    switch (result) {
      case E_UNEXPECTED: return "UNEXPECTED";
      case E_NOTIMPL: return "NOTIMPL";
      case E_OUTOFMEMORY: return "OUTOFMEM";
      case DXGI_ERROR_INVALID_CALL:
      case E_INVALIDARG: return "INVALIDARG";
      case E_NOINTERFACE: return "NOINTERFACE";
      case E_POINTER: return "POINTER";
      case E_HANDLE: return "HANDLE";
      case E_ABORT: return "ABORT";
      case E_FAIL: return "FAIL";
      case DXGI_ERROR_ACCESS_DENIED:
      case E_ACCESSDENIED: return "ACCESSDENIED";
      case DXGI_ERROR_ACCESS_LOST: return "ACCESS_LOST";
      case DXGI_ERROR_NAME_ALREADY_EXISTS:
      case DXGI_ERROR_ALREADY_EXISTS: return "ALREADY_EXISTS";
      case E_PENDING: return "PENDING";
      case E_BOUNDS: return "BOUNDS";
      case E_CHANGED_STATE: return "CHANGED_STATE";
      case E_ILLEGAL_STATE_CHANGE: return "ILLEGAL_STATE_CHANGE";
      case E_ILLEGAL_METHOD_CALL: return "ILLEGAL_METHOD_CALL";
      case RO_E_METADATA_NAME_NOT_FOUND: return "META_NAME_NOT_FOUND";
      case RO_E_METADATA_NAME_IS_NAMESPACE: return "META_NAME_IS_NAMESPACE";
      case RO_E_METADATA_INVALID_TYPE_FORMAT: return "META_INVALID_TYPE";
      case RO_E_INVALID_METADATA_FILE: return "INVALID_META_FILE";
      case RO_E_CLOSED: return "CLOSED";
      case RO_E_EXCLUSIVE_WRITE: return "EXCLUSIVE_WRITE";
      case RO_E_ERROR_STRING_NOT_FOUND: return "ERROR_STR_NOT_FOUND";
      case E_STRING_NOT_NULL_TERMINATED: return "STR_NOT_NULL_TERM";
      case E_ILLEGAL_DELEGATE_ASSIGNMENT: return "ILLEGAL_DELEGATE_ASSIGN";
      case E_ASYNC_OPERATION_NOT_STARTED: return "ASYNC_OP_NOT_STARTED";
      case CO_E_INIT_ONLY_SINGLE_THREADED: return "INIT_ONLY_SINGLE_THREADED";
      case CO_E_CANT_REMOTE: return "CANT_REMOTE";
      case CO_E_LAUNCH_PERMSSION_DENIED: return "LAUNCH_PERMSSION_DENIED";
      case CO_E_REMOTE_COMMUNICATION_FAILURE: return "REMOTE_COMM_FAILURE";
      case CO_E_IIDREG_INCONSISTENT: return "IIDREG_INCONSIST";
      case DXGI_ERROR_UNSUPPORTED:
      case CO_E_NOT_SUPPORTED: return "NOT_SUPPORTED";
      case CO_E_RELOAD_DLL: return "RELOAD_DLL";
      default: return "INTERNAL_ERROR";
    }
  }

  void pandora::video::d3d11::throwError(HRESULT result, const char* messageContent) {
    const char* d3dError = __getDirect3DError(result);

    // pre-compute total size to avoid having multiple dynamic allocs
    size_t prefixSize = strlen(messageContent);
    size_t errorSize = strlen(d3dError);
    auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + errorSize);
    
    // copy message in preallocated string
    if (!message->empty()) { // if no alloc failure
      memcpy((void*)message->data(),                 messageContent, prefixSize*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize]),    ": ",         size_t{2u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+2u]), d3dError,     errorSize *sizeof(char));
    }
    throw RuntimeException(std::move(message));
  }
  
# ifdef _P_VIDEO_SHADER_COMPILERS
    void pandora::video::d3d11::throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo) {
      const char* errorData = (errorMessage) ? (const char*)errorMessage->GetBufferPointer() : "missing/empty shader file/content";
    
      // pre-compute total size to avoid having multiple dynamic allocs
      size_t prefixSize = strlen(messagePrefix);
      size_t infoSize = strlen(shaderInfo);
      size_t errorSize = strlen(errorData);
      auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + infoSize + 3u + errorSize);
    
      // copy message in preallocated string
      if (!message->empty()) { // if no alloc failure
        memcpy((void*)message->data(),                           messagePrefix, prefixSize*sizeof(char));
        memcpy((void*)&(message->data()[prefixSize]),             " (",         size_t{2u}*sizeof(char));
        memcpy((void*)&(message->data()[prefixSize + 2u]),        shaderInfo,   infoSize  *sizeof(char));
        memcpy((void*)&(message->data()[prefixSize+infoSize+2u]), "): ",        size_t{3u}*sizeof(char));
        memcpy((void*)&(message->data()[prefixSize+infoSize+2u+3u]), errorData, errorSize *sizeof(char));
      }
      if (errorMessage)
        errorMessage->Release();
      throw RuntimeException(std::move(message));
    }
# endif

#endif
