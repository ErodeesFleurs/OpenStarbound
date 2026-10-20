import star.json;
import star.ordered_map;
import star.bi_map;
import star.thread;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.io_device;
import star.string;
import star.file;
import std;

import star.assets;
import star.asset_source;
import star.directory_asset_source;
import star.animated_part_set;
import star.audio;
import star.buffer;
import star.image;
import star.byte_array;
import star.vector;
import star.game_test;
import star.root;
import star.random;
import star.logging;


#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

import star.algorithm;
using namespace Star;

namespace {
  std::filesystem::path fsPath(String const& path) {
    return std::filesystem::path(path.utf8());
  }

  struct PngIoFailure {};

  class FailingPngNameBuffer : public Buffer {
  public:
    explicit FailingPngNameBuffer(ByteArray bytes) : Buffer(std::move(bytes)) {}
    String deviceName() const override { throw PngIoFailure{}; }
  };

  class FailingPngBuffer : public Buffer {
  public:
    FailingPngBuffer(ByteArray bytes, StreamOffset failAt) : Buffer(std::move(bytes)), m_failAt(failAt) {}

    void readFull(char* data, std::size_t length) override {
      failIfRequested();
      Buffer::readFull(data, length);
    }

    void writeFull(char const* data, std::size_t length) override {
      failIfRequested();
      Buffer::writeFull(data, length);
    }

    unsigned unwound = 0;

  private:
    void failIfRequested() {
      if (pos() >= m_failAt) {
        struct UnwindWitness {
          unsigned& count;
          ~UnwindWitness() { ++count; }
        } witness{unwound};
        throw PngIoFailure{};
      }
    }

    StreamOffset m_failAt;
  };

  Image pngTestImage(PixelFormat format) {
    Image image(2, 2, format);
    image.set(0, 0, Vec4B{255, 0, 0, 64});
    image.set(1, 0, Vec4B{0, 255, 0, 128});
    image.set(0, 1, Vec4B{0, 0, 255, 192});
    image.set(1, 1, Vec4B{255, 255, 0, 255});
    return image;
  }

  ByteArray pngTestBytes(Logger& logger) {
    auto buffer = std::make_shared<Buffer>();
    pngTestImage(PixelFormat::RGBA32).writePng(logger, buffer);
    return buffer->takeData();
  }
}

const GameTestRegistration assetsAll("AssetsTest", "All", [](GameTest&) {
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
});

const GameTestRegistration assetsSymlinkLoopIsNotFollowed("AssetsTest", "SymlinkLoopIsNotFollowed", [](GameTest& fixture) {
  // A symlink pointing back at one of its own parent directories used to make
  // the asset scan recurse until the operating system reported ELOOP; the
  // resulting exception escaped the Assets constructor and killed the game.
  auto tempDir = File::temporaryDirectory(fixture.root().random());
  auto finallyGuard = finally([&tempDir]() { File::removeDirectoryRecursive(tempDir); });

  std::error_code ec;
  std::filesystem::create_directories(fsPath(File::relativeTo(tempDir, "real")), ec);
  File::writeFile("data", File::relativeTo(File::relativeTo(tempDir, "real"), "asset.txt"));
  std::filesystem::create_directory_symlink(
      fsPath(File::relativeTo(tempDir, "real")), fsPath(File::relativeTo(tempDir, "link")), ec);
  std::filesystem::create_directory_symlink(fsPath(tempDir), fsPath(File::relativeTo(tempDir, "loop")), ec);
  if (ec)
    GTEST_SKIP() << "cannot create directory symlinks here: " << ec.message();

  DirectoryAssetSource source(fixture.root().logger(), tempDir);
  auto paths = source.assetPaths();
  EXPECT_TRUE(paths.contains("/real/asset.txt"));
  // A symlink to a sibling directory is not a loop, so it is still followed.
  EXPECT_TRUE(paths.contains("/link/asset.txt"));
  // The self-referential one has to be skipped, not recursed into forever.
  EXPECT_FALSE(paths.contains("/loop/real/asset.txt"));
  EXPECT_FALSE(paths.contains("/loop/link/asset.txt"));
});

