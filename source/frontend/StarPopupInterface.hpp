#pragma once

#include "StarAssets.hpp"
#include "StarPane.hpp"

namespace Star {

class PopupInterface;
using PopupInterfacePtr = SharedPtr<PopupInterface>;
class PopupInterface : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
  };

  explicit PopupInterface(Services services);

  virtual ~PopupInterface() = default;

  void displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound = {});

private:
  AssetsConstPtr m_assets;
};

}
