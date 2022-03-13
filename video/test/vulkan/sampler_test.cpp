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
# include <vector>
# include <video/vulkan/renderer.h>
# include <video/vulkan/sampler.h>

  using namespace pandora::video::vulkan;

  class VulkanSamplerTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create sampler state objects --

  TEST_F(VulkanSamplerTest, vkSreateSamplerStateParamsTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);

    Sampler::Builder builder(renderer.resourceManager());

    std::vector<Sampler> samplers;
    TextureWrap addrModes[3] { 
      TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat 
    };
    
    // base samplers
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,0.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,1.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 1.f,8.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 8.f,8.f)));
    samplers.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear,
                                                              addrModes, 0.f,4.f)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,0.f)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::linear,
                                                              addrModes, 1.f,1.f)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 0.f,1.f)));
    samplers.clear();
    
    // comparison samplers
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,0.f, true, StencilCompare::always)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,1.f, true, StencilCompare::greater)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 1.f,8.f, true, StencilCompare::equal)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 8.f,8.f, true, StencilCompare::notEqual)));
    samplers.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear,
                                                              addrModes, 0.f,4.f, true, StencilCompare::never)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                              addrModes, 0.f,0.f, true, StencilCompare::less)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::linear,
                                                              addrModes, 1.f,1.f, true, StencilCompare::lessEqual)));
    samplers.emplace_back(builder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                              addrModes, 0.f,1.f, true, StencilCompare::always)));
    samplers.clear();
    
    // anisotropic samplers
    samplers.emplace_back(builder.createSampler(SamplerParams(1u, addrModes, 0.f,0.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    samplers.emplace_back(builder.createSampler(SamplerParams(2u, addrModes, 0.f,1.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    samplers.emplace_back(builder.createSampler(SamplerParams(4u, addrModes, 1.f,8.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(8u, addrModes, 8.f,8.f)));
    samplers.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(renderer.maxSamplerAnisotropy(), addrModes, 0.f,4.f)));
    samplers.emplace_back(builder.createSampler(SamplerParams(renderer.maxSamplerAnisotropy()/2u, addrModes, 0.f,0.f)));
    samplers.clear();
    
    // comparison anisotropic samplers
    samplers.emplace_back(builder.createSampler(SamplerParams(1u, addrModes, 0.f,0.f, true, StencilCompare::always)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    samplers.emplace_back(builder.createSampler(SamplerParams(2u, addrModes, 0.f,1.f, true, StencilCompare::never)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    samplers.emplace_back(builder.createSampler(SamplerParams(4u, addrModes, 1.f,8.f, true, StencilCompare::equal)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(8u, addrModes, 8.f,8.f, true, StencilCompare::notEqual)));
    samplers.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    samplers.emplace_back(builder.createSampler(SamplerParams(renderer.maxSamplerAnisotropy(), addrModes, 0.f,4.f, true, StencilCompare::less)));
    samplers.emplace_back(builder.createSampler(SamplerParams(renderer.maxSamplerAnisotropy()/2u, addrModes, 0.f,0.f, true, StencilCompare::greater)));
    samplers.clear();
  }

  TEST_F(VulkanSamplerTest, vkSamplerStateParamsAccessorsTest) {
    ColorFloat border[4]{ 1.f, 2.f, 3.f, 4.f };
    ColorInt borderInt[4]{ 1, 2, 3, 4 };
    TextureWrap wrap[3]{ TextureWrap::repeatMirror, TextureWrap::mirrorOnce, TextureWrap::clampToEdge };

    SamplerParams filter;
    filter.setFilter(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest).textureWrap(wrap).borderColor(border).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(VK_FILTER_NEAREST, filter.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_NEAREST, filter.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_FALSE, filter.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter.descriptor().addressModeW);
    ASSERT_TRUE(filter.descriptor().pNext != nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_CUSTOM_EXT, filter.descriptor().borderColor);
    float* customColor = ((VkSamplerCustomBorderColorCreateInfoEXT*)filter.descriptor().pNext)->customBorderColor.float32;
    EXPECT_EQ(1.f, customColor[0]);
    EXPECT_EQ(2.f, customColor[1]);
    EXPECT_EQ(3.f, customColor[2]);
    EXPECT_EQ(4.f, customColor[3]);
    EXPECT_EQ((VkBool32)VK_FALSE, filter.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, filter.descriptor().compareOp);
    EXPECT_EQ(1.f, filter.descriptor().minLod);
    EXPECT_EQ(2.f, filter.descriptor().maxLod);
    EXPECT_EQ(3.f, filter.descriptor().mipLodBias);
    SamplerParams filter2;
    filter2.setFilter(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, true, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(VK_FILTER_LINEAR, filter2.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_NEAREST, filter2.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter2.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_FALSE, filter2.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter2.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter2.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter2.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter2.descriptor().addressModeW);
    EXPECT_TRUE(filter2.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter2.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_TRUE, filter2.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, filter2.descriptor().compareOp);
    EXPECT_EQ(0.f, filter2.descriptor().minLod);
    EXPECT_EQ(1.f, filter2.descriptor().maxLod);
    EXPECT_EQ(0.f, filter2.descriptor().mipLodBias);

    SamplerParams filter3;
    filter3.setAnisotropicFilter(1u).textureWrap(wrap).borderColor(borderInt).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(VK_FILTER_LINEAR, filter3.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter3.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter3.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_TRUE, filter3.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter3.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter3.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter3.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter3.descriptor().addressModeW);
    ASSERT_TRUE(filter3.descriptor().pNext != nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_INT_CUSTOM_EXT, filter3.descriptor().borderColor);
    int32_t* customColorInt = ((VkSamplerCustomBorderColorCreateInfoEXT*)filter3.descriptor().pNext)->customBorderColor.int32;
    EXPECT_EQ((int32_t)1, customColorInt[0]);
    EXPECT_EQ((int32_t)2, customColorInt[1]);
    EXPECT_EQ((int32_t)3, customColorInt[2]);
    EXPECT_EQ((int32_t)4, customColorInt[3]);
    EXPECT_EQ((VkBool32)VK_FALSE, filter3.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, filter3.descriptor().compareOp);
    EXPECT_EQ(1.f, filter3.descriptor().minLod);
    EXPECT_EQ(2.f, filter3.descriptor().maxLod);
    EXPECT_EQ(3.f, filter3.descriptor().mipLodBias);
    SamplerParams filter4;
    filter4.setAnisotropicFilter(8u, true, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(VK_FILTER_LINEAR, filter4.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter4.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter4.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_TRUE, filter4.descriptor().anisotropyEnable);
    EXPECT_EQ(8.f, filter4.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter4.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter4.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter4.descriptor().addressModeW);
    EXPECT_TRUE(filter4.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter4.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_TRUE, filter2.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, filter2.descriptor().compareOp);
    EXPECT_EQ(0.f, filter4.descriptor().minLod);
    EXPECT_EQ(1.f, filter4.descriptor().maxLod);
    EXPECT_EQ(0.f, filter4.descriptor().mipLodBias);

    SamplerParams filter5(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest, wrap, 1.f, 2.f);
    EXPECT_EQ(VK_FILTER_NEAREST, filter5.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter5.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_NEAREST, filter5.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_FALSE, filter5.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter5.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter5.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter5.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter5.descriptor().addressModeW);
    EXPECT_TRUE(filter5.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter5.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_FALSE, filter5.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, filter5.descriptor().compareOp);
    EXPECT_EQ(1.f, filter5.descriptor().minLod);
    EXPECT_EQ(2.f, filter5.descriptor().maxLod);
    EXPECT_EQ(0.f, filter5.descriptor().mipLodBias);
    SamplerParams filter6(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, wrap, 0.f, 1.f, true, StencilCompare::greaterEqual);
    EXPECT_EQ(VK_FILTER_LINEAR, filter6.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_NEAREST, filter6.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter6.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_FALSE, filter6.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter6.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter6.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter6.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter6.descriptor().addressModeW);
    EXPECT_TRUE(filter6.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter6.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_TRUE, filter2.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, filter2.descriptor().compareOp);
    EXPECT_EQ(0.f, filter6.descriptor().minLod);
    EXPECT_EQ(1.f, filter6.descriptor().maxLod);
    EXPECT_EQ(0.f, filter6.descriptor().mipLodBias);

    SamplerParams filter7(1u, wrap, 1.f, 2.f);
    EXPECT_EQ(VK_FILTER_LINEAR, filter7.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter7.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter7.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_TRUE, filter7.descriptor().anisotropyEnable);
    EXPECT_EQ(1.f, filter7.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter7.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter7.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter7.descriptor().addressModeW);
    EXPECT_TRUE(filter7.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter7.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_FALSE, filter7.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, filter7.descriptor().compareOp);
    EXPECT_EQ(1.f, filter7.descriptor().minLod);
    EXPECT_EQ(2.f, filter7.descriptor().maxLod);
    EXPECT_EQ(0.f, filter7.descriptor().mipLodBias);
    SamplerParams filter8(8u, wrap, 0.f, 1.f, true, StencilCompare::greaterEqual);
    EXPECT_EQ(VK_FILTER_LINEAR, filter8.descriptor().minFilter);
    EXPECT_EQ(VK_FILTER_LINEAR, filter8.descriptor().magFilter);
    EXPECT_EQ(VK_SAMPLER_MIPMAP_MODE_LINEAR, filter8.descriptor().mipmapMode);
    EXPECT_EQ((VkBool32)VK_TRUE, filter8.descriptor().anisotropyEnable);
    EXPECT_EQ(8.f, filter8.descriptor().maxAnisotropy);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, filter8.descriptor().addressModeU);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, filter8.descriptor().addressModeV);
    EXPECT_EQ(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, filter8.descriptor().addressModeW);
    EXPECT_TRUE(filter8.descriptor().pNext == nullptr);
    EXPECT_EQ(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, filter8.descriptor().borderColor);
    EXPECT_EQ((VkBool32)VK_TRUE, filter2.descriptor().compareEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, filter2.descriptor().compareOp);
    EXPECT_EQ(0.f, filter8.descriptor().minLod);
    EXPECT_EQ(1.f, filter8.descriptor().maxLod);
    EXPECT_EQ(0.f, filter8.descriptor().mipLodBias);
  }

#endif
