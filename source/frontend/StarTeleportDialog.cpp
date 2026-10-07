#include "StarJson.hpp"
#include "StarXXHash.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarEither.hpp"
#include "StarVector.hpp"
#include "StarPoly.hpp"
#include "StarVariant.hpp"
#include "StarGameTypes.hpp"
#include "StarColor.hpp"
#include "StarBiMap.hpp"
#include "StarMaybe.hpp"
#include "StarWeightedPool.hpp"
#include "StarDirectives.hpp"
#include "StarAssetPath.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarFont.hpp"
#include "StarIODevice.hpp"
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
#include "StarVersion.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarStrongTypedef.hpp"
#include "StarArray.hpp"
#include "StarNetElementFloatFields.hpp"
#include "StarNetElementSystem.hpp"
#include "StarIdMap.hpp"
#include "StarImage.hpp"
#include "StarMultiArray.hpp"
#include "StarMathCommon.hpp"
#include "StarInterpolation.hpp"
#include "StarRpcPromise.hpp"
#include "StarNetCompatibility.hpp"
#include <functional>
#include "StarSectorArray2D.hpp"
#include "StarPerlin.hpp"
#include "StarBTreeDatabase.hpp"
#include "StarOrderedSet.hpp"
#include "StarZSTDCompression.hpp"
#include "StarConfig.hpp"
#include <atomic>
#include <memory>
#include "StarAStar.hpp"
#include "StarPeriodicFunction.hpp"
#include "StarMatrix3.hpp"
#include "StarNetElement.hpp"

#include "StarLuaRoot.hpp"
import star.celestial_coordinate;
import star.sky_types;
import star.animation;
import star.particle;
import star.weather_types;
import star.force_regions;
import star.world_parameters;
import star.celestial_parameters;
import star.celestial_types;

#include "StarApplicationController.hpp"
#include "StarRenderer.hpp"
import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
import star.uuid;
import star.warping;

// Parse foundations before importing global bookmark value types on GCC.
import star.sky_parameters;
import star.system_world;
import star.player_universe_map;


import star.bookmark_interface;


import star.teleport_dialog;
import star.collision_block;
import star.liquid_types;
import star.tile_damage;
import star.worker_pool;
import star.tile_sector_array;
import star.world_layout;
import star.collision_generator;
import star.world_tiles;
import star.item_descriptor;
import star.chat_types;
import star.tile_modification;
import star.damage_types;
import star.status_types;
import star.damage;
import star.light_source;
import star.entity;
import star.interaction_types;
import star.world_geometry;
import star.wiring;
import star.quest_descriptor;
import star.interactive_entity;
import star.tile_entity;
import star.inspectable_entity;
import star.plant;
import star.plant_database;
import star.biome_placement;
import star.versioning_database;
import star.world_storage;
import star.player_types;
import star.damage_manager;
import star.net_packets;
import star.entity_rendering_types;
import star.sky_render_data;
import star.parallax;
import star.cellular_light_array;
import star.cellular_lighting;
import star.world_render_data;
import star.world_structure;
import star.chat_action;
import star.entity_rendering;
import star.world;
#include "StarLuaComponents.hpp"
import star.world_client_state;
import star.interpolation_tracker;
import star.ambient;
import star.weather;
import star.world_client;
import star.host_address;
import star.ai_types;
import star.socket;
import star.tcp;
#include "StarP2PNetworkingService.hpp"
import star.net_packet_socket;
import star.universe_connection;
import star.world_client_thread;
import star.universe;
// TODO: make this more thread safe
import star.universe_client;
import star.animated_part_set;
import star.networked_animator;
import star.humanoid;
import star.physics_entity;
import star.anchorable_entity;
import star.mobile_entity;
import star.actor_entity;
import star.tool_user_entity;
import star.lounging_entities;
import star.chatty_entity;
import star.emote_entity;
import star.portrait_entity;
import star.damage_bar_entity;
import star.nametag_entity;
import star.inventory_types;
import star.movement_controller;
import star.platformer_astar_types;
import star.actor_movement_controller;
#include "StarLuaActorMovementComponent.hpp"
#include "StarLuaAnimationComponent.hpp"
import star.radio_message_database;
import star.player;
import star.widget_parsing;
import star.gui_reader;
import star.game_timers;
import star.pane_manager;
import star.button_group;
import star.button_widget;
import star.image_widget;
import star.label_widget;
import star.list_widget;


