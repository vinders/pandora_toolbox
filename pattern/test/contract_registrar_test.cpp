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
#include <gtest/gtest.h>
#include <pattern/contract_registrar.h>

using namespace pandora::pattern;

class ContractRegistrarTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- common helpers --

template <typename _Registrar, typename _DataType>
void createRevoke(_Registrar& registrar, const _DataType& value1, const _DataType& value2) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);
  ASSERT_TRUE(contract2 != nullptr);

  EXPECT_TRUE(registrar.data()->empty());

  registrar.create(*contract1, value1);
  {
    auto lockedRef = registrar.data();
    EXPECT_FALSE(lockedRef->empty());
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(value1, lockedRef->at(contract1->ticket()));
  }

  registrar.create(*contract2, value2);
  {
    auto lockedRef = registrar.data();
    EXPECT_FALSE(lockedRef->empty());
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(value1, lockedRef->at(contract1->ticket()));
    ASSERT_TRUE(lockedRef->find(contract2->ticket()) != lockedRef->end());
    EXPECT_EQ(value2, lockedRef->at(contract2->ticket()));
  }

  registrar.onRevoke(*contract1);
  {
    auto lockedRef = registrar.data();
    EXPECT_FALSE(lockedRef->empty());
    EXPECT_TRUE(lockedRef->find(contract1->ticket()) == lockedRef->end());
    ASSERT_TRUE(lockedRef->find(contract2->ticket()) != lockedRef->end());
    EXPECT_EQ(value2, lockedRef->at(contract2->ticket()));
  }

  registrar.onRevoke(*contract2);
  EXPECT_TRUE(registrar.data()->empty());
}

template <typename _Registrar, typename _DataType>
void moveRegistrar(const _DataType& value1, const _DataType& value2) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);
  ASSERT_TRUE(contract2 != nullptr);

  _Registrar registrar;
  EXPECT_TRUE(registrar.data()->empty());
  registrar.create(*contract1, value1);
  registrar.create(*contract2, value2);
  EXPECT_FALSE(registrar.data()->empty());

  _Registrar moved = std::move(registrar);
  EXPECT_TRUE(registrar.data()->empty());
  {
    auto lockedRef = moved.data();
    EXPECT_FALSE(lockedRef->empty());
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(42, lockedRef->at(contract1->ticket()));
    ASSERT_TRUE(lockedRef->find(contract2->ticket()) != lockedRef->end());
    EXPECT_EQ(64, lockedRef->at(contract2->ticket()));
  }
}

template <typename _Registrar, typename _DataType>
void revokeInvalid(const _DataType& value1) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();

  _Registrar registrar;
  EXPECT_TRUE(registrar.data()->empty());

  registrar.create(*contract1, value1);
  registrar.onRevoke(*contract2);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(value1, lockedRef->at(contract1->ticket()));
  }
}


// -- direct registrar --

TEST_F(ContractRegistrarTest, direct_createRevoke) {
  DirectContractRegistrar<int> registrar;
  createRevoke(registrar, 42, 64);
}
TEST_F(ContractRegistrarTest, direct_moveRegistrar) {
  moveRegistrar<DirectContractRegistrar<int> >(42, 64);
}
TEST_F(ContractRegistrarTest, direct_revokeInvalid) {
  revokeInvalid<DirectContractRegistrar<int> >(42);
}

// -- deferrable registrar --

TEST_F(ContractRegistrarTest, deferrable_directCreateRevoke) {
  DeferrableContractRegistrar<int> registrar;
  createRevoke(registrar, 42, 64);
}
TEST_F(ContractRegistrarTest, deferrable_moveRegistrar) {
  moveRegistrar<DeferrableContractRegistrar<int> >(42, 64);
}
TEST_F(ContractRegistrarTest, deferrable_revokeInvalid) {
  revokeInvalid<DeferrableContractRegistrar<int> >(42);
}

TEST_F(ContractRegistrarTest, deferrable_deferredCreationIfEmpty) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->empty());

    registrar.create(*contract1, 42);

    EXPECT_TRUE(lockedData->empty());
  }// unlock
    
  ASSERT_TRUE(contract1 != nullptr);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(42, lockedRef->at(contract1->ticket()));
  }
}
TEST_F(ContractRegistrarTest, deferrable_deferredCreation) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  std::unique_ptr<Contract> contract3 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);
  ASSERT_TRUE(contract2 != nullptr);
  ASSERT_TRUE(contract3 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  registrar.create(*contract1, 42);
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->size() == 1);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));

    registrar.create(*contract2, 64);

    EXPECT_TRUE(lockedData->size() == 1);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));
  }// unlock

  registrar.create(*contract3, 8);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 3);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(42, lockedRef->at(contract1->ticket()));
    ASSERT_TRUE(lockedRef->find(contract2->ticket()) != lockedRef->end());
    EXPECT_EQ(64, lockedRef->at(contract2->ticket()));
    ASSERT_TRUE(lockedRef->find(contract3->ticket()) != lockedRef->end());
    EXPECT_EQ(8, lockedRef->at(contract3->ticket()));
  }
}

