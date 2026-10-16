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
    std::size_t read(char*, std::size_t) override { return 0; }
    std::size_t write(char const*, std::size_t) override { return 0; }
    StreamOffset pos() override { return 0; }
    void seek(StreamOffset, IOSeek) override {}
    std::size_t readAbsolute(StreamOffset, char*, std::size_t) override { return std::numeric_limits<std::size_t>::max(); }
    std::size_t writeAbsolute(StreamOffset, char const*, std::size_t) override { return std::numeric_limits<std::size_t>::max(); }
    std::shared_ptr<::Star::IODevice> clone() override { return std::make_shared<OverlongReadDevice>(); }
  };
}

TEST(FileTest, FullReadRejectsImpossibleLengths) {
  auto device = std::make_shared<OverlongReadDevice>();
  char buffer[4];
  // Without the length check these underflow the remaining length and loop
  // forever instead of failing.
  EXPECT_THROW(device->readFullAbsolute(0, buffer, sizeof(buffer)), IOException);
  EXPECT_THROW(device->writeFullAbsolute(0, buffer, sizeof(buffer)), IOException);
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
  auto file = File::ephemeralFile();
  file->resize(1000);
  file->resize(0);
  file->resize(500);
  EXPECT_EQ(file->size(), 500);

  auto dir = File::temporaryDirectory();
  File::makeDirectory(File::relativeTo(dir, "inner"));
  EXPECT_TRUE(File::isDirectory(File::relativeTo(dir, "inner") + "/"));
  File::removeDirectoryRecursive(dir);

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
  // The removal used to descend into directory symlinks, so a link pointing
  // outside of the directory deleted the target's contents.
  auto dir = File::temporaryDirectory();
  auto outside = File::temporaryDirectory();
  auto finallyGuard = finally([&dir, &outside]() {
    removeTree(dir);
    removeTree(outside);
  });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(outside, "real")), ec);
  File::writeFile("kept", File::relativeTo(File::relativeTo(outside, "real"), "keep.txt"));
  File::writeFile("doomed", File::relativeTo(dir, "file.txt"));
  std::filesystem::create_directory_symlink(
      fsPath(File::relativeTo(outside, "real")), fsPath(File::relativeTo(dir, "link")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  EXPECT_TRUE(File::isSymlink(File::relativeTo(dir, "link")));
  EXPECT_FALSE(File::isSymlink(File::relativeTo(dir, "file.txt")));

  EXPECT_NO_THROW(File::removeDirectoryRecursive(dir));
  EXPECT_FALSE(File::isDirectory(dir));
  // The directory the symlink pointed at has to be untouched.
  EXPECT_TRUE(File::isFile(File::relativeTo(File::relativeTo(outside, "real"), "keep.txt")));
}

TEST(FileTest, RemoveDirectoryRecursiveHandlesSymlinkLoops) {
  // A symlink pointing at one of its own parent directories used to make the
  // removal walk the same tree over and over until it errored out half way
  // through ("remove error: No such file or directory").
  auto dir = File::temporaryDirectory();
  auto finallyGuard = finally([&dir]() { removeTree(dir); });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(dir, "real")), ec);
  File::writeFile("doomed", File::relativeTo(File::relativeTo(dir, "real"), "file.txt"));
  std::filesystem::create_directory_symlink(fsPath(dir), fsPath(File::relativeTo(dir, "loop")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  EXPECT_NO_THROW(File::removeDirectoryRecursive(dir));
  EXPECT_FALSE(File::isDirectory(dir));
}

TEST(FileTest, OggDeviceCallbacksReadAndSeek) {
  auto device = std::make_shared<Buffer>(ByteArray::fromCString("abcdef"));
  IODeviceCallbacks adapter(device);
  ov_callbacks callbacks{};
  adapter.setupOggCallbacks(callbacks);

  char output[4]{};
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 2u);
  EXPECT_EQ(ByteArray(output, 4), ByteArray::fromCString("abcd"));
  EXPECT_EQ(callbacks.tell_func(&adapter), 4);

  EXPECT_EQ(callbacks.seek_func(&adapter, -2, SEEK_CUR), 0);
  EXPECT_EQ(callbacks.tell_func(&adapter), 2);
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 2u);
  EXPECT_EQ(ByteArray(output, 4), ByteArray::fromCString("cdef"));
  EXPECT_EQ(callbacks.read_func(output, 2, 2, &adapter), 0u);
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
  auto dir = File::temporaryDirectory();
  auto cleanup = finally([&]() { removeTree(dir); });
  auto path = File::relativeTo(dir, "pipe");
  ASSERT_EQ(::mkfifo(path.utf8Ptr(), 0600), 0);
  auto before = descriptorCount();
  for (unsigned i = 0; i < 8; ++i)
    EXPECT_THROW(File::open(path, IOMode::ReadWrite | IOMode::Append), IOException);
  EXPECT_EQ(descriptorCount(), before);
}

TEST(FileTest, FailedCloseConsumesDescriptorOwnership) {
  auto file = File::temporaryFile();
  auto path = file->fileName();
  auto cleanup = finally([&]() { File::remove(path); });
  int descriptor = descriptorForFile(path);
  ASSERT_GE(descriptor, 0);
  ASSERT_EQ(::close(descriptor), 0);

  EXPECT_THROW(file->close(), IOException);
  EXPECT_FALSE(file->isOpen());
  EXPECT_NO_THROW(file->close());
  // No destructor retry of the invalid or potentially reused descriptor.
  file.reset();
}

TEST(FileTest, DestructorReportsCloseFailureWithoutTerminating) {
  auto file = File::temporaryFile();
  auto path = file->fileName();
  auto cleanup = finally([&]() { File::remove(path); });
  int descriptor = descriptorForFile(path);
  ASSERT_GE(descriptor, 0);
  ASSERT_EQ(::close(descriptor), 0);
  file.reset();
}

TEST(FileTest, SyncFailureReachesCaller) {
  auto file = File::open("/dev/full", IOMode::Write);
  EXPECT_THROW(file->sync(), IOException);
  file->close();
}
#endif
