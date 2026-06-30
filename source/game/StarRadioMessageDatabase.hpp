#pragma once

#include "StarBiMap.hpp"
#include "StarJson.hpp"

namespace Star {

struct RadioMessageDatabaseExceptionTag { static constexpr char const* typeName = "RadioMessageDatabaseException"; };
using RadioMessageDatabaseException = TypedException<StarException, RadioMessageDatabaseExceptionTag>;

class RadioMessageDatabase;
using RadioMessageDatabasePtr = SharedPtr<RadioMessageDatabase>;
using RadioMessageDatabaseConstPtr = SharedPtr<RadioMessageDatabase const>;

enum class RadioMessageType { Generic, Mission, Quest, Tutorial };
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

  RadioMessage radioMessage(String const& messageName) const;
  RadioMessage createRadioMessage(Json const& config, Maybe<String> const& uniqueId = {}) const;

private:
  StringMap<RadioMessage> m_radioMessages;
};

}
