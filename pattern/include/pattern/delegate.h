/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <memory>
#include <functional>
#include <type_traits>
#include <mutex>
#include "./contract.h"
#include "./contract_registrar.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace thread { class SpinLock; }

  namespace pattern {
    /// @brief Delegate usage mode
    enum class DelegateMode: uint32_t {
      functionPointer = 0u, ///< C-style function pointers: faster (less overhead), less convenient and less possibilities
      lambda = 1u           ///< std::function and lambdas: easier, more possibilities, but slower
    };
    
    /// @class Delegate
    /// @brief Function delegate (function pointer / std::function / lambda)
    /// @warning The delegate function signature (_Params) should never contain 'moved' argument types (Type&&) : 
    ///          each moved argument would be 'moved' as many times as the number of registrations -> crash or undefined behaviour.
    template <typename _LockType = std::mutex,          ///< Internal mutex type.
              DelegateMode _Mode = DelegateMode::lambda,///< Type of registrations: function pointers or std::function/lambda.
              bool _IsDeferrable = true,                ///< Allow registrations/revocations to be deferred (true=deferrable; false=faster)
                                                        ///  When allowed, registrations/revocations can be triggered in delegate calls.
              typename _ReturnType = void, typename ... _Params> ///< Signature of registered functions.
    class Delegate final {
    public:
      using Type = Delegate<_LockType,_Mode,_IsDeferrable,_ReturnType,_Params...>;
      using function_type = typename std::conditional<(_Mode == DelegateMode::lambda),  std::function<_ReturnType(_Params...)>,
                                                                                        _ReturnType (*)(_Params...) >::type;
      using function_type_move = typename std::conditional<(_Mode == DelegateMode::lambda), function_type&&, function_type >::type;
      using registrar = typename std::conditional<_IsDeferrable,  DeferrableContractRegistrar<function_type,_LockType>, 
                                                                  DirectContractRegistrar<function_type,_LockType> >::type;
      
      Delegate() : _functions(std::make_shared<registrar>()) {
        this->_contractProvider = ContractProvider(this->_functions);
      }

      Delegate(const Type&) = delete;
      Delegate(Type&&) noexcept = default;
      Type& operator=(const Type&) = delete;
      Type& operator=(Type&&) noexcept = default;
      ~Delegate() = default;
      
      // -- getters --

      constexpr inline size_t size() const noexcept { return this->_functions->data().value().size(); } ///< Count registrations
      constexpr inline bool empty() const noexcept { return this->_functions->data().value().empty(); } ///< Check if delegate has registrations
      
      // -- registration --
      
      /// @brief Register a new function
      std::unique_ptr<Contract> registerFunction(function_type_move function) noexcept {
        std::unique_ptr<Contract> contract = this->_contractProvider.createContract();
        assert(contract != nullptr && contract->isValid());
        if (function != nullptr)
          this->_functions->create(*contract, std::move(function));
#       if defined(_CPP_REVISION) && _CPP_REVISION == 14
          return std::move(contract);
#       else
          return contract;
#       endif
      }
      inline std::unique_ptr<Contract> operator+=(function_type_move function) noexcept { return registerFunction(std::forward<function_type>(function)); }
      
      /// @brief Cancel a subscription
      inline void unregister(Contract& contract) noexcept { contract.revoke(); }
      inline void operator-=(Contract& contract) noexcept { unregister(contract); }

      /// @brief Remove all existing registrations
      inline void clear() noexcept { this->_contractProvider.clear(); }

      // -- execution --
      
      inline void operator()(_Params... args) const { run(std::forward<_Params>(args)...); }
      
      /// @brief Execute each function (ignore returned values)
      void run(_Params... args) const {
        auto lockedFunctions = this->_functions->data();
        for (auto& fct : lockedFunctions.value())
          fct.second(std::forward<_Params>(args)...);
      }
      
      /// @brief Execute each function and return results
      template <typename _Rep = _ReturnType>
      typename std::enable_if<!std::is_void<_Rep>::value, std::vector<_ReturnType> >::type runResults(_Params... args) const {
        std::vector<_ReturnType> results;
        auto lockedFunctions = this->_functions->data();
        for (auto& fct : lockedFunctions.value())
          results.emplace_back(fct.second(std::forward<_Params>(args)...));
        return results;
      }
      
    private:
      ContractProvider _contractProvider;
      mutable std::shared_ptr<registrar> _functions = std::make_shared<registrar>();
    };

    // -- aliases --

    /// @brief Standard delegate pattern - registrations/revocations need to wait until the end of 'run' before being processed 
    ///        (calling them from 'run' callback would result in a dead-lock).
    template <typename _ReturnType, typename ... _Params>
    using BasicDelegate = Delegate<std::mutex, DelegateMode::lambda, true, _ReturnType, _Params...>;

    /// @brief Deferrable observer pattern - registrations/revocations can be deferred during 'run' (slower).
    template <typename _ReturnType, typename ... _Params>
    using DeferrableDelegate = Delegate<std::mutex, DelegateMode::lambda, true, _ReturnType, _Params...>;

    /// @brief Real-time delegate pattern with C-style function pointers - registrations/revocations need to wait until the end of 'run'.
    template <typename _ReturnType, typename ... _Params>
    using RealTimeDelegate = Delegate<pandora::thread::SpinLock, DelegateMode::functionPointer, false, _ReturnType, _Params...>;
  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
