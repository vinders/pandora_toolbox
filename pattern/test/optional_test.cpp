#include <gtest/gtest.h>
#include <pattern/optional.h>

using namespace pandora::pattern;

class OptionalTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- mocks --

#define __P_GENERATE_MOCK(name, copyable, movable) \
        template <typename T, T _Empty = (T)0> class name { \
        public: \
          name() = default; \
          name(const T& value) : _value(value) {} \
          name(const name &) = copyable ; \
          name & operator=(const name &) = copyable ; \
          name(name &&) = movable ; \
          name & operator=(name &&) = movable ; \
          bool operator==(const name& rhs) const noexcept { return _value == rhs._value; } \
          bool operator!=(const name& rhs) const noexcept { return _value != rhs._value; } \
          bool operator<(const name& rhs) const noexcept { return _value < rhs._value; } \
          bool operator<=(const name& rhs) const noexcept { return _value <= rhs._value; } \
          bool operator>(const name& rhs) const noexcept { return _value > rhs._value; } \
          bool operator>=(const name& rhs) const noexcept { return _value >= rhs._value; } \
          const T& value() const noexcept { return _value; } \
        private: \
          T _value = _Empty; }

using BaseType = int;
__P_GENERATE_MOCK(Copyable, default, delete);
__P_GENERATE_MOCK(Movable, delete, default);
__P_GENERATE_MOCK(CopyMovable, default, default);


// -- test helpers --

template <typename T>
void _testEmpty() {
  Optional<T> opt;
  EXPECT_FALSE(opt);
  EXPECT_FALSE(opt.hasValue());
  EXPECT_TRUE(opt.get() == nullptr);
  EXPECT_TRUE(opt == nullptr);
  EXPECT_FALSE(opt != nullptr);

  Optional<T> rhs(nullptr);
  EXPECT_FALSE(rhs.hasValue());
  EXPECT_TRUE(opt == rhs);
  EXPECT_FALSE(opt != rhs);
  EXPECT_FALSE(opt < rhs);
  EXPECT_TRUE(opt <= rhs);
  EXPECT_FALSE(opt > rhs);
  EXPECT_TRUE(opt >= rhs);

  opt = nullptr;
  EXPECT_FALSE(opt.hasValue());
  opt.reset();
  EXPECT_FALSE(opt.hasValue());
}

template <typename T>
void _testConstEmpty() {
  const Optional<T> opt(nullptr);
  EXPECT_FALSE(opt);
  EXPECT_FALSE(opt.hasValue());
  EXPECT_TRUE(opt.get() == nullptr);
  EXPECT_TRUE(opt == nullptr);
  EXPECT_FALSE(opt != nullptr);

  const Optional<T> rhs(nullptr);
  EXPECT_FALSE(rhs.hasValue());
  EXPECT_TRUE(opt == rhs);
  EXPECT_FALSE(opt != rhs);
  EXPECT_FALSE(opt < rhs);
  EXPECT_TRUE(opt <= rhs);
  EXPECT_FALSE(opt > rhs);
  EXPECT_TRUE(opt >= rhs);
}

