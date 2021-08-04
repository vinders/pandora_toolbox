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
#include <memory/circular_queue.h>
#include "./_fake_classes_helper.h"

using namespace pandora::memory;

class CircularQueueTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- constructors/accessors --

TEST_F(CircularQueueTest, accessors) {
  CircularQueue<int, 2> baseType;
  CircularQueue<CopyObject, 2> copyType;
  CircularQueue<MoveObject, 2> moveType;
  CircularQueue<CopyMoveObject, 2> copyMoveType;
  EXPECT_EQ(size_t{ 0u }, baseType.size());
  EXPECT_EQ(size_t{ 0u }, copyType.size());
  EXPECT_EQ(size_t{ 0u }, moveType.size());
  EXPECT_EQ(size_t{ 0u }, copyMoveType.size());
  EXPECT_EQ(size_t{ 2u }, baseType.max_size());
  EXPECT_EQ(size_t{ 2u }, copyType.max_size());
  EXPECT_EQ(size_t{ 2u }, moveType.max_size());
  EXPECT_EQ(size_t{ 2u }, copyMoveType.max_size());
  EXPECT_EQ(size_t{ 2u }, baseType.capacity());
  EXPECT_EQ(size_t{ 2u }, copyType.capacity());
  EXPECT_EQ(size_t{ 2u }, moveType.capacity());
  EXPECT_EQ(size_t{ 2u }, copyMoveType.capacity());
  EXPECT_TRUE(baseType.empty());
  EXPECT_TRUE(copyType.empty());
  EXPECT_TRUE(moveType.empty());
  EXPECT_TRUE(copyMoveType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_FALSE(copyType.full());
  EXPECT_FALSE(moveType.full());
  EXPECT_FALSE(copyMoveType.full());

  baseType.emplace_back(42);
  copyType.emplace_back(42);
  moveType.emplace_back(42);
  copyMoveType.emplace_back(42);
  EXPECT_EQ(size_t{ 1u }, baseType.size());
  EXPECT_EQ(size_t{ 1u }, copyType.size());
  EXPECT_EQ(size_t{ 1u }, moveType.size());
  EXPECT_EQ(size_t{ 1u }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_FALSE(copyType.full());
  EXPECT_FALSE(moveType.full());
  EXPECT_FALSE(copyMoveType.full());
  EXPECT_EQ(42, baseType.front());
  EXPECT_EQ(42, copyType.front().value());
  EXPECT_EQ(42, moveType.front().value());
  EXPECT_EQ(42, copyMoveType.front().value());
  EXPECT_EQ(42, baseType.back());
  EXPECT_EQ(42, copyType.back().value());
  EXPECT_EQ(42, moveType.back().value());
  EXPECT_EQ(42, copyMoveType.back().value());

  baseType.emplace_back(21);
  copyType.emplace_back(21);
  moveType.emplace_back(21);
  copyMoveType.emplace_back(21);
  EXPECT_EQ(size_t{ 2u }, baseType.size());
  EXPECT_EQ(size_t{ 2u }, copyType.size());
  EXPECT_EQ(size_t{ 2u }, moveType.size());
  EXPECT_EQ(size_t{ 2u }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_TRUE(copyType.full());
  EXPECT_TRUE(moveType.full());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_EQ(42, baseType.front());
  EXPECT_EQ(42, copyType.front().value());
  EXPECT_EQ(42, moveType.front().value());
  EXPECT_EQ(42, copyMoveType.front().value());
  EXPECT_EQ(21, baseType.back());
  EXPECT_EQ(21, copyType.back().value());
  EXPECT_EQ(21, moveType.back().value());
  EXPECT_EQ(21, copyMoveType.back().value());

  baseType.pop_front();
  copyType.pop_front();
  moveType.pop_front();
  copyMoveType.pop_front();
  EXPECT_EQ(size_t{ 1u }, baseType.size());
  EXPECT_EQ(size_t{ 1u }, copyType.size());
  EXPECT_EQ(size_t{ 1u }, moveType.size());
  EXPECT_EQ(size_t{ 1u }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_FALSE(copyType.full());
  EXPECT_FALSE(moveType.full());
  EXPECT_FALSE(copyMoveType.full());
  EXPECT_EQ(21, baseType.front());
  EXPECT_EQ(21, copyType.front().value());
  EXPECT_EQ(21, moveType.front().value());
  EXPECT_EQ(21, copyMoveType.front().value());
  EXPECT_EQ(21, baseType.back());
  EXPECT_EQ(21, copyType.back().value());
  EXPECT_EQ(21, moveType.back().value());
  EXPECT_EQ(21, copyMoveType.back().value());

  baseType.emplace_back(64);
  copyType.emplace_back(64);
  moveType.emplace_back(64);
  copyMoveType.emplace_back(64);
  EXPECT_EQ(size_t{ 2u }, baseType.size());
  EXPECT_EQ(size_t{ 2u }, copyType.size());
  EXPECT_EQ(size_t{ 2u }, moveType.size());
  EXPECT_EQ(size_t{ 2u }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_TRUE(copyType.full());
  EXPECT_TRUE(moveType.full());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_EQ(21, baseType.front());
  EXPECT_EQ(21, copyType.front().value());
  EXPECT_EQ(21, moveType.front().value());
  EXPECT_EQ(21, copyMoveType.front().value());
  EXPECT_EQ(64, baseType.back());
  EXPECT_EQ(64, copyType.back().value());
  EXPECT_EQ(64, moveType.back().value());
  EXPECT_EQ(64, copyMoveType.back().value());

  baseType.pop_front();
  copyType.pop_front();
  moveType.pop_front();
  copyMoveType.pop_front();
  EXPECT_EQ(size_t{ 1u }, baseType.size());
  EXPECT_EQ(size_t{ 1u }, copyType.size());
  EXPECT_EQ(size_t{ 1u }, moveType.size());
  EXPECT_EQ(size_t{ 1u }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_FALSE(copyType.full());
  EXPECT_FALSE(moveType.full());
  EXPECT_FALSE(copyMoveType.full());
  EXPECT_EQ(64, baseType.front());
  EXPECT_EQ(64, copyType.front().value());
  EXPECT_EQ(64, moveType.front().value());
  EXPECT_EQ(64, copyMoveType.front().value());
  EXPECT_EQ(64, baseType.back());
  EXPECT_EQ(64, copyType.back().value());
  EXPECT_EQ(64, moveType.back().value());
  EXPECT_EQ(64, copyMoveType.back().value());

  baseType.pop_front();
  copyType.pop_front();
  moveType.pop_front();
  copyMoveType.pop_front();
  EXPECT_EQ(size_t{ 0u }, baseType.size());
  EXPECT_EQ(size_t{ 0u }, copyType.size());
  EXPECT_EQ(size_t{ 0u }, moveType.size());
  EXPECT_EQ(size_t{ 0u }, copyMoveType.size());
  EXPECT_TRUE(baseType.empty());
  EXPECT_TRUE(copyType.empty());
  EXPECT_TRUE(moveType.empty());
  EXPECT_TRUE(copyMoveType.empty());
  EXPECT_FALSE(baseType.full());
  EXPECT_FALSE(copyType.full());
  EXPECT_FALSE(moveType.full());
  EXPECT_FALSE(copyMoveType.full());
}
TEST_F(CircularQueueTest, accessorsFullQueue) {
  CircularQueue<int, 20> baseType;
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType;

  for (int i = 0; i < 20; ++i) {
    baseType.push_back(i);
    copyType.push_back(CopyObject(i));
    moveType.push_back(MoveObject(i));
    copyMoveType.push_back(CopyMoveObject(i));
  }
  EXPECT_EQ(size_t{ 20 }, baseType.size());
  EXPECT_EQ(size_t{ 20 }, copyType.size());
  EXPECT_EQ(size_t{ 20 }, moveType.size());
  EXPECT_EQ(size_t{ 20 }, copyMoveType.size());
  EXPECT_EQ(size_t{ 20 }, baseType.max_size());
  EXPECT_EQ(size_t{ 20 }, copyType.max_size());
  EXPECT_EQ(size_t{ 20 }, moveType.max_size());
  EXPECT_EQ(size_t{ 20 }, copyMoveType.max_size());
  EXPECT_EQ(size_t{ 20 }, baseType.capacity());
  EXPECT_EQ(size_t{ 20 }, copyType.capacity());
  EXPECT_EQ(size_t{ 20 }, moveType.capacity());
  EXPECT_EQ(size_t{ 20 }, copyMoveType.capacity());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_TRUE(copyType.full());
  EXPECT_TRUE(moveType.full());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_EQ(0, baseType.front());
  EXPECT_EQ(19, baseType.back());
  EXPECT_EQ(0, copyType.front().value());
  EXPECT_EQ(19, copyType.back().value());
  EXPECT_EQ(0, moveType.front().value());
  EXPECT_EQ(19, moveType.back().value());
  EXPECT_EQ(0, copyMoveType.front().value());
  EXPECT_EQ(19, copyMoveType.back().value());

  baseType.pop_front();
  copyType.pop_front();
  moveType.pop_front();
  copyMoveType.pop_front();
  baseType.push_back(20);
  copyType.push_back(CopyObject(20));
  moveType.push_back(MoveObject(20));
  copyMoveType.push_back(CopyMoveObject(20));
  EXPECT_EQ(size_t{ 20 }, baseType.size());
  EXPECT_EQ(size_t{ 20 }, copyType.size());
  EXPECT_EQ(size_t{ 20 }, moveType.size());
  EXPECT_EQ(size_t{ 20 }, copyMoveType.size());
  EXPECT_FALSE(baseType.empty());
  EXPECT_FALSE(copyType.empty());
  EXPECT_FALSE(moveType.empty());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_TRUE(copyType.full());
  EXPECT_TRUE(moveType.full());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_EQ(1, baseType.front());
  EXPECT_EQ(20, baseType.back());
  EXPECT_EQ(1, copyType.front().value());
  EXPECT_EQ(20, copyType.back().value());
  EXPECT_EQ(1, moveType.front().value());
  EXPECT_EQ(20, moveType.back().value());
  EXPECT_EQ(1, copyMoveType.front().value());
  EXPECT_EQ(20, copyMoveType.back().value());
}

TEST_F(CircularQueueTest, copyEmptyQueue) {
  CircularQueue<int, 20> baseType;
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyMoveObject, 20> copyMoveType;

  CircularQueue<int, 20> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_TRUE(baseTypeCopy.empty());
  CircularQueue<CopyObject, 20> copyTypeCopy(copyType);
  EXPECT_EQ(copyType.size(), copyTypeCopy.size());
  EXPECT_TRUE(copyTypeCopy.empty());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeCopy(copyMoveType);
  EXPECT_EQ(copyMoveType.size(), copyMoveTypeCopy.size());
  EXPECT_TRUE(copyMoveTypeCopy.empty());

  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_TRUE(baseType.empty());
  copyType = copyTypeCopy;
  EXPECT_EQ(copyTypeCopy.size(), copyType.size());
  EXPECT_TRUE(copyType.empty());
  copyMoveType = copyMoveTypeCopy;
  EXPECT_EQ(copyMoveTypeCopy.size(), copyMoveType.size());
  EXPECT_TRUE(copyMoveType.empty());
}
TEST_F(CircularQueueTest, copyQueue) {
  CircularQueue<int, 20> baseType;
  baseType.push_back(5);
  baseType.push_back(7);
  CircularQueue<CopyObject, 20> copyType;
  copyType.push_back(CopyObject(5));
  copyType.push_back(CopyObject(7));
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.push_back(CopyMoveObject(7));

  CircularQueue<int, 20> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_EQ(baseType.front(), baseTypeCopy.front());
  EXPECT_EQ(baseType.back(), baseTypeCopy.back());
  CircularQueue<CopyObject, 20> copyTypeCopy(copyType);
  EXPECT_EQ(copyType.size(), copyTypeCopy.size());
  EXPECT_EQ(copyType.front().value(), copyTypeCopy.front().value());
  EXPECT_EQ(copyType.back().value(), copyTypeCopy.back().value());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeCopy(copyMoveType);
  EXPECT_EQ(copyMoveType.size(), copyMoveTypeCopy.size());
  EXPECT_EQ(copyMoveType.front().value(), copyMoveTypeCopy.front().value());
  EXPECT_EQ(copyMoveType.back().value(), copyMoveTypeCopy.back().value());

  baseType.clear();
  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_EQ(baseTypeCopy.front(), baseType.front());
  EXPECT_EQ(baseTypeCopy.back(), baseType.back());
  copyType.clear();
  copyType = copyTypeCopy;
  EXPECT_EQ(copyTypeCopy.size(), copyType.size());
  EXPECT_EQ(copyTypeCopy.front().value(), copyType.front().value());
  EXPECT_EQ(copyTypeCopy.back().value(), copyType.back().value());
  copyMoveType.clear();
  copyMoveType = copyMoveTypeCopy;
  EXPECT_EQ(copyMoveTypeCopy.size(), copyMoveType.size());
  EXPECT_EQ(copyMoveTypeCopy.front().value(), copyMoveType.front().value());
  EXPECT_EQ(copyMoveTypeCopy.back().value(), copyMoveType.back().value());
}
TEST_F(CircularQueueTest, copyFullQueue) {
  CircularQueue<int, 20> baseType;
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  for (int i = 0; i < 20; ++i) {
    baseType.push_back(i);
    copyType.push_back(CopyObject(i));
    copyMoveType.push_back(CopyMoveObject(i));
  }
  baseType.pop_front();
  copyType.pop_front();
  copyMoveType.pop_front();
  baseType.push_back(20);
  copyType.push_back(CopyObject(20));
  copyMoveType.push_back(CopyMoveObject(20));

  CircularQueue<int, 20> baseTypeCopy(baseType);
  EXPECT_EQ(baseType.size(), baseTypeCopy.size());
  EXPECT_EQ(baseType.front(), baseTypeCopy.front());
  EXPECT_EQ(baseType.back(), baseTypeCopy.back());
  CircularQueue<CopyObject, 20> copyTypeCopy(copyType);
  EXPECT_EQ(copyType.size(), copyTypeCopy.size());
  EXPECT_EQ(copyType.front().value(), copyTypeCopy.front().value());
  EXPECT_EQ(copyType.back().value(), copyTypeCopy.back().value());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeCopy(copyMoveType);
  EXPECT_EQ(copyMoveType.size(), copyMoveTypeCopy.size());
  EXPECT_EQ(copyMoveType.front().value(), copyMoveTypeCopy.front().value());
  EXPECT_EQ(copyMoveType.back().value(), copyMoveTypeCopy.back().value());

  baseType.clear();
  EXPECT_TRUE(baseType.empty());
  baseType = baseTypeCopy;
  EXPECT_EQ(baseTypeCopy.size(), baseType.size());
  EXPECT_EQ(baseTypeCopy.front(), baseType.front());
  EXPECT_EQ(baseTypeCopy.back(), baseType.back());
  copyType.clear();
  EXPECT_TRUE(copyType.empty());
  copyType = copyTypeCopy;
  EXPECT_EQ(copyTypeCopy.size(), copyType.size());
  EXPECT_EQ(copyTypeCopy.front().value(), copyType.front().value());
  EXPECT_EQ(copyTypeCopy.back().value(), copyType.back().value());
  copyMoveType.clear();
  EXPECT_TRUE(copyMoveType.empty());
  copyMoveType = copyMoveTypeCopy;
  EXPECT_EQ(copyMoveTypeCopy.size(), copyMoveType.size());
  EXPECT_EQ(copyMoveTypeCopy.front().value(), copyMoveType.front().value());
  EXPECT_EQ(copyMoveTypeCopy.back().value(), copyMoveType.back().value());
}

TEST_F(CircularQueueTest, moveEmptyQueue) {
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType;

  CircularQueue<MoveObject, 20> moveTypeCopy(std::move(moveType));
  EXPECT_TRUE(moveType.empty());
  EXPECT_TRUE(moveTypeCopy.empty());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeCopy(std::move(copyMoveType));
  EXPECT_TRUE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveTypeCopy.empty());

  moveType.clear();
  moveType = std::move(moveTypeCopy);
  EXPECT_TRUE(moveType.empty());
  EXPECT_TRUE(moveTypeCopy.empty());
  copyMoveType.clear();
  copyMoveType = std::move(copyMoveTypeCopy);
  EXPECT_TRUE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveTypeCopy.empty());
}
TEST_F(CircularQueueTest, moveQueue) {
  CircularQueue<MoveObject, 20> moveType;
  moveType.push_back(MoveObject(5));
  moveType.push_back(MoveObject(7));
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.push_back(CopyMoveObject(7));

  CircularQueue<MoveObject, 20> moveTypeMoved(std::move(moveType));
  EXPECT_TRUE(moveType.empty());
  EXPECT_EQ(size_t{ 2u }, moveTypeMoved.size());
  EXPECT_EQ(5, moveTypeMoved.front().value());
  EXPECT_EQ(7, moveTypeMoved.back().value());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeMoved(std::move(copyMoveType));
  EXPECT_TRUE(copyMoveType.empty());
  EXPECT_EQ(size_t{ 2u }, copyMoveTypeMoved.size());
  EXPECT_EQ(5, copyMoveTypeMoved.front().value());
  EXPECT_EQ(7, copyMoveTypeMoved.back().value());

  moveType = std::move(moveTypeMoved);
  EXPECT_TRUE(moveTypeMoved.empty());
  EXPECT_EQ(size_t{ 2u }, moveType.size());
  EXPECT_EQ(5, moveType.front().value());
  EXPECT_EQ(7, moveType.back().value());
  copyMoveType = std::move(copyMoveTypeMoved);
  EXPECT_TRUE(copyMoveTypeMoved.empty());
  EXPECT_EQ(size_t{ 2u }, copyMoveType.size());
  EXPECT_EQ(5, copyMoveType.front().value());
  EXPECT_EQ(7, copyMoveType.back().value());
}
TEST_F(CircularQueueTest, moveFullQueue) {
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  for (int i = 0; i < 20; ++i) {
    moveType.push_back(MoveObject(i));
    copyMoveType.push_back(CopyMoveObject(i));
  }
  EXPECT_FALSE(moveType.empty());
  EXPECT_TRUE(moveType.full());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());
  moveType.pop_front();
  copyMoveType.pop_front();
  moveType.push_back(MoveObject(20));
  copyMoveType.push_back(CopyMoveObject(20));
  EXPECT_FALSE(moveType.empty());
  EXPECT_TRUE(moveType.full());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());

  CircularQueue<MoveObject, 20> moveTypeMoved(std::move(moveType));
  EXPECT_FALSE(moveTypeMoved.empty());
  EXPECT_TRUE(moveTypeMoved.full());
  EXPECT_EQ(size_t{ 20u }, moveTypeMoved.size());
  EXPECT_EQ(1, moveTypeMoved.front().value());
  EXPECT_EQ(20, moveTypeMoved.back().value());
  CircularQueue<CopyMoveObject, 20> copyMoveTypeMoved(std::move(copyMoveType));
  EXPECT_FALSE(copyMoveTypeMoved.empty());
  EXPECT_TRUE(copyMoveTypeMoved.full());
  EXPECT_EQ(size_t{ 20u }, copyMoveTypeMoved.size());
  EXPECT_EQ(1, copyMoveTypeMoved.front().value());
  EXPECT_EQ(20, copyMoveTypeMoved.back().value());

  moveType = std::move(moveTypeMoved);
  EXPECT_FALSE(moveType.empty());
  EXPECT_TRUE(moveType.full());
  EXPECT_EQ(size_t{ 20u }, moveType.size());
  EXPECT_EQ(1, moveType.front().value());
  EXPECT_EQ(20, moveType.back().value());
  copyMoveType = std::move(copyMoveTypeMoved);
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_EQ(size_t{ 20u }, copyMoveType.size());
  EXPECT_EQ(1, copyMoveType.front().value());
  EXPECT_EQ(20, copyMoveType.back().value());
}

TEST_F(CircularQueueTest, swapEmptyQueues) {
  CircularQueue<int, 20> baseType;
  CircularQueue<int, 20> baseType2;
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyObject, 20> copyType2;
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<MoveObject, 20> moveType2;
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType2;

  baseType.swap(baseType2);
  EXPECT_EQ(size_t{ 0 }, baseType.size());
  EXPECT_EQ(size_t{ 0 }, baseType2.size());
  baseType2.swap(baseType);
  EXPECT_EQ(size_t{ 0 }, baseType.size());
  EXPECT_EQ(size_t{ 0 }, baseType2.size());

  copyType.swap(copyType2);
  EXPECT_EQ(size_t{ 0 }, copyType.size());
  EXPECT_EQ(size_t{ 0 }, copyType2.size());
  copyType2.swap(copyType);
  EXPECT_EQ(size_t{ 0 }, copyType.size());
  EXPECT_EQ(size_t{ 0 }, copyType2.size());

  moveType.swap(moveType2);
  EXPECT_EQ(size_t{ 0 }, moveType.size());
  EXPECT_EQ(size_t{ 0 }, moveType2.size());
  moveType2.swap(moveType);
  EXPECT_EQ(size_t{ 0 }, moveType.size());
  EXPECT_EQ(size_t{ 0 }, moveType2.size());

  copyMoveType.swap(copyMoveType2);
  EXPECT_EQ(size_t{ 0 }, copyMoveType.size());
  EXPECT_EQ(size_t{ 0 }, copyMoveType2.size());
  copyMoveType2.swap(copyMoveType);
  EXPECT_EQ(size_t{ 0 }, copyMoveType.size());
  EXPECT_EQ(size_t{ 0 }, copyMoveType2.size());
}
TEST_F(CircularQueueTest, swapEmptyAndNonEmptyQueues) {
  CircularQueue<int, 20> baseType;
  CircularQueue<int, 20> baseType2;
  baseType.push_back(5);
  baseType.push_back(7);
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyObject, 20> copyType2;
  copyType.push_back(CopyObject(5));
  copyType.push_back(CopyObject(7));
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<MoveObject, 20> moveType2;
  moveType.push_back(MoveObject(5));
  moveType.push_back(MoveObject(7));
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType2;
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.push_back(CopyMoveObject(7));

  baseType.swap(baseType2);
  EXPECT_EQ(size_t{ 0 }, baseType.size());
  EXPECT_EQ(size_t{ 2 }, baseType2.size());
  EXPECT_EQ(5, baseType2.front());
  EXPECT_EQ(7, baseType2.back());
  baseType2.swap(baseType);
  EXPECT_EQ(size_t{ 2 }, baseType.size());
  EXPECT_EQ(5, baseType.front());
  EXPECT_EQ(7, baseType.back());
  EXPECT_EQ(size_t{ 0 }, baseType2.size());

  copyType.swap(copyType2);
  EXPECT_EQ(size_t{ 0 }, copyType.size());
  EXPECT_EQ(size_t{ 2 }, copyType2.size());
  EXPECT_EQ(5, copyType2.front().value());
  EXPECT_EQ(7, copyType2.back().value());
  copyType2.swap(copyType);
  EXPECT_EQ(size_t{ 2 }, copyType.size());
  EXPECT_EQ(5, copyType.front().value());
  EXPECT_EQ(7, copyType.back().value());
  EXPECT_EQ(size_t{ 0 }, copyType2.size());

  moveType.swap(moveType2);
  EXPECT_EQ(size_t{ 0 }, moveType.size());
  EXPECT_EQ(size_t{ 2 }, moveType2.size());
  EXPECT_EQ(5, moveType2.front().value());
  EXPECT_EQ(7, moveType2.back().value());
  moveType2.swap(moveType);
  EXPECT_EQ(size_t{ 2 }, moveType.size());
  EXPECT_EQ(5, moveType.front().value());
  EXPECT_EQ(7, moveType.back().value());
  EXPECT_EQ(size_t{ 0 }, moveType2.size());

  copyMoveType.swap(copyMoveType2);
  EXPECT_EQ(size_t{ 0 }, copyMoveType.size());
  EXPECT_EQ(size_t{ 2 }, copyMoveType2.size());
  EXPECT_EQ(5, copyMoveType2.front().value());
  EXPECT_EQ(7, copyMoveType2.back().value());
  copyMoveType2.swap(copyMoveType);
  EXPECT_EQ(size_t{ 2 }, copyMoveType.size());
  EXPECT_EQ(5, copyMoveType.front().value());
  EXPECT_EQ(7, copyMoveType.back().value());
  EXPECT_EQ(size_t{ 0 }, copyMoveType2.size());
}
TEST_F(CircularQueueTest, swapQueues) {
  CircularQueue<int, 20> baseType;
  CircularQueue<int, 20> baseType2;
  baseType.push_back(5);
  baseType.pop_front();
  baseType.push_back(5);
  baseType.push_back(7);
  baseType2.push_back(9);
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyObject, 20> copyType2;
  copyType.push_back(CopyObject(5));
  copyType.pop_front();
  copyType.push_back(CopyObject(5));
  copyType.push_back(CopyObject(7));
  copyType2.push_back(CopyObject(9));
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<MoveObject, 20> moveType2;
  moveType.push_back(MoveObject(5));
  moveType.pop_front();
  moveType.push_back(MoveObject(5));
  moveType.push_back(MoveObject(7));
  moveType2.push_back(MoveObject(9));
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType2;
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.pop_front();
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.push_back(CopyMoveObject(7));
  copyMoveType2.push_back(CopyMoveObject(9));

  baseType.swap(baseType2);
  EXPECT_EQ(size_t{ 1 }, baseType.size());
  EXPECT_EQ(9, baseType.front());
  EXPECT_EQ(9, baseType.back());
  EXPECT_EQ(size_t{ 2 }, baseType2.size());
  EXPECT_EQ(5, baseType2.front());
  EXPECT_EQ(7, baseType2.back());
  baseType2.swap(baseType);
  EXPECT_EQ(size_t{ 2 }, baseType.size());
  EXPECT_EQ(5, baseType.front());
  EXPECT_EQ(7, baseType.back());
  EXPECT_EQ(size_t{ 1 }, baseType2.size());
  EXPECT_EQ(9, baseType2.front());

  copyType.swap(copyType2);
  EXPECT_EQ(size_t{ 1 }, copyType.size());
  EXPECT_EQ(9, copyType.front().value());
  EXPECT_EQ(9, copyType.back().value());
  EXPECT_EQ(size_t{ 2 }, copyType2.size());
  EXPECT_EQ(5, copyType2.front().value());
  EXPECT_EQ(7, copyType2.back().value());
  copyType2.swap(copyType);
  EXPECT_EQ(size_t{ 2 }, copyType.size());
  EXPECT_EQ(5, copyType.front().value());
  EXPECT_EQ(7, copyType.back().value());
  EXPECT_EQ(size_t{ 1 }, copyType2.size());
  EXPECT_EQ(9, copyType2.front().value());
  EXPECT_EQ(9, copyType2.back().value());

  moveType.swap(moveType2);
  EXPECT_EQ(size_t{ 1 }, moveType.size());
  EXPECT_EQ(9, moveType.front().value());
  EXPECT_EQ(size_t{ 2 }, moveType2.size());
  EXPECT_EQ(5, moveType2.front().value());
  EXPECT_EQ(7, moveType2.back().value());
  moveType2.swap(moveType);
  EXPECT_EQ(size_t{ 2 }, moveType.size());
  EXPECT_EQ(5, moveType.front().value());
  EXPECT_EQ(7, moveType.back().value());
  EXPECT_EQ(size_t{ 1 }, moveType2.size());
  EXPECT_EQ(9, moveType2.front().value());
  
  copyMoveType.swap(copyMoveType2);
  EXPECT_EQ(size_t{ 1 }, copyMoveType.size());
  EXPECT_EQ(9, copyMoveType.front().value());
  EXPECT_EQ(9, copyMoveType.back().value());
  EXPECT_EQ(size_t{ 2 }, copyMoveType2.size());
  EXPECT_EQ(5, copyMoveType2.front().value());
  EXPECT_EQ(7, copyMoveType2.back().value());
  copyMoveType2.swap(copyMoveType);
  EXPECT_EQ(size_t{ 2 }, copyMoveType.size());
  EXPECT_EQ(5, copyMoveType.front().value());
  EXPECT_EQ(7, copyMoveType.back().value());
  EXPECT_EQ(size_t{ 1 }, copyMoveType2.size());
  EXPECT_EQ(9, copyMoveType2.front().value());
  EXPECT_EQ(9, copyMoveType2.back().value());
}
TEST_F(CircularQueueTest, swapFullQueues) {
  CircularQueue<int, 20> baseType;
  CircularQueue<int, 20> baseType2;
  baseType.push_back(5);
  baseType.pop_front();
  for (int i = 0; i < 20; ++i) {
    baseType.push_back(i);
    baseType2.push_back(i + 1);
  }
  CircularQueue<CopyObject, 20> copyType;
  CircularQueue<CopyObject, 20> copyType2;
  copyType.push_back(CopyObject(5));
  copyType.pop_front();
  for (int i = 0; i < 20; ++i) {
    copyType.push_back(CopyObject(i));
    copyType2.push_back(CopyObject(i + 1));
  }
  CircularQueue<MoveObject, 20> moveType;
  CircularQueue<MoveObject, 20> moveType2;
  moveType.push_back(MoveObject(5));
  moveType.pop_front();
  for (int i = 0; i < 20; ++i) {
    moveType.push_back(MoveObject(i));
    moveType2.push_back(MoveObject(i + 1));
  }
  CircularQueue<CopyMoveObject, 20> copyMoveType;
  CircularQueue<CopyMoveObject, 20> copyMoveType2;
  copyMoveType.push_back(CopyMoveObject(5));
  copyMoveType.pop_front();
  for (int i = 0; i < 20; ++i) {
    copyMoveType.push_back(CopyMoveObject(i));
    copyMoveType2.push_back(CopyMoveObject(i + 1));
  }

  baseType.swap(baseType2);
  EXPECT_EQ(size_t{ 20u }, baseType.size());
  EXPECT_EQ(1, baseType.front());
  EXPECT_EQ(20, baseType.back());
  EXPECT_EQ(size_t{ 20u }, baseType2.size());
  EXPECT_EQ(0, baseType2.front());
  EXPECT_EQ(19, baseType2.back());
  EXPECT_FALSE(baseType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_FALSE(baseType2.empty());
  EXPECT_TRUE(baseType2.full());
  baseType2.swap(baseType);
  EXPECT_EQ(size_t{ 20u }, baseType.size());
  EXPECT_EQ(0, baseType.front());
  EXPECT_EQ(19, baseType.back());
  EXPECT_EQ(size_t{ 20u }, baseType2.size());
  EXPECT_EQ(1, baseType2.front());
  EXPECT_EQ(20, baseType2.back());
  EXPECT_FALSE(baseType.empty());
  EXPECT_TRUE(baseType.full());
  EXPECT_FALSE(baseType2.empty());
  EXPECT_TRUE(baseType2.full());

  copyType.swap(copyType2);
  EXPECT_EQ(size_t{ 20u }, copyType.size());
  EXPECT_EQ(1, copyType.front().value());
  EXPECT_EQ(20, copyType.back().value());
  EXPECT_EQ(size_t{ 20u }, copyType2.size());
  EXPECT_EQ(0, copyType2.front().value());
  EXPECT_EQ(19, copyType2.back().value());
  EXPECT_FALSE(copyType.empty());
  EXPECT_TRUE(copyType.full());
  EXPECT_FALSE(copyType2.empty());
  EXPECT_TRUE(copyType2.full());
  copyType2.swap(copyType);
  EXPECT_EQ(size_t{ 20u }, copyType.size());
  EXPECT_EQ(0, copyType.front().value());
  EXPECT_EQ(19, copyType.back().value());
  EXPECT_EQ(size_t{ 20u }, copyType2.size());
  EXPECT_EQ(1, copyType2.front().value());
  EXPECT_EQ(20, copyType2.back().value());
  EXPECT_FALSE(copyType.empty());
  EXPECT_TRUE(copyType.full());
  EXPECT_FALSE(copyType2.empty());
  EXPECT_TRUE(copyType2.full());

  moveType.swap(moveType2);
  EXPECT_EQ(size_t{ 20u }, moveType.size());
  EXPECT_EQ(1, moveType.front().value());
  EXPECT_EQ(20, moveType.back().value());
  EXPECT_EQ(size_t{ 20u }, moveType2.size());
  EXPECT_EQ(0, moveType2.front().value());
  EXPECT_EQ(19, moveType2.back().value());
  EXPECT_FALSE(moveType.empty());
  EXPECT_TRUE(moveType.full());
  EXPECT_FALSE(moveType2.empty());
  EXPECT_TRUE(moveType2.full());
  moveType2.swap(moveType);
  EXPECT_EQ(size_t{ 20u }, moveType.size());
  EXPECT_EQ(0, moveType.front().value());
  EXPECT_EQ(19, moveType.back().value());
  EXPECT_EQ(size_t{ 20u }, moveType2.size());
  EXPECT_EQ(1, moveType2.front().value());
  EXPECT_EQ(20, moveType2.back().value());
  EXPECT_FALSE(moveType.empty());
  EXPECT_TRUE(moveType.full());
  EXPECT_FALSE(moveType2.empty());
  EXPECT_TRUE(moveType2.full());

  copyMoveType.swap(copyMoveType2);
  EXPECT_EQ(size_t{ 20u }, copyMoveType.size());
  EXPECT_EQ(1, copyMoveType.front().value());
  EXPECT_EQ(20, copyMoveType.back().value());
  EXPECT_EQ(size_t{ 20u }, copyMoveType2.size());
  EXPECT_EQ(0, copyMoveType2.front().value());
  EXPECT_EQ(19, copyMoveType2.back().value());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_FALSE(copyMoveType2.empty());
  EXPECT_TRUE(copyMoveType2.full());
  copyMoveType2.swap(copyMoveType);
  EXPECT_EQ(size_t{ 20u }, copyMoveType.size());
  EXPECT_EQ(0, copyMoveType.front().value());
  EXPECT_EQ(19, copyMoveType.back().value());
  EXPECT_EQ(size_t{ 20u }, copyMoveType2.size());
  EXPECT_EQ(1, copyMoveType2.front().value());
  EXPECT_EQ(20, copyMoveType2.back().value());
  EXPECT_FALSE(copyMoveType.empty());
  EXPECT_TRUE(copyMoveType.full());
  EXPECT_FALSE(copyMoveType2.empty());
  EXPECT_TRUE(copyMoveType2.full());
}


// -- operations --

template <typename _DataType>
void _pushPopOneQueue(int val) {
  CircularQueue<_DataType,1> data1;
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
  EXPECT_FALSE(data1.pop());

  EXPECT_TRUE(data1.push_back(_DataType(val)));
  EXPECT_EQ(size_t{ 1u }, data1.size());
  EXPECT_EQ(_DataType(val), data1.front());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType,1>*)&data1)->front());
  EXPECT_EQ(_DataType(val), data1.back());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType,1>*)&data1)->back());
  EXPECT_FALSE(data1.empty());
  EXPECT_TRUE(data1.full());
  EXPECT_FALSE(data1.push(_DataType(val)));

  EXPECT_TRUE(data1.pop_front());
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
  EXPECT_FALSE(data1.pop());
}

