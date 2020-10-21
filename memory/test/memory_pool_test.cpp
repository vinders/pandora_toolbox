#include <gtest/gtest.h>
#include <memory/memory_pool.h>

using namespace pandora::memory;

class MemoryPoolTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- helpers --

struct _PodType {
  int a;
  int b;
  char c;
  float d;
  bool operator==(const _PodType& rhs) const noexcept { return (a==rhs.a && b==rhs.b && c==rhs.c && d==rhs.d); }
};

class _PoolObjectType final {
public:
  _PoolObjectType() { ++counter; }
  _PoolObjectType(int value) : value(value) { ++counter; }
  _PoolObjectType(const _PoolObjectType& rhs) : value(rhs.value) { ++counter; }
  _PoolObjectType(_PoolObjectType&& rhs) noexcept : value(rhs.value) { ++counter; }
  _PoolObjectType& operator=(const _PoolObjectType&) = default;
  _PoolObjectType& operator=(_PoolObjectType&&) = default;
  ~_PoolObjectType() { --counter; }

  bool operator==(const _PoolObjectType& rhs) const noexcept { return (value == rhs.value); }
  static int counter;

private:
  int value = 0;
};
int _PoolObjectType::counter = 0;


// -- metadata accessors --

TEST_F(MemoryPoolTest, ctorAccessors) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  EXPECT_EQ(MemoryPoolAllocation::onStack, pool.allocationType());
  EXPECT_EQ(size_t{ 256u }, pool.size());
  EXPECT_EQ(size_t{ 256u }, pool.capacity());
  EXPECT_EQ(size_t{ 256u }, pool.allocated());
  EXPECT_EQ(size_t{ 256u }, pool.length<uint8_t>());
  EXPECT_EQ(size_t{ 64u }, pool.length<uint32_t>());
  EXPECT_NE(nullptr, pool.get());
  EXPECT_EQ(pool.get(), pool.first());
  EXPECT_EQ(pool.get(), pool.begin());
  EXPECT_EQ(pool.get(), pool.cbegin());
  EXPECT_EQ(pool.get(pool.size() - 1u), pool.last());
  EXPECT_EQ((void*)(pool.get(pool.size() - 4u)), (void*)(pool.last<uint32_t>()));
  EXPECT_EQ(pool.get(pool.size() - 1u) + 1u, pool.end());
  EXPECT_EQ(pool.get(pool.size() - 1u) + 1u, pool.cend());
  EXPECT_EQ(pool.get(), pool.get(0));
  EXPECT_EQ(pool.get(), pool.get<uint8_t>(0));
  EXPECT_EQ((void*)(pool.get()), (void*)(pool.get<uint32_t>(0)));
  EXPECT_EQ(pool.get(), pool.at<uint8_t>(0));
  EXPECT_EQ((void*)(pool.get()), (void*)(pool.at<uint32_t>(0)));
  EXPECT_EQ(pool.get(), pool.at<uint8_t>(0, 0));
  EXPECT_EQ((void*)(pool.get()), (void*)(pool.at<uint32_t>(0, 0)));
  EXPECT_EQ(pool.get(), &pool[0]);
  EXPECT_NE(nullptr, pool.get(pool.size() - 1u));
  EXPECT_EQ(nullptr, pool.get(pool.size()));
  EXPECT_EQ(nullptr, pool.get(pool.size()*2u));
  EXPECT_NE(nullptr, pool.at<uint32_t>(pool.length<uint32_t>() - 1u));
  EXPECT_EQ(nullptr, pool.at<uint32_t>(pool.length<uint32_t>()));
  EXPECT_EQ(pool.get() + 42u, pool.get(size_t{ 42u }));
  EXPECT_EQ(pool.get() + 105u, pool.get(size_t{ 105u }));
  EXPECT_EQ(pool.get() + 168u, pool.get(size_t{ 168u }));
  EXPECT_EQ(pool.get(size_t{ 42u }), &pool[42]);
  EXPECT_EQ(pool.get(size_t{ 42u }), pool.get<uint8_t>(size_t{ 42u }));
  EXPECT_EQ(pool.get(size_t{ 42u }), pool.at<uint8_t>(size_t{ 42u }));
  EXPECT_EQ(pool.get(size_t{ 42u }), pool.at<uint8_t>(size_t{ 42u }, size_t{ 0 }));
  EXPECT_EQ(pool.get(size_t{ 42u }), pool.at<uint8_t>(size_t{ 21u }, size_t{ 21u }));
  EXPECT_EQ((void*)(pool.get(size_t{ 42u })), (void*)(pool.get<uint32_t>(size_t{ 42u })));
  EXPECT_EQ((void*)(pool.get(size_t{ 168u })), (void*)(pool.at<uint32_t>(size_t{ 42u })));
  EXPECT_EQ((void*)(pool.get(size_t{ 42u })), (void*)(pool.at<uint32_t>(size_t{ 42u }, size_t{ 0 })));
  EXPECT_EQ((void*)(pool.get(size_t{ 168u })), (void*)(pool.at<uint32_t>(size_t{ 0 }, size_t{ 42u })));
  EXPECT_EQ((void*)(pool.get(size_t{ 105u })), (void*)(pool.at<uint32_t>(size_t{ 21u }, size_t{ 21u })));

  const MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true>& poolConstRef(pool);
  EXPECT_EQ(poolConstRef.get(), pool.get());
  EXPECT_EQ(size_t{ 256u }, poolConstRef.size());
  EXPECT_NE(nullptr, poolConstRef.get());
  EXPECT_EQ(poolConstRef.get(), poolConstRef.first());
  EXPECT_EQ(poolConstRef.get(), poolConstRef.begin());
  EXPECT_EQ(poolConstRef.get(), poolConstRef.cbegin());
  EXPECT_EQ(poolConstRef.get(poolConstRef.size() - 1u), poolConstRef.last());
  EXPECT_EQ((const void*)(poolConstRef.get(poolConstRef.size() - 4u)), (const void*)(poolConstRef.last<uint32_t>()));
  EXPECT_EQ(poolConstRef.get(poolConstRef.size() - 1u) + 1u, poolConstRef.end());
  EXPECT_EQ(poolConstRef.get(poolConstRef.size() - 1u) + 1u, poolConstRef.cend());
  EXPECT_EQ(poolConstRef.get(), poolConstRef.get(0));
  EXPECT_EQ(poolConstRef.get(), poolConstRef.get<uint8_t>(0));
  EXPECT_EQ((void*)(poolConstRef.get()), (void*)(poolConstRef.get<uint32_t>(0)));
  EXPECT_EQ(poolConstRef.get(), poolConstRef.at<uint8_t>(0));
  EXPECT_EQ((void*)(poolConstRef.get()), (void*)(poolConstRef.at<uint32_t>(0)));
  EXPECT_EQ(poolConstRef.get(), poolConstRef.at<uint8_t>(0, 0));
  EXPECT_EQ((void*)(poolConstRef.get()), (void*)(poolConstRef.at<uint32_t>(0, 0)));
  EXPECT_EQ(poolConstRef.get(), &poolConstRef[0]);
  EXPECT_NE(nullptr, poolConstRef.get(poolConstRef.size() - 1u));
  EXPECT_EQ(nullptr, poolConstRef.get(poolConstRef.size()));
  EXPECT_EQ(nullptr, poolConstRef.get(poolConstRef.size()*2u));

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> pool2;
  EXPECT_EQ(MemoryPoolAllocation::onHeap, pool2.allocationType());
  EXPECT_EQ(size_t{ 256u }, pool2.size());
  EXPECT_EQ(size_t{ 256u }, pool2.capacity());
  EXPECT_EQ(size_t{ 288u }, pool2.allocated());
  EXPECT_EQ(size_t{ 256u }, pool2.length<uint8_t>());
  EXPECT_EQ(size_t{ 64u }, pool2.length<uint32_t>());
  EXPECT_NE(nullptr, pool2.get());
  EXPECT_EQ(pool2.get(), pool2.first());
  EXPECT_EQ(pool2.get(), pool2.begin());
  EXPECT_EQ(pool2.get(), pool2.cbegin());
  EXPECT_EQ(pool2.get(pool2.size() - 1u), pool2.last());
  EXPECT_EQ((void*)(pool2.get(pool2.size() - 4u)), (void*)(pool2.last<uint32_t>()));
  EXPECT_EQ(pool2.get(pool2.size() - 1u) + 1u, pool2.end());
  EXPECT_EQ(pool2.get(pool2.size() - 1u) + 1u, pool2.cend());
  EXPECT_EQ(pool2.get(), pool2.get(0));
  EXPECT_EQ(pool2.get(), pool2.get<uint8_t>(0));
  EXPECT_EQ((void*)(pool2.get()), (void*)(pool2.get<uint32_t>(0)));
  EXPECT_EQ(pool2.get(), pool2.at<uint8_t>(0));
  EXPECT_EQ((void*)(pool2.get()), (void*)(pool2.at<uint32_t>(0)));
  EXPECT_EQ(pool2.get(), pool2.at<uint8_t>(0, 0));
  EXPECT_EQ((void*)(pool2.get()), (void*)(pool2.at<uint32_t>(0, 0)));
  EXPECT_EQ(pool2.get(), &pool2[0]);
  EXPECT_NE(nullptr, pool2.get(pool2.size() - 1u));
  EXPECT_EQ(nullptr, pool2.get(pool2.size()));
  EXPECT_EQ(nullptr, pool2.get(pool2.size()*2u));
  EXPECT_NE(nullptr, pool2.at<uint32_t>(pool2.length<uint32_t>() - 1u));
  EXPECT_EQ(nullptr, pool2.at<uint32_t>(pool2.length<uint32_t>()));
  EXPECT_EQ(pool2.get() + 42u, pool2.get(size_t{ 42u }));
  EXPECT_EQ(pool2.get() + 168u, pool2.get(size_t{ 168u }));
  EXPECT_EQ(pool2.get(size_t{ 42u }), &pool2[42]);
  EXPECT_EQ(pool2.get(size_t{ 42u }), pool2.get<uint8_t>(size_t{ 42u }));
  EXPECT_EQ(pool2.get(size_t{ 42u }), pool2.at<uint8_t>(size_t{ 42u }));
  EXPECT_EQ(pool2.get(size_t{ 42u }), pool2.at<uint8_t>(size_t{ 42u }, size_t{ 0 }));
  EXPECT_EQ(pool2.get(size_t{ 42u }), pool2.at<uint8_t>(size_t{ 21u }, size_t{ 21u }));
  EXPECT_EQ((void*)(pool2.get(size_t{ 42u })), (void*)(pool2.get<uint32_t>(size_t{ 42u })));
  EXPECT_EQ((void*)(pool2.get(size_t{ 168u })), (void*)(pool2.at<uint32_t>(size_t{ 42u })));
  EXPECT_EQ((void*)(pool2.get(size_t{ 42u })), (void*)(pool2.at<uint32_t>(size_t{ 42u }, size_t{ 0 })));
  EXPECT_EQ((void*)(pool2.get(size_t{ 168u })), (void*)(pool2.at<uint32_t>(size_t{ 0 }, size_t{ 42u })));
  EXPECT_EQ((void*)(pool2.get(size_t{ 105u })), (void*)(pool2.at<uint32_t>(size_t{ 21u }, size_t{ 21u })));

  MemoryPool<size_t{ 32000u }, size_t{ 256u }, MemoryPoolAllocation::automatic, false> pool3;
  EXPECT_EQ(MemoryPoolAllocation::onHeap, pool3.allocationType());
  EXPECT_EQ(size_t{ 32000u }, pool3.size());
  EXPECT_EQ(size_t{ 32000u }, pool3.capacity());
  EXPECT_EQ(size_t{ 32512u }, pool3.allocated());
  EXPECT_EQ(size_t{ 32000u }, pool3.length<uint8_t>());
  EXPECT_EQ(size_t{ 8000u }, pool3.length<uint32_t>());
  EXPECT_NE(nullptr, pool3.get());
  EXPECT_EQ(pool3.get(), pool3.first());
  EXPECT_EQ(pool3.get(), pool3.begin());
  EXPECT_EQ(pool3.get(), pool3.cbegin());
  EXPECT_EQ(pool3.get(pool3.size() - 1u), pool3.last());
  EXPECT_EQ((void*)(pool3.get(pool3.size() - 4u)), (void*)(pool3.last<uint32_t>()));
  EXPECT_EQ(pool3.get(pool3.size() - 1u) + 1u, pool3.end());
  EXPECT_EQ(pool3.get(pool3.size() - 1u) + 1u, pool3.cend());
  EXPECT_EQ(pool3.get(), pool3.get(0));
  EXPECT_EQ(pool3.get(), pool3.get<uint8_t>(0));
  EXPECT_EQ((void*)(pool3.get()), (void*)(pool3.get<uint32_t>(0)));
  EXPECT_EQ(pool3.get(), pool3.at<uint8_t>(0));
  EXPECT_EQ((void*)(pool3.get()), (void*)(pool3.at<uint32_t>(0)));
  EXPECT_EQ(pool3.get(), pool3.at<uint8_t>(0, 0));
  EXPECT_EQ((void*)(pool3.get()), (void*)(pool3.at<uint32_t>(0, 0)));
  EXPECT_EQ(pool3.get(), &pool3[0]);
  EXPECT_NE(nullptr, pool3.get(pool3.size() - 1u));
  EXPECT_NE(nullptr, pool3.get(pool3.size()));
  EXPECT_NE(nullptr, pool3.get<uint8_t>(pool3.size()));
  EXPECT_NE(nullptr, pool3.at<uint8_t>(pool3.size()));
}

