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
