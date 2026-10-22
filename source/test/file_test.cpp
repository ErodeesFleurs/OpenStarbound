#include <stdio.h>
#include "vorbis/vorbisfile.h"
#ifdef STAR_SYSTEM_LINUX
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

import star.file;
import star.io_device;
import star.string;
import star.buffer;
import star.format;
import star.random;

import std;

#include "gtest/gtest.h"
import star.io_device_callbacks;

import star.byte_array;
import star.algorithm;
using namespace Star;

namespace {
  // An IODevice that reports more bytes than it was asked for, the way
  // File::pread used to turn a failed read into SIZE_MAX.
  struct OverlongReadDevice : IODevice {
    [[nodiscard]] std::expected<std::size_t, StarException> read(char*, std::size_t) override { return 0; }
    [[nodiscard]] std::expected<std::size_t, StarException> write(char const*, std::size_t) override { return 0; }
    [[nodiscard]] std::expected<StreamOffset, StarException> pos() override { return 0; }
    [[nodiscard]] std::expected<void, StarException> seek(StreamOffset, IOSeek) override { return {}; }
    [[nodiscard]] std::expected<std::size_t, StarException> readAbsolute(StreamOffset, char*, std::size_t) override { return std::numeric_limits<std::size_t>::max(); }
    [[nodiscard]] std::expected<std::size_t, StarException> writeAbsolute(StreamOffset, char const*, std::size_t) override { return std::numeric_limits<std::size_t>::max(); }
    [[nodiscard]] std::expected<std::shared_ptr<::Star::IODevice>, StarException> clone() override { return std::make_shared<OverlongReadDevice>(); }
  };
}

TEST(FileTest, FullReadRejectsImpossibleLengths) {
  auto device = std::make_shared<OverlongReadDevice>();
  char buffer[4];
  // Without the length check these underflow the remaining length and loop
  // forever instead of failing.
  auto read = device->readFullAbsolute(0, buffer, sizeof(buffer));
  ASSERT_FALSE(read);
  EXPECT_TRUE(read.error().is<IOException>());
  auto write = device->writeFullAbsolute(0, buffer, sizeof(buffer));
  ASSERT_FALSE(write);
  EXPECT_TRUE(write.error().is<IOException>());
}

namespace {
  std::filesystem::path fsPath(String const& path) {
    return std::filesystem::path(path.utf8());
  }

  // Never follows symlinks, so it is safe even when the removal under test
  // misbehaves.
  void removeTree(String const& path) {
    std::error_code ignored;
    std::filesystem::remove_all(fsPath(path), ignored);
  }
}

TEST(FileTest, All) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  auto fileResult = File::ephemeralFile(random);
  ASSERT_TRUE(fileResult);
  auto file = std::move(*fileResult);
  ASSERT_TRUE(file->resize(1000));
  ASSERT_TRUE(file->resize(0));
  ASSERT_TRUE(file->resize(500));
  auto size = file->size();
  ASSERT_TRUE(size);
  EXPECT_EQ(*size, 500);

  auto dirResult = File::temporaryDirectory(random);
  ASSERT_TRUE(dirResult);
  auto dir = std::move(*dirResult);
  ASSERT_TRUE(File::makeDirectory(File::relativeTo(dir, "inner")));
  EXPECT_EQ(File::isDirectory(File::relativeTo(dir, "inner") + "/"), true);
  EXPECT_TRUE(File::removeDirectoryRecursive(dir));

#ifdef STAR_SYSTEM_FAMILY_WINDOWS
  EXPECT_EQ(File::baseName("/foo/bar"), "bar");
  EXPECT_EQ(File::baseName("\\foo\\bar\\"), "bar");
  EXPECT_EQ(File::baseName("/foo/bar/baz"), "baz");
  EXPECT_EQ(File::dirName("\\foo\\bar"), "\\foo");
  EXPECT_EQ(File::dirName("/foo\\bar/"), "/foo");
  EXPECT_EQ(File::dirName("/foo/bar\\baz"), "/foo/bar");
  EXPECT_EQ(File::dirName("foo/bar/baz"), "foo/bar");

  EXPECT_EQ(File::relativeTo("c:\\foo\\", "bar"), "c:\\foo\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo", "bar"), "c:\\foo\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo\\", "\\bar"), "\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo\\", ".\\bar"), "c:\\foo\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo\\.", ".\\bar"), "c:\\foo\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo\\.", "c:\\bar"), "c:\\bar");
  EXPECT_EQ(File::relativeTo("c:\\foo\\.", "c:bar\\"), "c:bar\\");
  EXPECT_EQ(File::relativeTo("c:\\foo.", "bar"), "c:\\foo.\\bar");