TEST_F(MemoryPoolTest, cloneMoveSwap) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  EXPECT_EQ(MemoryPoolAllocation::onStack, pool.allocationType());
  EXPECT_EQ(size_t{ 256u }, pool.size());

  memset((void*)pool.get(), 42, pool.size());
  *(pool.first()) = (uint8_t)16u;
  *(pool.last()) = (uint8_t)64u;
  EXPECT_EQ((uint8_t)16u, pool[0]);
  EXPECT_EQ((uint8_t)42u, pool[1]);
  EXPECT_EQ((uint8_t)42u, pool[pool.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, pool[pool.size() - 1u]);

  auto poolClone = pool.clone();
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolClone.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolClone.size());
  EXPECT_EQ((uint8_t)16u, poolClone[0]);
  EXPECT_EQ((uint8_t)42u, poolClone[1]);
  EXPECT_EQ((uint8_t)42u, poolClone[poolClone.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolClone[poolClone.size() - 1u]);

  memset((void*)poolClone.get(), 32, poolClone.size());
  *(poolClone.first()) = (uint8_t)8u;
  *(poolClone.last()) = (uint8_t)48u;
  EXPECT_EQ((uint8_t)8u, poolClone[0]);
  EXPECT_EQ((uint8_t)32u, poolClone[1]);
  EXPECT_EQ((uint8_t)32u, poolClone[poolClone.size() - 2u]);
  EXPECT_EQ((uint8_t)48u, poolClone[poolClone.size() - 1u]);

  poolClone = std::move(pool);
  EXPECT_EQ(size_t{ 256u }, poolClone.size());
  EXPECT_EQ((uint8_t)16u, poolClone[0]);
  EXPECT_EQ((uint8_t)42u, poolClone[1]);
  EXPECT_EQ((uint8_t)42u, poolClone[poolClone.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolClone[poolClone.size() - 1u]);
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> poolMoved(std::move(poolClone));
  EXPECT_EQ(size_t{ 256u }, poolMoved.size());
  EXPECT_EQ((uint8_t)16u, poolMoved[0]);
  EXPECT_EQ((uint8_t)42u, poolMoved[1]);
  EXPECT_EQ((uint8_t)42u, poolMoved[poolMoved.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolMoved[poolMoved.size() - 1u]);

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true> poolGuard;
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolGuard.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolGuard.size());
  memset((void*)poolGuard.get(), 42, poolGuard.size());
  *(poolGuard.first()) = (uint8_t)16u;
  *(poolGuard.last()) = (uint8_t)64u;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true> poolGuardMoved(std::move(poolGuard));
  EXPECT_EQ(size_t{ 256u }, poolGuardMoved.size());
  EXPECT_EQ((uint8_t)16u, poolGuardMoved[0]);
  EXPECT_EQ((uint8_t)42u, poolGuardMoved[1]);
  EXPECT_EQ((uint8_t)42u, poolGuardMoved[poolGuardMoved.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolGuardMoved[poolGuardMoved.size() - 1u]);
  poolGuard = std::move(poolGuardMoved);
  EXPECT_EQ(size_t{ 256u }, poolGuard.size());
  EXPECT_EQ((uint8_t)16u, poolGuard[0]);
  EXPECT_EQ((uint8_t)42u, poolGuard[1]);
  EXPECT_EQ((uint8_t)42u, poolGuard[poolGuard.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolGuard[poolGuard.size() - 1u]);

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  EXPECT_EQ(MemoryPoolAllocation::onHeap, poolH.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolH.size());

  memset((void*)poolH.get(), 42, poolH.size());
  *(poolH.first()) = (uint8_t)16u;
  *(poolH.last()) = (uint8_t)64u;
  EXPECT_EQ((uint8_t)16u, poolH[0]);
  EXPECT_EQ((uint8_t)42u, poolH[1]);
  EXPECT_EQ((uint8_t)42u, poolH[poolH.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolH[poolH.size() - 1u]);

  auto poolHClone = poolH.clone();
  EXPECT_EQ(MemoryPoolAllocation::onHeap, poolHClone.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolHClone.size());
  EXPECT_EQ((uint8_t)16u, poolHClone[0]);
  EXPECT_EQ((uint8_t)42u, poolHClone[1]);
  EXPECT_EQ((uint8_t)42u, poolHClone[poolHClone.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolHClone[poolHClone.size() - 1u]);

  memset((void*)poolHClone.get(), 32, poolHClone.size());
  *(poolHClone.first()) = (uint8_t)8u;
  *(poolHClone.last()) = (uint8_t)48u;
  EXPECT_EQ((uint8_t)8u, poolHClone[0]);
  EXPECT_EQ((uint8_t)32u, poolHClone[1]);
  EXPECT_EQ((uint8_t)32u, poolHClone[poolHClone.size() - 2u]);
  EXPECT_EQ((uint8_t)48u, poolHClone[poolHClone.size() - 1u]);

  poolH.swap(poolHClone);
  EXPECT_EQ(size_t{ 256u }, poolH.size());
  EXPECT_EQ(size_t{ 256u }, poolHClone.size());
  EXPECT_EQ((uint8_t)8u, poolH[0]);
  EXPECT_EQ((uint8_t)32u, poolH[1]);
  EXPECT_EQ((uint8_t)32u, poolH[poolHClone.size() - 2u]);
  EXPECT_EQ((uint8_t)48u, poolH[poolHClone.size() - 1u]);
  EXPECT_EQ((uint8_t)16u, poolHClone[0]);
  EXPECT_EQ((uint8_t)42u, poolHClone[1]);
  EXPECT_EQ((uint8_t)42u, poolHClone[poolHClone.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolHClone[poolHClone.size() - 1u]);

  poolH = std::move(poolHClone);
  EXPECT_EQ(size_t{ 256u }, poolH.size());
  EXPECT_EQ((uint8_t)16u, poolH[0]);
  EXPECT_EQ((uint8_t)42u, poolH[1]);
  EXPECT_EQ((uint8_t)42u, poolH[poolH.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolH[poolH.size() - 1u]);
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolHMoved(std::move(poolH));
  EXPECT_EQ(size_t{ 256u }, poolHMoved.size());
  EXPECT_EQ((uint8_t)16u, poolHMoved[0]);
  EXPECT_EQ((uint8_t)42u, poolHMoved[1]);
  EXPECT_EQ((uint8_t)42u, poolHMoved[poolHMoved.size() - 2u]);
  EXPECT_EQ((uint8_t)64u, poolHMoved[poolHMoved.size() - 1u]);
}

TEST_F(MemoryPoolTest, compare) {
  MemoryPool < size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true > pool;
  EXPECT_EQ(MemoryPoolAllocation::onStack, pool.allocationType());
  EXPECT_EQ(size_t{ 256u }, pool.size());
  memset((void*)pool.get(), 42, pool.size());
  *(pool.first()) = (uint8_t)16u;
  *(pool.last()) = (uint8_t)64u;

  MemoryPool < size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true > poolSame;
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolSame.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolSame.size());
  memset((void*)poolSame.get(), 42, poolSame.size());
  *(poolSame.first()) = (uint8_t)16u;
  *(poolSame.last()) = (uint8_t)64u;

  MemoryPool < size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true > poolDiff;
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolDiff.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolDiff.size());
  memset((void*)poolDiff.get(), 42, poolDiff.size());
  *(poolDiff.first()) = (uint8_t)8u;
  *(poolDiff.last()) = (uint8_t)48u;

  EXPECT_TRUE(pool == pool);
  EXPECT_TRUE(pool == poolSame);
  EXPECT_FALSE(pool == poolDiff);
  EXPECT_FALSE(pool != pool);
  EXPECT_FALSE(pool != poolSame);
  EXPECT_TRUE(pool != poolDiff);
  EXPECT_TRUE(pool <= pool);
  EXPECT_TRUE(pool <= poolSame);
  EXPECT_TRUE(pool >= pool);
  EXPECT_TRUE(pool >= poolSame);
  EXPECT_FALSE(pool < pool);
  EXPECT_FALSE(pool < poolSame);
  EXPECT_FALSE(pool > pool);
  EXPECT_FALSE(pool > poolSame);

  EXPECT_TRUE(pool.compare(pool) == 0);
  EXPECT_TRUE(pool.compare(poolSame) == 0);
  EXPECT_TRUE(pool.compare(poolDiff) != 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 0 }, poolSame.size()) == 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 0 }, size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 64u }, size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 64u }, poolSame.size() - 64u) == 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 64u }, size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(poolSame, size_t{ 1u }, poolSame.size() - 2u) == 0);
  EXPECT_TRUE(pool.compare(poolSame, pool.size(), poolSame.size()) == 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 0 }, poolDiff.size()) != 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 0 }, size_t{ 64u }) != 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 64u }, size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 64u }, poolDiff.size() - 64u) != 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 64u }, size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(poolDiff, size_t{ 1u }, poolDiff.size() - 2u) == 0);
  EXPECT_TRUE(pool.compare(poolDiff, pool.size(), poolDiff.size()) == 0);

  EXPECT_TRUE(pool.compare<uint8_t>(size_t{ 0 }, nullptr) > 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolSame.get(), poolSame.size()) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolSame.get(size_t{ 2u }), poolSame.size() - 2u) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolSame.get(), size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolSame.get(size_t{ 2u }), size_t{ 64u }) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolSame.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolSame.get(size_t{ 64u }), poolSame.size() - 64u) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolSame.get(), poolSame.size()) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolSame.get(size_t{ 64u }), size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 1u }, poolSame.get(), poolSame.size() - 2u) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 1u }, poolSame.get(size_t{ 1u }), poolSame.size() - 2u) == 0);
  EXPECT_TRUE(pool.compare(pool.size(), poolSame.get(), poolSame.size()) < 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolDiff.get(), poolDiff.size()) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolDiff.get(size_t{ 2u }), poolDiff.size() - 2u) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 0 }, poolDiff.get(), size_t{ 64u }) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 1u }, poolDiff.get(size_t{ 1u }), size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolDiff.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolDiff.get(size_t{ 64u }), poolDiff.size() - 64u) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolDiff.get(), poolDiff.size()) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 64u }, poolDiff.get(size_t{ 64u }), size_t{ 64u }) == 0);
  EXPECT_TRUE(pool.compare(size_t{ 1u }, poolDiff.get(), poolDiff.size() - 2u) != 0);
  EXPECT_TRUE(pool.compare(size_t{ 1u }, poolDiff.get(size_t{ 1u }), poolDiff.size() - 2u) == 0);
  EXPECT_TRUE(pool.compare(pool.size(), poolDiff.get(), poolDiff.size()) < 0);
}
TEST_F(MemoryPoolTest, compareAt) {
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true> pool;
  EXPECT_EQ(MemoryPoolAllocation::onStack, pool.allocationType());
  EXPECT_EQ(size_t{ 256u }, pool.size());
  memset((void*)pool.get(), 42, pool.size());
  *(pool.first()) = (uint8_t)16u;
  *(pool.last()) = (uint8_t)64u;

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true> poolSame;
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolSame.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolSame.size());
  memset((void*)poolSame.get(), 42, poolSame.size());
  *(poolSame.first()) = (uint8_t)16u;
  *(poolSame.last()) = (uint8_t)64u;

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onStack, true> poolDiff;
  EXPECT_EQ(MemoryPoolAllocation::onStack, poolDiff.allocationType());
  EXPECT_EQ(size_t{ 256u }, poolDiff.size());
  memset((void*)poolDiff.get(), 42, poolDiff.size());
  *(poolDiff.first()) = (uint8_t)8u;
  *(poolDiff.last()) = (uint8_t)48u;

  EXPECT_TRUE(pool.compareAt<uint32_t>(size_t{ 0 }, nullptr) > 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolSame.get(), poolSame.size()/4u) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 4u }), poolSame.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolSame.get(), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 2u }), size_t{ 16u }) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolSame.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolSame.get(size_t{ 64u }), poolSame.size()/4u - 16u) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolSame.get(), poolSame.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolSame.get(size_t{ 64u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 1u }, (uint32_t*)poolSame.get(), poolSame.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 1u }, (uint32_t*)poolSame.get(size_t{ 4u }), poolSame.size()/4u - 1u) == 0);
  EXPECT_TRUE(pool.compareAt(pool.size()/4u, (uint32_t*)poolSame.get(), poolSame.size()/4u) < 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolDiff.get(size_t{ 4u }), poolDiff.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, (uint32_t*)poolDiff.get(), size_t{ 16u }) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 1u }, (uint32_t*)poolDiff.get(size_t{ 4u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolDiff.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolDiff.get(size_t{ 64u }), poolDiff.size()/4u - 16u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 16u }, (uint32_t*)poolDiff.get(size_t{ 64u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 1u }, (uint32_t*)poolDiff.get(), poolDiff.size()/4 - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 1u }, (uint32_t*)poolDiff.get(size_t{ 4u }), poolDiff.size()/4u - 2u) == 0);
  EXPECT_TRUE(pool.compareAt(pool.size()/4u, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) < 0);

  EXPECT_TRUE(pool.compareAt<uint32_t>(size_t{ 0 }, size_t{ 0 }, nullptr) > 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolSame.get(), poolSame.size()/4u) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 4u }), poolSame.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolSame.get(), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 2u }), size_t{ 16u }) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 16u }, (uint32_t*)poolSame.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 64u }, size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 64u }), poolSame.size()/4u - 16u) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 32u }, size_t{ 8u }, (uint32_t*)poolSame.get(), poolSame.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 32u }, size_t{ 8u }, (uint32_t*)poolSame.get(size_t{ 64u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 1u }, (uint32_t*)poolSame.get(), poolSame.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 4u }, size_t{ 0 }, (uint32_t*)poolSame.get(size_t{ 4u }), poolSame.size()/4u - 1u) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, pool.size()/4u, (uint32_t*)poolSame.get(), poolSame.size()/4u) < 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 4u }, size_t{ 0 }, (uint32_t*)poolDiff.get(size_t{ 4u }), poolDiff.size()/4u - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 0 }, (uint32_t*)poolDiff.get(), size_t{ 16u }) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 4u }, size_t{ 0 }, (uint32_t*)poolDiff.get(size_t{ 4u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 64u }, size_t{ 0 }, (uint32_t*)poolDiff.get(), size_t{ 0 }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 32u }, size_t{ 8u }, (uint32_t*)poolDiff.get(size_t{ 64u }), poolDiff.size()/4u - 16u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 64u }, size_t{ 0 }, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 32u }, size_t{ 8u }, (uint32_t*)poolDiff.get(size_t{ 64u }), size_t{ 16u }) == 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 0 }, size_t{ 1u }, (uint32_t*)poolDiff.get(), poolDiff.size()/4 - 1u) != 0);
  EXPECT_TRUE(pool.compareAt(size_t{ 4u }, size_t{ 0 }, (uint32_t*)poolDiff.get(size_t{ 4u }), poolDiff.size()/4u - 2u) == 0);
  EXPECT_TRUE(pool.compareAt(pool.size(), size_t{ 0 }, (uint32_t*)poolDiff.get(), poolDiff.size()/4u) < 0);
}


