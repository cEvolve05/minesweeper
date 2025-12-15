#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "spdlog/spdlog.h"

class SettingView : StateAccessor<ui::SettingState> {

  public:
    SettingView(slint::ComponentHandle<UiEntry> uiEntry) : StateAccessor(uiEntry) {}

    ui::GameConfig getGameConfig() {
        auto self = *this;
        auto difficulty = self->get_difficulty();
        switch (difficulty) {
        case ui::Difficulty::Easy:
            return {.width = 9, .height = 9, .mine_count = 10};
            break;
        case ui::Difficulty::Medium:
            return {.width = 16, .height = 16, .mine_count = 40};
            break;
        case ui::Difficulty::Hard:
            return {.width = 30, .height = 16, .mine_count = 99};
            break;
        case ui::Difficulty::Custom:
            return {.width = self->get_custom_config_width(),
                    .height = self->get_custom_config_height(),
                    .mine_count = self->get_custom_config_mine_count()};
            break;
        default:
            spdlog::warn("Unknown difficulty, use Easy as default");
            return {.width = 9, .height = 9, .mine_count = 10};
        }
        return {.width = 9, .height = 9, .mine_count = 10};
    }
};