#include <gtest/gtest.h>
#include <vector>
#include <pattern/iterator.h>

using namespace pandora::pattern;

class IteratorTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- mocks --

class ObjectMock final {
public:
  ObjectMock() = default;
  ObjectMock(int value) : value(value) {}
  ObjectMock(const ObjectMock&) = default;
  ObjectMock(ObjectMock&&) = default;
  ObjectMock& operator=(const ObjectMock&) = default;
  ObjectMock& operator=(ObjectMock&&) = default;
  bool operator==(const ObjectMock& rhs) const { return (value == rhs.value); }
  bool operator!=(const ObjectMock& rhs) const { return (value != rhs.value); }
private:
  int value = 0;
};

template <typename _ItemType>
class MonodirectionalCollection final : public Iterable<_ItemType, false> {
public:
  MonodirectionalCollection() = default;
  ~MonodirectionalCollection() {}
  std::vector<_ItemType>& data() { return _items; }
  size_t size() const { return _items.size(); }

  _P_FORWARD_ITERATOR_MEMBERS(MonodirectionalCollection<_ItemType>, _ItemType, &_items[0])
  _ItemType* next(_ItemType* current, uint32_t currentIndex, size_t offset) override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
  const _ItemType* next(const _ItemType* current, uint32_t currentIndex, size_t offset) const override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
private:
  std::vector<_ItemType> _items;
};

template <typename _ItemType>
class BidirectionalCollection final : public Iterable<_ItemType, true> {
public:
  BidirectionalCollection() = default;
  ~BidirectionalCollection() {}
  std::vector<_ItemType>& data() { return _items; }
  size_t size() const { return _items.size(); }

  _P_BIDIRECT_ITERATOR_MEMBERS(BidirectionalCollection<_ItemType>, _ItemType, &_items[0], &(_items.back()))
  _ItemType* next(_ItemType* current, uint32_t currentIndex, size_t offset) override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
  const _ItemType* next(const _ItemType* current, uint32_t currentIndex, size_t offset) const override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
  _ItemType* previous(_ItemType* current, uint32_t currentIndex, size_t offset) override {
    return (current != nullptr && currentIndex >= offset && currentIndex - offset < _items.size()) ? &_items[currentIndex - offset] : nullptr;
  }
  const _ItemType* previous(const _ItemType* current, uint32_t currentIndex, size_t offset) const override {
    return (current != nullptr && currentIndex >= offset && currentIndex - offset < _items.size()) ? &_items[currentIndex - offset] : nullptr;
  }
private:
  std::vector<_ItemType> _items;
};

template <typename _ItemType>
class RandomAccessCollection final : public Iterable<_ItemType, true> {
public:
  RandomAccessCollection() = default;
  ~RandomAccessCollection() {}
  std::vector<_ItemType>& data() { return _items; }
  size_t size() const { return _items.size(); }

  _P_RANDOMACCESS_ITERATOR_MEMBERS(RandomAccessCollection<_ItemType>, _ItemType, &_items[0], &(_items.back()))
  _ItemType* next(_ItemType* current, uint32_t currentIndex, size_t offset) override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
  const _ItemType* next(const _ItemType* current, uint32_t currentIndex, size_t offset) const override {
    return (current != nullptr && currentIndex + offset < _items.size()) ? &_items[currentIndex + offset] : nullptr;
  }
  _ItemType* previous(_ItemType* current, uint32_t currentIndex, size_t offset) override {
    return (current != nullptr && currentIndex >= offset && currentIndex - offset < _items.size()) ? &_items[currentIndex - offset] : nullptr;
  }
  const _ItemType* previous(const _ItemType* current, uint32_t currentIndex, size_t offset) const override {
    return (current != nullptr && currentIndex >= offset && currentIndex - offset < _items.size()) ? &_items[currentIndex - offset] : nullptr;
  }
private:
  std::vector<_ItemType> _items;
};


// -- iterator tests --

