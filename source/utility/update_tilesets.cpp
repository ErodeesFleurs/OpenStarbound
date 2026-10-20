import star.tileset_database;
import star.native_runtime;
import star.json_extra;
import star.json;
import star.list;
import star.bi_map;
import star.data_stream;
import star.ordered_map;
import star.thread;
import star.io_device;
import star.image;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.id_map;
import star.random;
import star.periodic;
import star.periodic_function;
import star.casting;
import star.strong_typedef;
import star.set;
import star.color;
import star.lua;
import star.vector;
import star.game_types;
import star.material_types;
import star.audio;
import star.map;
import star.maybe;
import star.string;
import star.variant;
import star.ordered_set;
import std;
import star.format;
import star.version;
import star.logging;
import star.listener;

import star.collision_block;
import star.item_descriptor;
import star.asset_source;
import star.assets;
import star.lua_components;
import star.lua_animation_component;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.animated_part_set;
import star.drawable;
import star.animation;
import star.particle;
import star.mixer;
import star.light_source;
import star.networked_animator;
import star.damage_types;
import star.entity_rendering_types;
import star.entity_rendering;
import star.object;
import star.option_parser;
import star.version_option_parser;
import star.root_base;
import star.configuration;
import star.root;
import star.root_loader;
import star.tileset_updater;

import star.liquids_database;
import star.material_database;
import star.object_database;


import star.algorithm;
using namespace Star;

String const InboundNode = "/tilesets/inboundnode.png";
String const OutboundNode = "/tilesets/outboundnode.png";
Vec3B const SourceLiquidBorderColor(0x80, 0x80, 0x00);

void scanMaterials(Root& root, TilesetUpdater& updater) {
  auto materials = root.materialDatabase();

  for (String materialName : materials->materialNames()) {
    MaterialId id = materials->materialId(materialName);
    Maybe<String> path = materials->materialPath(id);
    if (!path)
      continue;
    String source = root.assets()->assetSource(*path);

    auto renderProfile = materials->materialRenderProfile(id);
    if (renderProfile == nullptr)
      continue;

    String tileset = materials->materialCategory(id);
    String imagePath = renderProfile->pieceImage(renderProfile->representativePiece, 0);
    std::shared_ptr<::Star::Image const> image = root.assets()->image(imagePath);

    Tiled::Properties properties;
    properties.set("material", materialName);
    properties.set("//name", materialName);
    properties.set("//shortdescription", materials->materialShortDescription(id));
    properties.set("//description", materials->materialDescription(id));

    auto tile = std::make_shared<Tile>(Tile{source, "materials", tileset.toLower(), materialName, image, properties});
    updater.defineTile(tile);
  }
}

// imagePosition might not be aligned to a whole number, i.e. the image origin
// might not align with the tile grid. We do, however want Tile Objects in Tiled
// to be grid-aligned (valid positions are offset relative to the grid not
// completely free-form), so we correct the alignment by adding padding to the
// image that we export.
// We're going to ignore the fact that some objects have imagePositions that
// aren't even aligned _to pixels_ (e.g. giftsmallmonsterbox).
Vec2U objectPositionPadding(Vec2I imagePosition) {
  int pixelsX = imagePosition.x();
  int pixelsY = imagePosition.y();

  // Unsigned modulo operation gives the padding to use (in pixels)
  unsigned padX = (unsigned)pixelsX % TilePixels;
  unsigned padY = (unsigned)pixelsY % TilePixels;
  return Vec2U(padX, padY);
}

