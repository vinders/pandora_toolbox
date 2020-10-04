/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cassert>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread/spin_lock.h>
#include "./ticket.h"
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# define constexpr
#endif

namespace pandora {
  namespace pattern {
    class Contract;
    class ContractProvider;

    /// @class RevocationListener
    /// @brief Object that needs to be updated everytime a contract is revoked (usually the same object that calls 'createContract()')
    class ContractListener {
    public:
      virtual ~ContractListener() = default;

      /// @brief Event handler called when a contract is revoked (during execution, not on destruction)
      /// @warning Not called when the contract provider is destroyed
      virtual void onRevoke(const Contract& contract) noexcept = 0;
    };

    /// @class RegistrationCollection
    /// @brief Shared collection of contract registrations. Only intended for internal usage (use ContractProvider instead).
    class RegistrationCollection final {
    public:
      RegistrationCollection() = default;
      RegistrationCollection(const RegistrationCollection&) = delete;
      RegistrationCollection(RegistrationCollection&&) = delete;
      RegistrationCollection& operator=(const RegistrationCollection&) = delete;
      RegistrationCollection& operator=(RegistrationCollection&&) = delete;

      inline RegistrationCollection(std::shared_ptr<ContractListener> listener) : _listener(listener) {}
      ~RegistrationCollection() noexcept { clear(); }

      void insert(Contract& contract) noexcept; ///< Register a contract (internal usage)
      void revoke(Contract& contract) noexcept; ///< Unregister a contract (internal usage)
      void clear() noexcept; ///< Unregister all contracts (internal usage)

      inline size_t size() const noexcept { return this->_contracts.size(); } ///< Number of active contracts

    private:
      pandora::thread::SpinLock _contractsLock;
      std::unordered_map<Ticket<bool,bool,false>::Id, Contract*> _contracts;
      std::shared_ptr<ContractListener> _listener = nullptr;
    };

    /// @class ContractProvider
    /// @brief Object providing contracts and keeping track of their existence. 
    ///        This object should be kept during the entire lifetime of the object that establishes those contracts.
    ///        Can typically be found in observables, delegates, thread managers...
    class ContractProvider final {
    public:
      ContractProvider() : _contracts(std::make_shared<RegistrationCollection>()) {}
      ContractProvider(std::shared_ptr<ContractListener> listener) : _contracts(std::make_shared<RegistrationCollection>(listener)) {}
      ContractProvider(const ContractProvider&) = delete;
      ContractProvider(ContractProvider&&) noexcept = default;
      ContractProvider& operator=(const ContractProvider&) = delete;
      ContractProvider& operator=(ContractProvider&&) noexcept = default;

      /// @brief Unregister all contracts on destruction
      ~ContractProvider() noexcept { clear(); }

      /// @brief Create a new contract and register it
      std::unique_ptr<Contract> createContract() noexcept;
      /// @brief Invalidate all contracts created
      inline void clear() noexcept { 
        if (this->_contracts != nullptr) 
          this->_contracts->clear(); 
      }

      inline size_t size() const noexcept { return (this->_contracts != nullptr) ? this->_contracts->size() : 0u; } ///< Number of active contracts

    private:
      std::shared_ptr<RegistrationCollection> _contracts;
    };

    // ---

    /// @class Contract
    /// @brief Registration contract, useful for patterns such as the observer/observable, delegates, thread managers...
    /// @warning Contracts should only be created by a ContractProvider entity that keeps a list of them.
    ///          That way, the ContractProvider can revoke all contracts if it's destroyed.
    class Contract final {
    public:
      using InternalTicket = Ticket<bool,bool,false>;

      Contract() = default;
      ~Contract() noexcept { revoke(); }

      Contract(const Contract&) = delete;
      Contract(Contract&& rhs) = delete;
      Contract& operator=(const Contract&) = delete;
      Contract& operator=(Contract&&) = delete;
      
      // -- getters --

      /// @brief Verify if current contract is still valid
      inline bool isValid() const noexcept { 
        std::lock_guard<pandora::thread::SpinLock> lock(this->_ticketStatusLock);
        return this->_ticket.isValid(); 
      }
      inline operator bool() const noexcept { return isValid(); }