template <typename _IteratorType, typename _CollectionType, typename _ValueType>
void _testBaseIterator() {
  _CollectionType collection;
  collection.data().emplace_back(5);
  collection.data().emplace_back(7);
  collection.data().emplace_back(42);

  _IteratorType empty;
  EXPECT_FALSE(empty.hasValue());
  EXPECT_TRUE(empty == collection.end());
  EXPECT_FALSE(empty != collection.end());
  EXPECT_TRUE(empty.base() == empty);
  EXPECT_TRUE(empty.index() == -1);

  _IteratorType first(collection, &(collection.data().front()));
  EXPECT_TRUE(first.hasValue());
  EXPECT_FALSE(first == collection.end());
  EXPECT_TRUE(first != collection.end());
  EXPECT_TRUE(first.base() == first);
  EXPECT_TRUE(first.data() == _ValueType(5));
  EXPECT_TRUE(*first == _ValueType(5));
  EXPECT_TRUE(first.index() == 0);
  const _IteratorType& firstConst = first;
  EXPECT_TRUE(firstConst.data() == _ValueType(5));
  EXPECT_TRUE(*firstConst == _ValueType(5));
  EXPECT_TRUE(firstConst == first);
  auto firstBegin = collection.begin();
  ASSERT_TRUE(firstBegin.hasValue());
  EXPECT_FALSE(firstBegin == collection.end());
  EXPECT_TRUE(firstBegin != collection.end());
  EXPECT_TRUE(firstBegin.data() == _ValueType(5));
  EXPECT_TRUE(*firstBegin == _ValueType(5));
  EXPECT_TRUE(first == firstBegin);
  EXPECT_TRUE(firstConst == firstBegin);
  auto firstConstBegin = collection.cbegin();
  ASSERT_TRUE(firstConstBegin.hasValue());
  EXPECT_FALSE(firstConstBegin == collection.end());
  EXPECT_TRUE(firstConstBegin != collection.end());
  EXPECT_FALSE(firstConstBegin == collection.cend());
  EXPECT_TRUE(firstConstBegin != collection.cend());
  EXPECT_TRUE(firstConstBegin.data() == _ValueType(5));
  EXPECT_TRUE(*firstConstBegin == _ValueType(5));

  _IteratorType second(collection, &(collection.data()[1]), 1u);
  ASSERT_TRUE(second.hasValue());
  EXPECT_FALSE(second == collection.end());
  EXPECT_TRUE(second != collection.end());
  EXPECT_TRUE(second.data() == _ValueType(7));
  EXPECT_TRUE(second.index() == 1);
  auto secondBegin = collection.begin() + 1;
  ASSERT_TRUE(secondBegin.hasValue());
  EXPECT_FALSE(secondBegin == collection.end());
  EXPECT_TRUE(secondBegin != collection.end());
  EXPECT_TRUE(secondBegin.data() == _ValueType(7));
  EXPECT_TRUE(second == secondBegin);

  EXPECT_FALSE(first == second);
  EXPECT_TRUE(first != second);
  EXPECT_TRUE(first < second);
  EXPECT_TRUE(first <= second);
  EXPECT_FALSE(first > second);
  EXPECT_FALSE(first >= second);
  EXPECT_FALSE(second < first);
  EXPECT_FALSE(second <= first);
  EXPECT_TRUE(second > first);
  EXPECT_TRUE(second >= first);
  EXPECT_TRUE((second - first) == 1);
  EXPECT_TRUE((first - second) == -1);

  _IteratorType emptyCopy(empty);
  EXPECT_FALSE(emptyCopy.hasValue());
  EXPECT_TRUE(emptyCopy == collection.end());
  EXPECT_FALSE(emptyCopy != collection.end());
  EXPECT_TRUE(emptyCopy == empty);
  _IteratorType emptyMove(std::move(emptyCopy));
  EXPECT_FALSE(emptyMove.hasValue());
  EXPECT_TRUE(emptyMove == collection.end());
  EXPECT_FALSE(emptyMove != collection.end());
  EXPECT_TRUE(emptyMove == empty);

  _IteratorType firstCopy(first);
  ASSERT_TRUE(firstCopy.hasValue());
  EXPECT_FALSE(firstCopy == collection.end());
  EXPECT_TRUE(firstCopy != collection.end());
  EXPECT_TRUE(firstCopy.data() == _ValueType(5));
  EXPECT_TRUE(firstCopy == first);
  _IteratorType firstMove(std::move(firstCopy));
  ASSERT_TRUE(firstMove.hasValue());
  EXPECT_FALSE(firstMove == collection.end());
  EXPECT_TRUE(firstMove != collection.end());
  EXPECT_TRUE(firstMove.data() == _ValueType(5));
  EXPECT_TRUE(firstMove == first);

  emptyCopy = firstMove;
  ASSERT_TRUE(emptyCopy.hasValue());
  EXPECT_FALSE(emptyCopy == collection.end());
  EXPECT_TRUE(emptyCopy != collection.end());
  EXPECT_TRUE(emptyCopy.data() == _ValueType(5));
  EXPECT_TRUE(emptyCopy == first);
  emptyMove = std::move(firstMove);
  ASSERT_TRUE(emptyMove.hasValue());
  EXPECT_FALSE(emptyMove == collection.end());
  EXPECT_TRUE(emptyMove != collection.end());
  EXPECT_TRUE(emptyMove.data() == _ValueType(5));
  EXPECT_TRUE(emptyMove == first);

  first.swap(second);
  ASSERT_TRUE(first.hasValue());
  EXPECT_TRUE(first.data() == _ValueType(7));
  ASSERT_TRUE(second.hasValue());
  EXPECT_TRUE(second.data() == _ValueType(5));
  EXPECT_FALSE(first == second);
  EXPECT_TRUE(first != second);
  EXPECT_FALSE(first < second);
  EXPECT_FALSE(first <= second);
  EXPECT_TRUE(first > second);
  EXPECT_TRUE(first >= second);
  EXPECT_TRUE(second < first);
  EXPECT_TRUE(second <= first);
  EXPECT_FALSE(second > first);
  EXPECT_FALSE(second >= first);
}

