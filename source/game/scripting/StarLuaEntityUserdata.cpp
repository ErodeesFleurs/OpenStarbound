#include "StarJsonExtra.hpp"
import star.json;
import star.id_map;
import star.ordered_map;
import star.lru_cache;
import star.thread;
import star.time;
import star.random;
import star.ttl_cache;
#include "StarCasting.hpp"
import star.game_types;
import star.maybe;
import star.poly;
import star.variant;
#include "StarStrongTypedef.hpp"
import star.list;
import star.rpc_promise;
import star.vector;
import star.rect;
import star.bi_map;
import star.a_star;
import star.data_stream;
import star.set;
import star.string;
import star.color;
import star.image_processing;
import star.hash;
import star.string_view;
import star.directives;
import star.asset_path;
import star.observer_stream;
import star.either;
import star.periodic_function;
import star.audio;
import star.map;
import star.weighted_pool;
import star.periodic;
import star.lua;
import star.ordered_set;
import star.io_device;
import star.ref_ptr;
import star.logging;
import star.listener;
import std;
import star.format;
import star.version;

#include "StarLuaRoot.hpp"

#include "StarLuaGameConverters.hpp"
#include "StarLuaComponents.hpp"
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.damage_types;
import star.world_geometry;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.tile_damage;
import star.interaction_types;
import star.celestial_coordinate;
import star.quest_descriptor;
import star.interactive_entity;
import star.collision_block;
import star.tile_entity;
import star.tile_modification;
import star.force_regions;
import star.world;
import star.physics_entity;
import star.movement_controller;
import star.platformer_astar_types;
import star.anchorable_entity;

import star.game_timers;
import star.actor_movement_controller;
import star.drawable;
import star.item;
import star.item_descriptor;
import star.item_database;
import star.item_recipe;
import star.animated_part_set;
import star.animation;
import star.particle;
import star.mixer;
import star.networked_animator;
import star.entity_rendering_types;
import star.stat_collection;
import star.status_effect_database;
import star.status_controller;
import star.status_effect_entity;
import star.scripted_entity;
import star.chat_action;
import star.chatty_entity;
import star.wiring;
import star.wire_entity;
import star.inspectable_entity;
import star.entity_rendering;
import star.object;
import star.container_entity;
import star.container_object;
import star.farmable_object;
import star.uuid;
import star.humanoid;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.ai_types;
import star.player_types;
import star.radio_message_database;
import star.player;
import star.aggressive_entity;
import star.monster_database;
import star.effect_emitter;
import star.monster;
import star.npc_database;
import star.npc;
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.stagehand;
import star.vehicle;
import star.projectile;
import star.item_drop;



import star.player_inventory;
import star.item_bag;

