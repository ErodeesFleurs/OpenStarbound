#pragma once

#include <optional>

#include "StarPane.hpp"

namespace Star {

STAR_CLASS(PopupInterface);
class PopupInterface : public Pane {
public:
  PopupInterface();

  virtual ~PopupInterface() {}

  void displayMessage(String const& message, String const& title, String const& subtitle, std::optional<String> const& onShowSound = {});

private:
};

}
