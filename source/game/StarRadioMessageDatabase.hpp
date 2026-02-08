#pragma once

#include "StarBiMap.hpp"
#include "StarException.hpp"
#include "StarJson.hpp"

import std;

namespace Star {

using RadioMessageDatabaseException = ExceptionDerived<"RadioMessageDatabaseException">;

enum class RadioMessageType { Generic,
                              Mission,
                              Quest,
                              Tutorial };
extern EnumMap<RadioMessageType> const RadioMessageTypeNames;

struct RadioMessage {
  String messageId;
  RadioMessageType type;
  bool unique;
  bool important;
  String text;
  String senderName;
  String portraitImage;
  int portraitFrames;
  float portraitSpeed;
  float textSpeed;
  float persistTime;
  String chatterSound;

  StringMap<RadioMessage> speciesAiMessage;
  StringMap<RadioMessage> speciesMessage;
};

class RadioMessageDatabase {
public:
  RadioMessageDatabase();

  [[nodiscard]] auto radioMessage(String const& messageName) const -> RadioMessage;
  [[nodiscard]] auto createRadioMessage(Json const& config, std::optional<String> const& uniqueId = {}) const -> RadioMessage;

private:
  StringMap<RadioMessage> m_radioMessages;
};

}// namespace Star
