#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "spdlog/spdlog.h"
#include <memory>
#include <slint_models.h>
#include <string_view>

class ScoreboardView : StateAccessor<ui::ScoreboardState> {

  public:
    ScoreboardView(slint::ComponentHandle<UiEntry> uiEntry) : StateAccessor(uiEntry) {}

    void addScoreboardItem(ui::ScoreboardItem item) {
        // spdlog::info("Adding scoreboard for {}", std::string_view(item.name.data(), item.name.size()));
        // auto& self = *this;
        // switch (item.difficulty) {
        // case ui::Difficulty::Easy:
        //     std::dynamic_pointer_cast<slint::VectorModel<ui::ScoreboardItem>>(self->get_easy_items())->push_back(item);
        //     break;
        // case ui::Difficulty::Medium:
        //     std::dynamic_pointer_cast<slint::VectorModel<ui::ScoreboardItem>>(self->get_medium_items())
        //         ->push_back(item);
        //     break;
        // case ui::Difficulty::Hard:
        //     std::dynamic_pointer_cast<slint::VectorModel<ui::ScoreboardItem>>(self->get_hard_items())->push_back(item);
        //     break;
        // case ui::Difficulty::Custom:
        //     std::dynamic_pointer_cast<slint::VectorModel<ui::ScoreboardItem>>(self->get_custom_items())
        //         ->push_back(item);
        //     break;
        // }
    }
};