template <typename _IteratorType, typename _CollectionType, typename _ValueType>
void _testBaseReversedIterator() {
  _CollectionType collection;
  collection.data().emplace_back(5);
  collection.data().emplace_back(7);
  collection.data().emplace_back(42);

  _IteratorType empty;
  EXPECT_FALSE(empty.hasValue());
  EXPECT_TRUE(empty == collection.rend());
  EXPECT_FALSE(empty != collection.rend());
  EXPECT_TRUE(empty.base() == empty);
  EXPECT_TRUE(empty.index() == -1);

  _IteratorType first(collection, &(collection.data().back()));
  EXPECT_TRUE(first.hasValue());
  EXPECT_FALSE(first == collection.rend());
  EXPECT_TRUE(first != collection.rend());
  EXPECT_TRUE(first.base() == first);
  EXPECT_TRUE(first.data() == _ValueType(42));
  EXPECT_TRUE(*first == _ValueType(42));
  EXPECT_TRUE(first.index() == 0);
  const _IteratorType& firstConst = first;
  EXPECT_TRUE(firstConst.data() == _ValueType(42));
  EXPECT_TRUE(*firstConst == _ValueType(42));
  EXPECT_TRUE(firstConst == first);
  auto firstBegin = collection.rbegin();
  ASSERT_TRUE(firstBegin.hasValue());
  EXPECT_FALSE(firstBegin == collection.rend());
  EXPECT_TRUE(firstBegin != collection.rend());
  EXPECT_TRUE(firstBegin.data() == _ValueType(42));
  EXPECT_TRUE(*firstBegin == _ValueType(42));
  EXPECT_TRUE(first == firstBegin);
  EXPECT_TRUE(firstConst == firstBegin);
  auto firstConstBegin = collection.crbegin();
  ASSERT_TRUE(firstConstBegin.hasValue());
  EXPECT_FALSE(firstConstBegin == collection.rend());
  EXPECT_TRUE(firstConstBegin != collection.rend());
  EXPECT_FALSE(firstConstBegin == collection.crend());
  EXPECT_TRUE(firstConstBegin != collection.crend());
  EXPECT_TRUE(firstConstBegin.data() == _ValueType(42));
  EXPECT_TRUE(*firstConstBegin == _ValueType(42));

  _IteratorType second(collection, &(collection.data()[1]), 1u);
  ASSERT_TRUE(second.hasValue());
  EXPECT_FALSE(second == collection.rend());
  EXPECT_TRUE(second != collection.rend());
  EXPECT_TRUE(second.data() == _ValueType(7));
  EXPECT_TRUE(second.index() == 1);
  auto secondBegin = collection.rbegin() + 1;
  ASSERT_TRUE(secondBegin.hasValue());
  EXPECT_FALSE(secondBegin == collection.rend());
  EXPECT_TRUE(secondBegin != collection.rend());
  EXPECT_TRUE(secondBegin.data() == _ValueType(7));
  EXPECT_TRUE(second == secondBegin);

  EXPECT_FALSE(first == second);
  EXPECT_TRUE(first != second);
  EXPECT_TRUE(first < second);
  EXPECT_TRUE(first <= second);
  EXPECT_FALSE(first > second);
  EXPECT_FALSE(first >= second);
  EXPECT_FALSE(second < first);
  EXPECT_FALSE(second <= first);
  EXPECT_TRUE(second > first);
  EXPECT_TRUE(second >= first);
  EXPECT_TRUE((second - first) == 1);
  EXPECT_TRUE((first - second) == -1);

  _IteratorType reverseConstruction(collection.begin());
  ASSERT_TRUE(reverseConstruction.hasValue());
  EXPECT_FALSE(reverseConstruction == collection.rend());
  EXPECT_TRUE(reverseConstruction != collection.rend());
  EXPECT_TRUE(reverseConstruction.data() == _ValueType(5));
  Iterator<typename _IteratorType::collection_type, typename _IteratorType::value_type, false, false,
           typename _IteratorType::iterator_category> unreverseConstruction(reverseConstruction);
  ASSERT_TRUE(unreverseConstruction.hasValue());
  EXPECT_FALSE(unreverseConstruction == collection.end());
  EXPECT_TRUE(unreverseConstruction != collection.end());
  EXPECT_TRUE(unreverseConstruction.data() == _ValueType(5));

  _IteratorType emptyCopy(empty);
  EXPECT_FALSE(emptyCopy.hasValue());
  EXPECT_TRUE(emptyCopy == collection.rend());
  EXPECT_FALSE(emptyCopy != collection.rend());
  EXPECT_TRUE(emptyCopy == empty);
  _IteratorType emptyMove(std::move(emptyCopy));
  EXPECT_FALSE(emptyMove.hasValue());
  EXPECT_TRUE(emptyMove == collection.rend());
  EXPECT_FALSE(emptyMove != collection.rend());
  EXPECT_TRUE(emptyMove == empty);

  _IteratorType firstCopy(first);
  ASSERT_TRUE(firstCopy.hasValue());
  EXPECT_FALSE(firstCopy == collection.rend());
  EXPECT_TRUE(firstCopy != collection.rend());
  EXPECT_TRUE(firstCopy.data() == _ValueType(42));
  EXPECT_TRUE(firstCopy == first);
  _IteratorType firstMove(std::move(firstCopy));
  ASSERT_TRUE(firstMove.hasValue());
  EXPECT_FALSE(firstMove == collection.rend());
  EXPECT_TRUE(firstMove != collection.rend());
  EXPECT_TRUE(firstMove.data() == _ValueType(42));
  EXPECT_TRUE(firstMove == first);

  emptyCopy = firstMove;
  ASSERT_TRUE(emptyCopy.hasValue());
  EXPECT_FALSE(emptyCopy == collection.rend());
  EXPECT_TRUE(emptyCopy != collection.rend());
  EXPECT_TRUE(emptyCopy.data() == _ValueType(42));
  EXPECT_TRUE(emptyCopy == first);
  emptyMove = std::move(firstMove);
  ASSERT_TRUE(emptyMove.hasValue());
  EXPECT_FALSE(emptyMove == collection.rend());
  EXPECT_TRUE(emptyMove != collection.rend());
  EXPECT_TRUE(emptyMove.data() == _ValueType(42));
  EXPECT_TRUE(emptyMove == first);

  first.swap(second);
  ASSERT_TRUE(first.hasValue());
  EXPECT_TRUE(first.data() == _ValueType(7));
  ASSERT_TRUE(second.hasValue());
  EXPECT_TRUE(second.data() == _ValueType(42));
  EXPECT_FALSE(first == second);
  EXPECT_TRUE(first != second);
  EXPECT_FALSE(first < second);
  EXPECT_FALSE(first <= second);
  EXPECT_TRUE(first > second);
  EXPECT_TRUE(first >= second);
  EXPECT_TRUE(second < first);
  EXPECT_TRUE(second <= first);
  EXPECT_FALSE(second > first);
  EXPECT_FALSE(second >= first);
}

