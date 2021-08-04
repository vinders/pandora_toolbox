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

#include <cstdint>
#include <functional>

namespace pandora {
  namespace pattern {
    /// @class OperationGuard
    /// @brief Security for operations that need to be reversed when execution is interrupted (destruction/exception)
    template <typename _DataType, // type of operand and values
              typename _Operator, // first operation: std::plus<_DataType>, std::minus<_DataType>, ...
              _DataType _OpValue, // value applied by '_Operator' to 'operand' on construction
              typename _EndOperator, // end operation: std::plus<_DataType>, std::minus<_DataType>, ...
              _DataType _EndOpValue> // value applied by '_EndOperator' to 'operand' on destruction
    class OperationGuard final {
    public:
      /// @brief Start of execution: first operation (operand <_Operator> _OpValue)
      explicit OperationGuard(_DataType& operand) noexcept
        : _operand(&operand) { 
        *(this->_operand) = _Operator{}(*(this->_operand), _OpValue);
      }
      /// @brief End of execution: end operation (operand <_EndOperator> _EndOpValue)
      ~OperationGuard() noexcept { release(); }
      
      OperationGuard(const OperationGuard&) = delete;
      OperationGuard(OperationGuard&&) = delete;
      OperationGuard& operator=(const OperationGuard&) = delete;
      OperationGuard& operator=(OperationGuard&&) = delete;
      
      /// @brief Call end operation directly, instead of waiting for destruction
      inline void release() noexcept {
        if (this->_operand != nullptr) {
          *(this->_operand) = _EndOperator{}(*(this->_operand), _EndOpValue);
          this->_operand = nullptr;
        }
      }
      
    private:
      _DataType* _operand;
    };
    
    // ---
    
    template <typename _DataType, _DataType _Increment>
    using CountGuard = OperationGuard<_DataType, std::plus<_DataType>, _Increment, std::minus<_DataType>, _Increment>;
  }
}