import star.celestial_database;

import star.client_context;
import star.team_client;

import star.quest_manager;

namespace Star {

TeleportDialog::TeleportDialog(UniverseClientPtr client,
    PaneManager* paneManager,
    Json config,
    EntityId sourceEntityId,
    TeleportBookmark currentLocation) {
  m_client = client;
  m_paneManager = paneManager;
  m_sourceEntityId = sourceEntityId;
  m_currentLocation = currentLocation;

  auto assets = Root::singleton().assets();

  GuiReader reader;

  reader.registerCallback("dismiss", bind(&Pane::dismiss, this));
  reader.registerCallback("teleport", bind(&TeleportDialog::teleport, this));
  reader.registerCallback("selectDestination", bind(&TeleportDialog::selectDestination, this));

  reader.construct(assets->json("/interface/windowconfig/teleportdialog.config:paneLayout"), this);

  config = assets->fetchJson(config);
  auto destList = fetchChild<ListWidget>("bookmarkList.bookmarkItemList");
  destList->registerMemberCallback("editBookmark", bind(&TeleportDialog::editBookmark, this));

  for (auto dest : config.getArray("destinations", JsonArray())) {
    if (auto prerequisite = dest.optString("prerequisiteQuest")) {
      if (!m_client->mainPlayer()->questManager()->hasCompleted(*prerequisite))
        continue;
    }

    auto warpAction = parseWarpAction(dest.getString("warpAction"));
    bool deploy = dest.getBool("deploy", false);
    if (warpAction == WarpAlias::OrbitedWorld && !m_client->canBeamDown(deploy))
      continue;

    auto entry = destList->addItem();
    entry->fetchChild<LabelWidget>("name")->setText(dest.getString("name"));
    entry->fetchChild<LabelWidget>("planetName")->setText(dest.getString("planetName", ""));
    if (dest.contains("icon"))
      entry->fetchChild<ImageWidget>("icon")->setImage(
          strf("/interface/bookmarks/icons/{}.png", dest.getString("icon")));
    entry->fetchChild<ButtonWidget>("editButton")->hide();

    if (dest.getBool("mission", false)) {
      // if the warpaction is for an instance world, set the uuid to the team uuid
      if (auto warpToWorld = warpAction.ptr<WarpToWorld>()) {
        if (auto worldId = warpToWorld->world.ptr<InstanceWorldId>())
          warpAction = WarpToWorld(InstanceWorldId(worldId->instance, m_client->teamUuid(), worldId->level), warpToWorld->target);
      }
    }

    m_destinations.append({warpAction, deploy});
  }

  String beamPartyMember = assets->json("/interface/windowconfig/teleportdialog.config:beamPartyMemberLabel").toString();
  String deployPartyMember = assets->json("/interface/windowconfig/teleportdialog.config:deployPartyMemberLabel").toString();
  String beamPartyMemberIcon = assets->json("/interface/windowconfig/teleportdialog.config:beamPartyMemberIcon").toString();
  String deployPartyMemberIcon = assets->json("/interface/windowconfig/teleportdialog.config:deployPartyMemberIcon").toString();

  if (config.getBool("includePartyMembers", false)) {
    auto teamClient = m_client->teamClient();
    for (auto member : teamClient->members()) {
      if (member.uuid == m_client->clientContext()->playerUuid() || member.warpMode == WarpMode::None)
        continue;

      auto entry = destList->addItem();
      entry->fetchChild<LabelWidget>("name")->setText(member.name);

      if (member.warpMode == WarpMode::DeployOnly)
        entry->fetchChild<LabelWidget>("planetName")->setText(deployPartyMember);
      else
        entry->fetchChild<LabelWidget>("planetName")->setText(beamPartyMember);

      if (member.warpMode == WarpMode::DeployOnly)
        entry->fetchChild<ImageWidget>("icon")->setImage(deployPartyMemberIcon);
      else
        entry->fetchChild<ImageWidget>("icon")->setImage(beamPartyMemberIcon);

      entry->fetchChild<ButtonWidget>("editButton")->hide();

      m_destinations.append({WarpToPlayer(member.uuid), member.warpMode == WarpMode::DeployOnly});
    }
  }

  if (config.getBool("includePlayerBookmarks", false)) {
    auto teleportBookmarks = m_client->mainPlayer()->universeMap()->teleportBookmarks();

    teleportBookmarks.sort([](auto const& a, auto const& b) { return a.bookmarkName.toLower() < b.bookmarkName.toLower(); });

    for (auto bookmark : teleportBookmarks) {
      auto entry = destList->addItem();
      setupBookmarkEntry(entry, bookmark);
      if (bookmark == m_currentLocation) {
        destList->setEnabled(destList->itemPosition(entry), false);
        entry->fetchChild<ButtonWidget>("editButton")->setEnabled(false);
      }
      m_destinations.append({WarpToWorld(bookmark.target.first, bookmark.target.second), false});
    }
  }

  fetchChild<ButtonWidget>("btnTeleport")->setEnabled(destList->selectedItem() != NPos);
}

void TeleportDialog::tick(float) {
  if (!m_client->worldClient()->playerCanReachEntity(m_sourceEntityId))
    dismiss();
}

void TeleportDialog::selectDestination() {
  auto destList = fetchChild<ListWidget>("bookmarkList.bookmarkItemList");
  fetchChild<ButtonWidget>("btnTeleport")->setEnabled(destList->selectedItem() != NPos);
}

void TeleportDialog::teleport() {
  auto destList = fetchChild<ListWidget>("bookmarkList.bookmarkItemList");
  if (destList->selectedItem() != NPos) {
    auto& destination = m_destinations[destList->selectedItem()];
    auto warpAction = destination.first;
    bool deploy = destination.second;

    auto warp = [this, deploy](WarpAction const& action, String const& animation = "default") {
      if (deploy)
        m_client->warpPlayer(action, true, "deploy", true);
      else
        m_client->warpPlayer(action, true, animation);
    };

    m_client->worldClient()->sendEntityMessage(m_sourceEntityId, "onTeleport", {printWarpAction(warpAction)});
    if (warpAction.is<WarpAlias>() && warpAction.get<WarpAlias>() == WarpAlias::OrbitedWorld) {
      warp(take(destination).first, "beam");
    } else {
      warp(take(destination).first);
    }
    dismiss();
  }
}

void TeleportDialog::editBookmark() {
  auto destList = fetchChild<ListWidget>("bookmarkList.bookmarkItemList");
  if (destList->selectedItem() != NPos) {
    size_t selectedItem = destList->selectedItem();
    auto bookmarks = m_client->mainPlayer()->universeMap()->teleportBookmarks();
    bookmarks.sort([](auto const& a, auto const& b) { return a.bookmarkName.toLower() < b.bookmarkName.toLower(); });
    selectedItem = selectedItem - (m_destinations.size() - bookmarks.size());
    if (bookmarks.size() > selectedItem) {
      auto editBookmarkDialog = make_shared<EditBookmarkDialog>(m_client->mainPlayer()->universeMap());
      editBookmarkDialog->setBookmark(bookmarks[selectedItem]);
      m_paneManager->displayPane(PaneLayer::ModalWindow, editBookmarkDialog);
    }
    dismiss();
  }
}

}