template <typename _IteratorType, typename _CollectionType, typename _ValueType>
void _testForwardIteration() {
  _CollectionType collection;
  collection.data().emplace_back(5);
  collection.data().emplace_back(7);
  collection.data().emplace_back(42);

  _IteratorType empty;
  EXPECT_FALSE(empty.hasValue());
  _IteratorType first(collection, &(collection.data().front()));
  ASSERT_TRUE(first.hasValue());
  auto firstBegin = collection.begin();
  ASSERT_TRUE(firstBegin.hasValue());
  _IteratorType second(collection, &(collection.data()[1]), 1u);
  ASSERT_TRUE(second.hasValue());

  EXPECT_FALSE((empty + 1).hasValue());
  EXPECT_FALSE((empty + collection.size()).hasValue());
  EXPECT_FALSE((++empty).hasValue());
  EXPECT_FALSE((empty++).hasValue());
  EXPECT_FALSE((empty += 1).hasValue());
  EXPECT_FALSE((empty += collection.size()).hasValue());

  _IteratorType firstCopy(first);
  _IteratorType firstAt(first);
  EXPECT_TRUE((first + 1).hasValue());
  EXPECT_TRUE((first + 1).index() == 1);
  EXPECT_TRUE((first + 1).data() == _ValueType(7));
  EXPECT_TRUE((first + 2).hasValue());
  EXPECT_TRUE((first + 2).index() == 2);
  EXPECT_TRUE((first + 2).data() == _ValueType(42));
  EXPECT_FALSE((first + collection.size()).hasValue());
  EXPECT_TRUE((second + 1).hasValue());
  EXPECT_TRUE((second + 1).index() == 2);
  EXPECT_TRUE((second + 1).data() == _ValueType(42));
  EXPECT_FALSE((second + 2).hasValue());
  _IteratorType firstPostInc = first++;
  EXPECT_TRUE(firstPostInc.hasValue());
  EXPECT_TRUE(firstPostInc.index() == 0);
  EXPECT_TRUE(firstPostInc.data() == _ValueType(5));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 1);
  EXPECT_TRUE(first.data() == _ValueType(7));
  _IteratorType firstPreInc = ++first;
  EXPECT_TRUE(firstPreInc.hasValue());
  EXPECT_TRUE(firstPreInc.index() == 2);
  EXPECT_TRUE(firstPreInc.data() == _ValueType(42));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 2);
  EXPECT_TRUE(first.data() == _ValueType(42));
  EXPECT_FALSE((++first).hasValue());
  EXPECT_FALSE((++first).hasValue());
  EXPECT_TRUE(first == collection.end());

  firstBegin += 2;
  EXPECT_TRUE(firstBegin.hasValue());
  EXPECT_TRUE(firstBegin.index() == 2);
  EXPECT_TRUE(firstBegin.data() == _ValueType(42));
  firstCopy += collection.size();
  EXPECT_FALSE(firstCopy.hasValue());

  EXPECT_TRUE(firstAt[0] == _ValueType(5));
  EXPECT_TRUE(firstAt[1] == _ValueType(7));
  EXPECT_TRUE(firstAt[2] == _ValueType(42));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(7));
  EXPECT_TRUE(firstAt[1] == _ValueType(42));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(42));

  int newValue = 0;
  for (auto& it : collection)
    it = _ValueType(++newValue);
  for (size_t i = 0; i < collection.size(); ++i)
    EXPECT_EQ(_ValueType(static_cast<int>(i + 1)), collection.data()[i]);

  newValue = 5;
  for (auto it = collection.begin(); it != collection.end(); ++it)
    *it = _ValueType(++newValue);
  for (size_t i = 0; i < collection.size(); ++i)
    EXPECT_EQ(_ValueType(static_cast<int>(i + 6)), collection.data()[i]);
}