#else
  EXPECT_EQ(File::baseName("/foo/bar"), "bar");
  EXPECT_EQ(File::baseName("/foo/bar/"), "bar");
  EXPECT_EQ(File::baseName("/foo/bar/baz"), "baz");
  EXPECT_EQ(File::dirName("/foo/bar"), "/foo");
  EXPECT_EQ(File::dirName("/foo/bar/"), "/foo");
  EXPECT_EQ(File::dirName("/foo/bar/baz"), "/foo/bar");
  EXPECT_EQ(File::dirName("foo/bar/baz"), "foo/bar");

  EXPECT_EQ(File::relativeTo("/foo", "bar"), "/foo/bar");
  EXPECT_EQ(File::relativeTo("/foo", "bar/"), "/foo/bar/");
  EXPECT_EQ(File::relativeTo("/foo", "/bar/"), "/bar/");
#endif
}

TEST(FileTest, RemoveDirectoryRecursiveKeepsSymlinkTargets) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  // The removal used to descend into directory symlinks, so a link pointing
  // outside of the directory deleted the target's contents.
  auto dirResult = File::temporaryDirectory(random);
  ASSERT_TRUE(dirResult);
  auto dir = std::move(*dirResult);
  auto outsideResult = File::temporaryDirectory(random);
  ASSERT_TRUE(outsideResult);
  auto outside = std::move(*outsideResult);
  auto finallyGuard = finally([&dir, &outside]() {
    removeTree(dir);
    removeTree(outside);
  });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(outside, "real")), ec);
  ASSERT_TRUE(File::writeFile("kept", File::relativeTo(File::relativeTo(outside, "real"), "keep.txt")));
  ASSERT_TRUE(File::writeFile("doomed", File::relativeTo(dir, "file.txt")));
  std::filesystem::create_directory_symlink(
      fsPath(File::relativeTo(outside, "real")), fsPath(File::relativeTo(dir, "link")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  EXPECT_EQ(File::isSymlink(File::relativeTo(dir, "link")), true);
  EXPECT_EQ(File::isSymlink(File::relativeTo(dir, "file.txt")), false);

  EXPECT_TRUE(File::removeDirectoryRecursive(dir));
  EXPECT_EQ(File::isDirectory(dir), false);
  // The directory the symlink pointed at has to be untouched.
  EXPECT_EQ(File::isFile(File::relativeTo(File::relativeTo(outside, "real"), "keep.txt")), true);
}

TEST(FileTest, RemoveDirectoryRecursiveHandlesSymlinkLoops) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  // A symlink pointing at one of its own parent directories used to make the
  // removal walk the same tree over and over until it errored out half way
  // through ("remove error: No such file or directory").
  auto dirResult = File::temporaryDirectory(random);
  ASSERT_TRUE(dirResult);
  auto dir = std::move(*dirResult);
  auto finallyGuard = finally([&dir]() { removeTree(dir); });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(dir, "real")), ec);
  ASSERT_TRUE(File::writeFile("doomed", File::relativeTo(File::relativeTo(dir, "real"), "file.txt")));
  std::filesystem::create_directory_symlink(fsPath(dir), fsPath(File::relativeTo(dir, "loop")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  EXPECT_TRUE(File::removeDirectoryRecursive(dir));
  EXPECT_EQ(File::isDirectory(dir), false);
}

TEST(FileTest, OggDeviceCallbacksReadAndSeek) {
  auto device = std::make_shared<Buffer>(ByteArray::fromCString("abcdef"));
  auto adapterResult = IODeviceCallbacks::create(device);
  ASSERT_TRUE(adapterResult);
  auto adapter = std::move(*adapterResult);
  ov_callbacks callbacks{};
  adapter.setupOggCallbacks(callbacks);

  char output[4]{};
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 2u);
  EXPECT_FALSE(adapter.takeError());
  EXPECT_EQ(ByteArray(output, 4), ByteArray::fromCString("abcd"));
  EXPECT_EQ(callbacks.tell_func(&adapter), 4);
  EXPECT_FALSE(adapter.takeError());

  EXPECT_EQ(callbacks.seek_func(&adapter, -2, SEEK_CUR), 0);
  EXPECT_FALSE(adapter.takeError());
  EXPECT_EQ(callbacks.tell_func(&adapter), 2);
  EXPECT_FALSE(adapter.takeError());
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 2u);
  EXPECT_FALSE(adapter.takeError());
  EXPECT_EQ(ByteArray(output, 4), ByteArray::fromCString("cdef"));
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 0u);
  EXPECT_FALSE(adapter.takeError());
}

