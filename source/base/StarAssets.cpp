#include "StarAssets.hpp"

#include "StarAssetPath.hpp"
#include "StarAudio.hpp"
#include "StarCasting.hpp"
#include "StarConfig.hpp"
#include "StarDataStreamDevices.hpp"
#include "StarDirectoryAssetSource.hpp"
#include "StarFile.hpp"
#include "StarFont.hpp"
#include "StarImage.hpp"
#include "StarImageLuaBindings.hpp" // IWYU pragma: export
#include "StarImageProcessing.hpp"
#include "StarIterator.hpp"
#include "StarJsonBuilder.hpp"
#include "StarJsonExtra.hpp"
#include "StarJsonPatch.hpp"
#include "StarLogging.hpp"
#include "StarLua.hpp"
#include "StarMemoryAssetSource.hpp"
#include "StarPackedAssetSource.hpp"
#include "StarSha256.hpp"
#include "StarTime.hpp"
#include "StarUtilityLuaBindings.hpp"

import std;

namespace Star {
using std::bind;

// if a ptr is returned, can be optionally used to format an error
static auto validateBasePath(std::string_view const& basePath) -> const char* {
  if (basePath.empty() || basePath[0] != '/')
    return "Path '{}' must be absolute";

  bool first = true;
  bool slashed = true;
  bool dotted = false;
  for (auto c : basePath) {
    if (c == '/') {
      if (!first) {
        if (slashed)
          return "Path '{}' contains consecutive //, not allowed";
        else if (dotted)
          return "Path '{}' '.' and '..' not allowed";
      }
      slashed = true;
      dotted = false;
    } else if (c == ':') {
      if (slashed)
        return "Path '{}' has ':' after directory";
      break;
    } else if (c == '?') {
      if (slashed)
        return "Path '{}' has '?' after directory";
      break;
    } else {
      slashed = false;
      dotted = c == '.';
    }
    first = false;
  }
  if (slashed)
    return "Path '{}' cannot be a file";

  return nullptr;
}

static auto validatePath(AssetPath const& components, bool canContainSubPath, bool canContainDirectives, bool throwing = true) -> bool {
  if (auto error = validateBasePath(components.basePath.utf8())) {
    if (throwing)
      throw AssetException::vformat(error, components.basePath.utf8());
    else
      return false;
  }

  if (!canContainSubPath && components.subPath) {
    if (throwing)
      throw AssetException::format("Path '{}' cannot contain sub-path", components);
    else
      return false;
  } else if (!canContainDirectives && !components.directives.empty()) {
    if (throwing)
      throw AssetException::format("Path '{}' cannot contain directives", components);
    else
      return false;
  }

  return true;
}

[[maybe_unused]] static auto validatePath(StringView path, [[maybe_unused]] bool canContainSubPath, bool canContainDirectives, bool throwing = true) -> bool {
  std::string_view const& str = path.utf8();

  size_t end = str.find_first_of(":?");
  auto basePath = str.substr(0, end);
  if (auto error = validateBasePath(basePath)) {
    if (throwing)
      throw AssetException::vformat(error, basePath);
    else
      return false;
  }

  bool subPath = false;
  if (str[end] == ':') {
    size_t beg = end + 1;
    if (beg != str.size()) {
      end = str.find_first_of('?', beg);
      if (end == std::numeric_limits<std::size_t>::max() && beg + 1 != str.size())
        subPath = true;
      else if (size_t _ = end - beg)
        subPath = true;
    }
  }

  if (subPath) {
    if (throwing)
      throw AssetException::format(std::string_view("Path '{}' cannot contain sub-path"), path);
    else
      return false;
  } else if (end != std::numeric_limits<std::size_t>::max() && str[end] == '?' && !canContainDirectives) {
    if (throwing)
      throw AssetException::format(std::string_view("Path '{}' cannot contain directives"), path);
    else
      return false;
  }

  return true;
}

auto FramesSpecification::getRect(String const& frame) const -> std::optional<RectU> {
  if (auto alias = aliases.ptr(frame)) {
    return frames.get(*alias);
  } else {
    return frames.maybe(frame);
  }
}

auto FramesSpecification::toJson() const -> Json {
  return JsonObject{
    {"aliases", jsonFromMap(aliases)},
    {"frames", jsonFromMapV(frames, jsonFromRectU)},
    {"file", framesFile}};
}

Assets::Assets(Settings settings, StringList assetSources) {
  const char* AssetsPatchSuffix = ".patch";
  const char* AssetsPatchListSuffix = ".patchlist";
  const char* AssetsLuaPatchSuffix = ".patch.lua";

  m_settings = std::move(settings);
  m_stopThreads = false;
  m_assetSources = std::move(assetSources);

  auto luaEngine = LuaEngine::create();
  m_luaEngine = luaEngine;
  auto pushGlobalContext = [&luaEngine](String const& name, LuaCallbacks&& callbacks) -> void {
    auto table = luaEngine->createTable();
    for (auto const& p : callbacks.callbacks())
      table.set(p.first, luaEngine->createWrappedFunction(p.second));
    luaEngine->setGlobal(name, table);
  };

  auto makeBaseAssetCallbacks = [this]() -> LuaCallbacks {
    LuaCallbacks callbacks;
    callbacks.registerCallbackWithSignature<StringSet, String>("byExtension", [this](auto&& PH1) -> auto { return scanExtension(std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<Json, String>("json", [this](auto&& PH1) -> auto { return json(std::forward<decltype(PH1)>(PH1)); });
    callbacks.registerCallbackWithSignature<bool, String>("exists", [this](auto&& PH1) -> auto { return assetExists(std::forward<decltype(PH1)>(PH1)); });

    callbacks.registerCallback("sourcePaths", [this](LuaEngine& engine, std::optional<bool> withMetaData) -> LuaTable {
      auto assetSources = this->assetSources();
      auto table = engine.createTable(assetSources.size(), 0);
      if (withMetaData.value()) {
        for (auto& assetSource : assetSources)
          table.set(assetSource, this->assetSourceMetadata(assetSource));
      } else {
        size_t i = 0;
        for (auto& assetSource : assetSources)
          table.set(++i, assetSource);
      }
      return table;
    });

    callbacks.registerCallback("sourceMetadata", [this](String const& sourcePath) -> std::optional<JsonObject> {
      if (auto assetSource = m_assetSourcePaths.rightPtr(sourcePath))
        return (*assetSource)->metadata();
      return {};
    });

    callbacks.registerCallback("origin", [this](String const& path) -> std::optional<String> {
      if (auto descriptor = this->assetDescriptor(path))
        return this->assetSourcePath(descriptor->source);
      return {};
    });

    callbacks.registerCallback("bytes", [this](String const& path) -> String {
      auto assetBytes = bytes(path);
      return {assetBytes->ptr(), assetBytes->size()};
    });

    callbacks.registerCallback("image", [this](String const& path) -> Image {
      auto assetImage = image(path);
      if (assetImage->bytesPerPixel() == 3)
        return assetImage->convert(PixelFormat::RGBA32);
      else
        return *assetImage;
    });

    callbacks.registerCallback("frames", [this](String const& path) -> Json {
      if (auto frames = imageFrames(path))
        return frames->toJson();
      return {};
    });

    callbacks.registerCallback("scan", [this](std::optional<String> const& a, std::optional<String> const& b) -> StringList {
      return b ? scan(a.value(), *b) : scan(a.value());
    });
    return callbacks;
  };

  pushGlobalContext("sb", LuaBindings::makeUtilityCallbacks());
  pushGlobalContext("assets", makeBaseAssetCallbacks());

  auto decorateLuaContext = [&](LuaContext& context, Ptr<MemoryAssetSource> newFiles) -> void {
    if (newFiles) {
      // re-add the assets callbacks with more functions
      context.remove("assets");
      auto callbacks = makeBaseAssetCallbacks();
      callbacks.registerCallback("add", [newFiles](LuaEngine& engine, String const& path, LuaValue const& data) -> void {
        ByteArray bytes;
        if (auto str = engine.luaMaybeTo<String>(data))
          bytes = ByteArray(str->utf8Ptr(), str->utf8Size());
        else if (auto image = engine.luaMaybeTo<Image>(data)) {
          newFiles->set(path, std::move(*image));
          return;
        } else {
          auto json = engine.luaTo<Json>(data).repr();
          bytes = ByteArray(json.utf8Ptr(), json.utf8Size());
        }
        newFiles->set(path, bytes);
      });

      callbacks.registerCallback("patch", [this, newFiles](String const& path, String const& patchPath) -> bool {
        if (newFiles->contains(path)) {
          if (auto image = newFiles->image(path)) {
            if (newFiles->contains(patchPath)) {
              newFiles->set(path, *applyImagePatches(image, path, {make_pair(patchPath, newFiles)}).get());
              return true;
            } else {
              if (auto asset = m_files.ptr(patchPath)) {
                newFiles->set(path, *applyImagePatches(image, path, {make_pair(patchPath, asset->source)}).get());
                return true;
              }
            }
          } else {
            if (newFiles->contains(patchPath)) {
              auto file = newFiles->read(path);
              auto json = applyJsonPatches(inputUtf8Json(file.begin(), file.end(), JsonParseType::Top), path, {make_pair(patchPath, newFiles)}).repr();
              newFiles->set(path, ByteArray(json.utf8Ptr(), json.utf8Size()));
              return true;
            } else {
              if (auto asset = m_files.ptr(patchPath)) {
                auto file = newFiles->read(path);
                auto json = applyJsonPatches(inputUtf8Json(file.begin(), file.end(), JsonParseType::Top), path, {make_pair(patchPath, asset->source)}).repr();
                newFiles->set(path, ByteArray(json.utf8Ptr(), json.utf8Size()));
                return true;
              }
            }
          }
        } else if (auto file = m_files.ptr(path)) {
          if (newFiles->contains(patchPath)) {
            file->patchSources.append(make_pair(patchPath, newFiles));
            return true;
          } else {
            if (auto asset = m_files.ptr(patchPath)) {
              file->patchSources.append(make_pair(patchPath, asset->source));
              return true;
            }
          }
        }
        return false;
      });

      callbacks.registerCallback("erase", [this](String const& path) -> bool {
        bool erased = m_files.erase(path);
        if (erased)
          m_filesByExtension[AssetPath::extension(path).toLower()].erase(path);
        return erased;
      });

      context.setCallbacks("assets", callbacks);
    }
  };

  auto addSource = [&](String const& sourcePath, Ptr<AssetSource> source) -> void {
    m_assetSourcePaths.add(sourcePath, source);

    for (auto const& filename : source->assetPaths()) {
      if (filename.contains(AssetsPatchSuffix, String::CaseInsensitive)) {
        if (filename.endsWith(AssetsPatchSuffix, String::CaseInsensitive)) {
          auto targetPatchFile = filename.substr(0, filename.size() - std::strlen(AssetsPatchSuffix));
          if (auto p = m_files.ptr(targetPatchFile))
            p->patchSources.append({filename, source});
        } else if (filename.endsWith(AssetsLuaPatchSuffix, String::CaseInsensitive)) {
          auto targetPatchFile = filename.substr(0, filename.size() - std::strlen(AssetsLuaPatchSuffix));
          if (auto p = m_files.ptr(targetPatchFile))
            p->patchSources.append({filename, source});
        } else if (filename.endsWith(AssetsPatchListSuffix, String::CaseInsensitive)) {
          auto stream = source->read(filename);
          size_t patchIndex = 0;
          for (auto const& patchPair : inputUtf8Json(stream.begin(), stream.end(), JsonParseType::Top).iterateArray()) {
            auto patches = patchPair.getArray("patches");
            for (auto& path : patchPair.getArray("paths")) {
              if (auto p = m_files.ptr(path.toString())) {
                for (size_t i = 0; i != patches.size(); ++i) {
                  auto& patch = patches[i];
                  if (patch.isType(Json::Type::String))
                    p->patchSources.append({patch.toString(), source});
                  else
                    p->patchSources.append({strf(std::string_view("{}:[{}].patches[{}]"), filename, patchIndex, i), source});
                }
              }
            }
            patchIndex++;
          }
        } else {
          for (int i = 0; i < 10; i++) {
            if (filename.endsWith(AssetsPatchSuffix + toString(i), String::CaseInsensitive)) {
              auto targetPatchFile = filename.substr(0, filename.size() - std::strlen(AssetsPatchSuffix) + 1);
              if (auto p = m_files.ptr(targetPatchFile))
                p->patchSources.append({filename, source});
              break;
            }
          }
        }
      }

      auto& descriptor = m_files[filename];
      descriptor.sourceName = filename;
      descriptor.source = source;
      m_filesByExtension[AssetPath::extension(filename).toLower()].insert(filename);
    }
  };

  auto runLoadScripts = [&](String const& groupName, String const& sourcePath, Ptr<AssetSource> source) -> void {
    auto metadata = source->metadata();
    if (auto scripts = metadata.ptr("scripts")) {
      if (auto scriptGroup = scripts->optArray(groupName)) {
        auto memoryName = strf(std::string_view("{}::{}"), metadata.value("name", File::baseName(sourcePath)), groupName);
        JsonObject memoryMetadata{{"name", memoryName}};
        auto memoryAssets = make_shared<MemoryAssetSource>(memoryName, memoryMetadata);
        auto now = Time::monotonicTime();
        Logger::info(std::string_view("Running {} scripts {}"), groupName, *scriptGroup);
        try {
          auto context = luaEngine->createContext();
          decorateLuaContext(context, memoryAssets);
          for (auto& jPath : *scriptGroup) {
            auto path = jPath.toString();
            auto script = source->read(path);
            context.load(script, path);
          }
        } catch (LuaException const& e) {
          Logger::error(std::string_view("Exception while running {} scripts from asset source '{}': {}"), groupName, sourcePath, e.what());
        }
        Logger::info(std::string_view("Took {} seconds to run {} scripts {}"), Time::monotonicTime() - now, groupName, *scriptGroup);
        if (!memoryAssets->empty())
          addSource(strf(std::string_view("{}::{}"), sourcePath, groupName), memoryAssets);
      }
    }
    // clear any caching that may have been trigered by load scripts as they may no longer be valid
    m_framesSpecifications.clear();
    m_assetsCache.clear();
  };

  List<std::pair<String, Ptr<AssetSource>>> sources;

  for (auto& sourcePath : m_assetSources) {
    Logger::info(std::string_view("Loading assets from: '{}'"), sourcePath);
    Ptr<AssetSource> source;
    if (File::isDirectory(sourcePath))
      source = std::make_shared<DirectoryAssetSource>(sourcePath, m_settings.pathIgnore);
    else
      source = std::make_shared<PackedAssetSource>(sourcePath);

    addSource(sourcePath, source);
    sources.append(make_pair(sourcePath, source));

    runLoadScripts("onLoad", sourcePath, source);
  }

  for (auto& pair : sources)
    runLoadScripts("postLoad", pair.first, pair.second);

  Sha256Hasher digest;

  for (auto const& assetPath : m_files.keys().transformed([](String const& s) -> String {
                                               return s.toLower();
                                             })
                                 .sorted()) {
    bool digestFile = true;
    for (auto const& pattern : m_settings.digestIgnore) {
      if (assetPath.regexMatch(pattern, false, false)) {
        digestFile = false;
        break;
      }
    }

    auto const& descriptor = m_files.get(assetPath);

    if (digestFile) {
      digest.push(assetPath);
      digest.push(DataStreamBuffer::serialize(descriptor.source->open(descriptor.sourceName)->size()));
      for (auto const& pair : descriptor.patchSources)
        digest.push(DataStreamBuffer::serialize(pair.second->open(AssetPath::removeSubPath(pair.first))->size()));
    }
  }

  m_digest = digest.compute();

  int workerPoolSize = m_settings.workerPoolSize;
  for (int i = 0; i < workerPoolSize; i++)
    m_workerThreads.append(Thread::invoke("Assets::workerMain", std::mem_fn(&Assets::workerMain), this));

  // preload.config contains an array of files which will be loaded and then told to persist
  Json preload = json("/preload.config");
  Logger::info(std::string_view("Preloading assets"));
  for (auto script : preload.iterateArray()) {
    auto type = AssetTypeNames.getLeft(script.getString("type"));
    auto path = script.getString("path");
    auto components = AssetPath::split(path);
    validatePath(components, type == AssetType::Json || type == AssetType::Image, type == AssetType::Image);

    auto asset = getAsset(AssetId{.type = type, .path = std::move(components)});
    // make this asset never unload
    asset->forcePersist = true;
  }
}

Assets::~Assets() {
  m_stopThreads = true;

  {
    // Should lock associated mutex to prevent loss of wakeups,
    MutexLocker locker(m_assetsMutex);
    // Notify all worker threads to allow them to stop
    m_assetsQueued.broadcast();
  }

  // Join them all
  m_workerThreads.clear();
}

void Assets::hotReload() const {
  MutexLocker assetsLocker(m_assetsMutex);
  m_assetsCache.clear();
  m_queue.clear();
  m_framesSpecifications.clear();
}

auto Assets::assetSources() const -> StringList {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSources;
}

auto Assets::assetSourceMetadata(String const& sourceName) const -> JsonObject {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSourcePaths.getRight(sourceName)->metadata();
}

auto Assets::digest() const -> ByteArray {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_digest;
}

auto Assets::assetExists(String const& path) const -> bool {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_files.contains(path);
}

auto Assets::assetDescriptor(String const& path) const -> std::optional<Assets::AssetFileDescriptor> {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_files.maybe(path);
}

auto Assets::assetSource(String const& path) const -> String {
  MutexLocker assetsLocker(m_assetsMutex);
  if (auto p = m_files.ptr(path))
    return m_assetSourcePaths.getLeft(p->source);
  throw AssetException(strf(std::string_view("No such asset '{}'"), path));
}

auto Assets::assetSourcePath(Ptr<AssetSource> const& source) const -> std::optional<String> {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSourcePaths.maybeLeft(source);
}

auto Assets::scan(String const& suffix) const -> StringList {
  if (suffix.beginsWith(".") && !suffix.substr(1).hasChar('.')) {
    return scanExtension(suffix).values();
  } else if (suffix.empty()) {
    return m_files.keys();
  } else {
    StringList result;
    for (auto const& fileEntry : m_files) {
      String const& file = fileEntry.first;
      if (file.endsWith(suffix, String::CaseInsensitive))
        result.append(file);
    }

    return result;
  }
}

auto Assets::scan(String const& prefix, String const& suffix) const -> StringList {
  StringList result;
  if (suffix.beginsWith(".") && !suffix.substr(1).hasChar('.')) {
    auto& filesWithExtension = scanExtension(suffix);
    for (auto const& file : filesWithExtension) {
      if (file.beginsWith(prefix, String::CaseInsensitive))
        result.append(file);
    }
  } else {
    for (auto const& fileEntry : m_files) {
      String const& file = fileEntry.first;
      if (file.beginsWith(prefix, String::CaseInsensitive) && file.endsWith(suffix, String::CaseInsensitive))
        result.append(file);
    }
  }
  return result;
}

const CaseInsensitiveStringSet NullExtensionScan;

auto Assets::scanExtension(String const& extension) const -> CaseInsensitiveStringSet const& {
  auto find = m_filesByExtension.find(extension.beginsWith(".") ? extension.substr(1) : extension);
  return find != m_filesByExtension.end() ? find->second : NullExtensionScan;
}

auto Assets::json(String const& path) const -> Json {
  auto components = AssetPath::split(path);
  validatePath(components, true, false);

  return as<JsonData>(getAsset(AssetId{.type = AssetType::Json, .path = std::move(components)}))->json;
}

auto Assets::fetchJson(Json const& v, String const& dir) const -> Json {
  if (v.isType(Json::Type::String))
    return Assets::json(AssetPath::relativeTo(dir, v.toString()));
  else
    return v;
}

void Assets::queueJsons(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) -> AssetId {
    auto components = AssetPath::split(path);
    validatePath(components, true, false);

    return AssetId{.type = AssetType::Json, .path = {components.basePath, {}, {}}};
  }));
}

void Assets::queueJsons(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, true, false);