template <typename _IteratorType, typename _CollectionType, typename _ValueType>
void _testBidirectionalIteration() {
  _CollectionType collection;
  collection.data().emplace_back(5);
  collection.data().emplace_back(7);
  collection.data().emplace_back(42);

  _IteratorType empty;
  EXPECT_FALSE(empty.hasValue());
  _IteratorType first(collection, &(collection.data().front()));
  ASSERT_TRUE(first.hasValue());
  auto firstBegin = collection.begin();
  ASSERT_TRUE(firstBegin.hasValue());
  _IteratorType second(collection, &(collection.data()[1]), 1u);
  ASSERT_TRUE(second.hasValue());

  EXPECT_FALSE((empty + 1).hasValue());
  EXPECT_FALSE((empty + collection.size()).hasValue());
  EXPECT_FALSE((++empty).hasValue());
  EXPECT_FALSE((empty++).hasValue());
  EXPECT_FALSE((empty += 1).hasValue());
  EXPECT_FALSE((empty += collection.size()).hasValue());

  EXPECT_FALSE((empty - 1).hasValue());
  EXPECT_FALSE((empty - collection.size()).hasValue());
  EXPECT_FALSE((--empty).hasValue());
  EXPECT_FALSE((empty--).hasValue());
  EXPECT_FALSE((empty -= 1).hasValue());
  EXPECT_FALSE((empty -= collection.size()).hasValue());

  _IteratorType firstCopy(first);
  _IteratorType firstCopy2(first);
  _IteratorType last(collection, &(collection.data().back()), static_cast<uint32_t>(collection.data().size()) - 1u);
  _IteratorType firstAt(first);
  EXPECT_TRUE((first + 1).hasValue());
  EXPECT_TRUE((first + 1).index() == 1);
  EXPECT_TRUE((first + 1).data() == _ValueType(7));
  EXPECT_TRUE((first + 2).hasValue());
  EXPECT_TRUE((first + 2).index() == 2);
  EXPECT_TRUE((first + 2).data() == _ValueType(42));
  EXPECT_FALSE((first + collection.size()).hasValue());
  EXPECT_TRUE((second + 1).hasValue());
  EXPECT_TRUE((second + 1).index() == 2);
  EXPECT_TRUE((second + 1).data() == _ValueType(42));
  EXPECT_FALSE((second + 2).hasValue());
  EXPECT_FALSE((first - 1).hasValue());
  EXPECT_TRUE((second - 1).hasValue());
  EXPECT_TRUE((second - 1).index() == 0);
  EXPECT_TRUE((second - 1).data() == _ValueType(5));
  EXPECT_FALSE((second - 2).hasValue());
  _IteratorType firstPostInc = first++;
  EXPECT_TRUE(firstPostInc.hasValue());
  EXPECT_TRUE(firstPostInc.index() == 0);
  EXPECT_TRUE(firstPostInc.data() == _ValueType(5));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 1);
  EXPECT_TRUE(first.data() == _ValueType(7));
  _IteratorType firstPreInc = ++first;
  EXPECT_TRUE(firstPreInc.hasValue());
  EXPECT_TRUE(firstPreInc.index() == 2);
  EXPECT_TRUE(firstPreInc.data() == _ValueType(42));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 2);
  EXPECT_TRUE(first.data() == _ValueType(42));
  EXPECT_FALSE((++first).hasValue());
  EXPECT_FALSE((++first).hasValue());
  EXPECT_TRUE(first == collection.end());
  _IteratorType lastPostDec = last--;
  EXPECT_TRUE(lastPostDec.hasValue());
  EXPECT_TRUE(lastPostDec.index() == 2);
  EXPECT_TRUE(lastPostDec.data() == _ValueType(42));
  EXPECT_TRUE(last.hasValue());
  EXPECT_TRUE(last.index() == 1);
  EXPECT_TRUE(last.data() == _ValueType(7));
  _IteratorType lastPreDec = --last;
  EXPECT_TRUE(lastPreDec.hasValue());
  EXPECT_TRUE(lastPreDec.index() == 0);
  EXPECT_TRUE(lastPreDec.data() == _ValueType(5));
  EXPECT_TRUE(last.hasValue());
  EXPECT_TRUE(last.index() == 0);
  EXPECT_TRUE(last.data() == _ValueType(5));
  EXPECT_FALSE((--last).hasValue());
  EXPECT_FALSE((--last).hasValue());
  EXPECT_TRUE(last == collection.end());

  firstBegin += 2;
  EXPECT_TRUE(firstBegin.hasValue());
  EXPECT_TRUE(firstBegin.index() == 2);
  EXPECT_TRUE(firstBegin.data() == _ValueType(42));
  firstBegin -= 2;
  EXPECT_TRUE(firstBegin.hasValue());
  EXPECT_TRUE(firstBegin.index() == 0);
  EXPECT_TRUE(firstBegin.data() == _ValueType(5));
  firstCopy += collection.size();
  EXPECT_FALSE(firstCopy.hasValue());
  firstCopy2 -= 1;
  EXPECT_FALSE(firstCopy2.hasValue());

  EXPECT_TRUE(firstAt[0] == _ValueType(5));
  EXPECT_TRUE(firstAt[1] == _ValueType(7));
  EXPECT_TRUE(firstAt[2] == _ValueType(42));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(7));
  EXPECT_TRUE(firstAt[1] == _ValueType(42));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(42));
  --firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(7));
  EXPECT_TRUE(firstAt[1] == _ValueType(42));
  --firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(5));
  EXPECT_TRUE(firstAt[1] == _ValueType(7));
  EXPECT_TRUE(firstAt[2] == _ValueType(42));

  int newValue = 0;
  for (auto& it : collection)
    it = _ValueType(++newValue);
  for (size_t i = 0; i < collection.size(); ++i)
    EXPECT_EQ(_ValueType(static_cast<int>(i + 1)), collection.data()[i]);

  newValue = 5;
  for (auto it = collection.begin(); it != collection.end(); ++it)
    *it = _ValueType(++newValue);
  for (size_t i = 0; i < collection.size(); ++i)
    EXPECT_EQ(_ValueType(static_cast<int>(i + 6)), collection.data()[i]);
}

