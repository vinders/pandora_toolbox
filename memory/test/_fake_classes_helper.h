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