template <typename T>
void _testInit(const T& value, const T& greater) {
  Optional<T> opt(value);
  EXPECT_TRUE(opt);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.get() != nullptr);
  EXPECT_TRUE(*(opt.get()) == value);
  EXPECT_TRUE(*opt == value);
  EXPECT_TRUE(opt.value() == value);
  EXPECT_FALSE(opt == nullptr);
  EXPECT_TRUE(opt != nullptr);
  const Optional<T>& ref = opt;
  EXPECT_TRUE(ref.hasValue());
  EXPECT_TRUE(ref.get() != nullptr);
  EXPECT_TRUE(*(ref.get()) == value);
  EXPECT_TRUE(*ref == value);
  EXPECT_TRUE(ref.value() == value);
  EXPECT_FALSE(ref == nullptr);
  EXPECT_TRUE(ref != nullptr);

  Optional<T> rhs(value);
  Optional<T> rhsGreater(greater);
  Optional<T> rhsEmpty(nullptr);
  EXPECT_TRUE(rhsGreater.hasValue());
  EXPECT_TRUE(rhsGreater.hasValue());
  EXPECT_FALSE(rhsEmpty.hasValue());
  EXPECT_TRUE(opt == rhs);
  EXPECT_FALSE(opt != rhs);
  EXPECT_FALSE(opt < rhs);
  EXPECT_TRUE(opt <= rhs);
  EXPECT_FALSE(opt > rhs);
  EXPECT_TRUE(opt >= rhs);
  EXPECT_FALSE(opt == rhsGreater);
  EXPECT_TRUE(opt != rhsGreater);
  EXPECT_TRUE(opt < rhsGreater);
  EXPECT_TRUE(opt <= rhsGreater);
  EXPECT_FALSE(opt > rhsGreater);
  EXPECT_FALSE(opt >= rhsGreater);
  EXPECT_FALSE(rhsGreater == opt);
  EXPECT_TRUE(rhsGreater != opt);
  EXPECT_FALSE(rhsGreater < opt);
  EXPECT_FALSE(rhsGreater <= opt);
  EXPECT_TRUE(rhsGreater > opt);
  EXPECT_TRUE(rhsGreater >= opt);
  EXPECT_FALSE(opt == rhsEmpty);
  EXPECT_TRUE(opt != rhsEmpty);
  EXPECT_FALSE(opt < rhsEmpty);
  EXPECT_FALSE(opt <= rhsEmpty);
  EXPECT_TRUE(opt > rhsEmpty);
  EXPECT_TRUE(opt >= rhsEmpty);
  EXPECT_FALSE(rhsEmpty == opt);
  EXPECT_TRUE(rhsEmpty != opt);
  EXPECT_TRUE(rhsEmpty < opt);
  EXPECT_TRUE(rhsEmpty <= opt);
  EXPECT_FALSE(rhsEmpty > opt);
  EXPECT_FALSE(rhsEmpty >= opt);

  opt = nullptr;
  EXPECT_FALSE(opt.hasValue());
  opt = value;
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == value);
  opt.reset();
  EXPECT_FALSE(opt.hasValue());
  opt.assign(value);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == value);
  opt.emplace(8);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == T{8});
}

template <typename T>
void _testInitMove(int value, int greater) {
  Optional<T> opt(std::move(T(value)));
  EXPECT_TRUE(opt);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.get() != nullptr);
  EXPECT_TRUE(*(opt.get()) == T(value));
  EXPECT_TRUE(*opt == T(value));
  EXPECT_TRUE(opt.value() == T(value));
  EXPECT_FALSE(opt == nullptr);
  EXPECT_TRUE(opt != nullptr);

  Optional<T> rhs(std::move(T(value)));
  Optional<T> rhsGreater(std::move(T(greater)));
  Optional<T> rhsEmpty(nullptr);
  EXPECT_TRUE(rhsGreater.hasValue());
  EXPECT_TRUE(rhsGreater.hasValue());
  EXPECT_FALSE(rhsEmpty.hasValue());
  EXPECT_TRUE(opt == rhs);
  EXPECT_FALSE(opt != rhs);
  EXPECT_FALSE(opt < rhs);
  EXPECT_TRUE(opt <= rhs);
  EXPECT_FALSE(opt > rhs);
  EXPECT_TRUE(opt >= rhs);
  EXPECT_FALSE(opt == rhsGreater);
  EXPECT_TRUE(opt != rhsGreater);
  EXPECT_TRUE(opt < rhsGreater);
  EXPECT_TRUE(opt <= rhsGreater);
  EXPECT_FALSE(opt > rhsGreater);
  EXPECT_FALSE(opt >= rhsGreater);
  EXPECT_FALSE(rhsGreater == opt);
  EXPECT_TRUE(rhsGreater != opt);
  EXPECT_FALSE(rhsGreater < opt);
  EXPECT_FALSE(rhsGreater <= opt);
  EXPECT_TRUE(rhsGreater > opt);
  EXPECT_TRUE(rhsGreater >= opt);
  EXPECT_FALSE(opt == rhsEmpty);
  EXPECT_TRUE(opt != rhsEmpty);
  EXPECT_FALSE(opt < rhsEmpty);
  EXPECT_FALSE(opt <= rhsEmpty);
  EXPECT_TRUE(opt > rhsEmpty);
  EXPECT_TRUE(opt >= rhsEmpty);
  EXPECT_FALSE(rhsEmpty == opt);
  EXPECT_TRUE(rhsEmpty != opt);
  EXPECT_TRUE(rhsEmpty < opt);
  EXPECT_TRUE(rhsEmpty <= opt);
  EXPECT_FALSE(rhsEmpty > opt);
  EXPECT_FALSE(rhsEmpty >= opt);

  opt = nullptr;
  EXPECT_FALSE(opt.hasValue());
  opt = T(value);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == T(value));
  opt.reset();
  EXPECT_FALSE(opt.hasValue());
  opt.assign(T(value));
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == T(value));
  opt.emplace(8);
  EXPECT_TRUE(opt.hasValue());
  EXPECT_TRUE(opt.value() == T(8));
}


