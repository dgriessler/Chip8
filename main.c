#include "chip8.h"

int main() {
    chip8State_t* state = chip8_init();
    chip8_loadGame(state, "..\\roms\\Tic-Tac-Toe.ch8");
    chip8_run(state);
    chip8_del(&state);
    return 0;
}