StringSet categorizeObject(Root& root, String const& objectName, Vec2U imageSize) {
  if (imageSize[0] >= 256 || imageSize[1] >= 256)
    return StringSet{"huge-objects"};

  auto assets = root.assets();
  auto objects = root.objectDatabase();

  Json defaultCategories = assets->json("/objects/defaultCategories.config");

  auto objectConfig = objects->getConfig(objectName);

  StringSet categories;
  if (objectConfig->category != defaultCategories.getString("category"))
    categories.insert("objects-by-category/" + objectConfig->category);
  for (String const& tag : objectConfig->colonyTags)
    categories.insert("objects-by-colonytag/" + tag);
  if (objectConfig->type != defaultCategories.getString("objectType"))
    categories.insert("objects-by-type/" + objectConfig->type);
  if (objectConfig->race != defaultCategories.getString("race"))
    categories.insert("objects-by-race/" + objectConfig->race);

  if (categories.size() == 0)
    categories.insert("objects-uncategorized");

  return transform<StringSet>(categories, [](String const& category) { return category.toLower(); });
}

void drawNodes(Assets const& assets, std::shared_ptr<::Star::Image> const& image, Vec2I imagePosition, JsonArray nodes, String nodeImagePath) {
  std::shared_ptr<::Star::Image const> nodeImage = assets.image(nodeImagePath);
  for (Json const& node : nodes) {
    Vec2I nodePos = jsonToVec2I(node) * TilePixels + Vec2I(0, TilePixels - nodeImage->height());
    Vec2U nodeImagePos = Vec2U(nodePos - imagePosition);
    image->drawInto(nodeImagePos, *nodeImage);
  }
}

void defineObjectOrientation(Root& root, TilesetUpdater& updater,
    String const& objectName,
    List<std::shared_ptr<::Star::ObjectOrientation>> const& orientations,
    int orientationIndex) {
  auto assets = root.assets();
  auto objects = root.objectDatabase();

  std::shared_ptr<::Star::ObjectOrientation> orientation = orientations[orientationIndex];

  Vec2I imagePosition = Vec2I(orientation->imagePosition * TilePixels);

  List<std::shared_ptr<::Star::Image const>> layers;
  unsigned width = 0, height = 0;
  for (auto const& imageLayer : orientation->imageLayers) {
    String imageName = AssetPath::join(imageLayer.imagePart().image).replaceTags(StringMap<String>{}, true, "default");

    std::shared_ptr<::Star::Image const> image = assets->image(imageName);
    layers.append(image);
    width = std::max(width, image->width());
    height = std::max(height, image->height());
  }

  Vec2U imagePadding = objectPositionPadding(imagePosition);
  imagePosition -= Vec2I(imagePadding);

  // Padding is added to the right hand side as well as the left so that
  // when objects are flipped in the editor, they're still aligned correctly.
  Vec2U imageSize(width + 2 * imagePadding.x(), height + imagePadding.y());

  std::shared_ptr<::Star::Image> combinedImage = std::make_shared<Image>(imageSize, PixelFormat::RGBA32);
  combinedImage->fill(Vec4B(0, 0, 0, 0));
  for (std::shared_ptr<::Star::Image const> const& layer : layers) {
    combinedImage->drawInto(imagePadding, *layer);
  }

  // Overlay the image with the wiring nodes:
  auto objectConfig = objects->getConfig(objectName);

  drawNodes(*assets, combinedImage, imagePosition, objectConfig->config.getArray("inputNodes", {}), InboundNode);
  drawNodes(*assets, combinedImage, imagePosition, objectConfig->config.getArray("outputNodes", {}), OutboundNode);

  std::shared_ptr<::Star::Object> example = objects->createObject(objectName);

  Tiled::Properties properties;
  properties.set("object", objectName);
  properties.set("imagePositionX", imagePosition.x());
  properties.set("imagePositionY", imagePosition.y());
  properties.set("//shortdescription", example->shortDescription());
  properties.set("//description", example->description());

  if (orientation->directionAffinity.isValid()) {
    Direction direction = *orientation->directionAffinity;
    if (orientation->flipImages)
      direction = -direction;
    properties.set("tilesetDirection", DirectionNames.getRight(direction));
  }

  StringSet tilesets = categorizeObject(root, objectName, imageSize);

  // tileName becomes part of the filename for the tile's image. Different
  // orientations require different images, so the tileName must be different
  // for each orientation.
  String tileName = objectName;
  if (orientationIndex != 0)
    tileName += "_orientation" + toString(orientationIndex);
  properties.set("//name", tileName);

  String source = assets->assetSource(objectConfig->path);

  for (String const& tileset : tilesets) {
    std::shared_ptr<::Star::Tile> tile = std::make_shared<Tile>(Tile{source, "objects", tileset, tileName, combinedImage, properties});
    updater.defineTile(tile);
  }
}