// -- read/write/clear/fill data --

TEST_F(MemoryPoolTest, clearFill) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t buffer[256];
  memset((void*)buffer, 0, pool.size());
  EXPECT_EQ((uint8_t)0, *pool.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *pool.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)pool.get(), (void*)buffer, pool.size()) == 0);
  EXPECT_EQ((uint8_t)0, *poolH.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *poolH.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)poolH.get(), (void*)buffer, pool.size()) == 0);

  pool.fill((uint8_t)42u);
  poolH.fill((uint8_t)42u);
  memset((void*)buffer, (uint8_t)42u, pool.size());
  EXPECT_EQ((uint8_t)42u, *pool.first<uint8_t>());
  EXPECT_EQ((uint8_t)42u, *pool.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)pool.get(), (void*)buffer, pool.size()) == 0);
  EXPECT_EQ((uint8_t)42u, *poolH.first<uint8_t>());
  EXPECT_EQ((uint8_t)42u, *poolH.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)poolH.get(), (void*)buffer, pool.size()) == 0);

  pool.clear();
  poolH.clear();
  memset((void*)buffer, 0, pool.size());
  EXPECT_EQ((uint8_t)0, *pool.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *pool.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)pool.get(), (void*)buffer, pool.size()) == 0);
  EXPECT_EQ((uint8_t)0, *poolH.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *poolH.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)poolH.get(), (void*)buffer, pool.size()) == 0);

  EXPECT_TRUE(pool.fill((uint8_t)42u, size_t{ 16u }, size_t{ 32u }));
  EXPECT_TRUE(pool.fill((uint8_t)64u, size_t{ 52u }, size_t{ 16u }));
  EXPECT_TRUE(poolH.fill((uint8_t)42u, size_t{ 16u }, size_t{ 32u }));
  EXPECT_TRUE(poolH.fill((uint8_t)64u, size_t{ 52u }, size_t{ 16u }));
  memset((void*)&buffer[16], (uint8_t)42u, size_t{ 32u });
  memset((void*)&buffer[52], (uint8_t)64u, size_t{ 16u });
  EXPECT_EQ((uint8_t)0, *pool.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *pool.get<uint8_t>(size_t{ 15u }));
  EXPECT_EQ((uint8_t)42u, *pool.get<uint8_t>(size_t{ 16u }));
  EXPECT_EQ((uint8_t)0, *pool.get<uint8_t>(size_t{ 51u }));
  EXPECT_EQ((uint8_t)64u, *pool.get<uint8_t>(size_t{ 52u }));
  EXPECT_EQ((uint8_t)0, *pool.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)pool.get(), (void*)buffer, pool.size()) == 0);
  EXPECT_EQ((uint8_t)0, *poolH.first<uint8_t>());
  EXPECT_EQ((uint8_t)0, *poolH.get<uint8_t>(size_t{ 15u }));
  EXPECT_EQ((uint8_t)42u, *poolH.get<uint8_t>(size_t{ 16u }));
  EXPECT_EQ((uint8_t)0, *poolH.get<uint8_t>(size_t{ 51u }));
  EXPECT_EQ((uint8_t)64u, *poolH.get<uint8_t>(size_t{ 52u }));
  EXPECT_EQ((uint8_t)0, *poolH.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)poolH.get(), (void*)buffer, pool.size()) == 0);

  pool.fill((uint8_t)42u);
  poolH.fill((uint8_t)42u);
  EXPECT_TRUE(pool.clear(size_t{ 16u }, size_t{ 32u }));
  EXPECT_TRUE(pool.clear(size_t{ 52u }, size_t{ 16u }));
  EXPECT_TRUE(poolH.clear(size_t{ 16u }, size_t{ 32u }));
  EXPECT_TRUE(poolH.clear(size_t{ 52u }, size_t{ 16u }));
  memset((void*)buffer, (uint8_t)42u, pool.size());
  memset((void*)&buffer[16], 0, size_t{ 32u });
  memset((void*)&buffer[52], 0, size_t{ 16u });
  EXPECT_EQ((uint8_t)42u, *pool.first<uint8_t>());
  EXPECT_EQ((uint8_t)42u, *pool.get<uint8_t>(size_t{ 15u }));
  EXPECT_EQ((uint8_t)0, *pool.get<uint8_t>(size_t{ 16u }));
  EXPECT_EQ((uint8_t)42u, *pool.get<uint8_t>(size_t{ 51u }));
  EXPECT_EQ((uint8_t)0, *pool.get<uint8_t>(size_t{ 52u }));
  EXPECT_EQ((uint8_t)42u, *pool.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)pool.get(), (void*)buffer, pool.size()) == 0);
  EXPECT_EQ((uint8_t)42u, *poolH.first<uint8_t>());
  EXPECT_EQ((uint8_t)42u, *poolH.get<uint8_t>(size_t{ 15u }));
  EXPECT_EQ((uint8_t)0, *poolH.get<uint8_t>(size_t{ 16u }));
  EXPECT_EQ((uint8_t)42u, *poolH.get<uint8_t>(size_t{ 51u }));
  EXPECT_EQ((uint8_t)0, *poolH.get<uint8_t>(size_t{ 52u }));
  EXPECT_EQ((uint8_t)42u, *poolH.last<uint8_t>());
  EXPECT_TRUE(memcmp((void*)poolH.get(), (void*)buffer, pool.size()) == 0);

  EXPECT_FALSE(pool.clear(size_t{ 16u }, size_t{ 10000u }));
  EXPECT_FALSE(poolH.clear(size_t{ 16u }, size_t{ 10000u }));
  EXPECT_FALSE(pool.clear(size_t{ 10000u }, size_t{ 16u }));
  EXPECT_FALSE(poolH.clear(size_t{ 10000u }, size_t{ 16u }));
  EXPECT_FALSE(pool.fill((uint8_t)42u, size_t{ 16u }, size_t{ 10000u }));
  EXPECT_FALSE(poolH.fill((uint8_t)42u, size_t{ 16u }, size_t{ 10000u }));
  EXPECT_FALSE(pool.fill((uint8_t)42u, size_t{ 10000u }, size_t{ 16u }));
  EXPECT_FALSE(poolH.fill((uint8_t)42u, size_t{ 10000u }, size_t{ 16u }));
}

