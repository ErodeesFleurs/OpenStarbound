#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.radio_message_database;

namespace Star {

EnumMap<RadioMessageType> const RadioMessageTypeNames{
    {RadioMessageType::Generic, "generic"},
    {RadioMessageType::Mission, "mission"},
    {RadioMessageType::Quest, "quest"},
    {RadioMessageType::Tutorial, "tutorial"},
};

RadioMessageDatabase::RadioMessageDatabase() {
  auto assets = Root::singleton().assets();
  auto& files = assets->scanExtension("radiomessages");
  for (auto& file : files) {
    try {
      Json messages = assets->json(file);
      for (auto pair : messages.iterateObject()) {
        if (m_radioMessages.contains(pair.first))
          throw RadioMessageDatabaseException(strf("Duplicate radiomessage {} in file {}", pair.first, file));

        m_radioMessages[pair.first] = createRadioMessage(pair.second, pair.first);
      }
    } catch (std::exception const& e) {
      throw RadioMessageDatabaseException(strf("Error loading radiomessages file {}", file), e);
    }
  }
}

RadioMessage RadioMessageDatabase::radioMessage(String const& messageName) const {
  if (auto message = m_radioMessages.maybe(messageName))
    return message.take();
  throw RadioMessageDatabaseException(strf("Unknown radio message {}", messageName));
}

RadioMessage RadioMessageDatabase::createRadioMessage(Json const& config,  Maybe<String> const& messageId) const {
  if (config.isType(Json::Type::String)) {
    return radioMessage(config.toString());
  } else if (config.isType(Json::Type::Object)) {
    Json const& defaults = Root::singleton().assets()->json("/radiomessages.config:messageDefaults");
    auto mergedConfig = jsonMerge(defaults, config);

    RadioMessage message;
    message.messageId = messageId.value(mergedConfig.getString("messageId", ""));
    if (message.messageId.empty())
      throw RadioMessageDatabaseException("Custom radio messages must specify a messageId!");
    message.type = RadioMessageTypeNames.getLeft(mergedConfig.getString("type"));
    // mission messages default to non unique because they are already restricted to play
    // once per session (cleared on player init) but should repeat when the mission is replayed
    if (message.type == RadioMessageType::Mission)
      message.unique = config.getBool("unique", false);
    else
      message.unique = mergedConfig.getBool("unique");
    message.important = mergedConfig.getBool("important");
    message.text = mergedConfig.getString("text");
    message.senderName = mergedConfig.getString("senderName");
    message.portraitImage = mergedConfig.getString("portraitImage");
    message.portraitFrames = mergedConfig.getInt("portraitFrames");
    message.portraitSpeed = mergedConfig.getFloat("portraitSpeed");
    message.textSpeed = mergedConfig.getFloat("textSpeed");
    message.persistTime = mergedConfig.getFloat("persistTime");
    message.chatterSound = mergedConfig.getString("chatterSound");

    auto merger = config.eraseKey("speciesAiMessage").eraseKey("speciesMessage");
    for (auto p : mergedConfig.getObject("speciesAiMessage", JsonObject()))
      message.speciesMessage.set(p.first, createRadioMessage(jsonMerge(merger, p.second), messageId));
    for (auto p : mergedConfig.getObject("speciesMessage", JsonObject()))
      message.speciesMessage.set(p.first, createRadioMessage(jsonMerge(merger, p.second), messageId));

    if (message.portraitFrames <= 0)
      throw RadioMessageDatabaseException(
          strf("Invalid portraitFrames {} in radio message config!", message.portraitFrames));

    return message;
  } else {
    throw RadioMessageDatabaseException("Invalid radio message specification; expected message name or configuration.");
  }
}

}
