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
import star.buffer;
import star.random;
import star.logging;

#include "gtest/gtest.h"

import star.byte_array;
import star.bytes;
import star.algorithm;
using namespace Star;

namespace {
  // Fault injection at the actual IO boundary; all database writes, root
  // selection, recovery and serialization still use the production code.
  struct FailingSyncBuffer : Buffer {
    bool failSync = false;
    unsigned syncCalls = 0;

    void sync() override {
      ++syncCalls;
      if (failSync)
        throw IOException("Injected sync failure");
    }
  };

  struct FailingCloseBuffer : Buffer {
    unsigned closeCalls = 0;

    void close() override {
      ++closeCalls;
      IODevice::close();
      throw IOException("Injected close failure");
    }
  };

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

  void putAll(RandomService& random, BTreeDatabase& db, const List<std::uint32_t>& keys) {
    for (std::uint32_t k : keys) {
      ByteArray val = genBlock(k);
      db.insert(toByteArray(k), val);

      int i = random.randi32();
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
    RandomService random;
    Logger logger;
    auto tmpFile = File::temporaryFile(random);
    auto finallyGuard = finally([&tmpFile]() { tmpFile->remove(); });

    Set<std::uint32_t> keySet;
    BTreeDatabase db(logger, "TestDB", 4);
    db.setAutoCommit(false);

    while (keySet.size() < testCount)
      keySet.add(random.randUInt(0, MaxKey));

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
    random.shuffle(keys);
    putAll(random, db, keys);

    EXPECT_EQ(db.recordCount(), testCount);

    random.shuffle(keys);
    checkAll(db, keys);

    // Random reads/writes with randCount cycles...
    for (std::uint32_t i = 0; i < randCount; ++i) {
      List<std::uint32_t> keysTemp(keys.begin(), keys.begin() + keys.size() / 2);

      random.shuffle(keysTemp);
      removeAll(db, keysTemp);

      random.shuffle(keysTemp);
      putAll(random, db, keysTemp);

      random.shuffle(keys);
      checkAll(db, keys);
    }

    db.commit();

    // Random reads/writes/rollbacks with rollbackCount cycles...
    for (std::uint32_t i = 0; i < rollbackCount ; ++i) {
      List<std::uint32_t> keysTemp(keys.begin(), keys.begin() + keys.size() / 2);
      random.shuffle(keysTemp);

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
  RandomService random;
  Logger logger;
  auto tmpFile = File::temporaryFile(random);
  auto finallyGuard = finally([&tmpFile]() { tmpFile->remove(); });

  BTreeDatabase db(logger, "TestDB", 4);
  db.setAutoCommit(false);
  db.setBlockSize(256);
  db.setIODevice(tmpFile);
  db.open();

  List<std::uint32_t> writeKeySet;
  List<std::uint32_t> deleteKeySet;

  while (writeKeySet.size() < 5000) {
    std::uint32_t key = random.randu32();
    writeKeySet.append(key);
    if (random.randf() > 0.3)
      deleteKeySet.append(key);
  }
  random.shuffle(writeKeySet);

  {
    auto writer = Thread::invoke("databaseTestWriter",
        [&random, &db, &writeKeySet]() {
          try {
            for (std::uint32_t k : writeKeySet) {
              ByteArray val = genBlock(k);
              db.insert(toByteArray(k), val);
              if (random.randi32() % 23 == 0)
                db.commit();
            }
          } catch (std::exception const&) {
            ExceptionDiagnostic const e(std::current_exception());
            SCOPED_TRACE(outputException(e, true));
            FAIL();
          }
        });

    auto deleter = Thread::invoke("databaseTestDeleter",
        [&random, &db, &deleteKeySet]() {
          try {
            for (std::uint32_t k : deleteKeySet) {
              db.remove(toByteArray(k));
              if (random.randi32() % 23 == 0)
                db.commit();
            }
          } catch (std::exception const&) {
            ExceptionDiagnostic const e(std::current_exception());
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
            } catch (std::exception const&) {
              ExceptionDiagnostic const e(std::current_exception());
              SCOPED_TRACE(outputException(e, true));
              FAIL();
            }
          }));
    }
  }
  db.close();
}


TEST(BTreeDatabaseTest, FailedCommitIsReportedWithoutDestructorRetry) {
  Logger logger;
  auto device = std::make_shared<FailingSyncBuffer>();
  auto db = std::make_unique<BTreeDatabase>(logger, "TestDB", 4);
  db->setAutoCommit(false);
  db->setIODevice(device);
  db->open();
  db->insert(toByteArray(0), genBlock(0));
  db->commit();
  db->insert(toByteArray(1), genBlock(1));
  device->failSync = true;

  EXPECT_THROW(db->commit(), IOException);
  EXPECT_TRUE(db->isOpen());
  unsigned callsAfterFailure = device->syncCalls;
  db.reset();
  EXPECT_EQ(device->syncCalls, callsAfterFailure);

  // sync() failure reports uncertain durability, not an undo of writes that
  // already reached the device.  The existing allocator may reuse a freed
  // leaf block before writeRoot(), so even the old root selector can expose
  // the new complete leaf.  Require a consistent, usable recovery, not a
  // particular visibility outcome for the failed transaction.
  device->failSync = false;
  BTreeDatabase recovered(logger);
  recovered.setIODevice(device);
  recovered.open();
  auto committed = recovered.find(toByteArray(0));
  ASSERT_TRUE(committed);
  EXPECT_EQ(*committed, genBlock(0));
  auto uncertain = recovered.find(toByteArray(1));
  if (uncertain)
    EXPECT_EQ(*uncertain, genBlock(1));
  auto recoveredCount = uncertain ? 2u : 1u;
  EXPECT_EQ(recovered.recordCount(), recoveredCount);
  EXPECT_EQ(recovered.totalBlockCount(),
      recovered.freeBlockCount() + recovered.indexBlockCount() + recovered.leafBlockCount());

  recovered.insert(toByteArray(2), genBlock(2));
  recovered.close();
  recovered.open();
  auto subsequent = recovered.find(toByteArray(2));
  ASSERT_TRUE(subsequent);
  EXPECT_EQ(*subsequent, genBlock(2));
  EXPECT_EQ(recovered.recordCount(), recoveredCount + 1);
  recovered.close();
}

TEST(BTreeDatabaseTest, FailedCloseIsReportedWithoutDestructorRetry) {
  Logger logger;
  auto device = std::make_shared<FailingSyncBuffer>();
  auto db = std::make_unique<BTreeDatabase>(logger, "TestDB", 4);
  db->setAutoCommit(false);
  db->setIODevice(device);
  db->open();
  db->insert(toByteArray(1), genBlock(1));
  device->failSync = true;

  EXPECT_THROW(db->close(true), IOException);
  EXPECT_TRUE(db->isOpen());
  EXPECT_TRUE(device->isOpen());
  unsigned callsAfterFailure = device->syncCalls;
  db.reset();
  EXPECT_EQ(device->syncCalls, callsAfterFailure);
}

TEST(BTreeDatabaseTest, FailedDeviceCloseIsConsumedOnce) {
  Logger logger;
  auto device = std::make_shared<FailingCloseBuffer>();
  BTreeDatabase db(logger, "TestDB", 4);
  db.setIODevice(device);
  db.open();

  EXPECT_THROW(db.close(true), IOException);
  EXPECT_FALSE(db.isOpen());
  EXPECT_FALSE(device->isOpen());
  EXPECT_NO_THROW(db.close(true));
  EXPECT_EQ(device->closeCalls, 1u);
}
