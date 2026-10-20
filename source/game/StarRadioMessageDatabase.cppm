module;

import std;
import star.string;
import star.maybe;
import star.format;
import star.bi_map;
import star.json;

namespace Star {

class Root;

struct RadioMessageDatabaseExceptionTag {
  static constexpr char const* name() { return "RadioMessageDatabaseException"; }
};
using RadioMessageDatabaseException = StarError<RadioMessageDatabaseExceptionTag, StarException>;

enum class RadioMessageType { Generic, Mission, Quest, Tutorial };
extern EnumMap<RadioMessageType> const RadioMessageTypeNames;

struct RadioMessage {
  // Break the default-construction trait cycle in the recursive StringMap members.
  RadioMessage() = default;

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
  explicit RadioMessageDatabase(Root& root);

  RadioMessage radioMessage(String const& messageName) const;
  RadioMessage createRadioMessage(Json const& config, Maybe<String> const& uniqueId = {}) const;

private:
  Root& m_root;
  StringMap<RadioMessage> m_radioMessages;
};

}

export module star.radio_message_database;

export namespace Star {
  using ::Star::RadioMessageDatabaseExceptionTag;
  using ::Star::RadioMessageDatabaseException;
  using ::Star::RadioMessageType;
  using ::Star::RadioMessageTypeNames;
  using ::Star::RadioMessage;
  using ::Star::RadioMessageDatabase;
}