template <typename _QueueType, typename _DataType>
void _pushPopMultipleInQueue(int val1, int val2) {
  _QueueType data2;
  EXPECT_FALSE(data2.pop());
  EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*) & data2)->front());
  EXPECT_EQ(_DataType(val1), data2.back());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*) & data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());

  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_TRUE(data2.push_back(_DataType(val2)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_FALSE(data2.push_back(_DataType(val1)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());

  data2.clear(); // reset head and tail
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_TRUE(data2.push_back(_DataType(val2)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_FALSE(data2.push_back(_DataType(val1)));
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());
}

template <typename _DataType>
void _emplacePopOneQueue(int val) {
  CircularQueue<_DataType, 1> data1;
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
  EXPECT_FALSE(data1.pop());

  EXPECT_TRUE(data1.emplace_back(val));
  EXPECT_EQ(size_t{ 1u }, data1.size());
  EXPECT_EQ(_DataType(val), data1.front());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType, 1>*) & data1)->front());
  EXPECT_EQ(_DataType(val), data1.back());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType, 1>*) & data1)->back());
  EXPECT_FALSE(data1.empty());
  EXPECT_TRUE(data1.full());
  EXPECT_FALSE(data1.emplace(val));

  EXPECT_TRUE(data1.pop_front());
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
  EXPECT_FALSE(data1.pop());
}

