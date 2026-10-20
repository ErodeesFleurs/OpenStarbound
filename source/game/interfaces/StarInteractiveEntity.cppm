module;
import std;
import star.type_identity;
import star.string;
import star.list;
import star.json;
import star.id_map;
import star.game_types;
import star.casting;
import star.vector;
import star.data_stream;
import star.strong_typedef;
import star.bi_map;

import star.interaction_types;

import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;

import star.light_source;

import star.entity;
import star.item_descriptor;
import star.celestial_coordinate;
import star.quest_descriptor;

import star.rect;
namespace Star {

class InteractiveEntity : public virtual Entity {
public:
  void const* queryType(TypeKey key) const noexcept override {
    return queryTypeOf<InteractiveEntity, Entity>(this, key);
  }
  std::string_view dynamicTypeName() const noexcept override {
    return Star::typeName<InteractiveEntity>();
  }

  // Interaction always takes place on the *server*, whether the interactive
  // entity is master or slave there.
  virtual InteractAction interact(InteractRequest const& request) = 0;

  // Defaults to metaBoundBox
  virtual RectF interactiveBoundBox() const;

  // Defaults to true
  virtual bool isInteractive() const;

  // Defaults to empty
  virtual List<QuestArcDescriptor> offeredQuests() const;
  virtual StringSet turnInQuests() const;

  // Defaults to position()
  virtual Vec2F questIndicatorPosition() const;
};

}

export module star.interactive_entity;

export namespace Star {
  using ::Star::InteractiveEntity;
}
