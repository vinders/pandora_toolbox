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
#include <vector>
#include <list>
#include <pattern/class_traits.h>

using namespace pandora::pattern;

class ClassTraitsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- mocks --

template <typename _DataType>
class TraitsTester final {
public:
  TraitsTester() = default;
  template <typename T = _DataType>
  TraitsTester(enable_if_copy_constructible<T, const T& > value) : _value(value) {}
  template <typename T = _DataType>
  TraitsTester(enable_if_move_constructible<T, T&& > value) noexcept : _value(std::move(value)) {}
  ~TraitsTester() = default;

  const _DataType& value() const noexcept { return _value; }

  // - copy / move -

  template <typename T = _DataType>
  TraitsTester(enable_if_copy_constructible<T, const TraitsTester<T>& > rhs) : _value(rhs._value) {}
  template <typename T = _DataType>
  TraitsTester(enable_if_move_constructible<T, TraitsTester<T>&& > rhs) noexcept : _value(std::move(rhs._value)) {}

  template <typename T = _DataType>
  enable_if_copy_constructible<T, TraitsTester<_DataType>& > operator=(const TraitsTester<T>& rhs) { _value = rhs._value; return *this; }
  template <typename T = _DataType>
  enable_if_move_constructible<T, TraitsTester<_DataType>& > operator=(TraitsTester<T>&& rhs) noexcept { _value = std::move(rhs._value); return *this; }

  // - comparisons -

  template <typename T = _DataType>
  inline bool operator==(enable_if_operator_equals<T, const TraitsTester<T>& > rhs) const noexcept { return (this->_value == rhs._value); }
  template <typename T = _DataType>
  inline bool operator!=(enable_if_operator_equals<T, const TraitsTester<T>& > rhs) const noexcept { return !(this->operator==(rhs)); }

  template <typename T = _DataType>
  inline bool operator<(enable_if_operator_less<T, const TraitsTester<T>& > rhs) const noexcept { return (this->_value < rhs._value); }
  template <typename T = _DataType>
  inline bool operator<=(enable_if_operator_less_eq<T, const TraitsTester<T>& > rhs) const noexcept { return (this->_value <= rhs._value); }
  template <typename T = _DataType>
  inline bool operator>(enable_if_operator_greater<T, const TraitsTester<T>& > rhs) const noexcept { return (this->_value > rhs._value); }
  template <typename T = _DataType>
  inline bool operator>=(enable_if_operator_greater_eq<T, const TraitsTester<T>& > rhs) const noexcept { return (this->_value >= rhs._value); }

private:
  _DataType _value;
};

#define __P_GENERATE_MOCK(name, copyable, movable) \
        template <typename T> class name { \
        public: \
          name() = default; \
          name(const T& value) : _value(value) {} \
          name(const name &) = copyable ; \
          name & operator=(const name &) = copyable ; \
          name(name &&) noexcept = movable ; \
          name & operator=(name &&) noexcept = movable ; \
          bool operator==(const name& rhs) const noexcept { return _value == rhs._value; } \
          bool operator!=(const name& rhs) const noexcept { return _value != rhs._value; } \
          bool operator<(const name& rhs) const noexcept { return _value < rhs._value; } \
          bool operator<=(const name& rhs) const noexcept { return _value <= rhs._value; } \
          bool operator>(const name& rhs) const noexcept { return _value > rhs._value; } \
          bool operator>=(const name& rhs) const noexcept { return _value >= rhs._value; } \
          void value(const T& val) noexcept { _value = val; } \
          const T& value() const noexcept { return _value; } \
        private: \
          T _value; }

using BaseType = int;
__P_GENERATE_MOCK(Copyable, default, delete);
__P_GENERATE_MOCK(Movable, delete, default);
__P_GENERATE_MOCK(CopyMovable, default, default);


// -- copy / move / comparisons --