void scanObjects(Root& root, TilesetUpdater& updater) {
  auto objects = root.objectDatabase();

  for (String const& objectName : objects->allObjects()) {
    auto orientations = objects->getOrientations(objectName);
    if (orientations.size() < 1) {
      root.logger().warn("Object {} has no orientations and will not be exported", objectName);
      continue;
    }

    // Always export the first orientation
    std::shared_ptr<::Star::ObjectOrientation> orientation = orientations[0];
    defineObjectOrientation(root, updater, objectName, orientations, 0);

    // If there are more than 2 orientations or the imagePositions are different
    // then horizontal flipping in the editor is not enough to get all the
    // orientations and display them correctly, so we export each orientation
    // as a separate tile.
    for (unsigned i = 1; i < orientations.size(); ++i) {
      if (i >= 2 || orientation->imagePosition != orientations[i]->imagePosition)
        defineObjectOrientation(root, updater, objectName, orientations, i);
    }
  }
}

void scanLiquids(Root& root, TilesetUpdater& updater) {
  auto liquids = root.liquidsDatabase();
  auto assets = root.assets();

  Vec2U imageSize(TilePixels, TilePixels);

  for (auto liquid : liquids->allLiquidSettings()) {
    std::shared_ptr<::Star::Image> image = std::make_shared<Image>(imageSize, PixelFormat::RGBA32);
    image->fill(liquid->liquidColor);

    String assetSource = assets->assetSource(liquid->path);

    Tiled::Properties properties;
    properties.set("liquid", liquid->name);
    properties.set("//name", liquid->name);
    auto tile = std::make_shared<Tile>(Tile{assetSource, "liquids", "liquids", liquid->name, image, properties});
    updater.defineTile(tile);

    std::shared_ptr<::Star::Image> sourceImage = std::make_shared<Image>(imageSize, PixelFormat::RGBA32);
    sourceImage->copyInto(Vec2U(), *image.get());
    sourceImage->fillRect(Vec2U(), Vec2U(image->width(), 1), SourceLiquidBorderColor);
    sourceImage->fillRect(Vec2U(), Vec2U(1, image->height()), SourceLiquidBorderColor);

    String sourceName = liquid->name + "_source";
    properties.set("source", true);
    properties.set("//name", sourceName);
    properties.set("//shortdescription", "Endless " + liquid->name);
    auto sourceTile = std::make_shared<Tile>(Tile{assetSource, "liquids", "liquids", sourceName, sourceImage, properties});
    updater.defineTile(sourceTile);
  }
}

int main(int argc, char** argv) {
  [[maybe_unused]] Star::NativeRuntime nativeRuntime;
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});

    rootLoader.setSummary("Updates Tiled JSON tilesets in unpacked assets directories");

    std::unique_ptr<::Star::Root> root;
    OptionParser::Options options;
    std::tie(root, options) = rootLoader.commandInitOrDie(argc, argv);

    TilesetUpdater updater(root->logger());

    for (String source : root->assets()->assetSources()) {
      root->logger().info("Assets source: \"{}\"", source);
      updater.defineAssetSource(source);
    }

    scanMaterials(*root, updater);
    scanObjects(*root, updater);
    scanLiquids(*root, updater);

    updater.exportTilesets();

    return 0;
  } catch (std::exception const&) {
    ExceptionDiagnostic const e(std::current_exception());
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
