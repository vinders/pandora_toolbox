#include <gtest/gtest.h>
#include <pattern/contract.h>

using namespace pandora::pattern;

class ContractTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};

// -- helpers --

class TestContractListener : public ContractListener {
public:
  TestContractListener() = default;
  virtual ~TestContractListener() = default;

  void onRevoke(const Contract& contract) noexcept override {
    _revokedTickets.emplace_back(contract.ticket());
  }

  inline void clear() noexcept { _revokedTickets.clear(); }

  inline const std::vector<uint64_t>& RevokedTickets() const noexcept { return _revokedTickets; }

private:
  std::vector<uint64_t> _revokedTickets;
};


// -- registration collection --

TEST_F(ContractTest, regCollection_accessors) {
  RegistrationCollection collection;
  EXPECT_EQ(size_t{ 0 }, collection.size());

  auto listener = std::make_shared<TestContractListener>();
  RegistrationCollection collectionWithListener(listener);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());

  Contract emptyContract;
  collection.revoke(emptyContract);
  EXPECT_EQ(size_t{ 0 }, collection.size());
  collectionWithListener.revoke(emptyContract);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract.ticket(), listener->RevokedTickets()[0]);
}

TEST_F(ContractTest, regCollection_insertRevoke) {
  RegistrationCollection collection;
  EXPECT_EQ(size_t{ 0 }, collection.size());

  auto listener = std::make_shared<TestContractListener>();
  RegistrationCollection collectionWithListener(listener);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());

  Contract emptyContract;
  collection.insert(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collection.size());
  collectionWithListener.insert(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collection.size());

  collection.revoke(emptyContract);
  EXPECT_EQ(size_t{ 0 }, collection.size());
  collectionWithListener.revoke(emptyContract);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract.ticket(), listener->RevokedTickets()[0]);
  listener->clear();

  Contract emptyContract2;
  Contract emptyContract3;
  collection.insert(emptyContract);
  collection.insert(emptyContract2);
  EXPECT_EQ(size_t{ 2 }, collection.size());
  collectionWithListener.insert(emptyContract);
  collectionWithListener.insert(emptyContract2);
  EXPECT_EQ(size_t{ 2 }, collection.size());

  collection.revoke(emptyContract2);
  EXPECT_EQ(size_t{ 1 }, collection.size());
  collectionWithListener.revoke(emptyContract2);
  EXPECT_EQ(size_t{ 1 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract2.ticket(), listener->RevokedTickets()[0]);

  collection.insert(emptyContract3);
  EXPECT_EQ(size_t{ 2 }, collection.size());
  collectionWithListener.insert(emptyContract3);
  EXPECT_EQ(size_t{ 2 }, collection.size());

  collection.revoke(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collection.size());
  collectionWithListener.revoke(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 2 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract.ticket(), listener->RevokedTickets()[1]);

  collection.revoke(emptyContract3);
  EXPECT_EQ(size_t{ 0 }, collection.size());
  collectionWithListener.revoke(emptyContract3);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 3 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract3.ticket(), listener->RevokedTickets()[2]);
}

TEST_F(ContractTest, regCollection_insertClear) {
  RegistrationCollection collection;
  EXPECT_EQ(size_t{ 0 }, collection.size());

  auto listener = std::make_shared<TestContractListener>();
  RegistrationCollection collectionWithListener(listener);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());

  Contract emptyContract;
  collection.insert(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collection.size());
  collectionWithListener.insert(emptyContract);
  EXPECT_EQ(size_t{ 1 }, collection.size());

  collection.clear();
  EXPECT_EQ(size_t{ 0 }, collection.size());
  collectionWithListener.revoke(emptyContract);
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(emptyContract.ticket(), listener->RevokedTickets()[0]);
  listener->clear();

  Contract emptyContract2;
  Contract emptyContract3;
  collection.insert(emptyContract);
  collection.insert(emptyContract2);
  EXPECT_EQ(size_t{ 2 }, collection.size());
  collectionWithListener.insert(emptyContract);
  collectionWithListener.insert(emptyContract2);
  EXPECT_EQ(size_t{ 2 }, collection.size());
  collection.insert(emptyContract3);
  EXPECT_EQ(size_t{ 3 }, collection.size());
  collectionWithListener.insert(emptyContract3);
  EXPECT_EQ(size_t{ 3 }, collection.size());

  collection.clear();
  EXPECT_EQ(size_t{ 0 }, collection.size());
  collectionWithListener.clear();
  EXPECT_EQ(size_t{ 0 }, collectionWithListener.size());
  ASSERT_EQ(size_t{ 3 }, listener->RevokedTickets().size());
}