template <typename _IteratorType, typename _CollectionType, typename _ValueType>
void _testBidirectionalReversedIteration() {
  _CollectionType collection;
  collection.data().emplace_back(5);
  collection.data().emplace_back(7);
  collection.data().emplace_back(42);

  _IteratorType empty;
  EXPECT_FALSE(empty.hasValue());
  _IteratorType first(collection, &(collection.data().back()));
  ASSERT_TRUE(first.hasValue());
  auto firstBegin = collection.rbegin();
  ASSERT_TRUE(firstBegin.hasValue());
  _IteratorType second(collection, &(collection.data()[1]), 1u);
  ASSERT_TRUE(second.hasValue());

  EXPECT_FALSE((empty + 1).hasValue());
  EXPECT_FALSE((empty + collection.size()).hasValue());
  EXPECT_FALSE((++empty).hasValue());
  EXPECT_FALSE((empty++).hasValue());
  EXPECT_FALSE((empty += 1).hasValue());
  EXPECT_FALSE((empty += collection.size()).hasValue());

  EXPECT_FALSE((empty - 1).hasValue());
  EXPECT_FALSE((empty - collection.size()).hasValue());
  EXPECT_FALSE((--empty).hasValue());
  EXPECT_FALSE((empty--).hasValue());
  EXPECT_FALSE((empty -= 1).hasValue());
  EXPECT_FALSE((empty -= collection.size()).hasValue());

  _IteratorType firstCopy(first);
  _IteratorType firstCopy2(first);
  _IteratorType last(collection, &(collection.data().front()), static_cast<uint32_t>(collection.data().size()) - 1u);
  _IteratorType firstAt(first);
  EXPECT_TRUE((first + 1).hasValue());
  EXPECT_TRUE((first + 1).index() == 1);
  EXPECT_TRUE((first + 1).data() == _ValueType(7));
  EXPECT_TRUE((first + 2).hasValue());
  EXPECT_TRUE((first + 2).index() == 2);
  EXPECT_TRUE((first + 2).data() == _ValueType(5));
  EXPECT_FALSE((first + collection.size()).hasValue());
  EXPECT_TRUE((second + 1).hasValue());
  EXPECT_TRUE((second + 1).index() == 2);
  EXPECT_TRUE((second + 1).data() == _ValueType(5));
  EXPECT_FALSE((second + 2).hasValue());
  EXPECT_FALSE((first - 1).hasValue());
  EXPECT_TRUE((second - 1).hasValue());
  EXPECT_TRUE((second - 1).index() == 0);
  EXPECT_TRUE((second - 1).data() == _ValueType(42));
  EXPECT_FALSE((second - 2).hasValue());
  _IteratorType firstPostInc = first++;
  EXPECT_TRUE(firstPostInc.hasValue());
  EXPECT_TRUE(firstPostInc.index() == 0);
  EXPECT_TRUE(firstPostInc.data() == _ValueType(42));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 1);
  EXPECT_TRUE(first.data() == _ValueType(7));
  _IteratorType firstPreInc = ++first;
  EXPECT_TRUE(firstPreInc.hasValue());
  EXPECT_TRUE(firstPreInc.index() == 2);
  EXPECT_TRUE(firstPreInc.data() == _ValueType(5));
  EXPECT_TRUE(first.hasValue());
  EXPECT_TRUE(first.index() == 2);
  EXPECT_TRUE(first.data() == _ValueType(5));
  EXPECT_FALSE((++first).hasValue());
  EXPECT_FALSE((++first).hasValue());
  EXPECT_TRUE(first == collection.rend());
  _IteratorType lastPostDec = last--;
  EXPECT_TRUE(lastPostDec.hasValue());
  EXPECT_TRUE(lastPostDec.index() == 2);
  EXPECT_TRUE(lastPostDec.data() == _ValueType(5));
  EXPECT_TRUE(last.hasValue());
  EXPECT_TRUE(last.index() == 1);
  EXPECT_TRUE(last.data() == _ValueType(7));
  _IteratorType lastPreDec = --last;
  EXPECT_TRUE(lastPreDec.hasValue());
  EXPECT_TRUE(lastPreDec.index() == 0);
  EXPECT_TRUE(lastPreDec.data() == _ValueType(42));
  EXPECT_TRUE(last.hasValue());
  EXPECT_TRUE(last.index() == 0);
  EXPECT_TRUE(last.data() == _ValueType(42));
  EXPECT_FALSE((--last).hasValue());
  EXPECT_FALSE((--last).hasValue());
  EXPECT_TRUE(last == collection.rend());

  firstBegin += 2;
  EXPECT_TRUE(firstBegin.hasValue());
  EXPECT_TRUE(firstBegin.index() == 2);
  EXPECT_TRUE(firstBegin.data() == _ValueType(5));
  firstBegin -= 2;
  EXPECT_TRUE(firstBegin.hasValue());
  EXPECT_TRUE(firstBegin.index() == 0);
  EXPECT_TRUE(firstBegin.data() == _ValueType(42));
  firstCopy += collection.size();
  EXPECT_FALSE(firstCopy.hasValue());
  firstCopy2 -= 1;
  EXPECT_FALSE(firstCopy2.hasValue());

  EXPECT_TRUE(firstAt[0] == _ValueType(42));
  EXPECT_TRUE(firstAt[1] == _ValueType(7));
  EXPECT_TRUE(firstAt[2] == _ValueType(5));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(7));
  EXPECT_TRUE(firstAt[1] == _ValueType(5));
  ++firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(5));
  --firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(7));
  EXPECT_TRUE(firstAt[1] == _ValueType(5));
  --firstAt;
  EXPECT_TRUE(firstAt[0] == _ValueType(42));
  EXPECT_TRUE(firstAt[1] == _ValueType(7));
  EXPECT_TRUE(firstAt[2] == _ValueType(5));

  int newValue = 5;
  for (auto it = collection.rbegin(); it != collection.rend(); ++it)
    *it = _ValueType(++newValue);
  for (size_t i = 0; i < collection.size(); ++i)
    EXPECT_EQ(_ValueType(static_cast<int>(8 - i)), collection.data()[i]);
}

