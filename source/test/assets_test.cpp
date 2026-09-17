#include "StarAssets.hpp"
#include "StarDirectoryAssetSource.hpp"
#include "StarFile.hpp"

#include <filesystem>

#include "gtest/gtest.h"

using namespace Star;

namespace {
  std::filesystem::path fsPath(String const& path) {
    return std::filesystem::path(path.utf8());
  }
}

TEST(AssetsTest, All) {
  EXPECT_EQ(AssetPath::removeDirectives("/foo/bar/baz??::?:??:asdfasdf??D?"), "/foo/bar/baz");
  EXPECT_EQ(AssetPath::directory("/foo/bar/baz"), "/foo/bar/");
  EXPECT_EQ(AssetPath::directory("foo/bar/baz"), "foo/bar/");
  EXPECT_EQ(AssetPath::directory("foo"), "");
  EXPECT_EQ(AssetPath::directory("/foo"), "/");
  EXPECT_EQ(AssetPath::filename(""), "");
  EXPECT_EQ(AssetPath::filename("foo"), "foo");
  EXPECT_EQ(AssetPath::filename("/foo"), "foo");
  EXPECT_EQ(AssetPath::filename("/foo/"), "");
  EXPECT_EQ(AssetPath::filename("/foo/bar"), "bar");

  //AssetPath compare = AssetPath{"/foo/bar/baz", String("baf"), {"whoa", "there"}};
  //EXPECT_EQ(AssetPath::split("/foo/bar/baz:baf?whoa?there"), compare);

  EXPECT_EQ(
      AssetPath::relativeTo("/foo/bar/baz:baf?whoa?there", "thing:sub?directive"), "/foo/bar/thing:sub?directive");
}

TEST(AssetsTest, SymlinkLoopIsNotFollowed) {
  // A symlink pointing back at one of its own parent directories used to make
  // the asset scan recurse until the operating system reported ELOOP; the
  // resulting exception escaped the Assets constructor and killed the game.
  auto tempDir = File::temporaryDirectory();
  auto finallyGuard = finally([&tempDir]() { File::removeDirectoryRecursive(tempDir); });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(tempDir, "real")), ec);
  File::writeFile("data", File::relativeTo(File::relativeTo(tempDir, "real"), "asset.txt"));
  std::filesystem::create_directory_symlink(
      fsPath(File::relativeTo(tempDir, "real")), fsPath(File::relativeTo(tempDir, "link")), ec);
  std::filesystem::create_directory_symlink(fsPath(tempDir), fsPath(File::relativeTo(tempDir, "loop")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  DirectoryAssetSource source(tempDir);
  auto paths = source.assetPaths();
  EXPECT_TRUE(paths.contains("/real/asset.txt"));
  // A symlink to a sibling directory is not a loop, so it is still followed.
  EXPECT_TRUE(paths.contains("/link/asset.txt"));
  // The self-referential one has to be skipped, not recursed into forever.
  EXPECT_FALSE(paths.contains("/loop/real/asset.txt"));
  EXPECT_FALSE(paths.contains("/loop/link/asset.txt"));
}