// -- contract provider --

TEST_F(ContractTest, provider_accessors) {
  ContractProvider provider;
  EXPECT_EQ(size_t{ 0 }, provider.size());

  auto listener = std::make_shared<TestContractListener>();
  ContractProvider providerWithListener(listener);
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());

  auto contract = provider.createContract();
  auto contractListened = providerWithListener.createContract();
  EXPECT_EQ(size_t{ 1 }, provider.size());
  EXPECT_EQ(size_t{ 1 }, providerWithListener.size());
  EXPECT_TRUE(contract->isValid());
  EXPECT_TRUE(contractListened->isValid());

  contract->revoke();
  contractListened->revoke();
  EXPECT_FALSE(contract->isValid());
  EXPECT_FALSE(contractListened->isValid());
  EXPECT_EQ(size_t{ 0 }, provider.size());
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(contractListened->ticket(), listener->RevokedTickets()[0]);
  listener->clear();

  contract = provider.createContract();
  contractListened = providerWithListener.createContract();
  EXPECT_EQ(size_t{ 1 }, provider.size());
  EXPECT_EQ(size_t{ 1 }, providerWithListener.size());
  EXPECT_TRUE(contract->isValid());
  EXPECT_TRUE(contractListened->isValid());
  ContractProvider providerMoved(std::move(provider));
  EXPECT_EQ(size_t{ 0 }, provider.size());
  EXPECT_EQ(size_t{ 1 }, providerMoved.size());
  ContractProvider providerWithListenerMoved(std::move(providerWithListener));
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());
  EXPECT_EQ(size_t{ 1 }, providerWithListenerMoved.size());

  contract->revoke();
  contractListened->revoke();
  EXPECT_FALSE(contract->isValid());
  EXPECT_FALSE(contractListened->isValid());
  EXPECT_EQ(size_t{ 0 }, providerMoved.size());
  EXPECT_EQ(size_t{ 0 }, providerWithListenerMoved.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(contractListened->ticket(), listener->RevokedTickets()[0]);
}

TEST_F(ContractTest, provider_createClear) {
  ContractProvider provider;
  EXPECT_EQ(size_t{ 0 }, provider.size());

  auto listener = std::make_shared<TestContractListener>();
  ContractProvider providerWithListener(listener);
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());

  auto contract = provider.createContract();
  auto contractListened = providerWithListener.createContract();
  EXPECT_EQ(size_t{ 1 }, provider.size());
  EXPECT_EQ(size_t{ 1 }, providerWithListener.size());
  EXPECT_TRUE(contract->isValid());
  EXPECT_TRUE(contractListened->isValid());

  provider.clear();
  providerWithListener.clear();
  EXPECT_FALSE(contract->isValid());
  EXPECT_FALSE(contractListened->isValid());
  EXPECT_EQ(size_t{ 0 }, provider.size());
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());
  ASSERT_EQ(size_t{ 1 }, listener->RevokedTickets().size());
  EXPECT_EQ(contractListened->ticket(), listener->RevokedTickets()[0]);
  listener->clear();

  contract = provider.createContract();
  auto contract2 = provider.createContract();
  contractListened = providerWithListener.createContract();
  auto contractListened2 = providerWithListener.createContract();
  EXPECT_EQ(size_t{ 2 }, provider.size());
  EXPECT_EQ(size_t{ 2 }, providerWithListener.size());
  EXPECT_TRUE(contract->isValid());
  EXPECT_TRUE(contract2->isValid());
  EXPECT_TRUE(contractListened->isValid());
  EXPECT_TRUE(contractListened2->isValid());

  provider.clear();
  providerWithListener.clear();
  EXPECT_FALSE(contract->isValid());
  EXPECT_FALSE(contract2->isValid());
  EXPECT_FALSE(contractListened->isValid());
  EXPECT_FALSE(contractListened2->isValid());
  EXPECT_EQ(size_t{ 0 }, provider.size());
  EXPECT_EQ(size_t{ 0 }, providerWithListener.size());
  ASSERT_EQ(size_t{ 2 }, listener->RevokedTickets().size());
}

