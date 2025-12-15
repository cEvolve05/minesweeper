#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "setting.h"
#include "spdlog/spdlog.h"
#include <map>
#include <memory>
#include <slint.h>
#include <slint_enums.h>
#include <slint_models.h>
#include <slint_sharedvector.h>
#include <slint_timer.h>
#include <utility>
#include <vector>

struct GridRef {
    bool isOpened = false;
    bool isMine = false;
    bool isFlag = false;
    int statusIndex;
};

class GameView : StateAccessor<ui::GameState> {
    std::map<std::pair<int, int>, GridRef> gridMap;
    std::shared_ptr<slint::VectorModel<ui::GridData>> shardData;
    std::pair<int, int> gridSize; // x, y
    slint::Timer* timeCounter;

  public:
    GameView(slint::ComponentHandle<UiEntry> uiEntry)
        : StateAccessor(uiEntry), timeCounter(new slint::Timer()),
          shardData(std::make_shared<slint::VectorModel<ui::GridData>>()) {
        auto self = *this;
        self->on_clicked([this](int x, int y) { open(x, y); });
        self->on_pointer_event([this](int x, int y, slint::private_api::PointerEvent event) {
            if (event.button == slint::private_api::PointerEventButton::Right &&
                event.kind == slint::private_api::PointerEventKind::Down) {
                flag(x, y);
            }
        });
        self->on_face_clicked([this, &uiEntry] { restart(); });
        self->set_grid_data(shardData);
        slint::invoke_from_event_loop([this] { restart(); });
    }

    void startTimeCounter() {
        timeCounter->start(slint::TimerMode::Repeated, std::chrono::milliseconds(1000), [this]() {
            auto self = *this;
            self->set_time_in_second(self->get_time_in_second() + 1);
        });
    }

    void stopTimeCounter() {
        timeCounter->stop();
        auto self = *this;
        self->set_time_in_second(0);
    }

    void restart() {
        SettingView setting(this->uiEntry);
        auto config = setting.getGameConfig();
        auto self = *this;
        stopTimeCounter();
        shardData->clear();
        gridMap.clear();
        auto totalGrids = config.width * config.height;
        self->set_grid_height(config.height);
        self->set_grid_width(config.width);
        self->set_flag_counter(config.mine_count);
        spdlog::info("GameView::restart width: {}, height: {}, mines: {}", config.width, config.height,
                     config.mine_count);

        // reset existing grid data and ref
        for (int x = 0; x < config.width; x++) {
            for (int y = 0; y < config.height; y++) {
                shardData->push_back(ui::GridData{.x = x, .y = y, .status = ui::GridStatus::Closed});
                gridMap[{x, y}] = GridRef{.statusIndex = static_cast<int>(shardData->row_count() - 1)};
            }
        }

        // generate mines
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        auto mineCount = config.mine_count;
        while (mineCount > 0) {
            int x = std::rand() % config.width;
            int y = std::rand() % config.height;
            auto& gridRef = gridMap[{x, y}];
            if (!gridRef.isMine) {
                gridRef.isMine = true;
                mineCount--;
            }
        }

        startTimeCounter();
    }

