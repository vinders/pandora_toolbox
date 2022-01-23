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
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <memory>
# include <video/d3d11/_private/_shared_resource.h>

  using namespace pandora::video::d3d11;

  class _D3d11SharedResourceTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- Helper - test resource --

  class _D3d11TestResource final {
  public:
    _D3d11TestResource() noexcept = default;
    _D3d11TestResource(int val) : _value(val) { *_counter = 1; }
    _D3d11TestResource(const _D3d11TestResource& rhs) = default;
    _D3d11TestResource& operator=(const _D3d11TestResource&) = default;

    int AddRef() noexcept { *_counter += 1; return *_counter; }
    int Release() noexcept { *_counter -= 1; return *_counter; }
    int count() const noexcept { return *_counter; }
    void count(int counter) const noexcept { *_counter = counter; }

    int value() const noexcept { return _value; }

  private:
    int _value = 0;
    std::shared_ptr<int> _counter = std::make_shared<int>(0);
  };


  // -- SharedResource --

  TEST_F(_D3d11SharedResourceTest, emptyValueContainer) {
    SharedResource<_D3d11TestResource> empty;
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.hasValue());
    EXPECT_TRUE(empty.get() == nullptr);
    EXPECT_TRUE(empty.extract() == nullptr);
    SharedResource<_D3d11TestResource> empty2(nullptr);
    EXPECT_FALSE(empty2);
    EXPECT_FALSE(empty2.hasValue());
    EXPECT_TRUE(empty2.get() == nullptr);
    EXPECT_TRUE(empty2.extract() == nullptr);

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.hasValue());
    EXPECT_TRUE(empty.get() == nullptr);
    EXPECT_TRUE(empty.extract() == nullptr);

    SharedResource<_D3d11TestResource> empty3(std::move(empty));
    EXPECT_FALSE(empty3);
    EXPECT_FALSE(empty3.hasValue());
    EXPECT_TRUE(empty3.get() == nullptr);
    EXPECT_TRUE(empty3.extract() == nullptr);

    EXPECT_TRUE(empty == empty2);
    EXPECT_TRUE(empty == empty3);
    EXPECT_TRUE(empty2 == empty3);
    EXPECT_FALSE(empty != empty2);
    EXPECT_FALSE(empty != empty3);
    EXPECT_FALSE(empty2 != empty3);
  }

  TEST_F(_D3d11SharedResourceTest, filledValueContainer) {
    _D3d11TestResource res1(1);
    EXPECT_EQ((int)1, res1.count());
    SharedResource<_D3d11TestResource> val1(&res1);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res1);
    EXPECT_EQ((int)1, res1.count());
    val1.release();
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.get() == nullptr);
    EXPECT_EQ((int)0, res1.count());
    res1.count(1);
    val1 = &res1;
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res1);
    EXPECT_EQ((int)1, res1.count());
    EXPECT_EQ(&res1, val1.extract());
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.get() == nullptr);
    EXPECT_EQ((int)1, res1.count());
    res1.count(1);
    val1 = std::move(SharedResource<_D3d11TestResource>(&res1));
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res1);
    EXPECT_EQ((int)1, res1.count());

    SharedResource<_D3d11TestResource> val1B(val1);
    EXPECT_TRUE(val1B);
    EXPECT_TRUE(val1B.hasValue());
    EXPECT_TRUE(val1B.get() == &res1);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res1);
    EXPECT_EQ((int)2, res1.count());
    SharedResource<_D3d11TestResource> val1C(std::move(val1));
    EXPECT_TRUE(val1C);
    EXPECT_TRUE(val1C.hasValue());
    EXPECT_TRUE(val1C.get() == &res1);
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.get() == nullptr);
    EXPECT_EQ((int)2, res1.count());
    val1 = val1B;
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res1);
    EXPECT_EQ((int)3, res1.count());

    _D3d11TestResource res2(2);
    EXPECT_EQ((int)1, res2.count());
    *(val1.address()) = &res2;
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.get() == &res2);
    EXPECT_EQ((int)2, res1.count());
    EXPECT_EQ((int)1, res2.count());

    val1.release();
    val1B.release();
    val1C.release();
    EXPECT_EQ((int)0, res1.count());
    EXPECT_EQ((int)0, res2.count());
  }

  // -- SharedResourceArray --

