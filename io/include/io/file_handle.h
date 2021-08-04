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