TEST_F(MemoryPoolTest, setRaw) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  uint8_t buffer[256];
  for (size_t i = pool.size(); i; --i)
    buffer[i - 1u] = (uint8_t)i;

  EXPECT_FALSE(pool.set<uint8_t>(size_t{ 16u }, buffer, size_t{ 10000u }));
  EXPECT_FALSE(poolH.set<uint8_t>(size_t{ 16u }, buffer, size_t{ 10000u }));
  EXPECT_FALSE(pool.set<uint8_t>(size_t{ 10000u }, buffer, size_t{ 16u }));
  EXPECT_FALSE(poolH.set<uint8_t>(size_t{ 10000u }, buffer, size_t{ 16u }));

  EXPECT_TRUE(pool.set<uint8_t>(size_t{ 0 }, buffer, pool.size()));
  EXPECT_TRUE(poolH.set<uint8_t>(size_t{ 0 }, buffer, poolH.size()));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)buffer, pool.size()) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)buffer, poolH.size()) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<uint8_t>(size_t{ 64u }, buffer, pool.size() - 64u));
  EXPECT_TRUE(poolH.set<uint8_t>(size_t{ 64u }, buffer, poolH.size() - 64u));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 64u }),(void*)buffer, pool.size() - 64u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 64u }),(void*)buffer, poolH.size() - 64u) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<uint8_t>(size_t{ 0 }, buffer, pool.size() - 64u));
  EXPECT_TRUE(poolH.set<uint8_t>(size_t{ 0 }, buffer, poolH.size() - 64u));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)buffer, pool.size() - 64u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)buffer, poolH.size() - 64u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - 64u),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(pool.size() - 64u),(void*)emptyBuffer, size_t{ 64u }) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<uint8_t>(size_t{ 64u }, buffer, pool.size() - 128u));
  EXPECT_TRUE(poolH.set<uint8_t>(size_t{ 64u }, buffer, poolH.size() - 128u));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 64u }),(void*)buffer, pool.size() - 128u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 64u }),(void*)buffer, poolH.size() - 128u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - 64u),(void*)emptyBuffer, size_t{ 64u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(pool.size() - 64u),(void*)emptyBuffer, size_t{ 64u }) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<uint8_t>(size_t{ 17u }, buffer, pool.size() - 53u));
  EXPECT_TRUE(poolH.set<uint8_t>(size_t{ 17u }, buffer, poolH.size() - 53u));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)emptyBuffer, size_t{ 17u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)emptyBuffer, size_t{ 17u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 17u }),(void*)buffer, pool.size() - 153u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 17u }),(void*)buffer, poolH.size() - 153u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - 36u),(void*)emptyBuffer, size_t{ 36u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(pool.size() - 36u),(void*)emptyBuffer, size_t{ 36u }) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<uint32_t>(size_t{ 24u }, (uint32_t*)buffer, pool.size()/4u - 14u));
  EXPECT_TRUE(poolH.set<uint32_t>(size_t{ 24u }, (uint32_t*)buffer, poolH.size()/4u - 14u));
  EXPECT_TRUE(memcmp((void*)pool.get(),(void*)emptyBuffer, size_t{ 24u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(),(void*)emptyBuffer, size_t{ 24u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 24u }),(void*)buffer, pool.size() - 56u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 24u }),(void*)buffer, poolH.size() - 56u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - 32u),(void*)emptyBuffer, size_t{ 32u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(pool.size() - 32u),(void*)emptyBuffer, size_t{ 32u }) == 0);
}