    queueAsset(AssetId{.type = AssetType::Json, .path = {components.basePath, {}, {}}});
  };
}

auto Assets::image(AssetPath const& path) const -> ConstPtr<Image> {
  return as<ImageData>(getAsset(AssetId{.type = AssetType::Image, .path = path}))->image;
}

void Assets::queueImages(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) -> AssetId {
    auto components = AssetPath::split(path);
    validatePath(components, true, true);

    return AssetId{.type = AssetType::Image, .path = std::move(components)};
  }));
}

void Assets::queueImages(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, true, true);

    queueAsset(AssetId{.type = AssetType::Image, .path = std::move(components)});
  };
}

auto Assets::tryImage(AssetPath const& path) const -> ConstPtr<Image> {
  if (auto imageData = as<ImageData>(tryAsset(AssetId{.type = AssetType::Image, .path = path})))
    return imageData->image;
  else
    return {};
}

auto Assets::imageFrames(String const& path) const -> ConstPtr<FramesSpecification> {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  MutexLocker assetsLocker(m_assetsMutex);
  return bestFramesSpecification(path);
}

auto Assets::audio(String const& path) const -> ConstPtr<Audio> {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<AudioData>(getAsset(AssetId{.type = AssetType::Audio, .path = std::move(components)}))->audio;
}