const GameTestRegistration assetsConstAnimationCachesRefreshFramesAndParts("AssetsTest", "ConstAnimationCachesRefreshFramesAndParts", [](GameTest&) {
  auto config = Json::parse(R"({
    "stateTypes": {
      "movement": {
        "default": "walk",
        "states": {
          "walk": {
            "frames": 2,
            "cycle": 1.0,
            "mode": "loop",
            "frameProperties": {"stateFrame": [10, 20]}
          }
        }
      }
    },
    "parts": {
      "body": {
        "properties": {"base": true},
        "partStates": {
          "movement": {
            "walk": {"frameProperties": {"partFrame": [30, 40]}}
          }
        }
      }
    }
  })");
  AnimatedPartSet animation(config, 1);
  animation.update(0.75f);
  AnimatedPartSet const frozen = animation;
  auto const& part = frozen.activePart("body");
  ASSERT_TRUE(part.activeState);
  EXPECT_EQ(part.activeState->frame, 1u);
  EXPECT_EQ(part.properties.get("partFrame").toInt(), 40);
  EXPECT_EQ(part.nextProperties.get("partFrame").toInt(), 30);
  auto const& state = frozen.activeState("movement");
  EXPECT_EQ(state.frame, 1u);
  EXPECT_EQ(state.nextFrame, 0u);
  EXPECT_FLOAT_EQ(state.frameProgress, 0.5f);
  EXPECT_EQ(state.properties.get("stateFrame").toInt(), 20);

  // Exercise callback-driven cache refresh on a genuinely const, still-dirty copy.
  AnimatedPartSet const callbackCopy = animation;
  unsigned stateCount = 0;
  callbackCopy.forEachActiveState([&](String const& name, auto const& active) {
    EXPECT_EQ(name, "movement");
    EXPECT_EQ(active.frame, 1u);
    ++stateCount;
  });
  unsigned partCount = 0;
  callbackCopy.forEachActivePart([&](String const& name, auto const& active) {
    EXPECT_EQ(name, "body");
    EXPECT_EQ(active.properties.get("partFrame").toInt(), 40);
    ++partCount;
  });
  EXPECT_EQ(stateCount, 1u);
  EXPECT_EQ(partCount, 1u);

  // The public nonconst parts interface remains writable.
  animation.parts().get("body").partProperties["base"] = false;
  animation.parts().get("body").activePartDirty = true;
  EXPECT_FALSE(animation.activePart("body").properties.get("base").toBool());
  animation.setStateTypeEnabled("movement", false);
  EXPECT_FALSE(animation.activePart("body").activeState);
});

const GameTestRegistration assetsWavSignaturesAndPcmSamples("AssetsTest", "WavSignaturesAndPcmSamples", [](GameTest&) {
  // Mono, 8 kHz, 16-bit PCM with two samples.
  std::array<unsigned char, 48> bytes = {
    'R', 'I', 'F', 'F', 40, 0, 0, 0, 'W', 'A', 'V', 'E',
    'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 1, 0,
    0x40, 0x1f, 0, 0, 0x80, 0x3e, 0, 0, 2, 0, 16, 0,
    'd', 'a', 't', 'a', 4, 0, 0, 0, 0x34, 0x12, 0xfe, 0xff
  };
  auto device = [&] {
    auto buffer = std::make_shared<Buffer>();
    buffer->writeFull(reinterpret_cast<char const*>(bytes.data()), bytes.size());
    buffer->seek(0);
    return buffer;
  };
  Audio audio(device(), "signature-regression.wav");
  EXPECT_FALSE(audio.compressed());
  EXPECT_EQ(audio.channels(), 1u);
  EXPECT_EQ(audio.sampleRate(), 8000u);
  std::array<std::int16_t, 2> samples{};
  ASSERT_EQ(audio.read(samples.data(), samples.size()), samples.size());
  EXPECT_EQ(samples[0], 0x1234);
  EXPECT_EQ(samples[1], -2);

  // High-bit signature bytes must be classified as unsigned characters.
  bytes[12] = 0xff;
  EXPECT_THROW((Audio(device(), "invalid-fmt.wav")), AudioException);
  bytes[12] = 'f';
  bytes[36] = 0x80;
  EXPECT_THROW((Audio(device(), "invalid-data.wav")), AudioException);
});