TEST_F(MemoryPoolTest, setPod) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PodType empty{ 0, 0, 0, 0.0f };
  _PodType pod1{ 1, 2, 3, 4.5f };
  _PodType pod2{ 5, 4, 3, 2.1f };
  _PodType pods[256/sizeof(_PodType)];
  for (int i = static_cast<int>(256u/sizeof(_PodType)); i; --i) {
    pods[i - 1] = _PodType{ i, i/2, (char)('a' + static_cast<char>(i%26)), 42.0f };
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(poolH.set<_PodType>(size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.get<_PodType>(size_t{ 0 })) == empty);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.set<_PodType>(size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType), &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType), &pod2, size_t{ 1u }));
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType)*4u, &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType)*4u, &pod2, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(*(poolH.get<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType))) == pod2);
  EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType))) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*4u), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*4u), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType)*4u)) == pod2);
  EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType)*4u)) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PodType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PodType)*5u) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(size_t{ 7u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.set<_PodType>(size_t{ 7u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType) + 13u, pods, size_t{ 2u }));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType) + 13u, pods, size_t{ 2u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 7u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 7u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(size_t{ 7u })) == pod1);
  EXPECT_TRUE(*(poolH.get<_PodType>(size_t{ 7u })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType) + 13u), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType) + 13u), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType) + 13u)) == pods[0]);
  EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType) + 13u)) == pods[0]);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2u + 13u), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2u + 13u), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType)*2u + 13u)) == pods[1]);
  EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType)*2u + 13u)) == pods[1]);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*3u + 13u), (void*)emptyBuffer, pool.size() - (sizeof(_PodType)*3u + 13u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*3u + 13u), (void*)emptyBuffer, poolH.size() - (sizeof(_PodType)*3u + 13u)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  EXPECT_TRUE(poolH.set<_PodType>(size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  for (size_t i = 256u/sizeof(_PodType); i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.get((i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.get((i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.get<_PodType>((i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.get<_PodType>((i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType), pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType), pods, (256u/sizeof(_PodType)) - 1u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType) + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType) + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType) + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType) + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType), pods, (256u/sizeof(_PodType)) - 2u));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType), pods, (256u/sizeof(_PodType)) - 2u));
  for (size_t i = (256u/sizeof(_PodType)) - 2u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType) + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType) + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType) + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType) + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.set<_PodType>(sizeof(_PodType)/2u, pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.set<_PodType>(sizeof(_PodType)/2u, pods, (256u/sizeof(_PodType)) - 1u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)/2u), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)/2u), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
}

