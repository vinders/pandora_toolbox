/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
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
