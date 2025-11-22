#include "entry.h"
#include "state_accessor.hpp"

int main(int argc, char** argv) {
    auto ui = ui::Entry::create();
    StateAccessor<ui::MainState> mainState(ui);

    mainState->on_set([&](int value) { mainState->set_number(value); });

    ui->run();
    return 0;
}