      /// @brief Get ticket identifier for current contract
      inline InternalTicket::Id ticket() const noexcept { return this->_ticket.id(); } // ID never changes -> no need to lock

      // -- operations --
      
      /// @brief Unregister from contract provider
      inline void revoke() noexcept {
        std::unique_lock<pandora::thread::SpinLock> lock(this->_ticketStatusLock);
        this->_ticket.invalidate();
        if (this->_registration != nullptr) {
          lock.unlock(); // required to avoid deadlock, when _registration calls "isValid"

          this->_registration->revoke(*this);
          this->_registration = nullptr;
        }
      }
      
      // -- comparisons --
      
      // ID never changes -> no need to lock (-> no deadlock if self-compared)
      constexpr inline bool operator==(const Contract& rhs) const { return (this->_ticket.id() == rhs._ticket.id()); }
      constexpr inline bool operator!=(const Contract& rhs) const { return (this->_ticket.id() != rhs._ticket.id()); }
      constexpr inline bool operator<(const Contract& rhs) const  { return (this->_ticket.id() < rhs._ticket.id()); }
      constexpr inline bool operator<=(const Contract& rhs) const { return (this->_ticket.id() <= rhs._ticket.id()); }
      constexpr inline bool operator>(const Contract& rhs) const  { return (this->_ticket.id() > rhs._ticket.id()); }
      constexpr inline bool operator>=(const Contract& rhs) const { return (this->_ticket.id() >= rhs._ticket.id()); }

      inline uint64_t operator()() const { return this->_ticket(); } // hash (ID never changes -> no need to lock)
      inline operator uint64_t() const { return this->_ticket(); }   // hash
      
    protected:
      // Create a contract - only allowed for contract providers
      inline Contract(std::shared_ptr<RegistrationCollection> registration) noexcept : _ticket(true), _registration(registration) {}

      // Unregistration triggered by contract provider
      inline void invalidate() noexcept {
        std::lock_guard<pandora::thread::SpinLock> lock(this->_ticketStatusLock);
        this->_ticket.invalidate();
        this->_registration = nullptr;
      }
      friend class ContractProvider;
      friend class RegistrationCollection;
      
    private:
      mutable pandora::thread::SpinLock _ticketStatusLock;
      InternalTicket _ticket{ false };
      std::shared_ptr<RegistrationCollection> _registration = nullptr;
    };

    // ---

    // -- ContractProvider implementation --

    inline std::unique_ptr<Contract> ContractProvider::createContract() noexcept {
      assert(this->_contracts != nullptr);
      auto contract = std::unique_ptr<Contract>(new Contract(this->_contracts));
      this->_contracts->insert(*contract);
#     if defined(_CPP_REVISION) && _CPP_REVISION == 14
        return std::move(contract);
#     else
        return contract;
#     endif
    }

    // -- RegistrationCollection implementation --

    inline void RegistrationCollection::clear() noexcept {
      std::lock_guard<pandora::thread::SpinLock> guard(this->_contractsLock);
      for (auto& it : this->_contracts) {
        if (it.second != nullptr) {
          if (it.second->isValid())
            it.second->invalidate();
          if (this->_listener != nullptr)
            this->_listener->onRevoke(*(it.second));
        }
      }
      this->_contracts = std::unordered_map<Ticket<bool,bool,false>::Id, Contract*>{};
    }

    inline void RegistrationCollection::insert(Contract& contract) noexcept {
      std::lock_guard<pandora::thread::SpinLock> guard(this->_contractsLock);
      this->_contracts.emplace(contract.ticket(), &contract);
    }

    inline void RegistrationCollection::revoke(Contract& contract) noexcept {
      {// scope
        std::lock_guard<pandora::thread::SpinLock> guard(this->_contractsLock);
        this->_contracts.erase(contract.ticket());
      }// end of scope -> unlock -> shorter critical section + no deadlock

      if (contract.isValid())
        contract.invalidate();
      if (this->_listener != nullptr)
        this->_listener->onRevoke(contract);
    }

  }
}
#if defined(_CPP_REVISION) && _CPP_REVISION == 14
# undef constexpr
#endif
