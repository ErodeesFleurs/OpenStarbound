import std;
import star.format;
import star.list;
import star.io_device;
import star.set;
import star.btree;
import star.ordered_map;
import star.lru_cache;
import star.data_stream_devices;
import star.thread;
import star.btree_database;
import star.file;
import star.random;

#include "gtest/gtest.h"

import star.byte_array;
import star.bytes;
import star.algorithm;
using namespace Star;

namespace {
  std::size_t const RandFactor = 0xd5a2f037;
  std::size_t const MaxSize = 400;
  std::uint32_t const MaxKey = 100000;

  ByteArray toByteArray(std::uint32_t k) {
    k = toBigEndian(k);
    return ByteArray((char*)(&k), sizeof(k));
  }

  ByteArray genBlock(std::uint32_t k) {
    // Make sure not empty, because we test for existence with empty()
    std::size_t size = (RandFactor * k) % (MaxSize - 1) + 1;
    std::uint8_t val = (std::uint8_t)(k % 256);

    ByteArray b(size, 0);

    for (std::size_t i = 0; i < size; ++i) {
      b[i] = val;
      val += 1;
    }

    return b;
  }

  bool checkBlock(std::uint32_t k, ByteArray b) {
    return genBlock(k) == b;
  }

  void putAll(BTreeDatabase& db, const List<std::uint32_t>& keys) {
    for (std::uint32_t k : keys) {
      ByteArray val = genBlock(k);
      db.insert(toByteArray(k), val);

      int i = Random::randi32();
      if (i % 23 == 0)
        db.commit();
    }
  }

  void checkAll(BTreeDatabase& db, const List<std::uint32_t>& keys) {
    for (std::uint32_t k : keys) {
      auto res = db.find(toByteArray(k));
      EXPECT_TRUE((bool)res);
      EXPECT_TRUE(checkBlock(k, *res));
    }

    // Also check that forAll works.

    Set<ByteArray> keySet;
    for (std::uint32_t k : keys)
      keySet.add(toByteArray(k));

    db.forAll([&keySet](ByteArray const& key, ByteArray const&) { EXPECT_TRUE(keySet.remove(key)); });

    EXPECT_TRUE(keySet.empty());
  }

  std::size_t removeAll(BTreeDatabase& db, const List<std::uint32_t>& keys) {
    std::size_t totalRemoved = 0;
    for (std::uint32_t k : keys) {
      auto old = db.find(toByteArray(k));
      EXPECT_TRUE(!old || *old == genBlock(k));

      if (db.remove(toByteArray(k))) {
        EXPECT_FALSE((bool)db.find(toByteArray(k)));
        ++totalRemoved;
      }
    }
    return totalRemoved;
  }

  void testBTreeDatabase(std::size_t testCount, std::size_t writeRepeat, std::size_t randCount, std::size_t rollbackCount, std::size_t blockSize) {
    auto tmpFile = File::temporaryFile();
    auto finallyGuard = finally([&tmpFile]() { tmpFile->remove(); });

    Set<std::uint32_t> keySet;
    BTreeDatabase db("TestDB", 4);
    db.setAutoCommit(false);

    while (keySet.size() < testCount)
      keySet.add(Random::randUInt(0, MaxKey));

    List<std::uint32_t> keys;
    for (std::uint32_t k : keySet) {
      for (std::uint32_t j = 0; j < writeRepeat; ++j)
        keys.append(k);
    }

    db.setIndexCacheSize(0);
    db.setBlockSize(blockSize);
    db.setIODevice(tmpFile);
    db.open();

    // record writes/reads repeated writeRepeat times randomly each cycle
    Random::shuffle(keys);
    putAll(db, keys);

    EXPECT_EQ(db.recordCount(), testCount);

    Random::shuffle(keys);
    checkAll(db, keys);

    // Random reads/writes with randCount cycles...
    for (std::uint32_t i = 0; i < randCount; ++i) {
      List<std::uint32_t> keysTemp(keys.begin(), keys.begin() + keys.size() / 2);

      Random::shuffle(keysTemp);
      removeAll(db, keysTemp);

      Random::shuffle(keysTemp);
      putAll(db, keysTemp);

      Random::shuffle(keys);
      checkAll(db, keys);
    }

    db.commit();

    // Random reads/writes/rollbacks with rollbackCount cycles...
    for (std::uint32_t i = 0; i < rollbackCount ; ++i) {
      List<std::uint32_t> keysTemp(keys.begin(), keys.begin() + keys.size() / 2);
      Random::shuffle(keysTemp);

      removeAll(db, keysTemp);
      db.rollback();

      checkAll(db, keys);
    }

    EXPECT_EQ(db.totalBlockCount(), db.freeBlockCount() + db.indexBlockCount() + db.leafBlockCount());

    // Now testing closing and reading

    db.close();

    // Set the wrong value, should be set to correct value in open()
    db.setBlockSize(blockSize + 512);

    db.open();

    // Checking values...

    checkAll(db, keys);

    EXPECT_EQ(db.totalBlockCount(), db.freeBlockCount() + db.indexBlockCount() + db.leafBlockCount());

    // Removing all records...

    std::size_t totalRemoved = removeAll(db, keys);

    EXPECT_EQ(totalRemoved, testCount);

    EXPECT_EQ(db.totalBlockCount(), db.freeBlockCount() + db.indexBlockCount() + db.leafBlockCount());

    db.close();
  }
}

