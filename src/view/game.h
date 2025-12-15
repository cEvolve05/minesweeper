#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "spdlog/spdlog.h"
#include <map>
#include <slint_enums.h>
#include <slint_models.h>
#include <slint_sharedvector.h>
#include <utility>
#include <vector>

struct GridRef {
    bool isOpened;
    bool isMine;
    bool isFlag;
    ui::GridStatus* status;
};

class GameView : StateAccessor<ui::GameState> {
    std::map<std::pair<int, int>, GridRef> gridMap;
    std::vector<ui::GridData> shardData;
    std::pair<int, int> gridSize; // x, y

  public:
    GameView(slint::ComponentHandle<UiEntry> uiEntry) : StateAccessor(uiEntry) {
        auto self = *this;
        self->on_clicked([this](int x, int y) { open(x, y); });
        self->on_pointer_event([this](int x, int y, slint::private_api::PointerEvent event) {
            if (event.button == slint::private_api::PointerEventButton::Right &&
                event.kind == slint::private_api::PointerEventKind::Down) {
                flag(x, y);
            }
        });
        self->on_pressed_changed(
            [](int x, int y, bool pressed) { spdlog::info("on_pressed_changed ({}, {}): {}", x, y, pressed); });
        self->set_grid_data(std::make_shared<slint::VectorModel<ui::GridData>>(shardData));
    }

    void restart(ui::GameConfig config) {
        shardData.clear();
        gridMap.clear();
        auto totalGrids = config.width * config.height;

        // reset existing grid data
        for (int x = 0; x < config.width; x++) {
            for (int y = 0; y < config.height; y++) {
                shardData.push_back(ui::GridData{.x = x, .y = y, .status = ui::GridStatus::Closed});
                gridMap[{x, y}] = GridRef{.isMine = false, .isFlag = false, .status = &shardData.back().status};
            }
        }

        //
    }

    void open(int x, int y) { spdlog::info("GameView::clicked ({}, {})", x, y); }

    void flag(int x, int y) { spdlog::info("GameView::flagged ({}, {})", x, y); }
};