template <typename _QueueType, typename _DataType>
void _emplacePopMultipleInQueue(int val1, int val2) {
  _QueueType data2;
  EXPECT_FALSE(data2.pop());
  EXPECT_TRUE(data2.emplace(val1));
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val1), data2.back());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());

  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.emplace(val1));
  EXPECT_TRUE(data2.emplace_back(val2));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_FALSE(data2.emplace_back(val1));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());

  data2.clear(); // reset head and tail
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.emplace(val1));
  EXPECT_TRUE(data2.emplace_back(val2));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_FALSE(data2.emplace_back(val1));
  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_FALSE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());
}

template <typename _DataType>
void _pushClearOneQueue(int val) {
  CircularQueue<_DataType, 1> data1;
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());

  data1.clear();
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());

  EXPECT_TRUE(data1.push_back(_DataType(val)));
  EXPECT_EQ(size_t{ 1u }, data1.size());
  EXPECT_EQ(_DataType(val), data1.front());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType, 1>*) & data1)->front());
  EXPECT_EQ(_DataType(val), data1.back());
  EXPECT_EQ(_DataType(val), ((const CircularQueue<_DataType, 1>*) & data1)->back());
  EXPECT_FALSE(data1.empty());
  EXPECT_TRUE(data1.full());
  EXPECT_FALSE(data1.push(_DataType(val)));

  data1.clear();
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
  
  data1.clear();
  EXPECT_EQ(size_t{ 0u }, data1.size());
  EXPECT_TRUE(data1.empty());
  EXPECT_FALSE(data1.full());
}

