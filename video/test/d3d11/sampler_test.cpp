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
# include <video/d3d11/renderer.h>
# include <video/d3d11/sampler.h>

  using namespace pandora::video::d3d11;

  class D3d11SamplerTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create sampler state objects --

  TEST_F(D3d11SamplerTest, createSamplerStateParamsTest) {
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

  TEST_F(D3d11SamplerTest, samplerStateParamsAccessorsTest) {
    ColorFloat border[4]{ 1.f, 2.f, 3.f, 4.f };
    TextureWrap wrap[3]{ TextureWrap::repeatMirror, TextureWrap::mirrorOnce, TextureWrap::clampToEdge };

    SamplerParams filter;
    filter.setFilter(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest).textureWrap(wrap).borderColor(border).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, filter.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter.descriptor().AddressW);
    EXPECT_EQ((FLOAT)1.f, filter.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)2.f, filter.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)3.f, filter.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)4.f, filter.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)3.f, filter.descriptor().MipLODBias);
    SamplerParams filter2;
    filter2.setFilter(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, true, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, filter2.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter2.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter2.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter2.descriptor().AddressW);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter2.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter2.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter2.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().MipLODBias);

    SamplerParams filter3;
    filter3.setAnisotropicFilter(1u).textureWrap(wrap).borderColor(border).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(D3D11_FILTER_ANISOTROPIC, filter3.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter3.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter3.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter3.descriptor().AddressW);
    EXPECT_EQ((FLOAT)1.f, filter3.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)2.f, filter3.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)3.f, filter3.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)4.f, filter3.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter3.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter3.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter3.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)3.f, filter3.descriptor().MipLODBias);
    SamplerParams filter4;
    filter4.setAnisotropicFilter(8u, true, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_ANISOTROPIC, filter4.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter4.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter4.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter4.descriptor().AddressW);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)8u, filter4.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter4.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter4.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().MipLODBias);

    SamplerParams filter5(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest, wrap, 1.f, 2.f);
    EXPECT_EQ(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, filter5.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter5.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter5.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter5.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter5.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter5.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter5.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter5.descriptor().MipLODBias);
    SamplerParams filter6(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, wrap, 0.f, 1.f, true, StencilCompare::greaterEqual);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, filter6.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter6.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter6.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter6.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter6.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter6.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter6.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter6.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter6.descriptor().MipLODBias);

    SamplerParams filter7(1u, wrap, 1.f, 2.f);
    EXPECT_EQ(D3D11_FILTER_ANISOTROPIC, filter7.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter7.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter7.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter7.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter7.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter7.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter7.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter7.descriptor().MipLODBias);
    SamplerParams filter8(8u, wrap, 0.f, 1.f, true, StencilCompare::greaterEqual);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_ANISOTROPIC, filter8.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter8.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter8.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter8.descriptor().AddressW);
    EXPECT_EQ((UINT)8u, filter8.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter8.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter8.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter8.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter8.descriptor().MipLODBias);
  }

#endif
