import std;
import star.maybe;
import star.format;
import star.json;
import star.json_extra;
import star.ordered_map;
import star.rect;
import star.bi_map;
import star.thread;
import star.ref_ptr;
import star.list;
import star.image;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.file;
import star.time;
import star.io_device;
import star.string;
import star.json_parser;
import star.json_builder;
import star.logging;
import star.random;
import star.font;
import star.audio;
import star.casting;
import star.lexical_cast;
import star.data_stream_devices;
import star.lua;

import star.assets;
import star.asset_source;
import star.directory_asset_source;
import star.image_lua_bindings;
import star.utility_lua_bindings;

import star.sha256;
import star.memory_asset_source;
import star.packed_asset_source;
import star.json_patch;

import star.byte_array;
import star.vector;
namespace Star {

// if a ptr is returned, can be optionally used to format an error
static const char* validateBasePath(std::string_view const& basePath) {
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

static bool validatePath(AssetPath const& components, bool canContainSubPath, bool canContainDirectives, bool throwing = true) {
  if (auto error = validateBasePath(components.basePath.utf8())) {
    if (throwing)
      throw AssetException::format(error, components.basePath.utf8());
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

Maybe<RectU> FramesSpecification::getRect(String const& frame) const {
  if (auto alias = aliases.ptr(frame)) {
    return frames.get(*alias);
  } else {
    return frames.maybe(frame);
  }
}

Json FramesSpecification::toJson() const {
  return JsonObject{
    {"aliases", jsonFromMap(aliases)},
    {"frames", jsonFromMapV(frames, jsonFromRectU)},
    {"file", framesFile}
  };
}

Assets::Assets(Logger& logger, RandomService& random, LogMap& logMap, Settings settings, StringList assetSources)
  : m_logger(logger) {
  const char* AssetsPatchSuffix = ".patch";
  const char* AssetsPatchListSuffix = ".patchlist";
  const char* AssetsLuaPatchSuffix = ".patch.lua";

  m_settings = std::move(settings);
  m_assetSources = std::move(assetSources);

  auto luaEngine = LuaEngine::create();
  luaEngine->setImageResolver([this](String const& path) { return image(path); });
  m_luaEngine = luaEngine;
  auto pushGlobalContext = [&luaEngine](String const& name, LuaCallbacks && callbacks) {
    auto table = luaEngine->createTable();
    for (auto const& p : callbacks.callbacks())
      table.set(p.first, luaEngine->createWrappedFunction(p.second));
    luaEngine->setGlobal(name, table);
  };

  auto makeBaseAssetCallbacks = [this]() {
    LuaCallbacks callbacks;
    callbacks.registerCallbackWithSignature<StringSet, String>("byExtension", std::bind(&Assets::scanExtension, this, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<Json, String>("json", std::bind(&Assets::json, this, std::placeholders::_1));
    callbacks.registerCallbackWithSignature<bool, String>("exists", std::bind(&Assets::assetExists, this, std::placeholders::_1));

    callbacks.registerCallback("sourcePaths", [this](LuaEngine& engine, Maybe<bool> withMetaData) -> LuaTable {
      auto assetSources = this->assetSources();
      auto table = engine.createTable(assetSources.size(), 0);
      if (withMetaData.value()) {
        for (auto& assetSource : assetSources)
          table.set(assetSource, this->assetSourceMetadata(assetSource));
      }
      else {
        std::size_t i = 0;
        for (auto& assetSource : assetSources)
          table.set(++i, assetSource);
      }
      return table;
    });

    callbacks.registerCallback("sourceMetadata", [this](String const& sourcePath) -> Maybe<JsonObject> {
      if (auto assetSource = m_assetSourcePaths.rightPtr(sourcePath))
        return (*assetSource)->metadata();
      return {};
    });

    callbacks.registerCallback("origin", [this](String const& path) -> Maybe<String> {
      if (auto descriptor = this->assetDescriptor(path))
        return this->assetSourcePath(descriptor->source);
      return {};
    });

    callbacks.registerCallback("bytes", [this](String const& path) -> String {
      auto assetBytes = bytes(path);
      return String(assetBytes->ptr(), assetBytes->size());
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
      return Json();
    });

    callbacks.registerCallback("scan", [this](Maybe<String> const& a, Maybe<String> const& b) -> StringList {
      return b ? scan(a.value(), *b) : scan(a.value());
    });
    return callbacks;
  };

  pushGlobalContext("sb", LuaBindings::makeUtilityCallbacks(random, logger, logMap));
  pushGlobalContext("assets", makeBaseAssetCallbacks());

  auto decorateLuaContext = [&](LuaContext& context, std::shared_ptr<MemoryAssetSource> newFiles) {
    if (newFiles) {
      // re-add the assets callbacks with more functions
      context.remove("assets");
      auto callbacks = makeBaseAssetCallbacks();
      callbacks.registerCallback("add", [newFiles](LuaEngine& engine, String const& path, LuaValue const& data) {
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
              newFiles->set(path, *applyImagePatches(image, path, {std::make_pair(patchPath, newFiles)}).get());
              return true;
            } else {
              if (auto asset = m_files.ptr(patchPath)) {
                newFiles->set(path, *applyImagePatches(image, path, {std::make_pair(patchPath, asset->source)}).get());
                return true;
              }
            }
          } else {
            if (newFiles->contains(patchPath)) {
              auto file = newFiles->read(path);
              auto json = applyJsonPatches(inputUtf8Json(file.begin(), file.end(), JsonParseType::Top), path, {std::make_pair(patchPath, newFiles)}).repr();
              newFiles->set(path, ByteArray(json.utf8Ptr(), json.utf8Size()));
              return true;
            } else {
              if (auto asset = m_files.ptr(patchPath)) {
              auto file = newFiles->read(path);
              auto json = applyJsonPatches(inputUtf8Json(file.begin(), file.end(), JsonParseType::Top), path, {std::make_pair(patchPath, asset->source)}).repr();
              newFiles->set(path, ByteArray(json.utf8Ptr(), json.utf8Size()));
                return true;
              }
            }
          }
        } else if (auto file = m_files.ptr(path)) {
          if (newFiles->contains(patchPath)) {
            file->patchSources.append(std::make_pair(patchPath, newFiles));
            return true;
          } else {
            if (auto asset = m_files.ptr(patchPath)) {
              file->patchSources.append(std::make_pair(patchPath, asset->source));
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

  auto addSource = [&](String const& sourcePath, std::shared_ptr<AssetSource> source) {
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
          std::size_t patchIndex = 0;
          for (auto const& patchPair : inputUtf8Json(stream.begin(), stream.end(), JsonParseType::Top).iterateArray()) {
            auto patches = patchPair.getArray("patches");
            for (auto& path : patchPair.getArray("paths")) {
              if (auto p = m_files.ptr(path.toString())) {
                for (std::size_t i = 0; i != patches.size(); ++i) {
                  auto& patch = patches[i];
                  if (patch.isType(Json::Type::String))
                    p->patchSources.append({patch.toString(), source});
                  else
                    p->patchSources.append({strf("{}:[{}].patches[{}]", filename, patchIndex, i), source});
                }
              }
            }
            patchIndex++;
          }
        } else {
          for (int i = 0; i < 10; i++) {
            if (filename.endsWith(AssetsPatchSuffix + toString(i), String::CaseInsensitive)) {
              auto targetPatchFile = filename.substr(0, filename.size() - std::strlen(AssetsPatchSuffix) - 1);
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

  auto runLoadScripts = [&](String const& groupName, String const& sourcePath, std::shared_ptr<AssetSource> source) {
    auto metadata = source->metadata();
    if (auto scripts = metadata.ptr("scripts")) {
      if (auto scriptGroup = scripts->optArray(groupName)) {
        auto memoryName = strf("{}::{}", metadata.value("name", File::baseName(sourcePath)), groupName);
        JsonObject memoryMetadata{ {"name", memoryName} };
        auto memoryAssets = std::make_shared<MemoryAssetSource>(memoryName, memoryMetadata);
        auto now = Time::monotonicTime();
        m_logger.info("Running {} scripts {}", groupName, *scriptGroup);
        try {
          auto context = luaEngine->createContext();
          decorateLuaContext(context, memoryAssets);
          for (auto& jPath : *scriptGroup) {
            auto path = jPath.toString();
            auto script = source->read(path);
            context.load(script, path);
          }
        } catch (LuaException const& e) {
          m_logger.error("Exception while running {} scripts from asset source '{}': {}", groupName, sourcePath, e.what());
        }
        m_logger.info("Took {} seconds to run {} scripts {}", Time::monotonicTime() - now, groupName, *scriptGroup);
        if (!memoryAssets->empty())
          addSource(strf("{}::{}", sourcePath, groupName), memoryAssets);
      }
    }
    // clear any caching that may have been trigered by load scripts as they may no longer be valid
    m_framesSpecifications.clear();
    m_assetsCache.clear();
  };

  List<std::pair<String, std::shared_ptr<AssetSource>>> sources;

  for (auto& sourcePath : m_assetSources) {
    m_logger.info("Loading assets from: '{}'", sourcePath);
    std::shared_ptr<AssetSource> source;
    if (File::isDirectory(sourcePath))
      source = std::make_shared<DirectoryAssetSource>(m_logger, sourcePath, m_settings.pathIgnore);
    else
      source = std::make_shared<PackedAssetSource>(sourcePath);

    addSource(sourcePath, source);
    sources.append(std::make_pair(sourcePath, source));

    runLoadScripts("onLoad", sourcePath, source);
  }

  for (auto& pair : sources)
    runLoadScripts("postLoad", pair.first, pair.second);

  Sha256Hasher digest;

  for (auto const& assetPath : m_files.keys().transformed([](String const& s) {
        return s.toLower();
      }).sorted()) {
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

  try {
    // Allocate the handle storage before starting any worker. A failed append
    // must not destroy a running temporary handle before shutdown can signal it.
    m_workerThreads.reserve(m_settings.workerPoolSize);
    for (unsigned i = 0; i < m_settings.workerPoolSize; ++i)
      m_workerThreads.append(Thread::invoke("Assets::workerMain", std::mem_fn(&Assets::workerMain), this));

    // preload.config contains files which will be loaded and then told to persist.
    Json preload = json("/preload.config");
    m_logger.info("Preloading assets");
    for (auto script : preload.iterateArray()) {
      auto type = AssetTypeNames.getLeft(script.getString("type"));
      auto path = script.getString("path");
      auto components = AssetPath::split(path);
      validatePath(components, type == AssetType::Json || type == AssetType::Image, type == AssetType::Image);

      auto asset = getAsset(AssetId{type, std::move(components)});
      asset->forcePersist = true;
    }
  } catch (...) {
    // The destructor is not called for a failed constructor. Join while every
    // mutex, condition variable, source and Lua context is still alive.
    stopWorkers();
    throw;
  }
}

Assets::~Assets() {
  if (auto error = stopWorkers()) {
    try {
      std::rethrow_exception(error);
    } catch (std::exception const& e) {
      m_logger.error("Asset worker failed during shutdown: {}", e.what());
    } catch (...) {
      m_logger.error("Asset worker failed during shutdown with an unknown exception");
    }
  }
}

std::exception_ptr Assets::stopWorkers() {
  {
    MutexLocker locker(m_assetsMutex);
    m_stopThreads = true;
    m_assetsQueued.broadcast();
    m_assetsDone.broadcast();
  }

  std::exception_ptr error;
  for (auto& worker : m_workerThreads) {
    try {
      worker.finish();
    } catch (...) {
      if (!error)
        error = std::current_exception();
    }
  }
  // finish() consumes the stored exception, so no handle destructor can throw.
  m_workerThreads.clear();
  return error;
}

void Assets::hotReload() const {
  MutexLocker assetsLocker(m_assetsMutex);
  m_assetsCache.clear();
  m_queue.clear();
  m_framesSpecifications.clear();
}

StringList Assets::assetSources() const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSources;
}

JsonObject Assets::assetSourceMetadata(String const& sourceName) const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSourcePaths.getRight(sourceName)->metadata();
}

ByteArray Assets::digest() const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_digest;
}

bool Assets::assetExists(String const& path) const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_files.contains(path);
}

Maybe<Assets::AssetFileDescriptor> Assets::assetDescriptor(String const& path) const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_files.maybe(path);
}

String Assets::assetSource(String const& path) const {
  MutexLocker assetsLocker(m_assetsMutex);
  if (auto p = m_files.ptr(path))
    return m_assetSourcePaths.getLeft(p->source);
  throw AssetException(strf("No such asset '{}'", path));
}

Maybe<String> Assets::assetSourcePath(std::shared_ptr<AssetSource> const& source) const {
  MutexLocker assetsLocker(m_assetsMutex);
  return m_assetSourcePaths.maybeLeft(source);
}

StringList Assets::scan(String const& suffix) const {
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

StringList Assets::scan(String const& prefix, String const& suffix) const {
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

CaseInsensitiveStringSet const& Assets::scanExtension(String const& extension) const {
  auto find = m_filesByExtension.find(extension.beginsWith(".") ? extension.substr(1) : extension);
  return find != m_filesByExtension.end() ? find->second : NullExtensionScan;
}

Json Assets::json(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, true, false);

  return as<JsonData>(getAsset(AssetId{AssetType::Json, std::move(components)}))->json;
}

Json Assets::fetchJson(Json const& v, String const& dir) const {
  if (v.isType(Json::Type::String))
    return Assets::json(AssetPath::relativeTo(dir, v.toString()));
  else
    return v;
}

void Assets::queueJsons(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) {
    auto components = AssetPath::split(path);
    validatePath(components, true, false);

    return AssetId{AssetType::Json, {components.basePath, {}, {}}};
  }));
}

void Assets::queueJsons(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, true, false);

    queueAsset(AssetId{AssetType::Json, {components.basePath, {}, {}}});
  };
}

std::shared_ptr<Image const> Assets::image(AssetPath const& path) const {
  return as<ImageData>(getAsset(AssetId{AssetType::Image, path}))->image;
}

void Assets::queueImages(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) {
    auto components = AssetPath::split(path);
    validatePath(components, true, true);

    return AssetId{AssetType::Image, std::move(components)};
  }));
}

void Assets::queueImages(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, true, true);

    queueAsset(AssetId{AssetType::Image, std::move(components)});
  };
}