void Assets::queueAudios(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) -> AssetId {
    auto components = AssetPath::split(path);
    validatePath(components, false, false);

    return AssetId{.type = AssetType::Audio, .path = std::move(components)};
  }));
}

void Assets::queueAudios(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, false, true);

    queueAsset(AssetId{.type = AssetType::Audio, .path = std::move(components)});
  };
}

auto Assets::tryAudio(String const& path) const -> ConstPtr<Audio> {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  if (auto audioData = as<AudioData>(tryAsset(AssetId{.type = AssetType::Audio, .path = std::move(components)})))
    return audioData->audio;
  else
    return {};
}

auto Assets::font(String const& path) const -> ConstPtr<Font> {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<FontData>(getAsset(AssetId{.type = AssetType::Font, .path = std::move(components)}))->font;
}

auto Assets::bytes(String const& path) const -> ConstPtr<ByteArray> {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<BytesData>(getAsset(AssetId{.type = AssetType::Bytes, .path = std::move(components)}))->bytes;
}

auto Assets::openFile(String const& path) const -> Ptr<IODevice> {
  return open(path);
}

void Assets::clearCache() {
  MutexLocker assetsLocker(m_assetsMutex);

  // Clear all assets that are not queued or broken.
  auto it = makeSMutableMapIterator(m_assetsCache);
  while (it.hasNext()) {
    auto const& pair = it.next();
    // Don't clean up queued, persistent, or broken assets.
    if (pair.second && !pair.second->shouldPersist() && !m_queue.contains(pair.first))
      it.remove();
  }
}

