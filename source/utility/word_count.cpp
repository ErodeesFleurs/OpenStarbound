#include "StarFile.hpp"
#include "StarLexicalCast.hpp"
#include "StarImage.hpp"
#include "StarRootLoader.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"
#include "StarJson.hpp"

using namespace Star;

int main(int argc, char** argv) {
  try {
    RootLoader rootLoader({{}, {}, {}, LogLevel::Error, false, {}});

    rootLoader.setSummary("Calculate a (very approximate) word count of user-facing text in assets");

    auto [root, options] = rootLoader.commandInitOrDie(argc, argv);

    StringMap<int> wordCounts;
    auto assets = root->assets();

    auto countWordsInType = [&](String const& type, function<int(Json const&)> countFunction, Maybe<function<bool(String const&)>> filterFunction = {}, Maybe<String> wordCountKey = {}) {
      auto files = assets->scanExtension(type).values();
      if (filterFunction)
        files.filter(*filterFunction);
      assets->queueJsons(files);
      for (auto& path : files) {
        auto json = assets->json(path);
        if (json.isNull())
          continue;

        String countKey = wordCountKey ? *wordCountKey : strf(".{} files", type);
        wordCounts[countKey] += countFunction(json);
      }
    };

    StringList itemFileTypes = {
        "tech",
        "item",
        "liqitem",
        "matitem",
        "miningtool",
        "flashlight",
        "wiretool",
        "beamaxe",
        "tillingtool",
        "painttool",
        "harvestingtool",
        "head",
        "chest",
        "legs",
        "back",
        "currencyitem",
        "consumable",
        "blueprint",
        "inspectiontool",
        "instrument",
        "thrownitem",
        "unlock",
        "activeitem",
        "augment" };

    for (auto& itemFileType : itemFileTypes) {
      countWordsInType(itemFileType, [](Json const& json) {
          int wordCount = 0;
          wordCount += json.getString("shortdescription", "").split(" ").count();
          wordCount += json.getString("description", "").split(" ").count();
          return wordCount;
        });
    }

    countWordsInType("object", [](Json const& json) {
        int wordCount = 0;
        wordCount += json.getString("shortdescription", "").split(" ").count();
        wordCount += json.getString("description", "").split(" ").count();
        wordCount += json.getString("apexDescription", "").split(" ").count();
        wordCount += json.getString("avianDescription", "").split(" ").count();
        wordCount += json.getString("glitchDescription", "").split(" ").count();
        wordCount += json.getString("floranDescription", "").split(" ").count();
        wordCount += json.getString("humanDescription", "").split(" ").count();
        wordCount += json.getString("hylotlDescription", "").split(" ").count();
        wordCount += json.getString("novakidDescription", "").split(" ").count();
        return wordCount;
      });

    countWordsInType("codex", [](Json const& json) {
        int wordCount = 0;
        wordCount += json.getString("title", "").split(" ").count();
        wordCount += json.getString("description", "").split(" ").count();
        for (auto contentPage : json.getArray("contentPages", JsonArray()))
          wordCount += contentPage.toString().split(" ").count();
        return wordCount;
      });

    countWordsInType("monstertype", [](Json const& json) {
        return json.getString("description", "").split(" ").count();
      });

    countWordsInType("radiomessages", [](Json const& json) {
        auto wordCount = 0;
        for (auto [_, messageConfig] : json.iterateObject())
          wordCount += messageConfig.getString("text", "").split(" ").count();
        return wordCount;
      });

    function<int(Json const& json)> countOnlyStrings;
    countOnlyStrings = [&](Json const& json) {
      int wordCount = 0;
      if (json.isType(Json::Type::Object)) {
        for (auto [_, value] : json.iterateObject())
          wordCount += countOnlyStrings(value);
      } else if (json.isType(Json::Type::Array)) {
        for (auto entry : json.iterateArray())
          wordCount += countOnlyStrings(entry);
      } else if (json.isType(Json::Type::String)) {
        if (!json.toString().beginsWith("/")) {
          wordCount += json.toString().split(" ").count();
        }
      }
      return wordCount;
    };

    function<bool(String const&)> dialogFilter = [](String const& filePath) { return filePath.beginsWith("/dialog/"); };
    countWordsInType("config", countOnlyStrings, dialogFilter, String("NPC dialog (.config files)"));

    countWordsInType("npctype", [&](Json const& json) {
        if (auto scriptConfig = json.get("scriptConfig", Json()))
          return countOnlyStrings(scriptConfig.get("dialog", Json()));
        return 0;
      }, {}, String("NPC dialog (.npctype files)"));

    countWordsInType("questtemplate", [&](Json const& json) {
        int wordCount = 0;
        wordCount += json.getString("title", "").split(" ").count();
        wordCount += json.getString("text", "").split(" ").count();
        wordCount += json.getString("completionText", "").split(" ").count();
        if (auto scriptConfig = json.get("scriptConfig", Json()))
          wordCount += countOnlyStrings(scriptConfig.get("generatedText", Json()));
        return wordCount;
      });

    countWordsInType("collection", [&](Json const& json) {
        int wordCount = 0;
        for (auto [_, collectable] : json.get("collectables", Json()).iterateObject())
          wordCount += collectable.getString("description", "").split(" ").count();
        return wordCount;
      });

    countWordsInType("cinematic", [&](Json const& json) {
        int wordCount = 0;
        for (auto panel : json.get("panels", Json()).iterateArray()) {
          auto panelText = panel.optString("text");
          // filter on pipes to ignore those long lists of backer names in the credits
          if (panelText && !panelText->contains("|"))
            wordCount += panelText->split(" ").count();
        }
        return wordCount;
      });

    countWordsInType("aimission", [&](Json const& json) {
        int wordCount = 0;
        for (auto [_, speciesText] : json.get("speciesText", Json()).iterateObject()) {
          wordCount += speciesText.getString("buttonText", "").split(" ").count();
          wordCount += speciesText.getString("repeatButtonText", "").split(" ").count();
          if (auto selectSpeech = speciesText.get("selectSpeech"))
            wordCount += selectSpeech.getString("text", "").split(" ").count();
        }
        return wordCount;
      });

    auto cockpitConfig = assets->json("/interface/cockpit/cockpit.config");
    int cockpitWordCount = 0;
    cockpitWordCount += countOnlyStrings(cockpitConfig.get("visitableTypeDescription"));
    cockpitWordCount += countOnlyStrings(cockpitConfig.get("worldTypeDescription"));
    wordCounts["planet descriptions (cockpit.config)"] = cockpitWordCount;

    int totalWordCount = 0;
    for (auto [wordCountLabel, wordCount] : wordCounts) {
      coutf("{} words in {}\n", wordCount, wordCountLabel);
      totalWordCount += wordCount;
    }
    coutf("approximately {} words total\n", totalWordCount);

    return 0;
  } catch (std::exception const& e) {
    cerrf("exception caught: {}\n", outputException(e, true));
    return 1;
  }
}