TEST_F(ClassTraitsTest, baseTypeTraits) {
  TraitsTester<BaseType> data(42);

  TraitsTester<BaseType> copy(data);
  EXPECT_EQ(data.value(), copy.value());

  EXPECT_TRUE(data.value() == copy.value());
  EXPECT_FALSE(data.value() != copy.value());
  EXPECT_TRUE(data.value() <= copy.value());
  EXPECT_FALSE(data.value() < copy.value());
  EXPECT_TRUE(data.value() >= copy.value());
  EXPECT_FALSE(data.value() > copy.value());

  TraitsTester<BaseType> small(4);
  EXPECT_FALSE(small.value() == data.value());
  EXPECT_TRUE(small.value() != data.value());
  EXPECT_TRUE(small.value() <= data.value());
  EXPECT_TRUE(small.value() < data.value());
  EXPECT_FALSE(small.value() >= data.value());
  EXPECT_FALSE(small.value() > data.value());

  data = small;
  EXPECT_EQ(small.value(), data.value());
  EXPECT_TRUE(small.value() == data.value());
  EXPECT_FALSE(small.value() != data.value());
}

TEST_F(ClassTraitsTest, copyOnlyTraits) {
  TraitsTester<Copyable<int> > data(Copyable<int>(42));

  TraitsTester<Copyable<int> > copy(data);
  EXPECT_EQ(data.value(), copy.value());

  EXPECT_TRUE(data.value() == copy.value());
  EXPECT_FALSE(data.value() != copy.value());
  EXPECT_TRUE(data.value() <= copy.value());
  EXPECT_FALSE(data.value() < copy.value());
  EXPECT_TRUE(data.value() >= copy.value());
  EXPECT_FALSE(data.value() > copy.value());

  TraitsTester<Copyable<int> > small(Copyable<int>(4));
  EXPECT_FALSE(small.value() == data.value());
  EXPECT_TRUE(small.value() != data.value());
  EXPECT_TRUE(small.value() <= data.value());
  EXPECT_TRUE(small.value() < data.value());
  EXPECT_FALSE(small.value() >= data.value());
  EXPECT_FALSE(small.value() > data.value());

  data = small;
  EXPECT_EQ(small.value(), data.value());
  EXPECT_TRUE(small.value() == data.value());
  EXPECT_FALSE(small.value() != data.value());
}

TEST_F(ClassTraitsTest, moveOnlyTraits) {
  TraitsTester<Movable<int> > data(Movable<int>(42));
  EXPECT_EQ(data.value(), 42);

  TraitsTester<Movable<int> > moved(TraitsTester<Movable<int> >(Movable<int>(42)));
  EXPECT_EQ(moved.value(), 42);

  data = TraitsTester<Movable<int> >(Movable<int>(4));
  EXPECT_EQ(data.value(), 4);
  data = TraitsTester<Movable<int> >(Movable<int>(42));
  EXPECT_EQ(data.value(), 42);
  EXPECT_TRUE(data.value() == moved.value());
  EXPECT_FALSE(data.value() != moved.value());
  EXPECT_TRUE(data.value() <= moved.value());
  EXPECT_FALSE(data.value() < moved.value());
  EXPECT_TRUE(data.value() >= moved.value());
  EXPECT_FALSE(data.value() > moved.value());

  TraitsTester<Movable<int> > small(Movable<int>(4));
  EXPECT_FALSE(small.value() == data.value());
  EXPECT_TRUE(small.value() != data.value());
  EXPECT_TRUE(small.value() <= data.value());
  EXPECT_TRUE(small.value() < data.value());
  EXPECT_FALSE(small.value() >= data.value());
  EXPECT_FALSE(small.value() > data.value());

  moved = std::move(data);
  EXPECT_FALSE(small.value() == moved.value());
  EXPECT_TRUE(small.value() != moved.value());
  EXPECT_TRUE(small.value() <= moved.value());
  EXPECT_TRUE(small.value() < moved.value());
  EXPECT_FALSE(small.value() >= moved.value());
  EXPECT_FALSE(small.value() > moved.value());
}