void Assets::cleanup() {
  MutexLocker assetsLocker(m_assetsMutex);

  double time = Time::monotonicTime();

  auto it = makeSMutableMapIterator(m_assetsCache);
  while (it.hasNext()) {
    auto pair = it.next();
    // Don't clean up broken assets or queued assets.
    if (pair.second && !m_queue.contains(pair.first)) {
      double liveTime = time - pair.second->time;
      if (liveTime > m_settings.assetTimeToLive) {
        // If the asset should persist, just refresh the access time.
        if (pair.second->shouldPersist())
          pair.second->time = time;
        else
          it.remove();
      }
    }
  }
}

auto Assets::AssetId::operator==(AssetId const& assetId) const -> bool {
  return std::tie(type, path) == std::tie(assetId.type, assetId.path);
}

auto Assets::AssetIdHash::operator()(AssetId const& id) const -> size_t {
  return hashOf(id.type, id.path.basePath, id.path.subPath, id.path.directives);
}

auto Assets::JsonData::shouldPersist() const -> bool {
  return forcePersist || json.unique();
}

auto Assets::ImageData::shouldPersist() const -> bool {
  return forcePersist || (!alias && image.use_count() != 1);
}

auto Assets::AudioData::shouldPersist() const -> bool {
  return forcePersist || audio.use_count() != 1;
}

auto Assets::FontData::shouldPersist() const -> bool {
  return forcePersist || font.use_count() != 1;
}

auto Assets::BytesData::shouldPersist() const -> bool {
  return forcePersist || bytes.use_count() != 1;
}