std::shared_ptr<Image const> Assets::tryImage(AssetPath const& path) const {
  if (auto imageData = as<ImageData>(tryAsset(AssetId{AssetType::Image, path})))
    return imageData->image;
  else
    return {};
}

std::shared_ptr<FramesSpecification const> Assets::imageFrames(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  MutexLocker assetsLocker(m_assetsMutex);
  return bestFramesSpecification(path);
}

std::shared_ptr<Audio const> Assets::audio(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<AudioData>(getAsset(AssetId{AssetType::Audio, std::move(components)}))->audio;
}

void Assets::queueAudios(StringList const& paths) const {
  queueAssets(paths.transformed([](String const& path) {
    auto components = AssetPath::split(path);
    validatePath(components, false, false);

    return AssetId{AssetType::Audio, std::move(components)};
  }));
}

void Assets::queueAudios(CaseInsensitiveStringSet const& paths) const {
  MutexLocker assetsLocker(m_assetsMutex);
  for (String const& path : paths) {
    auto components = AssetPath::split(path);
    validatePath(components, false, true);

    queueAsset(AssetId{AssetType::Audio, std::move(components)});
  };
}

std::shared_ptr<Audio const> Assets::tryAudio(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  if (auto audioData = as<AudioData>(tryAsset(AssetId{AssetType::Audio, std::move(components)})))
    return audioData->audio;
  else
    return {};
}

