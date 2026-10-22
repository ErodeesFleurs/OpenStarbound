import star.byte_array;
import std;
import star.string;
import star.encode;

#include "gtest/gtest.h"

using namespace Star;

TEST(ByteArrayTest, All) {
  auto res = ByteArray::fromCString("foobar");
  res.insert(2, 'a');
  res.insert(6, 'b');
  res.push_back('c');
  res.insert(9, 'd');
  EXPECT_EQ(res, ByteArray::fromCString("foaobabrcd"));

  auto a = hexDecode("0a0a0a");
  auto b = hexDecode("a0a0a0");
  auto c = hexDecode("818181");
  auto d = hexDecode("aaaaaa");
  auto e = hexDecode("000000");
  auto f = hexDecode("212121");

  auto g = hexDecode("a0a0a0");
  auto h = hexDecode("8181818181");
  auto i = hexDecode("2121218181");
  ASSERT_TRUE(a);
  ASSERT_TRUE(b);
  ASSERT_TRUE(c);
  ASSERT_TRUE(d);
  ASSERT_TRUE(e);
  ASSERT_TRUE(f);
  ASSERT_TRUE(g);
  ASSERT_TRUE(h);
  ASSERT_TRUE(i);

  EXPECT_EQ(a->andWith(*b), *e);
  EXPECT_EQ(a->orWith(*b), *d);
  EXPECT_EQ(b->xorWith(*c), *f);
  EXPECT_EQ(g->xorWith(*h), *f);
  EXPECT_EQ(g->xorWith(*h, true), *i);
  EXPECT_EQ(h->xorWith(*g, true), *i);
}