template <typename _QueueType, typename _DataType>
void _pushClearMultipleInQueue(int val1, int val2) {
  _QueueType data2;
  EXPECT_FALSE(data2.pop());
  EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_EQ(size_t{ 1u }, data2.size());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(size_t{ 0u }, data2.size());

  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_TRUE(data2.push_back(_DataType(val2)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_FALSE(data2.push_back(_DataType(val1)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_EQ(_DataType(val1), data2.front());
  EXPECT_EQ(_DataType(val1), ((const _QueueType*)&data2)->front());
  EXPECT_EQ(_DataType(val2), data2.back());
  EXPECT_EQ(_DataType(val2), ((const _QueueType*)&data2)->back());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  data2.clear();
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());

  for (size_t i = data2.capacity() - 1u; i > 0; --i)
    EXPECT_TRUE(data2.push(_DataType(val1)));
  EXPECT_TRUE(data2.push_back(_DataType(val2)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_FALSE(data2.push_back(_DataType(val1)));
  EXPECT_EQ(data2.capacity(), data2.size());
  EXPECT_FALSE(data2.empty());
  EXPECT_TRUE(data2.full());
  EXPECT_TRUE(data2.pop_front());
  EXPECT_EQ(data2.capacity() - 1u, data2.size());
  data2.clear();
  EXPECT_EQ(size_t{ 0u }, data2.size());
  EXPECT_TRUE(data2.empty());
  EXPECT_FALSE(data2.full());
  EXPECT_FALSE(data2.pop());
}

TEST_F(CircularQueueTest, pushPop) {
  _pushPopOneQueue<int>(5);
  _pushPopMultipleInQueue<CircularQueue<int,2>, int>(5, 7);
  _pushPopMultipleInQueue<CircularQueue<int,20>, int>(5, 7);

  _pushPopOneQueue<CopyObject>(5);
  _pushPopMultipleInQueue<CircularQueue<CopyObject,2>, CopyObject>(5, 7);
  _pushPopMultipleInQueue<CircularQueue<CopyObject,20>, CopyObject>(5, 7);

  _pushPopOneQueue<MoveObject>(5);
  _pushPopMultipleInQueue<CircularQueue<MoveObject,2>, MoveObject>(5, 7);
  _pushPopMultipleInQueue<CircularQueue<MoveObject,20>, MoveObject>(5, 7);

  _pushPopOneQueue<CopyMoveObject>(5);
  _pushPopMultipleInQueue<CircularQueue<CopyMoveObject,2>, CopyMoveObject>(5, 7);
  _pushPopMultipleInQueue<CircularQueue<CopyMoveObject,20>, CopyMoveObject>(5, 7);
}
TEST_F(CircularQueueTest, emplacePop) {
  _emplacePopOneQueue<int>(5);
  _emplacePopMultipleInQueue<CircularQueue<int, 2>, int>(5, 7);
  _emplacePopMultipleInQueue<CircularQueue<int, 20>, int>(5, 7);

  _emplacePopOneQueue<CopyObject>(5);
  _emplacePopMultipleInQueue<CircularQueue<CopyObject, 2>, CopyObject>(5, 7);
  _emplacePopMultipleInQueue<CircularQueue<CopyObject, 20>, CopyObject>(5, 7);

  _emplacePopOneQueue<MoveObject>(5);
  _emplacePopMultipleInQueue<CircularQueue<MoveObject, 2>, MoveObject>(5, 7);
  _emplacePopMultipleInQueue<CircularQueue<MoveObject, 20>, MoveObject>(5, 7);

  _emplacePopOneQueue<CopyMoveObject>(5);
  _emplacePopMultipleInQueue<CircularQueue<CopyMoveObject, 2>, CopyMoveObject>(5, 7);
  _emplacePopMultipleInQueue<CircularQueue<CopyMoveObject, 20>, CopyMoveObject>(5, 7);
}
TEST_F(CircularQueueTest, pushClear) {
  _pushClearOneQueue<int>(5);
  _pushClearMultipleInQueue<CircularQueue<int, 2>, int>(5, 7);
  _pushClearMultipleInQueue<CircularQueue<int, 20>, int>(5, 7);

  _pushClearOneQueue<CopyObject>(5);
  _pushClearMultipleInQueue<CircularQueue<CopyObject, 2>, CopyObject>(5, 7);
  _pushClearMultipleInQueue<CircularQueue<CopyObject, 20>, CopyObject>(5, 7);

  _pushClearOneQueue<MoveObject>(5);
  _pushClearMultipleInQueue<CircularQueue<MoveObject, 2>, MoveObject>(5, 7);
  _pushClearMultipleInQueue<CircularQueue<MoveObject, 20>, MoveObject>(5, 7);

  _pushClearOneQueue<CopyMoveObject>(5);
  _pushClearMultipleInQueue<CircularQueue<CopyMoveObject, 2>, CopyMoveObject>(5, 7);
  _pushClearMultipleInQueue<CircularQueue<CopyMoveObject, 20>, CopyMoveObject>(5, 7);
}


// -- iteration --

template <typename _DataType, size_t _Size>
void _iterateEmptyQueue() {
  CircularQueue<_DataType, _Size> data;
  const CircularQueue<_DataType, _Size>& cdata{ data };

  auto it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  auto cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  auto crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());

  auto ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
}

template <typename _DataType, size_t _Size>
void _iterateOneQueue() {
  CircularQueue<_DataType, _Size> data;
  const CircularQueue<_DataType, _Size>& cdata{ data };
  data.emplace(42);

  auto it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  EXPECT_EQ(_DataType(42), *it);
  EXPECT_EQ(_DataType(42), it.data());
  EXPECT_EQ(0, it.index());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  auto cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  EXPECT_EQ(_DataType(42), *cit);
  EXPECT_EQ(_DataType(42), cit.data());
  EXPECT_EQ(0, cit.index());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  auto crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  EXPECT_EQ(_DataType(42), *crefit);
  EXPECT_EQ(_DataType(42), crefit.data());
  EXPECT_EQ(0, crefit.index());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());

  auto ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  EXPECT_EQ(_DataType(42), *ccrefit);
  EXPECT_EQ(_DataType(42), ccrefit.data());
  EXPECT_EQ(0, ccrefit.index());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.pop();

  it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  ++it;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  it++;
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());

  cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  ++cit;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  cit++;
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());

  crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ++crefit;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  crefit++;
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());

  ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ++ccrefit;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
  ccrefit++;
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
}