const GameTestRegistration assetsFailedStartupJoinsWorkers("AssetsTest", "FailedStartupJoinsWorkers", [](GameTest& fixture) {
  auto tempDir = File::temporaryDirectory(fixture.root().random());
  auto cleanup = finally([&] { File::removeDirectoryRecursive(tempDir); });
  Assets::Settings settings{};
  settings.workerPoolSize = 4;
  settings.assetTimeToLive = 10;
  // Each failure occurs after worker launch, including successful preload work
  // followed by a failure. No commercial assets or process-global Root needed.
  // The game test environment turns every Error log into a gtest failure.
  // Assert exactly the intentional loading failure, without muting the sink:
  // extra errors (including errors from workers) must still fail the test.
  for (unsigned attempt = 0; attempt < 4; ++attempt)
    EXPECT_NONFATAL_FAILURE(
        EXPECT_THROW((Assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir})), AssetException),
        "Error was logged: Exception caught loading asset: /preload.config,");

  File::writeFile("not json", File::relativeTo(tempDir, "preload.config"));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THROW((Assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir})), AssetException),
      "Error was logged: Exception caught loading asset: /preload.config,");

  File::writeFile("{}", File::relativeTo(tempDir, "present.json"));
  File::writeFile(
      "[{\"type\":\"json\",\"path\":\"/present.json\"},{\"type\":\"json\",\"path\":\"/absent.json\"}]",
      File::relativeTo(tempDir, "preload.config"));
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THROW((Assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir})), AssetException),
      "Error was logged: Exception caught loading asset: /absent.json,");

  File::writeFile("[{\"type\":\"image\",\"path\":\"relative.png\"}]",
      File::relativeTo(tempDir, "preload.config"));
  EXPECT_THROW((Assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir})), AssetException);
  File::writeFile("[]", File::relativeTo(tempDir, "preload.config"));
  EXPECT_NO_THROW((Assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir})));
});

const GameTestRegistration assetsShutdownWithQueuedImageDependencies("AssetsTest", "ShutdownWithQueuedImageDependencies", [](GameTest& fixture) {
  auto tempDir = File::temporaryDirectory(fixture.root().random());
  auto cleanup = finally([&] { File::removeDirectoryRecursive(tempDir); });
  File::writeFile("[]", File::relativeTo(tempDir, "preload.config"));
  File::writeFile(pngTestBytes(fixture.root().logger()), File::relativeTo(tempDir, "sample.png"));
  Assets::Settings settings{};
  settings.workerPoolSize = 4;
  settings.assetTimeToLive = 10;
  for (unsigned attempt = 0; attempt < 16; ++attempt) {
    Assets assets(fixture.root().logger(), fixture.root().random(), fixture.root().logMap(), settings, {tempDir});
    assets.queueImages(StringList{"/sample.png?flipx", "/sample.png?flipy", "/sample.png"});
  }
});

const GameTestRegistration pngRoundTripPreservesPixelsAndMetadata("ImageTest", "PngRoundTripPreservesPixelsAndMetadata", [](GameTest& fixture) {
  for (auto format : {PixelFormat::RGB24, PixelFormat::RGBA32}) {
    auto image = pngTestImage(format);
    auto buffer = std::make_shared<Buffer>();
    image.writePng(fixture.root().logger(), buffer);
    buffer->seek(0);
    auto [size, decodedFormat] = Image::readPngMetadata(fixture.root().logger(), buffer);
    EXPECT_EQ(size, image.size());
    EXPECT_EQ(decodedFormat, format);
    buffer->seek(0);
    auto decoded = Image::readPng(fixture.root().logger(), buffer);
    EXPECT_EQ(decoded.pixelFormat(), format);
    for (unsigned y = 0; y < 2; ++y)
      for (unsigned x = 0; x < 2; ++x)
        EXPECT_EQ(decoded.get(x, y), image.get(x, y));
  }
});

