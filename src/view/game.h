#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"

class GameView : StateAccessor<ui::GameState> {
  public:
    GameView(slint::ComponentHandle<UiEntry> uiEntry) : StateAccessor(uiEntry) { auto self = *this; }
};