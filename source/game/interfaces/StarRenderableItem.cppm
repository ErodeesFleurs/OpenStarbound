module;
import std;
import star.json;
import star.audio;
import star.thread;
import star.list;
import star.map;
import star.set;
import star.maybe;
import star.bi_map;
import star.color;
import star.string;
import star.data_stream;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.game_types;

import star.mixer;
import star.drawable;
import star.entity_rendering_types;
import star.animation;
import star.particle;

import star.light_source;
import star.entity_rendering;

namespace Star {

  class RenderableItem;
  using RenderableItemPtr = std::shared_ptr<RenderableItem>;
  using RenderableItemConstPtr = std::shared_ptr<const RenderableItem>;
  using RenderableItemWeakPtr = std::weak_ptr<RenderableItem>;
  using RenderableItemConstWeakPtr = std::weak_ptr<const RenderableItem>;
  using RenderableItemUPtr = std::unique_ptr<RenderableItem>;
  using RenderableItemConstUPtr = std::unique_ptr<const RenderableItem>;

  class RenderableItem {
  public:
    virtual ~RenderableItem() {}

    virtual void render(RenderCallback* renderCallback, EntityRenderLayer renderLayer) = 0;
  };

}

export module star.renderable_item;

export namespace Star {
  using ::Star::RenderableItem;
  using ::Star::RenderableItemPtr;
  using ::Star::RenderableItemConstPtr;
  using ::Star::RenderableItemWeakPtr;
  using ::Star::RenderableItemConstWeakPtr;
  using ::Star::RenderableItemUPtr;
  using ::Star::RenderableItemConstUPtr;
}
