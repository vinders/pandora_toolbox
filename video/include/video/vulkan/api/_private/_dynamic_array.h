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

#if defined(_VIDEO_VULKAN_SUPPORT)
  namespace pandora {
    namespace video {
      namespace vulkan {
        // RAII array with size defined at runtime
        template <typename _DataType>
        struct DynamicArray final {
          constexpr inline DynamicArray() noexcept : value(nullptr) {}
          inline DynamicArray(size_t length) : value(new _DataType[length]()) {}
          ~DynamicArray() noexcept {
            if (value != nullptr)
              delete[] value;
          }
          DynamicArray(const DynamicArray&) = delete;
          DynamicArray& operator=(const DynamicArray&) = delete;
          inline DynamicArray(DynamicArray&& rhs) noexcept : value(rhs.value) { rhs.value = nullptr; }
          inline DynamicArray& operator=(DynamicArray&& rhs) noexcept { value = rhs.value; rhs.value = nullptr; return *this; }

          _DataType* value = nullptr;
        };
      }
    }
  }
#endif