// -- test cases --

TEST_F(OptionalTest, defaultCtorAccessors) {
  _testEmpty<BaseType>();
  _testConstEmpty<BaseType>();

  _testEmpty<Copyable<int> >();
  _testConstEmpty<Copyable<int> >();

  _testEmpty<Movable<int> >();
  _testConstEmpty<Movable<int> >();

  _testEmpty<CopyMovable<int> >();
  _testConstEmpty<CopyMovable<int> >();
}

TEST_F(OptionalTest, initCtorAccessors) {
  _testInit<BaseType>(5, 7);
  _testInit<Copyable<int> >(Copyable<int>(5), Copyable<int>(7));
  _testInitMove<Movable<int> >(5, 7);
  _testInit<CopyMovable<int> >(CopyMovable<int>(5), CopyMovable<int>(7));
  _testInitMove<CopyMovable<int> >(5, 7);
  Optional<CopyMovable<int> > opt(CopyMovable<int>(5));
  EXPECT_TRUE(opt->value() == 5); // test '->'
}

TEST_F(OptionalTest, copyMove) {
  Optional<BaseType> baseType(42);
  Optional<BaseType> baseCopy(baseType);
  Optional<BaseType> baseMove(std::move(baseType));
  EXPECT_TRUE(baseCopy.value() == 42);
  EXPECT_TRUE(baseMove.value() == 42);
  Optional<BaseType> baseTypeEmpty(nullptr);
  Optional<BaseType> baseCopyEmpty(baseTypeEmpty);
  Optional<BaseType> baseMoveEmpty(std::move(baseTypeEmpty));
  EXPECT_FALSE(baseCopyEmpty.hasValue());
  EXPECT_FALSE(baseMoveEmpty.hasValue());
  baseType = baseCopy;
  baseCopy = std::move(baseMove);
  EXPECT_TRUE(baseType.value() == 42);
  EXPECT_TRUE(baseCopy.value() == 42);

  Optional<Copyable<int> > copyable(Copyable<int>(42));
  Optional<Copyable<int> > copyableCopy(copyable);
  EXPECT_TRUE(copyableCopy.value().value() == 42);
  Optional<Copyable<int> > copyableEmpty(nullptr);
  Optional<Copyable<int> > copyableCopyEmpty(copyableEmpty);
  EXPECT_FALSE(copyableCopyEmpty.hasValue());
  copyable = copyableCopy;
  EXPECT_TRUE(copyable.value() == 42);

  Optional<Movable<int> > movable(Movable<int>(42));
  Optional<Movable<int> > movableMove(std::move(movable));
  EXPECT_TRUE(movableMove.value().value() == 42);
  Optional<Movable<int> > movableEmpty(nullptr);
  Optional<Movable<int> > movableMoveEmpty(std::move(movableEmpty));
  EXPECT_FALSE(movableMoveEmpty.hasValue());
  movable = std::move(movableMove);
  EXPECT_TRUE(movable.value() == 42);

  Optional<CopyMovable<int> > cm(CopyMovable<int>(42));
  Optional<CopyMovable<int> > cmCopy(cm);
  Optional<CopyMovable<int> > cmMove(std::move(cm));
  EXPECT_TRUE(cmCopy.value().value() == 42);
  EXPECT_TRUE(cmMove.value().value() == 42);
  Optional<CopyMovable<int> > cmEmpty(nullptr);
  Optional<CopyMovable<int> > cmCopyEmpty(cmEmpty);
  Optional<CopyMovable<int> > cmMoveEmpty(std::move(cmEmpty));
  EXPECT_FALSE(cmCopyEmpty.hasValue());
  EXPECT_FALSE(cmMoveEmpty.hasValue());
  cm = cmCopy;
  cmCopy = std::move(cmMove);
  EXPECT_TRUE(cm.value() == 42);
  EXPECT_TRUE(cmCopy.value() == 42);
}