auto Assets::parseFramesSpecification(Json const& frameConfig, String path) -> FramesSpecification {
  FramesSpecification framesSpecification;

  framesSpecification.framesFile = std::move(path);

  if (frameConfig.contains("frameList")) {
    for (auto const& pair : frameConfig.get("frameList").toObject()) {
      String frameName = pair.first;
      RectU rect = RectU(jsonToRectI(pair.second));
      if (rect.isEmpty())
        throw AssetException(
          strf(std::string_view("Empty rect in frame specification in image {} frame {}"), framesSpecification.framesFile, frameName));
      else
        framesSpecification.frames[frameName] = rect;
    }
  }

  if (frameConfig.contains("frameGrid")) {
    auto grid = frameConfig.get("frameGrid").toObject();

    Vec2U begin(jsonToVec2I(grid.value("begin", jsonFromVec2I(Vec2I()))));
    Vec2U size(jsonToVec2I(grid.get("size")));
    Vec2U dimensions(jsonToVec2I(grid.get("dimensions")));

    if (dimensions[0] == 0 || dimensions[1] == 0)
      throw AssetException(strf(std::string_view("Image {} \"dimensions\" in frameGrid cannot be zero"), framesSpecification.framesFile));

    if (grid.contains("names")) {
      auto nameList = grid.get("names");
      for (size_t y = 0; y < nameList.size(); ++y) {
        if (y >= dimensions[1])
          throw AssetException(strf(std::string_view("Image {} row {} is out of bounds for y-dimension {}"),
                                    framesSpecification.framesFile,
                                    y + 1,
                                    dimensions[1]));
        auto rowList = nameList.get(y);
        if (rowList.isNull())
          continue;
        for (unsigned x = 0; x < rowList.size(); ++x) {
          if (x >= dimensions[0])
            throw AssetException(strf(std::string_view("Image {} column {} is out of bounds for x-dimension {}"),
                                      framesSpecification.framesFile,
                                      x + 1,
                                      dimensions[0]));

          auto frame = rowList.get(x);
          if (frame.isNull())
            continue;
          auto frameName = frame.toString();
          if (!frameName.empty())
            framesSpecification.frames[frameName] =
              RectU::withSize(Vec2U(begin[0] + x * size[0], begin[1] + y * size[1]), size);
        }
      }
    } else {
      // If "names" not specified, use auto naming algorithm
      for (size_t y = 0; y < dimensions[1]; ++y)
        for (size_t x = 0; x < dimensions[0]; ++x)
          framesSpecification.frames[toString(y * dimensions[0] + x)] =
            RectU::withSize(Vec2U(begin[0] + x * size[0], begin[1] + y * size[1]), size);
    }
  }

  if (auto aliasesConfig = frameConfig.opt("aliases")) {
    auto aliases = aliasesConfig->objectPtr();
    for (auto const& pair : *aliases) {
      String const& key = pair.first;
      String value = pair.second.toString();

      // Resolve aliases to aliases by checking to see if the alias value in
      // the alias map itself.  Don't do this more than aliases.size() times to
      // avoid infinite cycles.
      for (size_t i = 0; i <= aliases->size(); ++i) {
        auto it = aliases->find(value);
        if (it != aliases->end()) {
          if (i == aliases->size())
            throw AssetException(strf(std::string_view("Infinite alias loop detected for alias '{}'"), key));

          value = it->second.toString();
        } else {
          break;
        }
      }

      if (!framesSpecification.frames.contains(value))
        throw AssetException(strf(std::string_view("No such frame '{}' found for alias '{}'"), value, key));
      framesSpecification.aliases[key] = std::move(value);
    }
  }

  return framesSpecification;
}

void Assets::queueAssets(List<AssetId> const& assetIds) const {
  MutexLocker assetsLocker(m_assetsMutex);

  for (auto const& id : assetIds)
    queueAsset(id);
}

void Assets::queueAsset(AssetId const& assetId) const {
  auto i = m_assetsCache.find(assetId);
  if (i != m_assetsCache.end()) {
    if (i->second)
      freshen(i->second);
  } else {
    auto j = m_queue.find(assetId);
    if (j == m_queue.end()) {
      m_queue[assetId] = QueuePriority::Load;
      m_assetsQueued.signal();
    }
  }
}

auto Assets::tryAsset(AssetId const& id) const -> std::shared_ptr<Assets::AssetData> {
  MutexLocker assetsLocker(m_assetsMutex);

  auto i = m_assetsCache.find(id);
  if (i != m_assetsCache.end()) {
    if (i->second) {
      freshen(i->second);
      return i->second;
    } else {
      throw AssetException::format(std::string_view("Error loading asset {}"), id.path);
    }
  } else {
    auto j = m_queue.find(id);
    if (j == m_queue.end()) {
      m_queue[id] = QueuePriority::Load;
      m_assetsQueued.signal();
    }
    return {};
  }
}

auto Assets::getAsset(AssetId const& id) const -> std::shared_ptr<Assets::AssetData> {
  MutexLocker assetsLocker(m_assetsMutex);

  while (true) {
    auto j = m_assetsCache.find(id);
    if (j != m_assetsCache.end()) {
      if (j->second) {
        auto asset = j->second;
        freshen(asset);
        return asset;
      } else {
        throw AssetException::format(std::string_view("Error loading asset {}"), id.path);
      }
    } else {
      // Try to load the asset in-thread, if we cannot, then the asset has been
      // queued so wait for a worker thread to finish it.
      if (!doLoad(id))
        m_assetsDone.wait(m_assetsMutex);
    }
  }
}

void Assets::workerMain() {
  while (true) {
    if (m_stopThreads)
      break;

    {
      RecursiveMutexLocker luaLocker(m_luaMutex);
      as<LuaEngine>(m_luaEngine.get())->collectGarbage();
    }

    MutexLocker assetsLocker(m_assetsMutex);

    AssetId assetId;
    QueuePriority queuePriority = QueuePriority::None;

    // Find the highest priority queue entry
    for (auto const& pair : m_queue) {
      if (pair.second == QueuePriority::Load || pair.second == QueuePriority::PostProcess) {
        assetId = pair.first;
        queuePriority = pair.second;
        if (pair.second == QueuePriority::Load)
          break;
      }
    }

    if (queuePriority != QueuePriority::Load && queuePriority != QueuePriority::PostProcess) {
      // Nothing in the queue that needs work
      m_assetsQueued.wait(m_assetsMutex);
      continue;
    }

    bool workIsBlocking;
    if (queuePriority == QueuePriority::PostProcess)
      workIsBlocking = !doPost(assetId);
    else
      workIsBlocking = !doLoad(assetId);

    if (workIsBlocking) {
      // We are blocking on some sort of busy asset, so need to wait on
      // something to complete here, rather than spinning and burning cpu.
      m_assetsDone.wait(m_assetsMutex);
      continue;
    }

    // After processing an asset, unlock the main asset mutex and yield so we
    // don't starve other threads.
    assetsLocker.unlock();
    Thread::yield();
  }
}

