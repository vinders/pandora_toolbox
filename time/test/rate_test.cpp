#include <gtest/gtest.h>
#include <time/rate.h>

using namespace pandora::time;

class RateTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- constructors / accessors --

TEST_F(RateTest, defaultCtorAccessors) {
  Rate rate;
  EXPECT_EQ(1, rate.numerator());
  EXPECT_EQ(1u, rate.denominator());

  const Rate rate2;
  EXPECT_EQ(1, rate2.numerator());
  EXPECT_EQ(1u, rate2.denominator());
}

TEST_F(RateTest, initCtorAccessors) {
  Rate rate(155, 227u);
  EXPECT_EQ(155, rate.numerator());
  EXPECT_EQ(227u, rate.denominator());

  const Rate rate2(30, 1001u);
  EXPECT_EQ(30, rate2.numerator());
  EXPECT_EQ(1001u, rate2.denominator());
}

TEST_F(RateTest, copyMoveCtorAccessors) {
  Rate rate(155, 227u);
  EXPECT_EQ(155, rate.numerator());
  EXPECT_EQ(227u, rate.denominator());

  Rate rateCopy(rate);
  EXPECT_EQ(155, rateCopy.numerator());
  EXPECT_EQ(227u, rateCopy.denominator());

  Rate rateMove(std::move(rateCopy));
  EXPECT_EQ(155, rateMove.numerator());
  EXPECT_EQ(227u, rateMove.denominator());

  Rate rateAssigned(1, 1u);
  EXPECT_EQ(1, rateAssigned.numerator());
  EXPECT_EQ(1u, rateAssigned.denominator());
  rateAssigned = rate;
  EXPECT_EQ(155, rateAssigned.numerator());
  EXPECT_EQ(227u, rateAssigned.denominator());
  rateAssigned = std::move(rateMove);
  EXPECT_EQ(155, rateAssigned.numerator());
  EXPECT_EQ(227u, rateAssigned.denominator());
}

TEST_F(RateTest, setters) {
  Rate rate;
  EXPECT_EQ(1, rate.numerator());
  EXPECT_EQ(1u, rate.denominator());

  rate.numerator(30);
  rate.denominator(1001u);
  EXPECT_EQ(30, rate.numerator());
  EXPECT_EQ(1001u, rate.denominator());
}

// -- comparisons / operations --

TEST_F(RateTest, equalityComparisons) {
  Rate defaultRate1;
  Rate defaultRate2;
  Rate initRate1(155, 227u);
  Rate initRate2(155,227u);
  EXPECT_TRUE(defaultRate1 == defaultRate2);
  EXPECT_TRUE(initRate1 == initRate2);
  EXPECT_FALSE(defaultRate1 == initRate2);
  EXPECT_FALSE(initRate1 == defaultRate2);
  EXPECT_FALSE(defaultRate2 == initRate1);
  EXPECT_FALSE(initRate2 == defaultRate1);
}

// -- formatters --

TEST_F(RateTest, toString) {
  Rate defaultRate;
  Rate initRate(155, 227u);
  EXPECT_EQ(std::string("1:1"), defaultRate.toString());
  EXPECT_EQ(std::string("1/1"), defaultRate.toString('/'));
  EXPECT_EQ(std::string("155:227"), initRate.toString());
  EXPECT_EQ(std::string("155/227"), initRate.toString('/'));
}

TEST_F(RateTest, computations) {
  Rate rate(64, 4u);
  EXPECT_TRUE(rate.compute() >= 15.999999999 && rate.compute() <= 16.000000001);
  EXPECT_TRUE(rate.toPercent() >= 1599.9999999 && rate.toPercent() <= 1600.0000001);
  EXPECT_EQ(1600, rate.toFloorPercent());
  EXPECT_EQ(1600, rate.toRoundPercent());

  rate = Rate(17, 200u);
  EXPECT_TRUE(rate.compute() >= 0.084999999999 && rate.compute() <= 0.085000000001);
  EXPECT_TRUE(rate.toPercent() >= 8.4999999999 && rate.toPercent() <= 8.5000000001);
  EXPECT_EQ(8, rate.toFloorPercent());
  EXPECT_EQ(9, rate.toRoundPercent());

  rate = Rate(-17, 200u);
  EXPECT_TRUE(rate.compute() <= -0.084999999999 && rate.compute() >= -0.085000000001);
  EXPECT_TRUE(rate.toPercent() <= -8.4999999999 && rate.toPercent() >= -8.5000000001);
  EXPECT_EQ(-8, rate.toFloorPercent());
  EXPECT_EQ(-9, rate.toRoundPercent());
}