TEST_F(MemoryPoolTest, setPodAt) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PodType empty{ 0, 0, 0, 0.0f };
  _PodType pod1{ 1, 2, 3, 4.5f };
  _PodType pod2{ 5, 4, 3, 2.1f };
  _PodType pods[256/sizeof(_PodType)];
  for (int i = static_cast<int>(256u/sizeof(_PodType)); i; --i) {
    pods[i - 1] = _PodType{ i, i/2, (char)('a' + static_cast<char>(i%26)), 42.0f };
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 0 })) == empty);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 1u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 1u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 4u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 4u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 1u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 1u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 4u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 4u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PodType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PodType)*5u) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 1u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 1u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 3u }, pods, size_t{ 2u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 3u }, pods, size_t{ 2u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 3u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 3u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 3u })) == pods[0]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 3u })) == pods[0]);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 4u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pods[1]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 4u })) == pods[1]);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - (sizeof(_PodType)*5u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*5u), (void*)emptyBuffer, poolH.size() - (sizeof(_PodType)*5u)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  for (size_t i = 256u/sizeof(_PodType); i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i - 1u)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i - 1u)) == pods[i - 1u]);
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 1u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 2u));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 2u));
  for (size_t i = (256u/sizeof(_PodType)) - 2u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);
}

TEST_F(MemoryPoolTest, setPodAtWithOffset) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PodType empty{ 0, 0, 0, 0.0f };
  _PodType pod1{ 1, 2, 3, 4.5f };
  _PodType pod2{ 5, 4, 3, 2.1f };
  _PodType pods[256/sizeof(_PodType)];
  for (int i = static_cast<int>(256u/sizeof(_PodType)); i; --i) {
    pods[i - 1] = _PodType{ i, i/2, (char)('a' + static_cast<char>(i%26)), 42.0f };
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 0 }, size_t{ 0 }, &empty, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 0 })) == empty);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 0 }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 1u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 1u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 4u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 4u }, &pod2, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 2u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 2u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 5u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 5u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*3u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PodType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*6u), (void*)emptyBuffer, poolH.size() - sizeof(_PodType)*6u) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 1u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 1u }, &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 3u }, pods, size_t{ 2u }));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 3u }, pods, size_t{ 2u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 2u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 2u })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 3u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 4u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 3u })) == pods[0]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 4u })) == pods[0]);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 5u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pods[1]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 5u })) == pods[1]);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*3), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - (sizeof(_PodType)*5u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*6u), (void*)emptyBuffer, poolH.size() - (sizeof(_PodType)*6u)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0 }, size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  EXPECT_TRUE(poolH.setAt<_PodType>(size_t{ 0 }, size_t{ 0 }, pods, 256u/sizeof(_PodType)));
  for (size_t i = 256u/sizeof(_PodType); i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i - 1u)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i - 1u)) == pods[i - 1u]);
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0u }, size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 0u }, pods, (256u/sizeof(_PodType)) - 1u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(size_t{ 0u }, size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 2u));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType), size_t{ 0u }, pods, (256u/sizeof(_PodType)) - 2u));
  for (size_t i = (256u/sizeof(_PodType)) - 2u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setAt<_PodType>(sizeof(_PodType)/2u, size_t{ 0u }, pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.setAt<_PodType>(sizeof(_PodType)/2u, size_t{ 1u }, pods, (256u/sizeof(_PodType)) - 2u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.get<_PodType>(sizeof(_PodType)/2u + (i - 1u)*sizeof(_PodType))) == pods[i - 1u]);
  }
  for (size_t i = (256u/sizeof(_PodType)) - 2u; i; --i) {
    EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)/2u + i*sizeof(_PodType)), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(poolH.get<_PodType>(sizeof(_PodType)/2u + i*sizeof(_PodType))) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)*3u/2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)/2u), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)/2u), (void*)emptyBuffer, sizeof(_PodType)/2u) == 0);
}