    void open(int x, int y) {
        auto& thisGridRef = gridMap[{x, y}];
        spdlog::info("GameView::clicked ({}, {})", x, y);
        if (!thisGridRef.isOpened) {
            if (thisGridRef.isFlag) {
                return;
            }
            thisGridRef.isOpened = true;
            if (thisGridRef.isMine) {
                setGridStatus(x, y, ui::GridStatus::OpenedMineRed);
                // reveal all mines
                for (auto& [loc, gridRef] : gridMap) {
                    if (gridRef.isMine && getGridStatus(gridRef) != ui::GridStatus::OpenedMineRed) {
                        setGridStatus(loc.first, loc.second, ui::GridStatus::OpenedMine);
                        gridRef.isOpened = true;
                    }
                }
                gameover();
                return;
            }
            auto mineCount = countAround(thisGridRef, [](const GridRef g) { return g.isMine; });
            switch (mineCount) {
            case 0:
                setGridStatus(x, y, ui::GridStatus::Opened);
                openAround(x, y);
                break;
            case 1:
                setGridStatus(x, y, ui::GridStatus::Number1);
                break;
            case 2:
                setGridStatus(x, y, ui::GridStatus::Number2);
                break;
            case 3:
                setGridStatus(x, y, ui::GridStatus::Number3);
                break;
            case 4:
                setGridStatus(x, y, ui::GridStatus::Number4);
                break;
            case 5:
                setGridStatus(x, y, ui::GridStatus::Number5);
                break;
            case 6:
                setGridStatus(x, y, ui::GridStatus::Number6);
                break;
            case 7:
                setGridStatus(x, y, ui::GridStatus::Number7);
                break;
            case 8:
                setGridStatus(x, y, ui::GridStatus::Number8);
                break;
            default:
                break;
            }
        } else {
            if (thisGridRef.isMine) {
                return;
            }
            auto flagCount = countAround(thisGridRef, [](const GridRef g) { return g.isFlag; });
            auto mineCount = countAround(thisGridRef, [](const GridRef g) { return g.isMine; });

            if (mineCount == 0) {
                countAround(thisGridRef, [this](const GridRef g) {
                    if (!g.isOpened && !g.isFlag) {
                        auto lo = getLocation(g);
                        open(lo.first, lo.second);
                    }
                    return false;
                });
            }
            if (flagCount == mineCount) {
                // open around grids if flag count matches mine count
                countAround(x, y, [this](const GridRef g) -> bool {
                    if (!g.isOpened && !g.isFlag) {
                        auto lo = getLocation(g);
                        open(lo.first, lo.second);
                    }
                    return false;
                });
                return;
            }
        }
    }

    void openAround(int x, int y) {
        spdlog::info("GameView::openAround ({}, {})", x, y);
        open(x, y);
        countAround(gridMap[{x, y}], [this](GridRef g) {
            if (getGridStatus(g) == ui::GridStatus::Opened) {
                auto lo = getLocation(g);
                open(lo.first, lo.second);
            }
            return false;
        });
    }

    void flag(int x, int y) {
        spdlog::info("GameView::flagged ({}, {})", x, y);
        auto& thisGridRef = gridMap[{x, y}];
        if (!thisGridRef.isOpened) {
            if (thisGridRef.isFlag) {
                thisGridRef.isFlag = false;
                setGridStatus(x, y, ui::GridStatus::Closed);
                auto self = *this;
                self->set_flag_counter(self->get_flag_counter() + 1);
            } else {
                thisGridRef.isFlag = true;
                setGridStatus(x, y, ui::GridStatus::ClosedFlag);
                auto self = *this;
                self->set_flag_counter(self->get_flag_counter() - 1);
            }
        }
    }

    void gameover() {
        spdlog::info("Game Over!");
        auto self = *this;
        auto time = self->get_time_in_second();
        // show game over
        stopTimeCounter();
    }

  private:
    void setGridStatus(int x, int y, ui::GridStatus status) { setGridStatus(gridMap[{x, y}], status); }
    void setGridStatus(GridRef gridRef, ui::GridStatus status) {
        auto old = *shardData->row_data(gridRef.statusIndex);
        old.status = status;
        shardData->set_row_data(gridRef.statusIndex, old);
    }
    ui::GridStatus getGridStatus(int x, int y) { return (*shardData->row_data(gridMap[{x, y}].statusIndex)).status; }
    ui::GridStatus getGridStatus(GridRef gridRef) { return (*shardData->row_data(gridRef.statusIndex)).status; }
    int countAround(int x, int y, std::function<bool(const GridRef)>&& predicate) {
        int count = 0;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                if (gridMap.contains({x + dx, y + dy})) {
                    if (predicate(gridMap[{x + dx, y + dy}])) {
                        count++;
                    }
                }
            }
        }
        return count;
    }
    int countAround(GridRef gridRef, std::function<bool(const GridRef)>&& predicate) {
        auto data = *shardData->row_data(gridRef.statusIndex);
        return countAround(data.x, data.y, std::move(predicate));
    }
    std::pair<int, int> getLocation(GridRef gridRef) {
        auto data = *shardData->row_data(gridRef.statusIndex);
        return {data.x, data.y};
    }
};