const GameTestRegistration damagedPngHeaderAndImageDataThrow("ImageTest", "DamagedPngHeaderAndImageDataThrow", [](GameTest& fixture) {
  auto bytes = pngTestBytes(fixture.root().logger());
  ASSERT_GT(bytes.size(), 41u);
  ASSERT_EQ(std::memcmp(bytes.ptr() + 37, "IDAT", 4), 0);
  auto badHeader = bytes;
  badHeader[29] ^= 1; // IHDR CRC: signature is intact, metadata parsing fails.
  auto badPixels = bytes;
  badPixels[41] = 0; // Invalid zlib header, reached after Image/rows allocation.
  for (unsigned attempt = 0; attempt < 16; ++attempt) {
    EXPECT_THROW(Image::readPngMetadata(fixture.root().logger(), std::make_shared<Buffer>(badHeader)), ImageException);
    EXPECT_THROW(Image::readPng(fixture.root().logger(), std::make_shared<Buffer>(badHeader)), ImageException);
    EXPECT_NO_THROW(Image::readPngMetadata(fixture.root().logger(), std::make_shared<Buffer>(badPixels)));
    EXPECT_THROW(Image::readPng(fixture.root().logger(), std::make_shared<Buffer>(badPixels)), ImageException);
  }
});

const GameTestRegistration pngCallbacksRethrowOriginalIoException("ImageTest", "PngCallbacksRethrowOriginalIoException", [](GameTest& fixture) {
  auto bytes = pngTestBytes(fixture.root().logger());
  for (unsigned attempt = 0; attempt < 16; ++attempt) {
    auto metadata = std::make_shared<FailingPngBuffer>(bytes, 8);
    EXPECT_THROW(Image::readPngMetadata(fixture.root().logger(), metadata), PngIoFailure);
    EXPECT_EQ(metadata->unwound, 1u);
    auto header = std::make_shared<FailingPngBuffer>(bytes, 8);
    EXPECT_THROW(Image::readPng(fixture.root().logger(), header), PngIoFailure);
    EXPECT_EQ(header->unwound, 1u);
    auto pixels = std::make_shared<FailingPngBuffer>(bytes, 41);
    EXPECT_THROW(Image::readPng(fixture.root().logger(), pixels), PngIoFailure);
    EXPECT_EQ(pixels->unwound, 1u);
    for (StreamOffset failAt : {0, 41}) {
      auto output = std::make_shared<FailingPngBuffer>(ByteArray{}, failAt);
      EXPECT_THROW(pngTestImage(PixelFormat::RGBA32).writePng(fixture.root().logger(), output), PngIoFailure);
      EXPECT_EQ(output->unwound, 1u);
    }
  }
});

const GameTestRegistration pngErrorCallbackContainsDiagnosticExceptions("ImageTest", "PngErrorCallbackContainsDiagnosticExceptions", [](GameTest& fixture) {
  auto damaged = pngTestBytes(fixture.root().logger());
  damaged[29] ^= 1;
  EXPECT_THROW(Image::readPngMetadata(fixture.root().logger(), std::make_shared<FailingPngNameBuffer>(damaged)), PngIoFailure);
  EXPECT_THROW(Image::readPng(fixture.root().logger(), std::make_shared<FailingPngNameBuffer>(damaged)), PngIoFailure);
});

const GameTestRegistration invalidPngWriteThrows("ImageTest", "InvalidPngWriteThrows", [](GameTest& fixture) {
  auto output = std::make_shared<Buffer>();
  EXPECT_THROW(Image().writePng(fixture.root().logger(), output), ImageException);
});
