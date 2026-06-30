#include "StarLayout.hpp"

namespace Star {

Layout::Layout(GuiContext& context) : Widget(context) {
  markAsContainer();
}

void Layout::update(float dt) {
  Widget::update(dt);
}

}
