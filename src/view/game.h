#include "entry.h"
#include "logic/singleton.hpp"
#include "logic/state_accessor.hpp"
#include "setting.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <map>
#include <memory>
#include <slint.h>
#include <slint_enums.h>
#include <slint_models.h>
#include <slint_sharedvector.h>
#include <slint_timer.h>
#include <unordered_set>
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
    bool gameRunning = true;

  public:
    GameView(slint::ComponentHandle<UiEntry> uiEntry)
        : StateAccessor(uiEntry), timeCounter(new slint::Timer()),
          shardData(std::make_shared<slint::VectorModel<ui::GridData>>()) {
        auto self = *this;
        self->on_clicked([this](int x, int y) {
            spdlog::info("clicked {}, {}", x, y);
            open(x, y);
            tryWin();
        });
        self->on_pointer_event([this](int x, int y, slint::private_api::PointerEvent event) {
            if (event.button == slint::private_api::PointerEventButton::Right &&
                event.kind == slint::private_api::PointerEventKind::Down) {
                spdlog::info("toggle-flag {}, {}", x, y);
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

    void stopTimeCounter(bool reset = true) {
        timeCounter->stop();
        auto self = *this;
        if (reset) {
            self->set_time_in_second(0);
        }
    }

    void restart() {
        SettingView setting(this->uiEntry);
        auto config = setting.getGameConfig();
        auto self = *this;
        stopTimeCounter();
        shardData->clear();
        gridMap.clear();
        gameRunning = true;
        auto totalGrids = config.width * config.height;
        self->set_grid_height(config.height);
        self->set_grid_width(config.width);
        self->set_flag_counter(config.mine_count);
        self->set_face_status(ui::FaceButtonStatus::Smile);
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

    void open(int x, int y, bool isOpenAround = true, bool isOpenConnectBlank = true) {
        // spdlog::info("GameView::clicked ({}, {})", x, y);
        if (!gameRunning) {
            return;
        }
        auto& thisGridRef = gridMap[{x, y}];
        if (thisGridRef.isFlag) {
            return;
        }
        if (thisGridRef.isOpened) {
            if (isOpenAround) {
                openAround(x, y);
            }
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
            if (isOpenConnectBlank) {
                openConnectBlank(x, y);
            }
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
    }

    // open around grids if flag count matches mine count and current grid is not empty (mineCount=0)
    void openAround(int x, int y) {
        spdlog::info("GameView::openAround ({}, {})", x, y);
        auto& thisGridRef = gridMap[{x, y}];
        auto flagCount = countAround(thisGridRef, [](const GridRef g) { return g.isFlag; });
        auto mineCount = countAround(thisGridRef, [](const GridRef g) { return g.isMine; });

        if (mineCount == 0) {
            return;
        }
        if (flagCount != mineCount) {
            return;
        }
        // open around grids if flag count matches mine count
        countAround(x, y, [this](const GridRef g) -> bool {
            if (!g.isOpened && !g.isFlag) {
                auto lo = getLocation(g);
                open(lo.first, lo.second);
            }
            return false;
        });
    }

    // open connected blank grids recursively
    void openConnectBlank(int x, int y) {
        spdlog::info("GameView::openConnectBlank ({}, {})", x, y);
        std::vector<std::pair<int, int>> visited({{x, y}});
        std::vector<std::pair<int, int>> pendingSearch({{x, y}});
        int iterateIndex = 0;

        // recursively get empty grids to be opened in visited
        do {
            auto current = pendingSearch[iterateIndex];
            countAround(current.first, current.second, [this, &pendingSearch, &visited](const GridRef g) -> bool {
                auto lo = getLocation(g);
                if (std::ranges::contains(visited, lo)) {
                    return false;
                }
                visited.push_back(lo);
                auto mineCount = countAround(g, [](const GridRef gr) { return gr.isMine; });
                if (mineCount > 0) {
                    return false;
                }
                pendingSearch.push_back(lo);

                return false;
            });
            iterateIndex++;
        } while (iterateIndex < pendingSearch.size());

        // open all pending grids
        for (auto [x, y] : visited) {
            if (!gridMap[{x, y}].isOpened) {
                open(x, y, false, false);
            }
        }
    }

    void flag(int x, int y) {
        if (!gameRunning) {
            return;
        }
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

    void tryWin() {
        if (!gameRunning) {
            return;
        }
        for (auto& [loc, gridRef] : gridMap) {
            if (!gridRef.isMine && !gridRef.isOpened) {
                return;
            }
        }
        spdlog::info("Win!");
        gameRunning = false;
        auto self = *this;
        auto time = self->get_time_in_second();
        // show win
        stopTimeCounter(false);
    }

    void gameover() {
        spdlog::info("Game Over!");
        gameRunning = false;
        auto self = *this;
        auto time = self->get_time_in_second();
        self->set_face_status(ui::FaceButtonStatus::Lose);
        // show game over
        stopTimeCounter(false);
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