TEST_F(MemoryPoolTest, iterateSetIn) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PodType empty{ 0, 0, 0, 0.0f };
  _PodType pod1{ 1, 2, 3, 4.5f };
  _PodType pod2{ 5, 4, 3, 2.1f };
  _PodType pods[256/sizeof(_PodType)];
  for (int i = static_cast<int>(256u/sizeof(_PodType)); i; --i) {
    pods[i - 1] = _PodType{ i, i/2, (char)('a' + static_cast<char>(i%26)), 42.0f };
  }

  EXPECT_FALSE(pool.setIn<uint8_t>(pool.end<uint8_t>(), (uint8_t*)&empty, size_t{ 1u }));
  EXPECT_FALSE(poolH.setIn<uint8_t>(poolH.end<uint8_t>(), (uint8_t*)&empty, size_t{ 1u }));
  EXPECT_FALSE(pool.setIn<_PodType>(pool.end<_PodType>(), &empty, size_t{ 1u }));
  EXPECT_FALSE(poolH.setIn<_PodType>(poolH.end<_PodType>(), &empty, size_t{ 1u }));

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.begin<_PodType>(), &empty, size_t{ 1u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.begin<_PodType>(), &empty, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)&empty, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 0 })) == empty);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.begin<_PodType>(), &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.begin<_PodType>(), &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 1u }), &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 1u }), &pod2, size_t{ 1u }));
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 4u }), &pod2, size_t{ 1u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 4u }), &pod2, size_t{ 1u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 0 }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 0 })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 1u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 1u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 4u }), (void*)&pod2, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pod2);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 4u })) == pod2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2u), (void*)emptyBuffer, sizeof(_PodType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PodType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PodType)*5u) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 1u }), &pod1, size_t{ 1u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 1u }), &pod1, size_t{ 1u }));
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 3u }), pods, size_t{ 2u }));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 3u }), pods, size_t{ 2u }));
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 1u }), (void*)&pod1, sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 1u })) == pod1);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 3u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 3u }), (void*)&pods[0], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 3u })) == pods[0]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 3u })) == pods[0]);
  EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(size_t{ 4u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(size_t{ 4u }), (void*)&pods[1], sizeof(_PodType)) == 0);
  EXPECT_TRUE(*(pool.at<_PodType>(size_t{ 4u })) == pods[1]);
  EXPECT_TRUE(*(poolH.at<_PodType>(size_t{ 4u })) == pods[1]);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*2), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*2), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PodType)*5u), (void*)emptyBuffer, pool.size() - (sizeof(_PodType)*5u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PodType)*5u), (void*)emptyBuffer, poolH.size() - (sizeof(_PodType)*5u)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.begin<_PodType>(), pods, 256u/sizeof(_PodType)));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.begin<_PodType>(), pods, 256u/sizeof(_PodType)));
  for (size_t i = 256u/sizeof(_PodType); i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i - 1u), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i - 1u)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i - 1u)) == pods[i - 1u]);
  }

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 1u }), pods, (256u/sizeof(_PodType)) - 1u));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 1u }), pods, (256u/sizeof(_PodType)) - 1u));
  for (size_t i = (256u/sizeof(_PodType)) - 1u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.setIn<_PodType>(pool.at<_PodType>(size_t{ 1u }), pods, (256u/sizeof(_PodType)) - 2u));
  EXPECT_TRUE(poolH.setIn<_PodType>(poolH.at<_PodType>(size_t{ 1u }), pods, (256u/sizeof(_PodType)) - 2u));
  for (size_t i = (256u/sizeof(_PodType)) - 2u; i; --i) {
    EXPECT_TRUE(memcmp((void*)pool.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(memcmp((void*)poolH.at<_PodType>(i), (void*)&pods[i - 1u], sizeof(_PodType)) == 0);
    EXPECT_TRUE(*(pool.at<_PodType>(i)) == pods[i - 1u]);
    EXPECT_TRUE(*(poolH.at<_PodType>(i)) == pods[i - 1u]);
  }
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(pool.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(poolH.size() - sizeof(_PodType)), (void*)emptyBuffer, sizeof(_PodType)) == 0);

  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, false> poolNoCheck;
  EXPECT_TRUE(poolNoCheck.setIn<uint8_t>(poolNoCheck.end<uint8_t>(), (uint8_t*)&empty, size_t{ 1u })); // writes in guard band
}


// -- read/write objects --

TEST_F(MemoryPoolTest, createDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PoolObjectType empty(0);
  _PoolObjectType obj1(42);
  _PoolObjectType obj2(64);

  EXPECT_FALSE(pool.create<_PoolObjectType>(pool.size(), empty));
  EXPECT_FALSE(poolH.create<_PoolObjectType>(poolH.size(), std::move(empty)));
  EXPECT_FALSE(pool.create<_PoolObjectType>(pool.size() - 1u, empty));
  EXPECT_FALSE(poolH.create<_PoolObjectType>(poolH.size() - 1u, std::move(empty)));
  EXPECT_FALSE(pool.destroy<_PoolObjectType>(pool.size()));
  EXPECT_FALSE(poolH.destroy<_PoolObjectType>(poolH.size()));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.create<_PoolObjectType>(size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.create<_PoolObjectType>(size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.create<_PoolObjectType>(sizeof(_PoolObjectType), obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(sizeof(_PoolObjectType), std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.create<_PoolObjectType>(sizeof(_PoolObjectType)*4u, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(sizeof(_PoolObjectType)*4u, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType))) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType))) == obj2);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType)*4u)) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType)*4u)) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType)));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType)*4u));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType)*4u));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.create<_PoolObjectType>(size_t{ 7u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(size_t{ 7u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.create<_PoolObjectType>(sizeof(_PoolObjectType) + 13u, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.create<_PoolObjectType>(sizeof(_PoolObjectType) + 13u, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 7u })) == obj1);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 7u })) == obj1);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType) + 13u)) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType) + 13u)) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u + 13u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*2u + 13u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u + 13u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*2u + 13u)) == 0);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 7u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 7u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType) + 13u));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType) + 13u));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}

