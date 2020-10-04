/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <deque>
#include <unordered_map>
#include <type_traits>
#include <mutex>
#include "./contract.h"
#include "./locked.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace pattern {
    /// @class DirectContractRegistrar
    /// @brief Basic registrations management: register/unregister calls will have to wait until current access to "data()" is closed.
    ///        More efficient than DeferrableContractRegistrar if registrations/unregistrations rarely occur (typically at startup/end of program).
    /// @warning No register/unregister operation is allowed in the data processing (callback/notify/...)!
    ///          -> data processing should never trigger the destruction of a Contract or ContractProvider. See DeferrableContractRegistrar to allow it.
    /// @description This instance should be passed as a shared_ptr to the corresponding ContractProvider(s).
    ///              ex: auto registrar = std::make_shared<DirectContractRegistrar<_RegisteredDataType> >();
    ///                  ContractProvider contractProvider(registrar);
    template <typename _RegisteredDataType, typename _LockType = std::mutex>
    class DirectContractRegistrar final : public ContractListener {
    public:
      using Type = DirectContractRegistrar<_RegisteredDataType,_LockType>;
      using registration_map = std::unordered_map<Contract::InternalTicket::Id, _RegisteredDataType>;
      using registered_data_move = typename std::conditional<std::is_class<_RegisteredDataType>::value, _RegisteredDataType&&, _RegisteredDataType>::type;

      DirectContractRegistrar() = default;
      DirectContractRegistrar(const Type&) = delete;
      DirectContractRegistrar(Type&& rhs) noexcept {
        std::lock_guard<_LockType> guard(rhs._registrationsLock);
        this->_registrations = std::move(rhs._registrations);
      }
      Type& operator=(const Type&) = delete;
      Type& operator=(Type&& rhs) noexcept {
        if (this == &rhs)
          return *this;
        std::lock_guard<_LockType> guard(this->_registrationsLock);
        std::lock_guard<_LockType> rhsGuard(rhs._registrationsLock);
        this->_registrations = std::move(rhs._registrations);
        return *this;
      }
      ~DirectContractRegistrar() noexcept { std::lock_guard<_LockType> guard(this->_registrationsLock); }

      // -- getters --

      /// @brief Read all registered data
      inline LockedRef<const registration_map, _LockType> data() noexcept {
        return LockedRef<const registration_map, _LockType>(this->_registrations, this->_registrationsLock);
      }

      // -- operations --

      /// @brief Add a new registration (call it right after the contract is created by a ContractProvider)
      inline void create(const Contract& contract, registered_data_move regData) noexcept {
        std::lock_guard<_LockType> guard(this->_registrationsLock);
        this->_registrations.emplace(contract.ticket(), std::move(regData));
      }

      // -- event handlers --

      /// @brief Remove a registration on contract destruction (called by ContractProvider)
      inline void onRevoke(const Contract& contract) noexcept override {
        std::lock_guard<_LockType> guard(this->_registrationsLock);
        this->_registrations.erase(contract.ticket());
      }

    private:
      registration_map _registrations;
      mutable _LockType _registrationsLock;
    };

    // ---

    /// @class DeferrableContractRegistrar
    /// @brief Manage registrations and defer them if necessary (to allow non-blocking registrations during a target access)
    /// @description This instance should be passed as a shared_ptr to the corresponding ContractProvider(s).
    ///              ex: auto registrar = std::make_shared<DeferrableContractRegistrar<_RegisteredDataType> >();
    ///                  ContractProvider contractProvider(registrar);
    template <typename _RegisteredDataType, typename _LockType = std::mutex>
    class DeferrableContractRegistrar final : public ContractListener {
    public:
      using Type = DeferrableContractRegistrar<_RegisteredDataType,_LockType>;
      using registration_map = std::unordered_map<Contract::InternalTicket::Id, _RegisteredDataType>;
      using registered_data_move = typename std::conditional<std::is_class<_RegisteredDataType>::value, _RegisteredDataType&&, _RegisteredDataType>::type;

      DeferrableContractRegistrar() = default;
      DeferrableContractRegistrar(const Type&) = delete;
      DeferrableContractRegistrar(Type&& rhs) noexcept {
        { std::lock_guard<_LockType> guard(rhs._registrationsLock);
          this->_registrations = std::move(rhs._registrations); }
        { std::lock_guard<_LockType> guard(rhs._pendingDataLock);
          this->_pendingRegistrations = std::move(rhs._pendingRegistrations);
          this->_pendingRevocations = std::move(rhs._pendingRevocations); }
      }
      Type& operator=(const Type&) = delete;
      Type& operator=(Type&& rhs) noexcept {
        if (this == &rhs)
          return *this;
        { std::lock_guard<_LockType> guard(this->_registrationsLock);
          std::lock_guard<_LockType> rhsGuard(rhs._registrationsLock);
          this->_registrations = std::move(rhs._registrations); }
        { std::lock_guard<_LockType> guard(this->_pendingDataLock);
          std::lock_guard<_LockType> rhsGuard(rhs._pendingDataLock);
          this->_pendingRegistrations = std::move(rhs._pendingRegistrations);
          this->_pendingRevocations = std::move(rhs._pendingRevocations); }
        return *this;
      }
      ~DeferrableContractRegistrar() noexcept { std::lock_guard<_LockType> guard(this->_registrationsLock); }

      // -- getters --

      /// @brief Read all registered data
      inline LockedRef<const registration_map, _LockType> data() noexcept {
        std::unique_lock<_LockType> guard(this->_registrationsLock);
        _processPendingRequests();
        return LockedRef<const registration_map, _LockType>(this->_registrations, std::move(guard));
      }

      // -- operations --

      /// @brief Add a new registration (call it right after the contract is created by a ContractProvider)
      inline void create(const Contract& contract, registered_data_move regData) noexcept {
        std::unique_lock<_LockType> guard(this->_registrationsLock, std::defer_lock);
        if (guard.try_lock())
          this->_registrations.emplace(contract.ticket(), std::move(regData));
        else {
          std::lock_guard<_LockType> guardPending(this->_pendingDataLock);
          this->_pendingRegistrations.emplace_back(contract.ticket(), std::move(regData));
        }
      }

      // -- event handlers --

      /// @brief Remove a registration on contract destruction (called by ContractProvider)
      inline void onRevoke(const Contract& contract) noexcept override {
        std::unique_lock<_LockType> guard(this->_registrationsLock, std::defer_lock);
        if (guard.try_lock()) {
          if (this->_registrations.find(contract.ticket()) != this->_registrations.end()) {
            this->_registrations.erase(contract.ticket());
          }
          else {
            guard.unlock();
            _cancelPendingRegistration(contract.ticket());
          }
        }
        else {
          std::lock_guard<_LockType> guardPending(this->_pendingDataLock);
          this->_pendingRevocations.emplace_back(contract.ticket());

          for (auto it = this->_pendingRegistrations.begin(); it != this->_pendingRegistrations.end(); ++it) {
            if (it->first == contract.ticket()) {
              this->_pendingRegistrations.erase(it);
              break;
            }
          }
        }
      }

    protected:
      // apply all pending operations (_registrationsLock must be locked!)
      void _processPendingRequests() noexcept {
        std::lock_guard<_LockType> guard(this->_pendingDataLock);

        if (!this->_pendingRevocations.empty())
          _processPendingRevocations();
        if (!this->_pendingRegistrations.empty())
          _processPendingRegistrations();
      }

      // revoke a pending registration
      void _cancelPendingRegistration(const Contract::InternalTicket::Id& ticket) noexcept {
        std::lock_guard<_LockType> guard(this->_pendingDataLock);
        for (auto it = this->_pendingRegistrations.begin(); it != this->_pendingRegistrations.end(); ++it) {
          if (it->first == ticket) {
            this->_pendingRegistrations.erase(it);
            break;
          }
        }
      }

    private:
      // apply pending registrations (_registrationsLock & _pendingDataLock must be locked!)
      void _processPendingRegistrations() noexcept {
        for (auto& reg : this->_pendingRegistrations) {
          this->_registrations.emplace(reg.first, std::move(reg.second));
        }
        this->_pendingRegistrations.clear();
      }
      // remove revoked registrations (_registrationsLock & _pendingDataLock must be locked!)
      void _processPendingRevocations() noexcept {
        for (auto& ticket : this->_pendingRevocations)
          this->_registrations.erase(ticket);
        this->_pendingRevocations.clear();
      }

    private:
      registration_map _registrations;
      std::deque<std::pair<Contract::InternalTicket::Id,_RegisteredDataType> > _pendingRegistrations;
      std::deque<Contract::InternalTicket::Id> _pendingRevocations;
      mutable _LockType _registrationsLock;
      mutable _LockType _pendingDataLock;
    };

  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
