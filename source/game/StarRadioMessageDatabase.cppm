module;

import std;
import star.string;
import star.maybe;
import star.format;
import star.bi_map;
import star.json;

namespace Star {

struct RadioMessageDatabaseExceptionTag {
  static constexpr char const* name() { return "RadioMessageDatabaseException"; }
};
using RadioMessageDatabaseException = StarError<RadioMessageDatabaseExceptionTag, StarException>;
struct RadioMessage;
using RadioMessagePtr = std::shared_ptr<RadioMessage>;
using RadioMessageConstPtr = std::shared_ptr<const RadioMessage>;
using RadioMessageWeakPtr = std::weak_ptr<RadioMessage>;
using RadioMessageConstWeakPtr = std::weak_ptr<const RadioMessage>;
using RadioMessageUPtr = std::unique_ptr<RadioMessage>;
using RadioMessageConstUPtr = std::unique_ptr<const RadioMessage>;
class RadioMessageDatabase;
using RadioMessageDatabasePtr = std::shared_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstPtr = std::shared_ptr<const RadioMessageDatabase>;
using RadioMessageDatabaseWeakPtr = std::weak_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstWeakPtr = std::weak_ptr<const RadioMessageDatabase>;
using RadioMessageDatabaseUPtr = std::unique_ptr<RadioMessageDatabase>;
using RadioMessageDatabaseConstUPtr = std::unique_ptr<const RadioMessageDatabase>;

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
