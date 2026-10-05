module;

#include "StarBiMap.hpp"
#include "StarJson.hpp"

namespace Star {

struct RadioMessageDatabaseExceptionTag {
  static constexpr char const* name() { return "RadioMessageDatabaseException"; }
};
using RadioMessageDatabaseException = StarError<RadioMessageDatabaseExceptionTag, StarException>;
STAR_STRUCT(RadioMessage);
STAR_CLASS(RadioMessageDatabase);

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
  RadioMessageDatabase();

  RadioMessage radioMessage(String const& messageName) const;
  RadioMessage createRadioMessage(Json const& config, Maybe<String> const& uniqueId = {}) const;

private:
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
  using ::Star::RadioMessagePtr;
  using ::Star::RadioMessageConstPtr;
  using ::Star::RadioMessageDatabase;
  using ::Star::RadioMessageDatabasePtr;
  using ::Star::RadioMessageDatabaseConstPtr;
}