template <typename _DataType, size_t _Size>
void _iterateMultipleInQueue() {
  CircularQueue<_DataType, _Size> data;
  const CircularQueue<_DataType, _Size>& cdata{ data };
  data.emplace(42); // move tail
  data.pop();
  for (int i = 0; i < static_cast<int>(data.capacity()); ++i)
    data.emplace(i);

  auto it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  auto cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  auto crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  auto ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  int index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index), *it);
    EXPECT_EQ(_DataType(index), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    ++it;
    ++cit;
    ++crefit;
    ++ccrefit;
    ++index;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.pop();

  it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index + 1), *it);
    EXPECT_EQ(_DataType(index + 1), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index + 1), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index + 1), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index + 1), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it++;
    cit++;
    crefit++;
    ccrefit++;
    index++;
  }
  EXPECT_EQ(index, static_cast<int>(data.size()));
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  it = data.begin();
  EXPECT_FALSE(it == data.end());
  EXPECT_TRUE(it != data.end());
  cit = data.cbegin();
  EXPECT_FALSE(cit == data.cend());
  EXPECT_TRUE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_FALSE(crefit == cdata.end());
  EXPECT_TRUE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_FALSE(ccrefit == cdata.cend());
  EXPECT_TRUE(ccrefit != cdata.cend());
  index = 0;
  while (it != data.end()) {
    EXPECT_EQ(_DataType(index + 1), *it);
    EXPECT_EQ(_DataType(index + 1), it.data());
    EXPECT_EQ(index, it.index());
    EXPECT_EQ(_DataType(index + 1), cit.data());
    EXPECT_EQ(index, cit.index());
    EXPECT_EQ(_DataType(index + 1), crefit.data());
    EXPECT_EQ(index, crefit.index());
    EXPECT_EQ(_DataType(index + 1), ccrefit.data());
    EXPECT_EQ(index, ccrefit.index());
    it += 4;
    cit += 4;
    crefit += 4;
    ccrefit += 4;
    index += 4;
  }
  EXPECT_TRUE(index / 4 == (static_cast<int>(data.size()) + 3) / 4);
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());

  data.clear();

  it = data.begin();
  EXPECT_TRUE(it == data.end());
  EXPECT_FALSE(it != data.end());
  cit = data.cbegin();
  EXPECT_TRUE(cit == data.cend());
  EXPECT_FALSE(cit != data.cend());
  crefit = cdata.begin();
  EXPECT_TRUE(crefit == cdata.end());
  EXPECT_FALSE(crefit != cdata.end());
  ccrefit = cdata.cbegin();
  EXPECT_TRUE(ccrefit == cdata.cend());
  EXPECT_FALSE(ccrefit != cdata.cend());
}