template <typename Function>
auto Assets::unlockDuring(Function f) const -> decltype(auto) {
  m_assetsMutex.unlock();
  try {
    auto r = f();
    m_assetsMutex.lock();
    return r;
  } catch (...) {
    m_assetsMutex.lock();
    throw;
  }
}

auto Assets::bestFramesSpecification(String const& image) const -> ConstPtr<FramesSpecification> {
  if (auto framesSpecification = m_framesSpecifications.maybe(image)) {
    return *framesSpecification;
  }

  String framesFile;

  if (auto bestFramesFile = m_bestFramesFiles.maybe(image)) {
    framesFile = *bestFramesFile;

  } else {
    String searchPath = AssetPath::directory(image);
    String filePrefix = AssetPath::filename(image);
    filePrefix = filePrefix.substr(0, filePrefix.findLast('.'));

    auto subdir = [](String const& dir) -> String {
      auto dirsplit = dir.substr(0, dir.size() - 1).rsplit("/", 1);
      if (dirsplit.size() < 2)
        return "";
      else
        return dirsplit[0] + "/";
    };

    std::optional<String> foundFramesFile;

    // look for <full-path-minus-extension>.frames or default.frames up to root
    while (!searchPath.empty()) {
      String framesPath = searchPath + filePrefix + ".frames";
      if (m_files.contains(framesPath)) {
        foundFramesFile = framesPath;
        break;
      }

      framesPath = searchPath + "default.frames";
      if (m_files.contains(framesPath)) {
        foundFramesFile = framesPath;
        break;
      }

      searchPath = subdir(searchPath);
    }

    if (foundFramesFile) {
      framesFile = std::move(*foundFramesFile);
      m_bestFramesFiles[image] = framesFile;

    } else {
      return {};
    }
  }

  auto framesSpecification = unlockDuring([&]() -> std::shared_ptr<Star::FramesSpecification> {
    return std::make_shared<FramesSpecification>(parseFramesSpecification(readJson(framesFile), framesFile));
  });
  m_framesSpecifications[image] = framesSpecification;

  return framesSpecification;
}

auto Assets::open(String const& path) const -> Ptr<IODevice> {
  if (auto p = m_files.ptr(path))
    return p->source->open(p->sourceName);
  throw AssetException(strf(std::string_view("No such asset '{}'"), path));
}

auto Assets::read(String const& path) const -> ByteArray {
  if (auto p = m_files.ptr(path))
    return p->source->read(p->sourceName);
  throw AssetException(strf(std::string_view("No such asset '{}'"), path));
}

auto Assets::readImage(String const& path) const -> ConstPtr<Image> {
  if (auto p = m_files.ptr(path)) {
    ConstPtr<Image> image;
    if (auto memorySource = as<MemoryAssetSource>(p->source))
      image = memorySource->image(p->sourceName);
    if (!image)
      image = std::make_shared<Image>(Image::readPng(p->source->open(p->sourceName)));

    if (!p->patchSources.empty()) {
      return applyImagePatches(image, path, p->patchSources);
    }
    return image;
  }
  throw AssetException(strf(std::string_view("No such asset '{}'"), path));
}

auto Assets::applyImagePatches(ConstPtr<Image> image, String const& path, List<std::pair<String, Ptr<AssetSource>>> patches) const -> ConstPtr<Image> {
  RecursiveMutexLocker luaLocker(m_luaMutex);
  auto* luaEngine = as<LuaEngine>(m_luaEngine.get());
  LuaValue result = luaEngine->createUserData(*image);
  luaLocker.unlock();
  for (auto const& pair : patches) {
    auto& patchPath = pair.first;
    auto& patchSource = pair.second;
    auto patchStream = patchSource->read(patchPath);
    if (patchPath.endsWith(".lua")) {
      std::pair<AssetSource*, String> contextKey = std::make_pair(patchSource.get(), patchPath);
      luaLocker.lock();
      Ptr<LuaContext>& context = m_patchContexts[contextKey];
      if (!context) {
        context = std::make_shared<LuaContext>(luaEngine->createContext());
        context->load(patchStream, patchPath);
      }
      auto newResult = context->invokePath<LuaValue>("patch", result, path);
      if (!newResult.is<LuaNilType>()) {
        if (auto ud = newResult.ptr<LuaUserData>()) {
          if (ud->is<Image>())
            result = std::move(newResult);
          else
            Logger::warn("Patch '{}' for image '{}' returned a non-Image userdata value, ignoring", patchPath, path);
        } else {
          Logger::warn("Patch '{}' for image '{}' returned a non-Image value, ignoring", patchPath, path);
        }
      }
      luaLocker.unlock();
    } else {
      Logger::warn(std::string_view("Patch '{}' for image '{}' isn't a Lua script, ignoring"), patchPath, path);
    }
  }
  return std::make_shared<Image>(std::move(result.get<LuaUserData>().get<Image>()));
}

auto Assets::checkPatchArray(String const& path, Ptr<AssetSource> const& source, Json const result, JsonArray const patchData, std::optional<Json> const external) const -> Json {
  auto externalRef = external.value();
  auto newResult = result;
  for (auto const& patch : patchData) {
    switch (patch.type()) {
    case Json::Type::Array:// if the patch is an array, go down recursively until we get objects
      try {
        newResult = checkPatchArray(path, source, newResult, patch.toArray(), externalRef);
      } catch (JsonPatchTestFail const& e) {
        Logger::debug(std::string_view("Patch test failure from file {} in source: '{}' at '{}'. Caused by: {}"), path, source->metadata().value("name", ""), m_assetSourcePaths.getLeft(source), e.what());
      } catch (JsonPatchException const& e) {
        Logger::error(std::string_view("Could not apply patch from file {} in source: '{}' at '{}'.  Caused by: {}"), path, source->metadata().value("name", ""), m_assetSourcePaths.getLeft(source), e.what());
      }
      break;
    case Json::Type::Object:// if its an object, check for operations, or for if an external file is needed for patches to reference
      newResult = JsonPatching::applyOperation(newResult, patch, externalRef);
      break;
    case Json::Type::String:
      try {
        externalRef = json(patch.toString());
      } catch (...) {
        throw JsonPatchTestFail(strf(std::string_view("Unable to load reference asset: {}"), patch.toString()));
      }
      break;
    default:
      throw JsonPatchException(strf(std::string_view("Patch data is wrong type: {}"), Json::typeName(patch.type())));
      break;
    }
  }
  return newResult;
}

