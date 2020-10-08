/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdio>

namespace pandora { 
  namespace io {
    /// @class FileHandle
    /// @brief Managed file handle, with automatic closing on destruction
    class FileHandle final {
    public:
      /// @brief Create empty file handle
      FileHandle() = default;
      /// @brief Create managed file handle
      FileHandle(FILE* handle) noexcept : _handle(handle) {}
      /// @brief Close file handle on destruction
      ~FileHandle() { close(); }

      FileHandle(const FileHandle&) = delete;
      FileHandle& operator=(const FileHandle&) = delete;
      FileHandle(FileHandle&& rhs) noexcept : _handle(rhs._handle) { rhs._handle = nullptr; }
      FileHandle& operator=(FileHandle&& rhs) noexcept { close(); this->_handle = rhs._handle; rhs._handle = nullptr; return *this; }

      inline bool isOpen() const noexcept { return (this->_handle != nullptr); }
      inline operator bool() const noexcept { return isOpen(); }

      inline const FILE* handle() const noexcept { return this->_handle; }
      inline FILE* handle() noexcept { return this->_handle; }

      /// @brief Close current file handle
      inline void close() noexcept {
        if (this->_handle != nullptr) {
          fflush(this->_handle);
          fclose(this->_handle);
          this->_handle = nullptr;
        }
      }

    private:
      FILE* _handle = nullptr;
    };

  }
}
