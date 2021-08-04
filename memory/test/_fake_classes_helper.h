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
# pragma once

#include <cstddef>
#define __FAKE_ITEM_CLASS(className, copiable, movable) \
        class className { \
        public: \
          className () = default; \
          className (int val) : _val(val) {} \
          className (const className &) = copiable ; \
          className (className &&) = movable ; \
          className & operator=(const className &) = copiable ; \
          className & operator=(className &&) = movable ; \
          int value() const { return _val; } \
          bool operator==(const className & rhs) const { return _val == rhs._val; } \
          bool operator!=(const className & rhs) const { return _val != rhs._val; } \
        private: \
          int _val = 0; \
        }

__FAKE_ITEM_CLASS(CopyObject, default, delete);
__FAKE_ITEM_CLASS(MoveObject, delete, default);
__FAKE_ITEM_CLASS(CopyMoveObject, default, default);
#undef __FAKE_ITEM_CLASS