TEST_F(OptionalTest, swapOperation) {
  Optional<BaseType> baseType(42);
  Optional<BaseType> baseType2(21);
  ASSERT_TRUE(baseType.hasValue() && baseType2.hasValue());
  EXPECT_EQ(42, baseType.value());
  EXPECT_EQ(21, baseType2.value());
  std::swap(baseType, baseType2);
  ASSERT_TRUE(baseType.hasValue() && baseType2.hasValue());
  EXPECT_EQ(21, baseType.value());
  EXPECT_EQ(42, baseType2.value());
  baseType2.swap(baseType);
  ASSERT_TRUE(baseType.hasValue() && baseType2.hasValue());
  EXPECT_EQ(42, baseType.value());
  EXPECT_EQ(21, baseType2.value());

  Optional<Copyable<int> > copyable(Copyable<int>(42));
  Optional<Copyable<int> > copyable2(Copyable<int>(21));
  ASSERT_TRUE(copyable.hasValue() && copyable2.hasValue());
  EXPECT_EQ(42, copyable.value().value());
  EXPECT_EQ(21, copyable2.value().value());
  std::swap(copyable, copyable2);
  ASSERT_TRUE(copyable.hasValue() && copyable2.hasValue());
  EXPECT_EQ(21, copyable.value().value());
  EXPECT_EQ(42, copyable2.value().value());
  copyable2.swap(copyable);
  ASSERT_TRUE(copyable.hasValue() && copyable2.hasValue());
  EXPECT_EQ(42, copyable.value().value());
  EXPECT_EQ(21, copyable2.value().value());

  Optional<Movable<int> > movable(Movable<int>(42));
  Optional<Movable<int> > movable2(Movable<int>(21));
  ASSERT_TRUE(movable.hasValue() && movable2.hasValue());
  EXPECT_EQ(42, movable.value().value());
  EXPECT_EQ(21, movable2.value().value());
  std::swap(movable, movable2);
  ASSERT_TRUE(movable.hasValue() && movable2.hasValue());
  EXPECT_EQ(21, movable.value().value());
  EXPECT_EQ(42, movable2.value().value());
  movable2.swap(movable);
  ASSERT_TRUE(movable.hasValue() && movable2.hasValue());
  EXPECT_EQ(42, movable.value().value());
  EXPECT_EQ(21, movable2.value().value());

  Optional<CopyMovable<int> > cm(CopyMovable<int>(42));
  Optional<CopyMovable<int> > cm2(CopyMovable<int>(21));
  ASSERT_TRUE(cm.hasValue() && cm2.hasValue());
  EXPECT_EQ(42, cm.value().value());
  EXPECT_EQ(21, cm2.value().value());
  std::swap(cm, cm2);
  ASSERT_TRUE(cm.hasValue() && cm2.hasValue());
  EXPECT_EQ(21, cm.value().value());
  EXPECT_EQ(42, cm2.value().value());
  cm2.swap(cm);
  ASSERT_TRUE(cm.hasValue() && cm2.hasValue());
  EXPECT_EQ(42, cm.value().value());
  EXPECT_EQ(21, cm2.value().value());
}