auto Assets::readJson(String const& path) const -> Json {
  ByteArray streamData = read(path);
  try {
    return applyJsonPatches(inputUtf8Json(streamData.begin(), streamData.end(), JsonParseType::Top), path, m_files.get(path).patchSources);
  } catch (std::exception const& e) {
    throw JsonParsingException(strf(std::string_view("Cannot parse json file: {}"), path), e);
  }
}

auto Assets::applyJsonPatches(Json const& input, String const& path, List<std::pair<String, Ptr<AssetSource>>> patches) const -> Json {
  Json result = input;
  for (auto const& pair : patches) {
    auto patchAssetPath = AssetPath::split(pair.first);
    auto& patchBasePath = patchAssetPath.basePath;
    auto& patchSource = pair.second;
    auto patchStream = patchSource->read(patchBasePath);
    if (patchBasePath.endsWith(".lua")) {
      std::pair<AssetSource*, String> contextKey = std::make_pair(patchSource.get(), patchBasePath);
      RecursiveMutexLocker luaLocker(m_luaMutex);
      // Kae: i don't like that lock. perhaps have a LuaEngine and patch context cache per worker thread later on?
      Ptr<LuaContext>& context = m_patchContexts[contextKey];
      if (!context) {
        context = std::make_shared<LuaContext>(as<LuaEngine>(m_luaEngine.get())->createContext());
        context->load(patchStream, patchBasePath);
      }
      auto newResult = context->invokePath<Json>("patch", result, path);
      if (newResult)
        result = std::move(newResult);
    } else {
      try {
        auto patchJson = inputUtf8Json(patchStream.begin(), patchStream.end(), JsonParseType::Top);
        if (patchAssetPath.subPath)
          patchJson = patchJson.query(*patchAssetPath.subPath);
        if (patchJson.isType(Json::Type::Array)) {
          auto patchData = patchJson.toArray();
          try {
            result = checkPatchArray(pair.first, patchSource, result, patchData, {});
          } catch (JsonPatchTestFail const& e) {
            Logger::debug(std::string_view("Patch test failure from file {} in source: '{}' at '{}'. Caused by: {}"), pair.first, patchSource->metadata().value("name", ""), m_assetSourcePaths.getLeft(patchSource), e.what());
          } catch (JsonPatchException const& e) {
            Logger::error(std::string_view("Could not apply patch from file {} in source: '{}' at '{}'.  Caused by: {}"), pair.first, patchSource->metadata().value("name", ""), m_assetSourcePaths.getLeft(patchSource), e.what());
          }
        } else if (patchJson.isType(Json::Type::Object)) {
          result = jsonMergeNulling(result, patchJson.toObject());
        }
      } catch (std::exception const& e) {
        throw JsonParsingException(strf(std::string_view("Cannot parse json patch file: {} in source {}"), patchBasePath, patchSource->metadata().value("name", "")), e);
      }
    }
  }
  return result;
}

auto Assets::doLoad(AssetId const& id) const -> bool {
  try {
    // loadAsset automatically manages the queue and freshens the asset
    // data.
    return (bool)loadAsset(id);
  } catch (std::exception const& e) {
    Logger::error(std::string_view("Exception caught loading asset: {}, {}"), id.path, outputException(e, true));
  } catch (...) {
    Logger::error(std::string_view("Unknown exception caught loading asset: {}"), id.path);
  }

  // There was an exception, remove the asset from the queue and fill the cache
  // with null so that getAsset will throw.
  m_assetsCache[id] = {};
  m_assetsDone.broadcast();
  m_queue.remove(id);
  return true;
}

auto Assets::doPost(AssetId const& id) const -> bool {
  std::shared_ptr<AssetData> assetData;
  try {
    assetData = m_assetsCache.get(id);
    if (id.type == AssetType::Audio)
      assetData = postProcessAudio(assetData);
  } catch (std::exception const& e) {
    Logger::error(std::string_view("Exception caught post-processing asset: {}, {}"), id.path, outputException(e, true));
  } catch (...) {
    Logger::error(std::string_view("Unknown exception caught post-processing asset: {}"), id.path);
  }

  m_queue.remove(id);
  if (assetData) {
    assetData->needsPostProcessing = false;
    m_assetsCache[id] = assetData;
    freshen(assetData);
    m_assetsDone.broadcast();
  }

  return true;
}

auto Assets::loadAsset(AssetId const& id) const -> std::shared_ptr<Assets::AssetData> {
  if (auto asset = m_assetsCache.value(id))
    return asset;

  if (m_queue.value(id, QueuePriority::None) == QueuePriority::Working)
    return {};

  try {
    m_queue[id] = QueuePriority::Working;
    std::shared_ptr<AssetData> assetData;

    try {
      if (id.type == AssetType::Json) {
        assetData = loadJson(id.path);
      } else if (id.type == AssetType::Image) {
        assetData = loadImage(id.path);
      } else if (id.type == AssetType::Audio) {
        assetData = loadAudio(id.path);
      } else if (id.type == AssetType::Font) {
        assetData = loadFont(id.path);
      } else if (id.type == AssetType::Bytes) {
        assetData = loadBytes(id.path);
      }

    } catch (StarException const& e) {
      if (id.type == AssetType::Image && m_settings.missingImage) {
        Logger::error(std::string_view("Could not load image asset '{}', using placeholder default.\n{}"), id.path, outputException(e, false));
        assetData = loadImage({*m_settings.missingImage, {}, {}});
      } else if (id.type == AssetType::Audio && m_settings.missingAudio) {
        Logger::error(std::string_view("Could not load audio asset '{}', using placeholder default.\n{}"), id.path, outputException(e, false));
        assetData = loadAudio({*m_settings.missingAudio, {}, {}});
      } else {
        throw;
      }
    }

    if (assetData) {
      if (assetData->needsPostProcessing)
        m_queue[id] = QueuePriority::PostProcess;
      else
        m_queue.remove(id);
      m_assetsCache[id] = assetData;
      m_assetsDone.broadcast();
      freshen(assetData);

    } else {
      // We have failed to load an asset because it depends on an asset
      // currently being worked on.  Mark it as needing loading and move it to
      // the end of the queue.
      m_queue[id] = QueuePriority::Load;
      m_assetsQueued.signal();
      m_queue.toBack(id);
    }

    return assetData;

  } catch (...) {
    m_queue.remove(id);
    m_assetsCache[id] = {};
    m_assetsDone.broadcast();
    throw;
  }
}