// -- contract utility --

TEST_F(ContractTest, contractAccessors) {
  Contract emptyContract;
  EXPECT_FALSE(emptyContract.isValid());
  EXPECT_FALSE(emptyContract);
  auto ticket1 = emptyContract.ticket();
  EXPECT_EQ(ticket1, emptyContract.ticket());
  EXPECT_EQ(ticket1, emptyContract());
  EXPECT_EQ(ticket1, static_cast<uint64_t>(emptyContract));

  ContractProvider provider;
  auto providedContract = provider.createContract();
  EXPECT_TRUE(providedContract->isValid());
  EXPECT_TRUE(*providedContract);
  auto ticket2 = providedContract->ticket();
  EXPECT_EQ(ticket2, providedContract->ticket());
  EXPECT_EQ(ticket2, (*providedContract)());
  EXPECT_EQ(ticket2, static_cast<uint64_t>(*providedContract));

  EXPECT_NE(ticket1, ticket2);
  EXPECT_TRUE(emptyContract == emptyContract);
  EXPECT_TRUE(*providedContract == *providedContract);
  EXPECT_FALSE(emptyContract == *providedContract);
  EXPECT_FALSE(emptyContract != emptyContract);
  EXPECT_FALSE(*providedContract != *providedContract);
  EXPECT_TRUE(emptyContract != *providedContract);

  EXPECT_TRUE(emptyContract <= emptyContract);
  EXPECT_FALSE(emptyContract < emptyContract);
  EXPECT_TRUE(*providedContract <= *providedContract);
  EXPECT_FALSE(*providedContract < *providedContract);
  EXPECT_TRUE(emptyContract <= *providedContract);
  EXPECT_TRUE(emptyContract < *providedContract);
  EXPECT_FALSE(*providedContract <= emptyContract);
  EXPECT_FALSE(*providedContract < emptyContract);

  EXPECT_TRUE(emptyContract >= emptyContract);
  EXPECT_FALSE(emptyContract > emptyContract);
  EXPECT_TRUE(*providedContract >= *providedContract);
  EXPECT_FALSE(*providedContract > *providedContract);
  EXPECT_FALSE(emptyContract >= *providedContract);
  EXPECT_FALSE(emptyContract > *providedContract);
  EXPECT_TRUE(*providedContract >= emptyContract);
  EXPECT_TRUE(*providedContract > emptyContract);
}

TEST_F(ContractTest, contractRevoke) {
  Contract emptyContract;
  EXPECT_FALSE(emptyContract.isValid());
  EXPECT_FALSE(emptyContract);
  auto ticket1 = emptyContract.ticket();

  ContractProvider provider;
  auto providedContract = provider.createContract();
  EXPECT_TRUE(providedContract->isValid());
  EXPECT_TRUE(*providedContract);
  auto ticket2 = providedContract->ticket();
  EXPECT_NE(ticket1, ticket2);

  emptyContract.revoke();
  EXPECT_FALSE(emptyContract.isValid());
  EXPECT_FALSE(emptyContract);
  EXPECT_EQ(ticket1, emptyContract.ticket());
  providedContract->revoke();
  EXPECT_FALSE(providedContract->isValid());
  EXPECT_FALSE(*providedContract);
  EXPECT_EQ(ticket2, providedContract->ticket());
}