// ---

TEST_F(IteratorTest, constructorsAccessors) {
  _testBaseIterator<Iterator<MonodirectionalCollection<int>, int, false, false, std::forward_iterator_tag>, MonodirectionalCollection<int>, int>();
  _testBaseIterator<Iterator<MonodirectionalCollection<ObjectMock>, ObjectMock, false, false, std::forward_iterator_tag>, MonodirectionalCollection<ObjectMock>, ObjectMock>();

  _testBaseIterator<Iterator<BidirectionalCollection<int>, int, false, false, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBaseIterator<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, false, false, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();
  _testBaseReversedIterator<Iterator<BidirectionalCollection<int>, int, false, true, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBaseReversedIterator<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, false, true, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();

  _testBaseIterator<Iterator<RandomAccessCollection<int>, int, false, false, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBaseIterator<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, false, false, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
  _testBaseReversedIterator<Iterator<RandomAccessCollection<int>, int, false, true, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBaseReversedIterator<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, false, true, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
}

TEST_F(IteratorTest, forwardIterator) {
  _testForwardIteration<Iterator<MonodirectionalCollection<int>, int, false, false, std::forward_iterator_tag>, MonodirectionalCollection<int>, int>();
  _testForwardIteration<Iterator<MonodirectionalCollection<ObjectMock>, ObjectMock, false, false, std::forward_iterator_tag>, MonodirectionalCollection<ObjectMock>, ObjectMock>();

  _testForwardIteration<Iterator<MonodirectionalCollection<int>, int, true, false, std::forward_iterator_tag>, MonodirectionalCollection<int>, int>();
  _testForwardIteration<Iterator<MonodirectionalCollection<ObjectMock>, ObjectMock, true, false, std::forward_iterator_tag>, MonodirectionalCollection<ObjectMock>, ObjectMock>();
}