TEST_F(ClassTraitsTest, copyMoveTraits) {
  TraitsTester<CopyMovable<int> > data(CopyMovable<int>(42));
  TraitsTester<CopyMovable<int> > copy(data);
  EXPECT_EQ(data.value(), copy.value());
  TraitsTester<CopyMovable<int> > moved(std::move(data));
  EXPECT_EQ(copy.value(), moved.value());

  data = copy;
  EXPECT_EQ(data.value(), copy.value());
  EXPECT_TRUE(data.value() == moved.value());
  EXPECT_FALSE(data.value() != moved.value());
  EXPECT_TRUE(data.value() <= moved.value());
  EXPECT_FALSE(data.value() < moved.value());
  EXPECT_TRUE(data.value() >= moved.value());
  EXPECT_FALSE(data.value() > moved.value());

  TraitsTester<CopyMovable<int> > small(CopyMovable<int>(4));
  EXPECT_FALSE(small.value() == data.value());
  EXPECT_TRUE(small.value() != data.value());
  EXPECT_TRUE(small.value() <= data.value());
  EXPECT_TRUE(small.value() < data.value());
  EXPECT_FALSE(small.value() >= data.value());
  EXPECT_FALSE(small.value() > data.value());

  moved = std::move(data);
  EXPECT_FALSE(small.value() == moved.value());
  EXPECT_TRUE(small.value() != moved.value());
  EXPECT_TRUE(small.value() <= moved.value());
  EXPECT_TRUE(small.value() < moved.value());
  EXPECT_FALSE(small.value() >= moved.value());
  EXPECT_FALSE(small.value() > moved.value());
}


// -- iterators --

template <typename _It>
int getValue(enable_if_iterator<_It, const _It&> it) noexcept { return *it; }
template <typename _It>
int getValueBiDirectional(enable_if_iterator_reversible<_It, const _It&> it) noexcept { return *it; }

TEST_F(ClassTraitsTest, iteratorTraits) {
  EXPECT_TRUE(is_forward_iterator<std::vector<char>::iterator>::value);
  EXPECT_TRUE(is_bidirectional_iterator<std::vector<char>::iterator>::value);
  EXPECT_TRUE(is_random_access_iterator<std::vector<char>::iterator>::value);

  EXPECT_TRUE(is_forward_iterator<std::list<char>::iterator>::value);
  EXPECT_TRUE(is_bidirectional_iterator<std::list<char>::iterator>::value);
  EXPECT_FALSE(is_random_access_iterator<std::list<char>::iterator>::value);

  std::vector<int> vec;
  vec.push_back(42);
  vec.push_back(24);
  EXPECT_TRUE(getValue<std::vector<int>::iterator>(vec.begin()) == 42);
  EXPECT_TRUE(getValue<std::vector<int>::iterator>(vec.begin() + 1) == 24);
  EXPECT_TRUE(getValueBiDirectional<std::vector<int>::iterator>(vec.begin()) == 42);
  EXPECT_TRUE(getValueBiDirectional<std::vector<int>::iterator>(vec.begin() + 1) == 24);
}


// -- constructors / assignment enablers --

template <typename T>
struct EnablerTester : public ConstructorSelector<T>, public AssignmentSelector<T> {
  EnablerTester() = default;
  EnablerTester(const EnablerTester<T>& rhs) : ConstructorSelector<T>(rhs) { value = rhs.value; }
  EnablerTester(EnablerTester<T>&& rhs) noexcept : ConstructorSelector<T>(std::move(rhs)) { value = std::move(rhs.value); }
  EnablerTester& operator=(const EnablerTester<T>& rhs) { AssignmentSelector<T>::operator=(rhs);  value = rhs.value; }
  EnablerTester& operator=(EnablerTester<T>&& rhs) noexcept { AssignmentSelector<T>::operator=(std::move(rhs)); value = std::move(rhs.value); }
  T value;
};

TEST_F(ClassTraitsTest, enablers) {
  EnablerTester<BaseType> baseType;
  baseType.value = 42;
  EnablerTester<BaseType> baseCopy(baseType);
  EnablerTester<BaseType> baseMove(std::move(baseType));
  EXPECT_TRUE(baseCopy.value == 42);
  EXPECT_TRUE(baseMove.value == 42);

  EnablerTester<Copyable<int> > copyable;
  copyable.value.value(42);
  EnablerTester<Copyable<int> > copyableCopy(copyable);
  EXPECT_TRUE(copyableCopy.value.value() == 42);

  EnablerTester<Movable<int> > movable;
  movable.value.value(42);
  EnablerTester<Movable<int> > movableMove(std::move(movable));
  EXPECT_TRUE(movableMove.value.value() == 42);

  EnablerTester<CopyMovable<int> > copyMovable;
  copyMovable.value.value(42);
  EnablerTester<CopyMovable<int> > copyMovableCopy(copyMovable);
  EnablerTester<CopyMovable<int> > copyMovableMove(std::move(copyMovable));
  EXPECT_TRUE(copyableCopy.value.value() == 42);
  EXPECT_TRUE(copyMovableMove.value.value() == 42);
}