auto Assets::loadJson(AssetPath const& path) const -> std::shared_ptr<Assets::AssetData> {
  Json json;

  if (path.subPath) {
    auto topJson =
      as<JsonData>(loadAsset(AssetId{.type = AssetType::Json, .path = {path.basePath, {}, {}}}));
    if (!topJson)
      return {};

    try {
      auto newData = std::make_shared<JsonData>();
      newData->json = topJson->json.query(*path.subPath);
      return newData;
    } catch (StarException const& e) {
      throw AssetException(strf(std::string_view("Could not read JSON value {}"), path), e);
    }
  } else {
    return unlockDuring([&]() -> std::shared_ptr<Star::Assets::JsonData> {
      try {
        auto newData = std::make_shared<JsonData>();
        newData->json = readJson(path.basePath);
        return newData;
      } catch (StarException const& e) {
        throw AssetException(strf(std::string_view("Could not read JSON asset {}"), path), e);
      }
    });
  }
}

auto Assets::loadImage(AssetPath const& path) const -> std::shared_ptr<Assets::AssetData> {
  validatePath(path, true, true);
  if (!path.directives.empty()) {
    std::shared_ptr<ImageData> source =
      as<ImageData>(loadAsset(AssetId{.type = AssetType::Image, .path = {path.basePath, path.subPath, {}}}));
    if (!source)
      return {};
    StringMap<ConstPtr<Image>> references;
    StringList referencePaths;

    for (auto& directives : path.directives.list())
      directives.loadOperations();

    path.directives.forEach([&](auto const& entry, Directives const&) -> auto {
      addImageOperationReferences(entry.operation, referencePaths);
    });// TODO: This can definitely be better, was changed quickly to support the new Directives.

    for (auto const& ref : referencePaths) {
      auto components = AssetPath::split(ref);
      validatePath(components, true, false);
      auto refImage = as<ImageData>(loadAsset(AssetId{.type = AssetType::Image, .path = std::move(components)}));
      if (!refImage)
        return {};
      references[ref] = refImage->image;
    }

    return unlockDuring([&]() -> std::shared_ptr<Star::Assets::ImageData> {
      auto newData = std::make_shared<ImageData>();
      auto newImage = path.directives.applyNewImage(*source->image, [&](String const& ref) -> auto* { return references.get(ref).get(); });
      newData->image = std::make_shared<Image>(std::move(newImage));
      return newData;
    });

  } else if (path.subPath) {
    auto imageData = as<ImageData>(loadAsset(AssetId{.type = AssetType::Image, .path = {path.basePath, {}, {}}}));
    if (!imageData)
      return {};

    // Base image must have frames data associated with it.
    if (!imageData->frames)
      throw AssetException::format(std::string_view("No associated frames file found for image '{}' while resolving image frame '{}'"), path.basePath, path);

    if (auto alias = imageData->frames->aliases.ptr(*path.subPath)) {
      imageData = as<ImageData>(loadAsset(AssetId{.type = AssetType::Image, .path = {path.basePath, *alias, path.directives}}));
      if (!imageData)
        return {};

      auto newData = std::make_shared<ImageData>();
      newData->image = imageData->image;
      newData->alias = true;
      return newData;

    } else {
      auto frameRect = imageData->frames->frames.ptr(*path.subPath);
      if (!frameRect)
        throw AssetException(strf(std::string_view("No such frame {} in frames spec {}"), *path.subPath, imageData->frames->framesFile));

      return unlockDuring([&]() -> std::shared_ptr<Star::Assets::ImageData> {
        // Need to flip frame coordinates because frame configs assume top
        // down image coordinates
        auto newData = std::make_shared<ImageData>();
        newData->image = std::make_shared<Image>(imageData->image->subImage(
          Vec2U(frameRect->xMin(), imageData->image->height() - frameRect->yMax()), frameRect->size()));
        return newData;
      });
    }

  } else {
    auto imageData = std::make_shared<ImageData>();
    imageData->image = unlockDuring([&]() -> ConstPtr<Image> {
      return readImage(path.basePath);
    });
    imageData->frames = bestFramesSpecification(path.basePath);

    return imageData;
  }
}

auto Assets::loadAudio(AssetPath const& path) const -> std::shared_ptr<Assets::AssetData> {
  return unlockDuring([&]() -> std::shared_ptr<Star::Assets::AudioData> {
    auto newData = std::make_shared<AudioData>();
    newData->audio = std::make_shared<Audio>(open(path.basePath), path.basePath);
    newData->needsPostProcessing = newData->audio->compressed();
    return newData;
  });
}

auto Assets::loadFont(AssetPath const& path) const -> std::shared_ptr<Assets::AssetData> {
  return unlockDuring([&]() -> std::shared_ptr<Star::Assets::FontData> {
    auto newData = std::make_shared<FontData>();
    newData->font = Font::loadFont(std::make_shared<ByteArray>(read(path.basePath)));
    return newData;
  });
}

auto Assets::loadBytes(AssetPath const& path) const -> std::shared_ptr<Assets::AssetData> {
  return unlockDuring([&]() -> std::shared_ptr<Star::Assets::BytesData> {
    auto newData = std::make_shared<BytesData>();
    newData->bytes = std::make_shared<ByteArray>(read(path.basePath));
    return newData;
  });
}

auto Assets::postProcessAudio(std::shared_ptr<AssetData> const& original) const -> std::shared_ptr<Assets::AssetData> {
  return unlockDuring([&]() -> std::shared_ptr<AssetData> {
    if (auto audioData = as<AudioData>(original)) {
      if (audioData->audio->totalTime() < m_settings.audioDecompressLimit) {
        auto audio = std::make_shared<Audio>(*audioData->audio);
        audio->uncompress();

        auto newData = std::make_shared<AudioData>();
        newData->audio = audio;
        return newData;
      } else {
        return audioData;
      }
    } else {
      return {};
    }
  });
}

void Assets::freshen(std::shared_ptr<AssetData> const& asset) const {
  asset->time = Time::monotonicTime();
}

}// namespace Star