TEST_F(MemoryPoolTest, createAtDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PoolObjectType empty(0);
  _PoolObjectType obj1(42);
  _PoolObjectType obj2(64);

  EXPECT_FALSE(pool.createAt<_PoolObjectType>(pool.size(), empty));
  EXPECT_FALSE(poolH.createAt<_PoolObjectType>(poolH.size(), std::move(empty)));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 1u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 1u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 4u }, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 4u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 4u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 4u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 1u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 1u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 3u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 3u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 3u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 3u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}

TEST_F(MemoryPoolTest, createAtWithOffsetDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  _PoolObjectType empty(0);
  _PoolObjectType obj1(42);
  _PoolObjectType obj2(64);

  EXPECT_FALSE(pool.createAt<_PoolObjectType>(size_t{ 0 }, pool.size(), empty));
  EXPECT_FALSE(poolH.createAt<_PoolObjectType>(poolH.size(), size_t{ 0 }, std::move(empty)));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 4u }, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 4u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 2u })) == obj2);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 5u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*3u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*6u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*6u) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 4u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 4u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.createAt<_PoolObjectType>(size_t{ 0 }, size_t{ 3u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.createAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 3u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 2u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*3), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*5u)) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 3u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 3u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}

TEST_F(MemoryPoolTest, emplaceDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  int empty = 0;
  int obj1 = 42;
  int obj2 = 64;

  EXPECT_FALSE(pool.emplace<_PoolObjectType>(pool.size(), empty));
  EXPECT_FALSE(poolH.emplace<_PoolObjectType>(poolH.size(), std::move(empty)));
  EXPECT_FALSE(pool.emplace<_PoolObjectType>(pool.size() - 1u, empty));
  EXPECT_FALSE(poolH.emplace<_PoolObjectType>(pool.size() - 1u, std::move(empty)));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(sizeof(_PoolObjectType), obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(sizeof(_PoolObjectType), std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(sizeof(_PoolObjectType)*4u, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(sizeof(_PoolObjectType)*4u, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType))) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType))) == obj2);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType)*4u)) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType)*4u)) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType)));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType)*4u));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType)*4u));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(size_t{ 7u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(size_t{ 7u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplace<_PoolObjectType>(sizeof(_PoolObjectType) + 13u, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplace<_PoolObjectType>(sizeof(_PoolObjectType) + 13u, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(size_t{ 7u })) == obj1);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(size_t{ 7u })) == obj1);
  EXPECT_TRUE(*(pool.get<_PoolObjectType>(sizeof(_PoolObjectType) + 13u)) == obj2);
  EXPECT_TRUE(*(poolH.get<_PoolObjectType>(sizeof(_PoolObjectType) + 13u)) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, size_t{ 7u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType) + 7u), (void*)emptyBuffer, size_t{ 6u }) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u + 13u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*2u + 13u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u + 13u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*2u + 13u)) == 0);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(size_t{ 7u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(size_t{ 7u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroy<_PoolObjectType>(sizeof(_PoolObjectType) + 13u));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroy<_PoolObjectType>(sizeof(_PoolObjectType) + 13u));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}

TEST_F(MemoryPoolTest, emplaceAtDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  int empty = 0;
  int obj1 = 42;
  int obj2 = 64;

  EXPECT_FALSE(pool.emplaceAt<_PoolObjectType>(pool.size(), empty));
  EXPECT_FALSE(poolH.emplaceAt<_PoolObjectType>(poolH.size(), std::move(empty)));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 1u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 1u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 4u }, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 4u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 4u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 4u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 1u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 1u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 3u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 3u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 3u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 3u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}

TEST_F(MemoryPoolTest, emplaceAtWithOffsetDestroy) {
  MemoryPool<size_t{ 256u }, size_t{ 0 }, MemoryPoolAllocation::onStack, true> pool;
  MemoryPool<size_t{ 256u }, size_t{ 16u }, MemoryPoolAllocation::onHeap, true> poolH;
  uint8_t emptyBuffer[256];
  memset((void*)emptyBuffer, 0, pool.size());
  int empty = 0;
  int obj1 = 42;
  int obj2 = 64;

  EXPECT_FALSE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, pool.size(), empty));
  EXPECT_FALSE(poolH.emplaceAt<_PoolObjectType>(poolH.size(), size_t{ 0 }, std::move(empty)));
  int countBefore = _PoolObjectType::counter;

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, empty));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, std::move(empty)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 0 })) == empty);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 0 }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 4u }, obj2));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 4u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 6, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 0 })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 2u })) == obj2);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 5u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*3u), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, pool.size() - sizeof(_PoolObjectType)*5u) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*6u), (void*)emptyBuffer, poolH.size() - sizeof(_PoolObjectType)*6u) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(countBefore + 5, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 0 }));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 4u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 4u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);

  pool.clear();
  poolH.clear();
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }, obj1));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }, std::move(obj1)));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.emplaceAt<_PoolObjectType>(size_t{ 0 }, size_t{ 3u }, obj2));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.emplaceAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 3u }, std::move(obj2)));
  EXPECT_EQ(countBefore + 4, _PoolObjectType::counter);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 1u })) == obj1);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 2u })) == obj1);
  EXPECT_TRUE(*(pool.at<_PoolObjectType>(size_t{ 3u })) == obj2);
  EXPECT_TRUE(*(poolH.at<_PoolObjectType>(size_t{ 4u })) == obj2);
  EXPECT_TRUE(memcmp((void*)pool.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(size_t{ 0 }), (void*)emptyBuffer, sizeof(_PoolObjectType)*2u) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*2), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*3), (void*)emptyBuffer, sizeof(_PoolObjectType)) == 0);
  EXPECT_TRUE(memcmp((void*)pool.get(sizeof(_PoolObjectType)*4u), (void*)emptyBuffer, pool.size() - (sizeof(_PoolObjectType)*4u)) == 0);
  EXPECT_TRUE(memcmp((void*)poolH.get(sizeof(_PoolObjectType)*5u), (void*)emptyBuffer, poolH.size() - (sizeof(_PoolObjectType)*5u)) == 0);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(countBefore + 3, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 1u }));
  EXPECT_EQ(countBefore + 2, _PoolObjectType::counter);
  EXPECT_TRUE(pool.destroyAt<_PoolObjectType>(size_t{ 0 }, size_t{ 3u }));
  EXPECT_EQ(countBefore + 1, _PoolObjectType::counter);
  EXPECT_TRUE(poolH.destroyAt<_PoolObjectType>(sizeof(_PoolObjectType), size_t{ 3u }));
  EXPECT_EQ(countBefore, _PoolObjectType::counter);
}
