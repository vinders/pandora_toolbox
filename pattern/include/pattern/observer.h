/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <memory>
#include <type_traits>
#include <mutex>
#include "./contract.h"
#include "./contract_registrar.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace pattern {
    /// @class Observer
    /// @brief Observer interface - event listener of observable
    template <typename... _Params>
    class Observer {
    public:
      virtual ~Observer() = default;
      
      virtual void onUpdate(_Params... args) = 0;
    };
    
    // ---
    
    /// @class Observable
    /// @brief Observable pattern - event-oriented notification system
    /// @warning The 'notify' method should never contain 'moved' argument types (Type&&) : 
    ///          each moved argument would be 'moved' as many times as the number of observers -> crash or undefined behaviour.
    template <typename _LockType = std::mutex, ///< Internal mutex type .
              bool _IsDeferrable = true,       ///< Allow registrations/revocations to be deferred (true=deferrable; false=faster)
                                               ///  When allowed, registrations/revocations can be triggered in the 'notify' method without causing a dead-lock.
              typename... _Params>             ///< Template arguments of observer (signature arguments of 'notify' method)
    class Observable {
    public:
      using Type = Observable<_LockType,_IsDeferrable,_Params...>;
      using observer_type = Observer<_Params...>;
      using registrar = typename std::conditional<_IsDeferrable,  DeferrableContractRegistrar<std::shared_ptr<observer_type>,_LockType>, 
                                                                  DirectContractRegistrar<std::shared_ptr<observer_type>,_LockType> >::type;
      
      Observable() : _observers(std::make_shared<registrar>()) {
        this->_contractProvider = ContractProvider(this->_observers);
      }
      
      Observable(const Type&) = delete;
      Observable(Type&&) noexcept = default;
      Type& operator=(const Type&) = delete;
      Type& operator=(Type&&) noexcept = default;
      virtual ~Observable() = default;
      
      // -- getters --
      
      constexpr inline size_t size() const noexcept { return this->_observers->data().value().size(); } ///< Count registered observers
      constexpr inline bool empty() const noexcept { return this->_observers->data().value().empty(); } ///< Check if observable has observers
      
      // -- registration --
      
      /// @brief Register a new observer
      std::unique_ptr<Contract> addObserver(std::shared_ptr<observer_type> observer) {
        std::unique_ptr<Contract> contract = this->_contractProvider.createContract();
        assert(contract != nullptr && contract->isValid());
        if (observer != nullptr)
          this->_observers->create(*contract, std::move(observer));
#       if defined(_CPP_REVISION) && _CPP_REVISION == 14
          return std::move(contract);
#       else
          return contract;
#       endif
      }
      
      /// @brief Cancel a subscription
      void unregister(Contract& contract) noexcept { contract.revoke(); }
      
      /// @brief Remove all existing registrations
      inline void clear() noexcept { this->_contractProvider.clear(); }
      
      // -- notification --
      
      /// @brief Send event notification to all registered observers. 
      void notify(_Params... args) const {
        auto lockedObservers = this->_observers->data();
        for (auto& obs : lockedObservers.value())
          obs.second->onUpdate(std::forward<_Params>(args)...);
      }
      
    private:
      ContractProvider _contractProvider;
      mutable std::shared_ptr<registrar> _observers = std::make_shared<registrar>();
    };

    // -- aliases --

    /// @brief Standard observer pattern - registrations/revocations need to wait until the end of 'notify' before being processed 
    ///        (calling them from 'onUpdate' would result in a dead-lock).
    template <typename... _Params>
    using BasicObservable = Observable<std::mutex,false,_Params...>;

    /// @brief Deferrable observer pattern - registrations/revocations can be deferred during 'notify' (slower).
    template <typename... _Params>
    using DeferrableObservable = Observable<std::mutex,true,_Params...>;
  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
