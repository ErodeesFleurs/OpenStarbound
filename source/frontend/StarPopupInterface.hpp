#pragma once

#include "StarPane.hpp"

namespace Star {

class PopupInterface;
using PopupInterfacePtr = SharedPtr<PopupInterface>;
class PopupInterface : public Pane {
public:
  PopupInterface();

  virtual ~PopupInterface() = default;

  void displayMessage(String const& message, String const& title, String const& subtitle, Maybe<String> const& onShowSound = {});

private:
};

}