TEST_F(ContractRegistrarTest, deferrable_deferredRevocationAlone) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  registrar.create(*contract1, 42);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(42, lockedRef->at(contract1->ticket()));

    registrar.onRevoke(*contract1);

    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(42, lockedRef->at(contract1->ticket()));
  }// unlock

  ASSERT_TRUE(contract1 != nullptr);
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->empty());
  }
}
TEST_F(ContractRegistrarTest, deferrable_deferredRevocation) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  std::unique_ptr<Contract> contract3 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);
  ASSERT_TRUE(contract2 != nullptr);
  ASSERT_TRUE(contract3 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  registrar.create(*contract1, 42);
  registrar.create(*contract2, 64);
  registrar.create(*contract3, 8);
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->size() == 3);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));
    ASSERT_TRUE(lockedData->find(contract2->ticket()) != lockedData->end());
    EXPECT_EQ(64, lockedData->at(contract2->ticket()));
    ASSERT_TRUE(lockedData->find(contract3->ticket()) != lockedData->end());
    EXPECT_EQ(8, lockedData->at(contract3->ticket()));

    registrar.onRevoke(*contract1);
    EXPECT_TRUE(lockedData->size() == 3);
  }// unlock

  registrar.onRevoke(*contract3);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract2->ticket()) != lockedRef->end());
    EXPECT_EQ(64, lockedRef->at(contract2->ticket()));
  }
}

TEST_F(ContractRegistrarTest, deferrable_deferredCreationsAndRevocations) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  std::unique_ptr<Contract> contract2 = provider.createContract();
  std::unique_ptr<Contract> contract3 = provider.createContract();
  std::unique_ptr<Contract> contract4 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);
  ASSERT_TRUE(contract2 != nullptr);
  ASSERT_TRUE(contract3 != nullptr);
  ASSERT_TRUE(contract4 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  registrar.create(*contract1, 42);
  registrar.create(*contract2, 64);
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->size() == 2);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));
    ASSERT_TRUE(lockedData->find(contract2->ticket()) != lockedData->end());
    EXPECT_EQ(64, lockedData->at(contract2->ticket()));

    registrar.create(*contract3, 8);
    registrar.onRevoke(*contract1);
    EXPECT_TRUE(lockedData->size() == 2);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));
    ASSERT_TRUE(lockedData->find(contract2->ticket()) != lockedData->end());
    EXPECT_EQ(64, lockedData->at(contract2->ticket()));
  }// unlock

  registrar.create(*contract4, 16);
  registrar.onRevoke(*contract2);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 2);
    ASSERT_TRUE(lockedRef->find(contract3->ticket()) != lockedRef->end());
    EXPECT_EQ(8, lockedRef->at(contract3->ticket()));
    ASSERT_TRUE(lockedRef->find(contract4->ticket()) != lockedRef->end());
    EXPECT_EQ(16, lockedRef->at(contract4->ticket()));
  }
}
TEST_F(ContractRegistrarTest, deferrable_deferredCreationRevoked) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->empty());

    registrar.create(*contract1, 42);
    EXPECT_TRUE(lockedData->empty());

    registrar.onRevoke(*contract1);
    EXPECT_TRUE(lockedData->empty());
  }// unlock

  ASSERT_TRUE(contract1 != nullptr);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->empty());
  }
}
TEST_F(ContractRegistrarTest, deferrable_deferredCreationDirectRevoked) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->empty());

    registrar.create(*contract1, 42);
    EXPECT_TRUE(lockedData->empty());
  }// unlock

  registrar.onRevoke(*contract1);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->empty());
  }
}
TEST_F(ContractRegistrarTest, deferrable_deferredRevocationRecreated) {
  ContractProvider provider;
  std::unique_ptr<Contract> contract1 = provider.createContract();
  ASSERT_TRUE(contract1 != nullptr);

  DeferrableContractRegistrar<int> registrar;
  registrar.create(*contract1, 42);
  {
    auto lockedData = registrar.data();
    EXPECT_TRUE(lockedData->size() == 1);
    ASSERT_TRUE(lockedData->find(contract1->ticket()) != lockedData->end());
    EXPECT_EQ(42, lockedData->at(contract1->ticket()));

    registrar.onRevoke(*contract1);
    EXPECT_TRUE(lockedData->size() == 1);

    registrar.create(*contract1, 32);
    EXPECT_TRUE(lockedData->size() == 1);
  }// unlock

  ASSERT_TRUE(contract1 != nullptr);
  {
    auto lockedRef = registrar.data();
    EXPECT_TRUE(lockedRef->size() == 1);
    ASSERT_TRUE(lockedRef->find(contract1->ticket()) != lockedRef->end());
    EXPECT_EQ(32, lockedRef->at(contract1->ticket()));
  }
}
