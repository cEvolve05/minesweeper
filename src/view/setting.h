#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "spdlog/spdlog.h"

class SettingView : StateAccessor<ui::SettingState> {

  public:
    SettingView(slint::ComponentHandle<UiEntry> uiEntry) : StateAccessor(uiEntry) {}
};