TEST_F(IteratorTest, bidirectionalIterator) {
  _testBidirectionalIteration<Iterator<BidirectionalCollection<int>, int, false, false, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBidirectionalIteration<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, false, false, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();
  _testBidirectionalReversedIteration<Iterator<BidirectionalCollection<int>, int, false, true, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBidirectionalReversedIteration<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, false, true, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();

  _testBidirectionalIteration<Iterator<RandomAccessCollection<int>, int, false, false, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBidirectionalIteration<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, false, false, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
  _testBidirectionalReversedIteration<Iterator<RandomAccessCollection<int>, int, false, true, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBidirectionalReversedIteration<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, false, true, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
  
  _testBidirectionalIteration<Iterator<BidirectionalCollection<int>, int, true, false, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBidirectionalIteration<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, true, false, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();
  _testBidirectionalReversedIteration<Iterator<BidirectionalCollection<int>, int, true, true, std::bidirectional_iterator_tag>, BidirectionalCollection<int>, int>();
  _testBidirectionalReversedIteration<Iterator<BidirectionalCollection<ObjectMock>, ObjectMock, true, true, std::bidirectional_iterator_tag>, BidirectionalCollection<ObjectMock>, ObjectMock>();

  _testBidirectionalIteration<Iterator<RandomAccessCollection<int>, int, true, false, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBidirectionalIteration<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, true, false, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
  _testBidirectionalReversedIteration<Iterator<RandomAccessCollection<int>, int, true, true, std::random_access_iterator_tag>, RandomAccessCollection<int>, int>();
  _testBidirectionalReversedIteration<Iterator<RandomAccessCollection<ObjectMock>, ObjectMock, true, true, std::random_access_iterator_tag>, RandomAccessCollection<ObjectMock>, ObjectMock>();
}
