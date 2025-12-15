#include "entry.h"
#include "view/game.h"

int main(int argc, char** argv) {
    auto ui = ui::Entry::create();

    GameView gameView(ui);

    ui->run();
    return 0;
}