std::shared_ptr<Font const> Assets::font(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<FontData>(getAsset(AssetId{AssetType::Font, std::move(components)}))->font;
}

std::shared_ptr<ByteArray const> Assets::bytes(String const& path) const {
  auto components = AssetPath::split(path);
  validatePath(components, false, false);

  return as<BytesData>(getAsset(AssetId{AssetType::Bytes, std::move(components)}))->bytes;
}

std::shared_ptr<IODevice> Assets::openFile(String const& path) const {
  return open(path);
}

void Assets::clearCache() {
  MutexLocker assetsLocker(m_assetsMutex);

  // Clear all assets that are not queued or broken.
  for (auto it = m_assetsCache.begin(); it != m_assetsCache.end();) {
    auto const& pair = *it;
    // Don't clean up queued, persistent, or broken assets.
    if (pair.second && !pair.second->shouldPersist() && !m_queue.contains(pair.first))
      it = m_assetsCache.erase(it);
    else
      ++it;
  }
}

void Assets::cleanup() {
  MutexLocker assetsLocker(m_assetsMutex);

  double time = Time::monotonicTime();

  for (auto it = m_assetsCache.begin(); it != m_assetsCache.end();) {
    auto pair = *it;
    // Don't clean up broken assets or queued assets.
    if (pair.second && !m_queue.contains(pair.first)) {
      double liveTime = time - pair.second->time;
      if (liveTime > m_settings.assetTimeToLive) {
        // If the asset should persist, just refresh the access time.
        if (pair.second->shouldPersist())
          pair.second->time = time;
        else {
          it = m_assetsCache.erase(it);
          continue;
        }
      }
    }
    ++it;
  }
}

