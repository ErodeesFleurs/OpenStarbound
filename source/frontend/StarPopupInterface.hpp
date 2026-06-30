#pragma once

#include "StarIAssets.hpp"
#include "StarPane.hpp"

namespace Star {

class PopupInterface;
using PopupInterfacePtr = SharedPtr<PopupInterface>;
class PopupInterface : public Pane {
public:
  struct Services {
    IAssetsConstPtr assets;
  };

  explicit PopupInterface(Services services);

  virtual ~PopupInterface() = default;

  void displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound = {});

private:
  IAssetsConstPtr m_assets;
};

}