#ifdef STAR_SYSTEM_LINUX
namespace {
  int descriptorForFile(String const& path) {
    for (auto const& entry : std::filesystem::directory_iterator("/proc/self/fd")) {
      std::error_code ec;
      auto target = std::filesystem::read_symlink(entry.path(), ec);
      if (!ec && target == fsPath(path))
        return std::stoi(entry.path().filename().string());
    }
    return -1;
  }

  std::size_t descriptorCount() {
    auto entries = std::filesystem::directory_iterator("/proc/self/fd");
    return std::distance(entries, std::filesystem::directory_iterator());
  }
}

TEST(FileTest, AppendSeekFailureReleasesDescriptor) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  auto dirResult = File::temporaryDirectory(random);
  ASSERT_TRUE(dirResult);
  auto dir = std::move(*dirResult);
  auto cleanup = finally([&]() { removeTree(dir); });
  auto path = File::relativeTo(dir, "pipe");
  ASSERT_EQ(::mkfifo(path.utf8Ptr(), 0600), 0);
  auto before = descriptorCount();
  for (unsigned i = 0; i < 8; ++i) {
    auto opened = File::open(path, IOMode::ReadWrite | IOMode::Append);
    ASSERT_FALSE(opened);
    EXPECT_TRUE(opened.error().is<IOException>());
  }
  EXPECT_EQ(descriptorCount(), before);
}

TEST(FileTest, FailedCloseConsumesDescriptorOwnership) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  auto fileResult = File::temporaryFile(random);
  ASSERT_TRUE(fileResult);
  auto file = std::move(*fileResult);
  auto path = file->fileName();
  auto cleanup = finally([&]() { EXPECT_TRUE(File::remove(path)); });
  int descriptor = descriptorForFile(path);
  ASSERT_GE(descriptor, 0);
  ASSERT_EQ(::close(descriptor), 0);

  auto closed = file->close();
  ASSERT_FALSE(closed);
  EXPECT_TRUE(closed.error().is<IOException>());
  EXPECT_FALSE(file->isOpen());
  EXPECT_TRUE(file->close());
  // No destructor retry of the invalid or potentially reused descriptor.
  file.reset();
}

TEST(FileTest, DestructorReportsCloseFailureWithoutTerminating) {
  auto randomResult = RandomService::create();
  ASSERT_TRUE(randomResult);
  auto& random = **randomResult;
  auto fileResult = File::temporaryFile(random);
  ASSERT_TRUE(fileResult);
  auto file = std::move(*fileResult);
  auto path = file->fileName();
  auto cleanup = finally([&]() { EXPECT_TRUE(File::remove(path)); });
  int descriptor = descriptorForFile(path);
  ASSERT_GE(descriptor, 0);
  ASSERT_EQ(::close(descriptor), 0);
  file.reset();
}

TEST(FileTest, SyncFailureReachesCaller) {
  auto fileResult = File::open("/dev/full", IOMode::Write);
  ASSERT_TRUE(fileResult);
  auto file = std::move(*fileResult);
  auto synced = file->sync();
  ASSERT_FALSE(synced);
  EXPECT_TRUE(synced.error().is<IOException>());
  EXPECT_TRUE(file->close());
}
#endif