TEST_F(CircularQueueTest, iterateEmpty) {
  _iterateEmptyQueue<int, 1>();
  _iterateEmptyQueue<int, 2>();
  _iterateEmptyQueue<int, 20>();

  _iterateEmptyQueue<CopyObject, 1>();
  _iterateEmptyQueue<CopyObject, 2>();
  _iterateEmptyQueue<CopyObject, 20>();

  _iterateEmptyQueue<MoveObject, 1>();
  _iterateEmptyQueue<MoveObject, 2>();
  _iterateEmptyQueue<MoveObject, 20>();

  _iterateEmptyQueue<CopyMoveObject, 1>();
  _iterateEmptyQueue<CopyMoveObject, 2>();
  _iterateEmptyQueue<CopyMoveObject, 20>();
}
TEST_F(CircularQueueTest, iterateOne) {
  _iterateOneQueue<int, 1>();
  _iterateOneQueue<int, 2>();
  _iterateOneQueue<int, 20>();

  _iterateOneQueue<CopyObject, 1>();
  _iterateOneQueue<CopyObject, 2>();
  _iterateOneQueue<CopyObject, 20>();

  _iterateOneQueue<MoveObject, 1>();
  _iterateOneQueue<MoveObject, 2>();
  _iterateOneQueue<MoveObject, 20>();

  _iterateOneQueue<CopyMoveObject, 1>();
  _iterateOneQueue<CopyMoveObject, 2>();
  _iterateOneQueue<CopyMoveObject, 20>();
}
TEST_F(CircularQueueTest, iterateMultiple) {
  _iterateMultipleInQueue<int, 2>();
  _iterateMultipleInQueue<int, 20>();

  _iterateMultipleInQueue<CopyObject, 2>();
  _iterateMultipleInQueue<CopyObject, 20>();

  _iterateMultipleInQueue<MoveObject, 2>();
  _iterateMultipleInQueue<MoveObject, 20>();

  _iterateMultipleInQueue<CopyMoveObject, 2>();
  _iterateMultipleInQueue<CopyMoveObject, 20>();
}