# define __P_TEST_ARRAY_SIZE size_t{8u}

  TEST_F(_D3d11SharedResourceTest, emptyArrayContainer) {
    SharedResourceArray<_D3d11TestResource,__P_TEST_ARRAY_SIZE> values;
    EXPECT_FALSE(values);
    EXPECT_TRUE(values.empty());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_EQ(__P_TEST_ARRAY_SIZE, values.maxSize());
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_FALSE(values.insert(1, SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.replace(0, SharedResource<_D3d11TestResource>{}));
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.insert(0, SharedResource<_D3d11TestResource>{}));
    EXPECT_TRUE(values.replace(0, SharedResource<_D3d11TestResource>{}));
    EXPECT_TRUE(values.replace(0, nullptr));
    EXPECT_FALSE(values.replace(1, SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.replace(1, nullptr));
    EXPECT_TRUE(values);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_EQ(SharedResource<_D3d11TestResource>{}, values.at(0));
    EXPECT_FALSE(values.at(0).hasValue());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.getFrom(0)[0] == nullptr);
    values.erase(0);
    EXPECT_FALSE(values);
    EXPECT_TRUE(values.empty());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.insert(0, nullptr));
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());

    SharedResource<_D3d11TestResource> resCopied1{};

    EXPECT_TRUE(values.append(SharedResource<_D3d11TestResource>{}));
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_FALSE(values.at(0).hasValue());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.append(nullptr));
    EXPECT_EQ((size_t)2, values.size());
    EXPECT_FALSE(values.at(1).hasValue());
    EXPECT_TRUE(values.get()[1] == nullptr);
    values.clear();
    EXPECT_FALSE(values);
    EXPECT_TRUE(values.empty());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(resCopied1));
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_FALSE(values.at(0).hasValue());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.insert(0, resCopied1));
    EXPECT_TRUE(values.replace(0, resCopied1));
    EXPECT_TRUE(values.replace(1, SharedResource<_D3d11TestResource>{}));
    EXPECT_EQ((size_t)2, values.size());
    EXPECT_FALSE(values.at(0).hasValue());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_FALSE(values.at(1).hasValue());
    EXPECT_TRUE(values.get()[1] == nullptr);

    size_t length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.append(SharedResource<_D3d11TestResource>{}));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.insert(0, SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.insert((uint32_t)values.size(), SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.replace((uint32_t)values.size(), SharedResource<_D3d11TestResource>{}));
    EXPECT_EQ(values.maxSize(), values.size());

    values.clear();
    EXPECT_FALSE(values);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(SharedResource<_D3d11TestResource>{}));
    EXPECT_EQ((size_t)1, values.size());
    length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.insert((uint32_t)values.size(), SharedResource<_D3d11TestResource>{}));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(SharedResource<_D3d11TestResource>{}));
    EXPECT_FALSE(values.insert(0, SharedResource<_D3d11TestResource>{}));
    EXPECT_TRUE(values.replace(0, SharedResource<_D3d11TestResource>{}));

    SharedResourceArray<_D3d11TestResource, (size_t)8> moved(std::move(values));
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_EQ(moved.maxSize(), moved.size());
    EXPECT_EQ((size_t)0, values.size());
    values = std::move(moved);
    EXPECT_TRUE(moved.get()[0] == nullptr);
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_EQ((size_t)0, moved.size());

    values.erase((uint32_t)values.size()); // out of range
    EXPECT_EQ(values.maxSize(), values.size());
    values.erase((uint32_t)values.size() - 1u);
    EXPECT_EQ(values.maxSize() - 1u, values.size());
    values.erase(0);
    EXPECT_EQ(values.maxSize() - 2u, values.size());
    values.erase((uint32_t)values.size()); // out of range
    EXPECT_EQ(values.maxSize() - 2u, values.size());
  }

  TEST_F(_D3d11SharedResourceTest, filledArrayContainer) {
    _D3d11TestResource res1(1);
    _D3d11TestResource res2(2);
    _D3d11TestResource res3(3);
    _D3d11TestResource res4(4);
    _D3d11TestResource res5(5);
    EXPECT_EQ((int)1, res1.count());
    EXPECT_EQ((int)1, res2.count());
    EXPECT_EQ((int)1, res3.count());
    EXPECT_EQ((int)1, res4.count());
    EXPECT_EQ((int)1, res5.count());

    SharedResourceArray<_D3d11TestResource, (size_t)8> values;
    EXPECT_TRUE(values.append(SharedResource<_D3d11TestResource>(&res1)));
    ASSERT_EQ((size_t)1, values.size());
    EXPECT_EQ(&res1, values.at(0).extract());
    EXPECT_TRUE(values.append(SharedResource<_D3d11TestResource>(&res2)));
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(&res1, values.at(0).extract());
    EXPECT_EQ(&res2, values.at(1).extract());
    EXPECT_TRUE(values.insert(0, SharedResource<_D3d11TestResource>(&res3)));
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(&res3, values.at(0).extract());
    EXPECT_EQ(&res1, values.at(1).extract());
    EXPECT_EQ(&res2, values.at(2).extract());
    EXPECT_TRUE(values.insert(1, SharedResource<_D3d11TestResource>(&res4)));
    ASSERT_EQ((size_t)4, values.size());
    EXPECT_EQ(&res3, values.at(0).extract());
    EXPECT_EQ(&res4, values.at(1).extract());
    EXPECT_EQ(&res1, values.at(2).extract());
    EXPECT_EQ(&res2, values.at(3).extract());
    EXPECT_TRUE(values.replace(1, SharedResource<_D3d11TestResource>(&res5)));
    ASSERT_EQ((size_t)4, values.size());
    EXPECT_EQ(&res3, values.at(0).extract());
    EXPECT_EQ(&res5, values.at(1).extract());
    EXPECT_EQ(&res1, values.at(2).extract());
    EXPECT_EQ(&res2, values.at(3).extract());
    values.erase(1);
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(&res3, values.at(0).extract());
    EXPECT_EQ(&res1, values.at(1).extract());
    EXPECT_EQ(&res2, values.at(2).extract());
    values.erase(0);
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(&res1, values.at(0).extract());
    EXPECT_EQ(&res2, values.at(1).extract());
    values.erase(1);
    ASSERT_EQ((size_t)1, values.size());
    EXPECT_EQ(&res1, values.at(0).extract());
    values.clear();
    EXPECT_EQ((size_t)0, values.size());
  }

  // -- SharedResourceId --

  TEST_F(_D3d11SharedResourceTest, sharedResourceIdTest) {
    SharedResourceId<1> empty1;
    SharedResourceId<2> empty2;
    SharedResourceId<4> empty4;
    EXPECT_EQ(size_t{ 1u }, empty1.length());
    EXPECT_EQ(size_t{ 2u }, empty2.length());
    EXPECT_EQ(size_t{ 4u }, empty4.length());
    EXPECT_EQ((uint32_t)0, empty1.id()[0]);
    EXPECT_EQ((uint32_t)0, empty2.id()[0]);
    EXPECT_EQ((uint32_t)0, empty2.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4.id()[0]);
    EXPECT_EQ((uint32_t)0, empty4.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4.id()[2]);
    EXPECT_EQ((uint32_t)0, empty4.id()[3]);
    SharedResourceId<1> empty1B;
    SharedResourceId<2> empty2B;
    SharedResourceId<4> empty4B;
    EXPECT_EQ(size_t{ 1u }, empty1B.length());
    EXPECT_EQ(size_t{ 2u }, empty2B.length());
    EXPECT_EQ(size_t{ 4u }, empty4B.length());
    EXPECT_EQ((uint32_t)0, empty1B.id()[0]);
    EXPECT_EQ((uint32_t)0, empty2B.id()[0]);
    EXPECT_EQ((uint32_t)0, empty2B.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4B.id()[0]);
    EXPECT_EQ((uint32_t)0, empty4B.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4B.id()[2]);
    EXPECT_EQ((uint32_t)0, empty4B.id()[3]);

    empty1.id()[0] = 42;
    EXPECT_EQ((uint32_t)42, empty1.id()[0]);
    EXPECT_FALSE(empty1 == empty1B);
    EXPECT_TRUE(empty1 != empty1B);
    EXPECT_FALSE(empty1 < empty1B);
    EXPECT_FALSE(empty1 <= empty1B);
    EXPECT_TRUE(empty1 > empty1B);
    EXPECT_TRUE(empty1 >= empty1B);
    empty1B.id()[0] = 24;
    EXPECT_EQ((uint32_t)24, empty1B.id()[0]);
    EXPECT_FALSE(empty1 == empty1B);
    EXPECT_TRUE(empty1 != empty1B);
    EXPECT_FALSE(empty1 < empty1B);
    EXPECT_FALSE(empty1 <= empty1B);
    EXPECT_TRUE(empty1 > empty1B);
    EXPECT_TRUE(empty1 >= empty1B);
    empty1B.id()[0] = 42;
    EXPECT_EQ((uint32_t)42, empty1B.id()[0]);
    EXPECT_TRUE(empty1 == empty1B);
    EXPECT_FALSE(empty1 != empty1B);
    EXPECT_FALSE(empty1 < empty1B);
    EXPECT_TRUE(empty1 <= empty1B);
    EXPECT_FALSE(empty1 > empty1B);
    EXPECT_TRUE(empty1 >= empty1B);

    empty2.id()[1] = 2;
    EXPECT_EQ((uint32_t)0, empty2.id()[0]);
    EXPECT_EQ((uint32_t)2, empty2.id()[1]);
    EXPECT_FALSE(empty2 == empty2B);
    EXPECT_TRUE(empty2 != empty2B);
    EXPECT_FALSE(empty2 < empty2B);
    EXPECT_FALSE(empty2 <= empty2B);
    EXPECT_TRUE(empty2 > empty2B);
    EXPECT_TRUE(empty2 >= empty2B);
    empty2.id()[0] = 4;
    EXPECT_EQ((uint32_t)4, empty2.id()[0]);
    EXPECT_EQ((uint32_t)2, empty2.id()[1]);
    EXPECT_FALSE(empty2 == empty2B);
    EXPECT_TRUE(empty2 != empty2B);
    EXPECT_FALSE(empty2 < empty2B);
    EXPECT_FALSE(empty2 <= empty2B);
    EXPECT_TRUE(empty2 > empty2B);
    EXPECT_TRUE(empty2 >= empty2B);
    empty2B.id()[1] = 2;
    EXPECT_EQ((uint32_t)0, empty2B.id()[0]);
    EXPECT_EQ((uint32_t)2, empty2B.id()[1]);
    EXPECT_FALSE(empty2 == empty2B);
    EXPECT_TRUE(empty2 != empty2B);
    EXPECT_FALSE(empty2 < empty2B);
    EXPECT_FALSE(empty2 <= empty2B);
    EXPECT_TRUE(empty2 > empty2B);
    EXPECT_TRUE(empty2 >= empty2B);
    empty2B.id()[0] = 4;
    EXPECT_EQ((uint32_t)4, empty2B.id()[0]);
    EXPECT_EQ((uint32_t)2, empty2B.id()[1]);
    EXPECT_TRUE(empty2 == empty2B);
    EXPECT_FALSE(empty2 != empty2B);
    EXPECT_FALSE(empty2 < empty2B);
    EXPECT_TRUE(empty2 <= empty2B);
    EXPECT_FALSE(empty2 > empty2B);
    EXPECT_TRUE(empty2 >= empty2B);

    empty4.id()[1] = 2;
    EXPECT_EQ((uint32_t)0, empty4.id()[0]);
    EXPECT_EQ((uint32_t)2, empty4.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4.id()[2]);
    EXPECT_EQ((uint32_t)0, empty4.id()[3]);
    EXPECT_FALSE(empty4 == empty4B);
    EXPECT_TRUE(empty4 != empty4B);
    EXPECT_FALSE(empty4 < empty4B);
    EXPECT_FALSE(empty4 <= empty4B);
    EXPECT_TRUE(empty4 > empty4B);
    EXPECT_TRUE(empty4 >= empty4B);
    empty4.id()[0] = 1;
    EXPECT_EQ((uint32_t)1, empty4.id()[0]);
    EXPECT_EQ((uint32_t)2, empty4.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4.id()[2]);
    EXPECT_EQ((uint32_t)0, empty4.id()[3]);
    EXPECT_FALSE(empty4 == empty4B);
    EXPECT_TRUE(empty4 != empty4B);
    EXPECT_FALSE(empty4 < empty4B);
    EXPECT_FALSE(empty4 <= empty4B);
    EXPECT_TRUE(empty4 > empty4B);
    EXPECT_TRUE(empty4 >= empty4B);
    empty4.id()[3] = 4;
    EXPECT_EQ((uint32_t)1, empty4.id()[0]);
    EXPECT_EQ((uint32_t)2, empty4.id()[1]);
    EXPECT_EQ((uint32_t)0, empty4.id()[2]);
    EXPECT_EQ((uint32_t)4, empty4.id()[3]);
    EXPECT_FALSE(empty4 == empty4B);
    EXPECT_TRUE(empty4 != empty4B);
    EXPECT_FALSE(empty4 < empty4B);
    EXPECT_FALSE(empty4 <= empty4B);
    EXPECT_TRUE(empty4 > empty4B);
    EXPECT_TRUE(empty4 >= empty4B);
    empty4.id()[2] = 3;
    EXPECT_EQ((uint32_t)1, empty4.id()[0]);
    EXPECT_EQ((uint32_t)2, empty4.id()[1]);
    EXPECT_EQ((uint32_t)3, empty4.id()[2]);
    EXPECT_EQ((uint32_t)4, empty4.id()[3]);
    EXPECT_FALSE(empty4 == empty4B);
    EXPECT_TRUE(empty4 != empty4B);
    EXPECT_FALSE(empty4 < empty4B);
    EXPECT_FALSE(empty4 <= empty4B);
    EXPECT_TRUE(empty4 > empty4B);
    EXPECT_TRUE(empty4 >= empty4B);
    empty4B.id()[0] = 1;
    empty4B.id()[1] = 2;
    empty4B.id()[2] = 3;
    empty4B.id()[3] = 4;
    EXPECT_EQ((uint32_t)1, empty4B.id()[0]);
    EXPECT_EQ((uint32_t)2, empty4B.id()[1]);
    EXPECT_EQ((uint32_t)3, empty4B.id()[2]);
    EXPECT_EQ((uint32_t)4, empty4B.id()[3]);
    EXPECT_TRUE(empty4 == empty4B);
    EXPECT_FALSE(empty4 != empty4B);
    EXPECT_FALSE(empty4 < empty4B);
    EXPECT_TRUE(empty4 <= empty4B);
    EXPECT_FALSE(empty4 > empty4B);
    EXPECT_TRUE(empty4 >= empty4B);

    SharedResourceId<1> copy1(empty1);
    SharedResourceId<2> copy2(empty2);
    SharedResourceId<4> copy4(empty4);
    EXPECT_EQ((uint32_t)42, copy1.id()[0]);
    EXPECT_TRUE(empty1 == copy1);
    EXPECT_FALSE(empty1 != copy1);
    EXPECT_EQ((uint32_t)4, copy2.id()[0]);
    EXPECT_EQ((uint32_t)2, copy2.id()[1]);
    EXPECT_TRUE(empty2 == copy2);
    EXPECT_FALSE(empty2 != copy2);
    EXPECT_EQ((uint32_t)1, copy4.id()[0]);
    EXPECT_EQ((uint32_t)2, copy4.id()[1]);
    EXPECT_EQ((uint32_t)3, copy4.id()[2]);
    EXPECT_EQ((uint32_t)4, copy4.id()[3]);
    EXPECT_TRUE(empty4 == copy4);
    EXPECT_FALSE(empty4 != copy4);

    SharedResourceId<1> move1(std::move(copy1));
    SharedResourceId<2> move2(std::move(copy2));
    SharedResourceId<4> move4(std::move(copy4));
    EXPECT_EQ((uint32_t)42, move1.id()[0]);
    EXPECT_TRUE(empty1 == move1);
    EXPECT_FALSE(empty1 != move1);
    EXPECT_EQ((uint32_t)4, move2.id()[0]);
    EXPECT_EQ((uint32_t)2, move2.id()[1]);
    EXPECT_TRUE(empty2 == move2);
    EXPECT_FALSE(empty2 != move2);
    EXPECT_EQ((uint32_t)1, move4.id()[0]);
    EXPECT_EQ((uint32_t)2, move4.id()[1]);
    EXPECT_EQ((uint32_t)3, move4.id()[2]);
    EXPECT_EQ((uint32_t)4, move4.id()[3]);
    EXPECT_TRUE(empty4 == move4);
    EXPECT_FALSE(empty4 != move4);
  }

#endif
