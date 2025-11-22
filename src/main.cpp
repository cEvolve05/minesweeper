#include "main.h"
#include "base.h"
#include "state_accessor.hpp"

int main(int argc, char** argv) {
    auto ui = ui::Main::create();
    StateAccessor<ui::MainState> mainState(ui);

    mainState->on_set([&](int value) { mainState->set_number(value); });

    ui->run();
    return 0;
}