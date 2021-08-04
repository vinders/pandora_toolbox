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
#include <time/rate_factory.h>

using namespace pandora::time;

class RateFactoryTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- rate factory --

TEST_F(RateFactoryTest, smpteRateFactory) {
  EXPECT_EQ(1u, RateFactory::fromSmpteRate(SmpteRate::film_24p).denominator());
  EXPECT_EQ(1001u, RateFactory::fromSmpteRate(SmpteRate::ntsc_24p).denominator());
  EXPECT_EQ(1u, RateFactory::fromSmpteRate(SmpteRate::pal_25p).denominator());
  EXPECT_EQ(1u, RateFactory::fromSmpteRate(SmpteRate::pal_50pi).denominator());
  EXPECT_EQ(1001u, RateFactory::fromSmpteRate(SmpteRate::ntsc_30p).denominator());
  EXPECT_EQ(1001u, RateFactory::fromSmpteRate(SmpteRate::ntsc_60pi).denominator());
  EXPECT_EQ(1u, RateFactory::fromSmpteRate(SmpteRate::ntsc_non_drop_30p).denominator());
  EXPECT_EQ(1u, RateFactory::fromSmpteRate(SmpteRate::ntsc_non_drop_60pi).denominator());
  EXPECT_EQ(Rate(1, 1u), RateFactory::fromSmpteRate((SmpteRate)123456789)); // default
}

TEST_F(RateFactoryTest, audioSampleRateFactory) {
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::note_a).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_8k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_16k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_24k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_32k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_44k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_48k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_96k).denominator());
  EXPECT_EQ(1u, RateFactory::fromAudioSampleRate(AudioSampleRate::audio_192k).denominator());
  EXPECT_EQ(Rate(123456789, 1u), RateFactory::fromAudioSampleRate((AudioSampleRate)123456789)); // default
}