namespace Star {

LuaMethods<std::shared_ptr<Entity>> LuaUserDataMethods<std::shared_ptr<Entity>>::make() {
    LuaMethods<std::shared_ptr<Entity>> methods;

    // general entity methods
    methods.registerMethod("exists",
    [&](std::shared_ptr<Entity> const& entity) -> bool {
        return entity->inWorld();
    });

    methods.registerMethod("id",
    [&](std::shared_ptr<Entity> const& entity) -> EntityId {
        return entity->entityId();
    });

    methods.registerMethod("canDamage",
    [&](std::shared_ptr<Entity> const& entity, EntityId const& otherId) -> bool {
        if (entity->inWorld()) {
            auto other = entity->world()->entity(otherId);
            
            if (!other || !entity->getTeam().canDamage(other->getTeam(), false))
            return false;
            
            return true;
        }
        return false;
    });

    methods.registerMethod("damageTeam",
    [&](std::shared_ptr<Entity> const& entity) -> Json {
        return entity->getTeam().toJson();
    });

    methods.registerMethod("aggressive",
    [&](std::shared_ptr<Entity> const& entity) -> Json {
        if (auto monster = as<Monster>(entity))
            return monster->aggressive();
        if (auto npc = as<Npc>(entity))
            return npc->aggressive();
        return false;
    });

    methods.registerMethod("type",
    [&](std::shared_ptr<Entity> const& entity, LuaEngine& engine) -> LuaString {
        return engine.createString(EntityTypeNames.getRight(entity->entityType()));
    });

    methods.registerMethod("typeName",
    [&](std::shared_ptr<Entity> const& entity, LuaEngine&) -> Maybe<String> {
        if (auto monster = as<Monster>(entity))
            return monster->typeName();
        if (auto npc = as<Npc>(entity))
            return npc->npcType();
        if (auto vehicle = as<Vehicle>(entity))
            return vehicle->name();
        if (auto object = as<Object>(entity))
            return object->name();
        if (auto itemDrop = as<ItemDrop>(entity)) {
            if (itemDrop->item())
            return itemDrop->item()->name();
        }
        return {};
    });

    methods.registerMethod("position",
    [&](std::shared_ptr<Entity> const& entity) -> Vec2F {
        return entity->position();
    });

    methods.registerMethod("metaBoundBox",
    [&](std::shared_ptr<Entity> const& entity) -> RectF {
        return entity->metaBoundBox();
    });

    methods.registerMethod("velocity",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<Vec2F> {
        if (auto mobileEntity = as<MobileEntity>(entity))
            return mobileEntity->movementController()->velocity();

        return {};
    });

    methods.registerMethod("name",
    [&](std::shared_ptr<Entity> const& entity) -> String {
        return entity->name();
    });

    methods.registerMethod("description",
    [&](std::shared_ptr<Entity> const& entity, Maybe<String> const& species) -> Maybe<String> {
        if (auto inspectableEntity = as<InspectableEntity>(entity)) {
            if (species)
            return inspectableEntity->inspectionDescription(*species);
        }

        return entity->description();
    });

    methods.registerMethod("uniqueId",
    [&](std::shared_ptr<Entity> const& entity) -> LuaNullTermWrapper<Maybe<String>> {
        return entity->uniqueId();
    });

    methods.registerMethod("getParameter",
    [&](std::shared_ptr<Entity> const& entity, String const& parameterName, Maybe<Json> const& defaultValue) -> Json {
        Json val = Json();
        
        bool handled = true;
        if (auto objectEntity = as<Object>(entity)) {
            val = objectEntity->configValue(parameterName);
        } else if (auto npcEntity = as<Npc>(entity)) {
            val = npcEntity->scriptConfigParameter(parameterName);
        } else if (auto projectileEntity = as<Projectile>(entity)) {
            val = projectileEntity->configValue(parameterName);
        } else if (auto stagehandEntity = as<Stagehand>(entity)) {
            val = stagehandEntity->configValue(parameterName);
        } else {
            handled = false;
        }
        if (!val && defaultValue && handled)
            val = *defaultValue;

        return val;
    });

    methods.registerMethod("sendMessage",
    [&](std::shared_ptr<Entity> const& entity, String const& message, LuaVariadic<Json> args) -> RpcPromise<Json> {
        if (entity->inWorld()) {
            return entity->world()->sendEntityMessage(entity->entityId(), message, JsonArray::from(std::move(args)));
        }
        return RpcPromise<Json>::createFailed("Entity not in world");
    });

    // scripted entity methods
    methods.registerMethod("callScript",
    [&](std::shared_ptr<Entity> const& entity, String const& function, LuaVariadic<LuaValue> const& args) -> Maybe<LuaValue> {
        auto scrEntity = as<ScriptedEntity>(entity);
        if (!scrEntity || !scrEntity->isMaster() || !entity->inWorld())
            throw StarException::format("Entity {} does not exist or is not a local master scripted entity", entity->entityId());
        return scrEntity->callScript(function, args);
    });

    // nametag entity methods
    methods.registerMethod("nametag",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<Json> {
        Json result;
        if (auto nametagEntity = as<NametagEntity>(entity)) {
            result = JsonObject{
                {"nametag", nametagEntity->nametag()},
                {"displayed", nametagEntity->displayNametag()},
                {"color", jsonFromColor(Color::rgb(nametagEntity->nametagColor()))},
                {"origin", jsonFromVec2F(nametagEntity->nametagOrigin())},
            };
            if (auto status = nametagEntity->statusText())
                result.set("status", *status);
        }

        return result;
    });

    // portrait entity methods
    methods.registerMethod("portrait",
    [&](std::shared_ptr<Entity> const& entity, String const& portraitMode) -> LuaNullTermWrapper<Maybe<List<Drawable>>> {
        if (auto portraitEntity = as<PortraitEntity>(entity))
            return portraitEntity->portrait(PortraitModeNames.getLeft(portraitMode));

        return {};
    });


    // damage bar entity methods
    methods.registerMethod("health",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<Vec2F> {
        if (auto dmgEntity = as<DamageBarEntity>(entity)) {
            return Vec2F(dmgEntity->health(), dmgEntity->maxHealth());
        }
        return {};
    });

    // interactive entity methods
    methods.registerMethod("isInteractive",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<bool> {
        if (auto interactEntity = as<InteractiveEntity>(entity))
            return interactEntity->isInteractive();
        return {};
    });

    // chatty entity methods
    methods.registerMethod("mouthPosition",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<Vec2F> {
        if (auto chatty = as<ChattyEntity>(entity))
            return chatty->mouthPosition();
        return {};
    });

    // actor entity methods

    // status controller methods, they're networked anyway so might as well make them available to read
    methods.registerMethod("statusProperty", [&](std::shared_ptr<Entity> entity, String name, Json const& def = Json()) -> Maybe<Json> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->statusProperty(name, def);
        return {};
    });
    methods.registerMethod("stat", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<float> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->stat(name);
        return {};
    });
    methods.registerMethod("statPositive", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->statPositive(name);
        return {};
    });
    methods.registerMethod("resourceNames", [&](std::shared_ptr<Entity> entity) -> Maybe<StringList> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resourceNames();
        return {};
    });
    methods.registerMethod("resource", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<float> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resource(name);
        return {};
    });
    methods.registerMethod("isResource", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->isResource(name);
        return {};
    });
    methods.registerMethod("resourcePositive", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resourcePositive(name);
        return {};
    });
    methods.registerMethod("resourceLocked", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resourceLocked(name);
        return {};
    });
    methods.registerMethod("resourceMax", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<float> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resourceMax(name);
        return {};
    });
    methods.registerMethod("resourcePercentage", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<float> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->resourcePercentage(name);
        return {};
    });
    methods.registerMethod("getPersistentEffects", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<JsonArray> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->getPersistentEffects(name).transformed(jsonFromPersistentStatusEffect);
        return {};
    });
    methods.registerMethod("activeUniqueStatusEffectSummary", [&](std::shared_ptr<Entity> entity) -> Maybe<List<JsonArray>> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->activeUniqueStatusEffectSummary().transformed([](std::pair<UniqueStatusEffect, Maybe<float>> effect) {
            JsonArray effectJson = {effect.first};
            if (effect.second)
                effectJson.append(*effect.second);
            return effectJson;
            });;
        return {};
    });
    methods.registerMethod("uniqueStatusEffectActive", [&](std::shared_ptr<Entity> entity, String name) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->statusController()->uniqueStatusEffectActive(name);
        return {};
    });

    // movement controller methods, they're networked anyway so might as well make them available to read

    methods.registerMethod("mass", [&](std::shared_ptr<Entity> entity) -> Maybe<float> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->mass();
        return {};
    });
    methods.registerMethod("boundBox", [&](std::shared_ptr<Entity> entity) -> Maybe<RectF> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->collisionPoly().boundBox();
        return {};
    });
    methods.registerMethod("collisionPoly", [&](std::shared_ptr<Entity> entity) -> Maybe<PolyF> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->collisionPoly();
        return {};
    });
    methods.registerMethod("collisionBody", [&](std::shared_ptr<Entity> entity) -> Maybe<PolyF> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->collisionBody();
        return {};
    });
    methods.registerMethod("collisionBoundBox", [&](std::shared_ptr<Entity> entity) -> Maybe<RectF> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->collisionBody().boundBox();
        return {};
    });
    methods.registerMethod("localBoundBox", [&](std::shared_ptr<Entity> entity) -> Maybe<RectF> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->localBoundBox();
        return {};
    });
    methods.registerMethod("rotation", [&](std::shared_ptr<Entity> entity) -> Maybe<float> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->rotation();
        return {};
    });
    methods.registerMethod("isColliding", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->isColliding();
        return {};
    });
    methods.registerMethod("isNullColliding", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->isNullColliding();
        return {};
    });
    methods.registerMethod("isCollisionStuck", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->isCollisionStuck();
        return {};
    });
    methods.registerMethod("stickingDirection", [&](std::shared_ptr<Entity> entity) -> Maybe<float> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->stickingDirection();
        return {};
    });
    methods.registerMethod("liquidPercentage", [&](std::shared_ptr<Entity> entity) -> Maybe<float> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->liquidPercentage();
        return {};
    });
    methods.registerMethod("liquidId", [&](std::shared_ptr<Entity> entity) -> Maybe<std::uint8_t> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->liquidId();
        return {};
    });
    methods.registerMethod("onGround", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->onGround();
        return {};
    });
    methods.registerMethod("zeroG", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->zeroG();
        return {};
    });
    methods.registerMethod("atWorldLimit", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->atWorldLimit();
        return {};
    });
    methods.registerMethod("anchorState", [&](std::shared_ptr<Entity> entity) -> LuaVariadic<LuaValue> {
        if (auto actor = as<ActorEntity>(entity))
            if (auto anchorState = actor->movementController()->anchorState())
            return LuaVariadic<LuaValue>{LuaInt(anchorState->entityId), LuaInt(anchorState->positionIndex)};
        return LuaVariadic<LuaValue>();
    });
    // slightly inconsistent for the sake of being more clear what the function is
    methods.registerMethod("baseMovementParameters", [&](std::shared_ptr<Entity> entity) -> Maybe<Json> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->baseParameters().toJson();
        return {};
    });
    // slightly inconsistent for the sake of being more clear what the function is
    methods.registerMethod("movementParameters", [&](std::shared_ptr<Entity> entity) -> Maybe<Json> {
        if (auto mobile = as<MobileEntity>(entity))
            return mobile->movementController()->parameters().toJson();
        return {};
    });

    methods.registerMethod("walking", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->walking();
        return {};
    });
    methods.registerMethod("running", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->running();
        return {};
    });
    methods.registerMethod("movingDirection", [&](std::shared_ptr<Entity> entity) -> Maybe<int> {
        if (auto actor = as<ActorEntity>(entity))
            return numericalDirection(actor->movementController()->movingDirection());
        return {};
    });
    methods.registerMethod("facingDirection", [&](std::shared_ptr<Entity> entity) -> Maybe<int> {
        if (auto actor = as<ActorEntity>(entity))
            return numericalDirection(actor->movementController()->facingDirection());
        return {};
    });
    methods.registerMethod("crouching", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->crouching();
        return {};
    });
    methods.registerMethod("flying", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->flying();
        return {};
    });
    methods.registerMethod("falling", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->falling();
        return {};
    });
    methods.registerMethod("canJump", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->canJump();
        return {};
    });
    methods.registerMethod("jumping", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->jumping();
        return {};
    });
    methods.registerMethod("groundMovement", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->groundMovement();
        return {};
    });
    methods.registerMethod("liquidMovement", [&](std::shared_ptr<Entity> entity) -> Maybe<bool> {
        if (auto actor = as<ActorEntity>(entity))
            return actor->movementController()->liquidMovement();
        return {};
    }); 

    // tool user entity methods
    methods.registerMethod("handItem",
    [&](std::shared_ptr<Entity> const& entity, String const& handName) -> Maybe<String> {
        ToolHand toolHand;
        if (handName == "primary") {
            toolHand = ToolHand::Primary;
        } else if (handName == "alt") {
            toolHand = ToolHand::Alt;
        } else {
            throw StarException(strf("Unknown tool hand {}", handName));
        }

        if (auto toolUser = as<ToolUserEntity>(entity)) {
            if (auto item = toolUser->handItem(toolHand)) {
                return item->name();
            }
        }

        return {};
    });

    methods.registerMethod("handItemDescriptor",
    [&](std::shared_ptr<Entity> const& entity, String const& handName) -> Json {
        ToolHand toolHand;
        if (handName == "primary") {
            toolHand = ToolHand::Primary;
        } else if (handName == "alt") {
            toolHand = ToolHand::Alt;
        } else {
            throw StarException(strf("Unknown tool hand {}", handName));
        }

        if (auto toolUser = as<ToolUserEntity>(entity)) {
            if (auto item = toolUser->handItem(toolHand)) {
                return item->descriptor().toJson();
            }
        }

        return Json();
    });

    methods.registerMethod("aimPosition",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<Vec2F> {
        if (auto toolUser = as<ToolUserEntity>(entity))
            return toolUser->aimPosition();
        return {};
    });


    // humanoid entity methods
    methods.registerMethod("species",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<String> {
        if (auto player = as<Player>(entity)) {
            return player->species();
        } else if (auto npc = as<Npc>(entity)) {
            return npc->species();
        } else {
            return {};
        }
    });

    methods.registerMethod("gender",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<String> {
        if (auto player = as<Player>(entity)) {
            return GenderNames.getRight(player->gender());
        } else if (auto npc = as<Npc>(entity)) {
            return GenderNames.getRight(npc->gender());
        } else {
            return {};
        }
    });

    // player methods
    methods.registerMethod("currency",
    [&](std::shared_ptr<Entity> const& entity, String const& currencyType) -> Maybe<std::uint64_t> {
        if (auto player = as<Player>(entity)) {
            return player->currency(currencyType);
        }
        return {};
    });

    methods.registerMethod("hasCountOfItem",
    [&](std::shared_ptr<Entity> const& entity, Json descriptor, Maybe<bool> exactMatch) -> Maybe<std::uint64_t> {
        if (auto player = as<Player>(entity)) {
            return player->inventory()->hasCountOfItem(ItemDescriptor(descriptor), exactMatch.value(false));
        }
        return {};
    });

    // loungeable entity methods
    methods.registerMethod("loungingEntities",
    [&](std::shared_ptr<Entity> const& entity, Maybe<std::size_t> anchorIndex) -> Maybe<List<EntityId>> {
        if (!entity->inWorld())
            return {};
        if (auto loungeable = as<LoungeableEntity>(entity))
            return loungeable->entitiesLoungingIn(anchorIndex.value()).values();
        return {};
    });

    methods.registerMethod("loungeableOccupied",
    [&](std::shared_ptr<Entity> const& entity, Maybe<std::size_t> anchorIndex) -> Maybe<bool> {
        if (!entity->inWorld())
            return {};
        auto loungeable = as<LoungeableEntity>(entity);
        std::size_t anchor = anchorIndex.value();
        if (loungeable && loungeable->anchorCount() > anchor)
            return !loungeable->entitiesLoungingIn(anchor).empty();
        return {};
    });

    methods.registerMethod("loungeableAnchorCount",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<std::size_t> {
        if (!entity->inWorld())
            return {};
        if (auto loungeable = as<LoungeableEntity>(entity))
            return loungeable->anchorCount();
        return {};
    });

    // object methods
    methods.registerMethod("objectSpaces",
    [&](std::shared_ptr<Entity> const& entity) -> List<Vec2I> {
        if (auto tileEntity = as<TileEntity>(entity))
            return tileEntity->spaces();
        return {};
    });

    // farmables
    methods.registerMethod("farmableStage",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<int> {
        if (auto farmable = as<FarmableObject>(entity)) {
            return farmable->stage();
        }

        return {};
    });

    // containers
    methods.registerMethod("containerSize",
    [&](std::shared_ptr<Entity> const& entity) -> Maybe<int> {
        if (auto container = as<ContainerObject>(entity))
            return container->containerSize();

        return {};
    });

    methods.registerMethod("containerClose",
    [&](std::shared_ptr<Entity> const& entity) -> bool {
        if (auto container = as<ContainerObject>(entity)) {
            container->containerClose();
            return true;
        }

        return false;
    });

    methods.registerMethod("containerOpen",
    [&](std::shared_ptr<Entity> const& entity) -> bool {
        if (auto container = as<ContainerObject>(entity)) {
            container->containerOpen();
            return true;
        }

        return false;
    });

    methods.registerMethod("containerItems",
    [&](std::shared_ptr<Entity> const& entity) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            JsonArray res;
            auto itemDb = Root::singleton().itemDatabase();
            for (auto const& item : container->itemBag()->items())
                res.append(itemDb->toJson(item));
            return res;
        }

        return Json();
    });

    methods.registerMethod("containerItemAt",
    [&](std::shared_ptr<Entity> const& entity, std::size_t offset) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto items = container->itemBag()->items();
            if (offset < items.size()) {
                return itemDb->toJson(items.at(offset));
            }
        }

        return Json();
    });

    methods.registerMethod("containerConsume",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Maybe<bool> {
        if (auto container = as<ContainerObject>(entity)) {
            auto toConsume = ItemDescriptor(items);
            return container->consumeItems(toConsume).result();
        }

        return {};
    });

    methods.registerMethod("containerConsumeAt",
    [&](std::shared_ptr<Entity> const& entity, std::size_t offset, int count) -> Maybe<bool> {
        if (auto container = as<ContainerObject>(entity)) {
            if (offset < container->containerSize()) {
                return container->consumeItems(offset, count).result();
            }
        }

        return {};
    });

    methods.registerMethod("containerAvailable",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Maybe<std::size_t> {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemBag = container->itemBag();
            auto toCheck = ItemDescriptor(items);
            return itemBag->available(toCheck);
        }

        return {};
    });

    methods.registerMethod("containerTakeAll",
    [&](std::shared_ptr<Entity> const& entity) -> Json {
        auto itemDb = Root::singleton().itemDatabase();
        if (auto container = as<ContainerObject>(entity)) {
            if (auto itemList = container->clearContainer().result()) {
                JsonArray res;
                for (auto item : *itemList)
                    res.append(itemDb->toJson(item));
                return res;
            }
        }

        return Json();
    });

    methods.registerMethod("containerTakeAt",
    [&](std::shared_ptr<Entity> const& entity, std::size_t offset) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            if (offset < container->containerSize()) {
                if (auto res = container->takeItems(offset).result())
                    return itemDb->toJson(*res);
            }
        }

        return Json();
    });

    methods.registerMethod("containerTakeNumItemsAt",
    [&](std::shared_ptr<Entity> const& entity, std::size_t offset, int const& count) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            if (offset < container->containerSize()) {
                if (auto res = container->takeItems(offset, count).result())
                    return itemDb->toJson(*res);
            }
        }

        return Json();
    });

    methods.registerMethod("containerItemsCanFit",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Maybe<std::size_t> {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto itemBag = container->itemBag();
            auto toSearch = itemDb->fromJson(items);
            return itemBag->itemsCanFit(toSearch);
        }

        return {};
    });

    methods.registerMethod("containerItemsFitWhere",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto itemBag = container->itemBag();
            auto toSearch = itemDb->fromJson(items);
            auto res = itemBag->itemsFitWhere(toSearch);
            return JsonObject{
                {"leftover", res.leftover},
                {"slots", jsonFromList<std::size_t>(res.slots)}
            };
        }

        return Json();
    });

    methods.registerMethod("containerAddItems",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto toInsert = itemDb->fromJson(items);
            if (auto res = container->addItems(toInsert).result())
                return itemDb->toJson(*res);
        }

        return items;
    });

    methods.registerMethod("containerStackItems",
    [&](std::shared_ptr<Entity> const& entity, Json const& items) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto toInsert = itemDb->fromJson(items);
            if (auto res = container->addItems(toInsert).result())
                return itemDb->toJson(*res);
        }

        return items;
    });

    methods.registerMethod("containerPutItemsAt",
    [&](std::shared_ptr<Entity> const& entity, Json const& items, std::size_t offset) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto toInsert = itemDb->fromJson(items);
            if (offset < container->containerSize()) {
                if (auto res = container->putItems(offset, toInsert).result())
                    return itemDb->toJson(*res);
            }
        }

        return items;
    });

    methods.registerMethod("containerSwapItems",
    [&](std::shared_ptr<Entity> const& entity, Json const& items, std::size_t offset, bool noCombine) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto toSwap = itemDb->fromJson(items);
            if (offset < container->containerSize()) {
                if (auto res = container->swapItems(offset, toSwap, !noCombine).result())
                    return itemDb->toJson(*res);
            }
        }

        return items;
    });

    methods.registerMethod("containerItemApply",
    [&](std::shared_ptr<Entity> const& entity, Json const& items, std::size_t offset) -> Json {
        if (auto container = as<ContainerObject>(entity)) {
            auto itemDb = Root::singleton().itemDatabase();
            auto toSwap = itemDb->fromJson(items);
            if (offset < container->containerSize()) {
                if (auto res = container->swapItems(offset, toSwap, false).result())
                    return itemDb->toJson(*res);
            }
        }

        return items;
    });

    methods.registerMethod("movingCollisionCount",
    [&](std::shared_ptr<Entity> const& entity) -> std::size_t {
        if (auto phys = as<PhysicsEntity>(entity)) {
            return phys->movingCollisionCount();
        }

        return 0;
    });

    methods.registerMethod("movingCollision",
    [&](std::shared_ptr<Entity> const& entity, std::size_t index) -> Maybe<PhysicsMovingCollision> {
        if (auto phys = as<PhysicsEntity>(entity)) {
            return phys->movingCollision(index);
        }

        return {};
    });

    return methods;
} 

}
