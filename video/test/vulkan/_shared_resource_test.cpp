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
#if defined(_VIDEO_VULKAN_SUPPORT)
# include <gtest/gtest.h>
# include <memory>
# include <video/vulkan/_private/_shared_resource.h>

  using namespace pandora::video::vulkan;

  class _VulkanSharedResourceTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };

  // -- Helper - test resource --

  void _fakeDestroyFunc(VkDevice, void*, const VkAllocationCallbacks*) {}
  void _fakeDestroyFunc2(VkDevice, void**, const VkAllocationCallbacks*) {}


  // -- SharedResource --

  TEST_F(_VulkanSharedResourceTest, emptyValueContainer) {
    DeviceResourceManager context = std::make_shared<ScopedDeviceContext>();

    ScopedResource<void*> empty;
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.hasValue());
    EXPECT_TRUE(empty.value() == nullptr);
    EXPECT_TRUE(empty.extract() == nullptr);
    ScopedResource<void*> empty2(nullptr, context, _fakeDestroyFunc);
    EXPECT_FALSE(empty2);
    EXPECT_FALSE(empty2.hasValue());
    EXPECT_TRUE(empty2.value() == nullptr);
    EXPECT_TRUE(empty2.extract() == nullptr);

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.hasValue());
    EXPECT_TRUE(empty.value() == nullptr);
    EXPECT_TRUE(empty.extract() == nullptr);

    ScopedResource<void*> empty3(std::move(empty));
    EXPECT_FALSE(empty3);
    EXPECT_FALSE(empty3.hasValue());
    EXPECT_TRUE(empty3.value() == nullptr);
    EXPECT_TRUE(empty3.extract() == nullptr);

    EXPECT_TRUE(empty == empty2);
    EXPECT_TRUE(empty == empty3);
    EXPECT_TRUE(empty2 == empty3);
    EXPECT_FALSE(empty != empty2);
    EXPECT_FALSE(empty != empty3);
    EXPECT_FALSE(empty2 != empty3);
  }

  TEST_F(_VulkanSharedResourceTest, filledValueContainer) {
    DeviceResourceManager context = std::make_shared<ScopedDeviceContext>();

    void* res1((void*)1);
    ScopedResource<void**> val1(&res1, context, _fakeDestroyFunc2);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.value() == &res1);
    val1.release();
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.value() == nullptr);
    val1 = ScopedResource<void**>(&res1, context, _fakeDestroyFunc2);
    EXPECT_EQ(&res1, val1.extract());
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.value() == nullptr);
    val1 = std::move(ScopedResource<void**>(&res1, context, _fakeDestroyFunc2));
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.hasValue());
    EXPECT_TRUE(val1.value() == &res1);

    ScopedResource<void**> val1B(std::move(val1));
    EXPECT_TRUE(val1B);
    EXPECT_TRUE(val1B.hasValue());
    EXPECT_TRUE(val1B.value() == &res1);
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.hasValue());
    EXPECT_TRUE(val1.value() == nullptr);

    val1.release();
    val1B.release();
  }

#endif