bool Assets::AssetId::operator==(AssetId const& assetId) const {
  return std::tie(type, path) == std::tie(assetId.type, assetId.path);
}

std::size_t Assets::AssetIdHash::operator()(AssetId const& id) const {
  return hashOf(id.type, id.path.basePath, id.path.subPath, id.path.directives);
}

bool Assets::JsonData::shouldPersist() const {
  return forcePersist || !json.unique();
}

bool Assets::ImageData::shouldPersist() const {
  return forcePersist || (!alias && image.use_count() != 1);
}

bool Assets::AudioData::shouldPersist() const {
  return forcePersist || audio.use_count() != 1;
}

bool Assets::FontData::shouldPersist() const {
  return forcePersist || font.use_count() != 1;
}

bool Assets::BytesData::shouldPersist() const {
  return forcePersist || bytes.use_count() != 1;
}

FramesSpecification Assets::parseFramesSpecification(Json const& frameConfig, String path) {
  FramesSpecification framesSpecification;

  framesSpecification.framesFile = std::move(path);

  if (frameConfig.contains("frameList")) {
    for (auto const& pair : frameConfig.get("frameList").toObject()) {
      String frameName = pair.first;
      RectU rect = RectU(jsonToRectI(pair.second));
      if (rect.isEmpty())
        throw AssetException(
            strf("Empty rect in frame specification in image {} frame {}", framesSpecification.framesFile, frameName));
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
      throw AssetException(strf("Image {} \"dimensions\" in frameGrid cannot be zero", framesSpecification.framesFile));

    if (grid.contains("names")) {
      auto nameList = grid.get("names");
      for (std::size_t y = 0; y < nameList.size(); ++y) {
        if (y >= dimensions[1])
          throw AssetException(strf("Image {} row {} is out of bounds for y-dimension {}",
              framesSpecification.framesFile,
              y + 1,
              dimensions[1]));
        auto rowList = nameList.get(y);
        if (rowList.isNull())
          continue;
        for (unsigned x = 0; x < rowList.size(); ++x) {
          if (x >= dimensions[0])
            throw AssetException(strf("Image {} column {} is out of bounds for x-dimension {}",
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
      for (std::size_t y = 0; y < dimensions[1]; ++y)
        for (std::size_t x = 0; x < dimensions[0]; ++x)
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
      for (std::size_t i = 0; i <= aliases->size(); ++i) {
        auto it = aliases->find(value);
        if (it != aliases->end()) {
          if (i == aliases->size())
            throw AssetException(strf("Infinite alias loop detected for alias '{}'", key));

          value = it->second.toString();
        } else {
          break;
        }
      }

      if (!framesSpecification.frames.contains(value))
        throw AssetException(strf("No such frame '{}' found for alias '{}'", value, key));
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

std::shared_ptr<Assets::AssetData> Assets::tryAsset(AssetId const& id) const {
  MutexLocker assetsLocker(m_assetsMutex);

  auto i = m_assetsCache.find(id);
  if (i != m_assetsCache.end()) {
    if (i->second) {
      freshen(i->second);
      return i->second;
    } else {
      throw AssetException::format("Error loading asset {}", id.path);
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

std::shared_ptr<Assets::AssetData> Assets::getAsset(AssetId const& id) const {
  MutexLocker assetsLocker(m_assetsMutex);

  while (true) {
    if (m_stopThreads)
      throw AssetException("Asset loading stopped");
    auto j = m_assetsCache.find(id);
    if (j != m_assetsCache.end()) {
      if (j->second) {
        auto asset = j->second;
        freshen(asset);
        return asset;
      } else {
        throw AssetException::format("Error loading asset {}", id.path);
      }
    } else {
      // Try to load the asset in-thread, if we cannot, then the asset has been
      // queued so wait for a worker thread to finish it.
      if (!doLoad(id) && !m_stopThreads)
        m_assetsDone.wait(m_assetsMutex);
    }
  }
}

void Assets::workerMain() {
  while (true) {
    // Do not hold the asset mutex while collecting Lua garbage.
    {
      RecursiveMutexLocker luaLocker(m_luaMutex);
      as<LuaEngine>(m_luaEngine.get())->collectGarbage();
    }

    MutexLocker assetsLocker(m_assetsMutex);
    if (m_stopThreads)
      break;

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

    if (workIsBlocking && !m_stopThreads) {
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
decltype(auto) Assets::unlockDuring(Function f) const {
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

std::shared_ptr<FramesSpecification const> Assets::bestFramesSpecification(String const& image) const {
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

    Maybe<String> foundFramesFile;

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
      framesFile = foundFramesFile.take();
      m_bestFramesFiles[image] = framesFile;

    } else {
      return {};
    }
  }

  auto framesSpecification = unlockDuring([&]() {
      return std::make_shared<FramesSpecification>(parseFramesSpecification(readJson(framesFile), framesFile));
    });
  m_framesSpecifications[image] = framesSpecification;

  return framesSpecification;
}

std::shared_ptr<IODevice> Assets::open(String const& path) const {
  if (auto p = m_files.ptr(path))
    return p->source->open(p->sourceName);
  throw AssetException(strf("No such asset '{}'", path));
}

ByteArray Assets::read(String const& path) const {
  if (auto p = m_files.ptr(path))
    return p->source->read(p->sourceName);
  throw AssetException(strf("No such asset '{}'", path));
}

std::shared_ptr<Image const> Assets::readImage(String const& path) const {
  if (auto p = m_files.ptr(path)) {
    std::shared_ptr<Image const> image;
    if (auto memorySource = as<MemoryAssetSource>(p->source))
      image = memorySource->image(p->sourceName);
    if (!image)
      image = std::make_shared<Image>(Image::readPng(m_logger, p->source->open(p->sourceName)));

    if (!p->patchSources.empty()) {
      return applyImagePatches(image, path, p->patchSources);
    }
    return image;
  }
  throw AssetException(strf("No such asset '{}'", path));
}

std::shared_ptr<Image const> Assets::applyImagePatches(std::shared_ptr<Image const> image, String const& path, List<std::pair<String, std::shared_ptr<AssetSource>>> patches) const {
  RecursiveMutexLocker luaLocker(m_luaMutex);
  LuaEngine* luaEngine = as<LuaEngine>(m_luaEngine.get());
  LuaValue result = luaEngine->createUserData(*image);
  luaLocker.unlock();
  for (auto const& pair : patches) {
    auto& patchPath = pair.first;
    auto& patchSource = pair.second;
    auto patchStream = patchSource->read(patchPath);
    if (patchPath.endsWith(".lua")) {
      std::pair<AssetSource*, String> contextKey = std::make_pair(patchSource.get(), patchPath);
      luaLocker.lock();
      std::shared_ptr<LuaContext>& context = m_patchContexts[contextKey];
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
            m_logger.warn("Patch '{}' for image '{}' returned a non-Image userdata value, ignoring", patchPath, path);
        } else {
          m_logger.warn("Patch '{}' for image '{}' returned a non-Image value, ignoring", patchPath, path);
        }
      }
      luaLocker.unlock();
    } else {
      m_logger.warn("Patch '{}' for image '{}' isn't a Lua script, ignoring", patchPath, path);
    }
  }
  return std::make_shared<Image>(std::move(result.get<LuaUserData>().get<Image>()));
}

Json Assets::checkPatchArray(String const& path, std::shared_ptr<AssetSource> const& source, Json const result, JsonArray const patchData, Maybe<Json> const external) const {
  auto externalRef = external.value();
  auto newResult = result;
  for (auto const& patch : patchData) {
    switch(patch.type()) {
      case Json::Type::Array: // if the patch is an array, go down recursively until we get objects
        try {
          newResult = checkPatchArray(path, source, newResult, patch.toArray(), externalRef);
        } catch (JsonPatchTestFail const& e) {
          m_logger.debug("Patch test failure from file {} in source: '{}' at '{}'. Caused by: {}", path, source->metadata().value("name", ""), m_assetSourcePaths.getLeft(source), e.what());
        } catch (JsonPatchException const& e) {
          m_logger.error("Could not apply patch from file {} in source: '{}' at '{}'.  Caused by: {}", path, source->metadata().value("name", ""), m_assetSourcePaths.getLeft(source), e.what());
        }
        break;
      case Json::Type::Object: // if its an object, check for operations, or for if an external file is needed for patches to reference
        newResult = JsonPatching::applyOperation(newResult, patch, externalRef);
        break;
      case Json::Type::String:
        try {
          externalRef = json(patch.toString());
        } catch (...) {
          throw JsonPatchTestFail(strf("Unable to load reference asset: {}", patch.toString()));
        }
        break;
      default:
        throw JsonPatchException(strf("Patch data is wrong type: {}", Json::typeName(patch.type())));
        break;
    }
  }
  return newResult;
}

Json Assets::readJson(String const& path) const {
  ByteArray streamData = read(path);
  try {
    return applyJsonPatches(inputUtf8Json(streamData.begin(), streamData.end(), JsonParseType::Top), path, m_files.get(path).patchSources);
  } catch (std::exception const& e) {
    throw JsonParsingException(strf("Cannot parse json file: {}", path), e);
  }
}

Json Assets::applyJsonPatches(Json const& input, String const& path, List<std::pair<String, std::shared_ptr<AssetSource>>> patches) const {
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
      std::shared_ptr<LuaContext>& context = m_patchContexts[contextKey];
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
            m_logger.debug("Patch test failure from file {} in source: '{}' at '{}'. Caused by: {}", pair.first, patchSource->metadata().value("name", ""), m_assetSourcePaths.getLeft(patchSource), e.what());
          } catch (JsonPatchException const& e) {
            m_logger.error("Could not apply patch from file {} in source: '{}' at '{}'.  Caused by: {}", pair.first, patchSource->metadata().value("name", ""), m_assetSourcePaths.getLeft(patchSource), e.what());
          }
        } else if (patchJson.isType(Json::Type::Object)) {
          result = jsonMergeNulling(result, patchJson.toObject());
        }
      } catch (std::exception const& e) {
        throw JsonParsingException(strf("Cannot parse json patch file: {} in source {}", patchBasePath, patchSource->metadata().value("name", "")), e);
      }
    }
  }
  return result;
}

bool Assets::doLoad(AssetId const& id) const {
  try {
    // loadAsset automatically manages the queue and freshens the asset
    // data.
    return (bool)loadAsset(id);
  } catch (std::exception const& e) {
    m_logger.error("Exception caught loading asset: {}, {}", id.path, outputException(e, true));
  } catch (...) {
    m_logger.error("Unknown exception caught loading asset: {}", id.path);
  }

  // There was an exception, remove the asset from the queue and fill the cache
  // with null so that getAsset will throw.
  m_assetsCache[id] = {};
  m_assetsDone.broadcast();
  m_queue.remove(id);
  return true;
}

bool Assets::doPost(AssetId const& id) const {
  std::shared_ptr<AssetData> assetData;
  try {
    assetData = m_assetsCache.get(id);
    if (id.type == AssetType::Audio)
      assetData = postProcessAudio(assetData);
  } catch (std::exception const& e) {
    m_logger.error("Exception caught post-processing asset: {}, {}", id.path, outputException(e, true));
  } catch (...) {
    m_logger.error("Unknown exception caught post-processing asset: {}", id.path);
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

std::shared_ptr<Assets::AssetData> Assets::loadAsset(AssetId const& id) const {
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
        m_logger.error("Could not load image asset '{}', using placeholder default.\n{}", id.path, outputException(e, false));
        assetData = loadImage({*m_settings.missingImage, {}, {}});
      } else if (id.type == AssetType::Audio && m_settings.missingAudio) {
        m_logger.error("Could not load audio asset '{}', using placeholder default.\n{}", id.path, outputException(e, false));
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

std::shared_ptr<Assets::AssetData> Assets::loadJson(AssetPath const& path) const {
  Json json;

  if (path.subPath) {
    auto topJson =
      as<JsonData>(loadAsset(AssetId{AssetType::Json, {path.basePath, {}, {}}}));
    if (!topJson)
      return {};

    try {
      auto newData = std::make_shared<JsonData>();
      newData->json = topJson->json.query(*path.subPath);
      return newData;
    } catch (StarException const& e) {
      throw AssetException(strf("Could not read JSON value {}", path), e);
    }
  } else {
    return unlockDuring([&]() {
      try {
        auto newData = std::make_shared<JsonData>();
        newData->json = readJson(path.basePath);
        return newData;
      } catch (StarException const& e) {
        throw AssetException(strf("Could not read JSON asset {}", path), e);
      }
    });
  }
}

std::shared_ptr<Assets::AssetData> Assets::loadImage(AssetPath const& path) const {
  validatePath(path, true, true);
  if (!path.directives.empty()) {
    std::shared_ptr<ImageData> source =
        as<ImageData>(loadAsset(AssetId{AssetType::Image, {path.basePath, path.subPath, {}}}));
    if (!source)
      return {};
    StringMap<std::shared_ptr<Image const>> references;
    StringList referencePaths;

    for (auto& directives : path.directives.list())
      directives.loadOperations();

    path.directives.forEach([&](auto const& entry, Directives const&) {
      addImageOperationReferences(entry.operation, referencePaths);
    }); // TODO: This can definitely be better, was changed quickly to support the new Directives.


    for (auto const& ref : referencePaths) {
      auto components = AssetPath::split(ref);
      validatePath(components, true, false);
      auto refImage = as<ImageData>(loadAsset(AssetId{AssetType::Image, std::move(components)}));
      if (!refImage)
        return {};
      references[ref] = refImage->image;
    }

    return unlockDuring([&]() {
      auto newData = std::make_shared<ImageData>();
      auto newImage = path.directives.applyNewImage(*source->image, [&](String const& ref) { return references.get(ref).get(); });
      newData->image = std::make_shared<Image>(std::move(newImage));
      return newData;
    });

  } else if (path.subPath) {
    auto imageData = as<ImageData>(loadAsset(AssetId{AssetType::Image, {path.basePath, {}, {}}}));
    if (!imageData)
      return {};

    // Base image must have frames data associated with it.
    if (!imageData->frames)
      throw AssetException::format("No associated frames file found for image '{}' while resolving image frame '{}'", path.basePath, path);

    if (auto alias = imageData->frames->aliases.ptr(*path.subPath)) {
      imageData = as<ImageData>(loadAsset(AssetId{AssetType::Image, {path.basePath, *alias, path.directives}}));
      if (!imageData)
        return {};

      auto newData = std::make_shared<ImageData>();
      newData->image = imageData->image;
      newData->alias = true;
      return newData;

    } else {
      auto frameRect = imageData->frames->frames.ptr(*path.subPath);
      if (!frameRect)
        throw AssetException(strf("No such frame {} in frames spec {}", *path.subPath, imageData->frames->framesFile));

      return unlockDuring([&]() {
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
    imageData->image = unlockDuring([&]() {
      return readImage(path.basePath);
    });
    imageData->frames = bestFramesSpecification(path.basePath);

    return imageData;
  }
}

std::shared_ptr<Assets::AssetData> Assets::loadAudio(AssetPath const& path) const {
  return unlockDuring([&]() {
    auto newData = std::make_shared<AudioData>();
    newData->audio = std::make_shared<Audio>(open(path.basePath), path.basePath);
    newData->needsPostProcessing = newData->audio->compressed();
    return newData;
  });
}

std::shared_ptr<Assets::AssetData> Assets::loadFont(AssetPath const& path) const {
  return unlockDuring([&]() {
    auto newData = std::make_shared<FontData>();
    newData->font = Font::loadFont(m_fontLibrary, std::make_shared<ByteArray>(read(path.basePath)));
    return newData;
  });
}

std::shared_ptr<Assets::AssetData> Assets::loadBytes(AssetPath const& path) const {
  return unlockDuring([&]() {
    auto newData = std::make_shared<BytesData>();
    newData->bytes = std::make_shared<ByteArray>(read(path.basePath));
    return newData;
  });
}

std::shared_ptr<Assets::AssetData> Assets::postProcessAudio(std::shared_ptr<AssetData> const& original) const {
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

}