TEST(BTreeDatabaseTest, Consistency) {
  testBTreeDatabase(500, 3, 5, 5, 512);

  // Test a range of block sizes to make sure there are not off by one errors
  // in maximum index / leaf size calculations.
  for (std::size_t i = 0; i < 16; ++i)
    testBTreeDatabase(30, 2, 2, 2, 200 + i);
}

TEST(BTreeDatabaseTest, Threading) {
  auto tmpFile = File::temporaryFile();
  auto finallyGuard = finally([&tmpFile]() { tmpFile->remove(); });

  BTreeDatabase db("TestDB", 4);
  db.setAutoCommit(false);
  db.setBlockSize(256);
  db.setIODevice(tmpFile);
  db.open();

  List<std::uint32_t> writeKeySet;
  List<std::uint32_t> deleteKeySet;

  while (writeKeySet.size() < 5000) {
    std::uint32_t key = Random::randu32();
    writeKeySet.append(key);
    if (Random::randf() > 0.3)
      deleteKeySet.append(key);
  }
  Random::shuffle(writeKeySet);

  {
    auto writer = Thread::invoke("databaseTestWriter",
        [&db, &writeKeySet]() {
          try {
            for (std::uint32_t k : writeKeySet) {
              ByteArray val = genBlock(k);
              db.insert(toByteArray(k), val);
              if (Random::randi32() % 23 == 0)
                db.commit();
            }
          } catch (std::exception const& e) {
            SCOPED_TRACE(outputException(e, true));
            FAIL();
          }
        });

    auto deleter = Thread::invoke("databaseTestDeleter",
        [&db, &deleteKeySet]() {
          try {
            for (std::uint32_t k : deleteKeySet) {
              db.remove(toByteArray(k));
              if (Random::randi32() % 23 == 0)
                db.commit();
            }
          } catch (std::exception const& e) {
            SCOPED_TRACE(outputException(e, true));
            FAIL();
          }
        });

    writer.finish();
    deleter.finish();

    db.close(false);
    tmpFile->open(IOMode::Read);
    db.open();

    EXPECT_EQ(db.totalBlockCount(), db.freeBlockCount() + db.indexBlockCount() + db.leafBlockCount());

    List<ThreadFunction<void>> readers;
    for (std::size_t i = 0; i < 5; ++i) {
      readers.append(Thread::invoke("databaseTestReader",
          [&db, &writeKeySet, &deleteKeySet]() {
            try {
              for (std::uint32_t k : writeKeySet) {
                if (auto res = db.find(toByteArray(k)))
                  EXPECT_TRUE(checkBlock(k, *res));
                else
                  EXPECT_TRUE(deleteKeySet.contains(k));
              }
            } catch (std::exception const& e) {
              SCOPED_TRACE(outputException(e, true));
              FAIL();
            }
          }));
    }
  }
}

