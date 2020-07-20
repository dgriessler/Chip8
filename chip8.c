#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <math.h>
#include <stdlib.h>
#include "chip8.h"

uint8_t chip8_fontset[CHIP8_FONTSET_SIZE] =
        {
                /*
                 * Example: 0 = 0xF0 0x90 0x90 0x90 0xF0
                 * 1111 0000
                 * 1001 0000
                 * 1001 0000
                 * 1001 0000
                 * 1111 0000
                 * ****
                 * *  *
                 * *  *
                 * *  *
                 * ****
                 */
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

chip8State_t* chip8_init(void) {
    chip8State_t* state = calloc(sizeof(chip8State_t), 1);

    // clear registers
    state->V = calloc(CHIP8_REGISTERS_SIZE, sizeof(uint8_t));
    // reset index register
    state->I = 0;
    // reset stack pointer
    state->SP = 0;
    // reset stack
    state->stack = calloc(CHIP8_STACK_SIZE, sizeof(uint16_t));
    // program counter starts at 0x200
    state->PC = CHIP8_PC_START;
    // reset timers
    state->delay = 0;
    state->sound = 0;
    // clear memory
    state->memory = calloc(CHIP8_MEM_SIZE, sizeof(uint8_t));
    for (int i = 0; i < CHIP8_FONTSET_SIZE; ++i) {
        state->memory[i] = chip8_fontset[i];
    }
    // clear display
    state->gfx = calloc(CHIP8_GRAPHICS_SIZE, sizeof(uint8_t));

    state->keys = calloc(CHIP8_KEYS_SIZE, sizeof(uint8_t));
    state->drawFlag = false;
    state->isGameLoaded = false;
    state->log = fopen("..\\logs\\log.txt", "w");
    state->cycle = 1;

    return state;
}

void chip8_del(chip8State_t** state) {
    if (state != NULL) {
        free((*state)->V);
        (*state)->V = NULL;
        free((*state)->stack);
        (*state)->stack = NULL;
        free((*state)->memory);
        (*state)->memory = NULL;
        free((*state)->gfx);
        (*state)->gfx = NULL;
        free((*state)->keys);
        (*state)->keys = NULL;
        fclose((*state)->log);
        (*state)->log = NULL;
        free(*state);
        *state = NULL;
    }
}

enum chip8_decodeState chip8_decode0x0000(chip8State_t* state, uint16_t opcode) {
    switch(opcode & 0x00FFu) {
        case 0x00E0:
            // 00E0: clears the screen
            fprintf(state->log, "00E0: clears the screen\n");
            for (int i = 0; i < CHIP8_GRAPHICS_SIZE; i++) {
                state->gfx[i] = 0;
            }
            state->drawFlag = true;
            state->PC += 2;
            break;
        case 0x00EE:
            // 00EE: Returns from a subroutine
            fprintf(state->log, "00EE: Returns from a subroutine\n");
            if (state->SP == 0) {
                fprintf(stderr, "Stack is empty!\n");
                return Chip8_Decode_State_Invalid;
            } else {
                state->SP--;
                state->PC = state->stack[state->SP] + 2;
            }
            break;
        default: {
            // 0NNN: Calls machine code routine at address NNN. Not necessary for most ROMs.
            fprintf(state->log, "0NNN: Calls machine code routine at address NNN. Not necessary for most ROMs.\n");
            state->PC = opcode & 0x0FFFu;
            break;
        }
    }
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x1000(chip8State_t* state, uint16_t opcode) {
    // 1NNN: Jumps to address NNN
    fprintf(state->log, "1NNN: Jumps to address NNN\n");
    state->PC = opcode & 0xFFFu;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x2000(chip8State_t* state, uint16_t opcode) {
    // 2NNN: Calls subroutine at NNN
    fprintf(state->log, "2NNN: Calls subroutine at NNN\n");
    if (state->SP == CHIP8_STACK_SIZE) {
        fprintf(stderr, "Stack is full!\n");
        return Chip8_Decode_State_Invalid;
    } else {
        state->stack[state->SP] = state->PC;
        state->SP++;
        state->PC = opcode & 0x0FFFu;
    }
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x3000(chip8State_t* state, uint16_t opcode) {
    // 3XNN: Skips the next instruction if VX equals NN
    fprintf(state->log, "3XNN: Skips the next instruction if VX equals NN\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = opcode & 0x00FFu;
    if (state->V[X] == NN) {
        state->PC += 2;
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x4000(chip8State_t* state, uint16_t opcode) {
    // 4XNN: Skips the next instruction if VX doesn't equal NN
    fprintf(state->log, "4XNN: Skips the next instruction if VX doesn't equal NN\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = opcode & 0x00FFu;
    if (state->V[X] != NN) {
        state->PC += 2;
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x5000(chip8State_t* state, uint16_t opcode) {
    // 5XY0: Skips the next instruction if VX equals VY
    fprintf(state->log, "5XY0: Skips the next instruction if VX equals VY\n");
    if ((opcode & 0x000Fu) > 0) {
        fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
        return Chip8_Decode_State_Invalid;
    } else {
        uint8_t X = (opcode & 0x0F00u) >> 8u;
        uint8_t Y = (opcode & 0x00F0u) >> 4u;
        if (state->V[X] == state->V[Y]) {
            state->PC += 2;
        }
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x6000(chip8State_t* state, uint16_t opcode) {
    // 6XNN: Sets VX to NN
    fprintf(state->log, "6XNN: Sets VX to NN\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = opcode & 0x00FFu;
    state->V[X] = NN;
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x7000(chip8State_t* state, uint16_t opcode) {
    // 7XNN: Adds NN to VX. (Carry flag is not changed)
    fprintf(state->log, "7XNN: Adds NN to VX. (Carry flag is not changed)\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = opcode & 0x00FFu;
    state->V[X] += NN;
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x8000(chip8State_t* state, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t Y = (opcode & 0x00F0u) >> 4u;
    switch (opcode & 0x000Fu) {
        case 0x0000:
            // 8XY0: Sets VX to the value of VY
            fprintf(state->log, "8XY0: Sets VX to the value of VY\n");
            state->V[X] = state->V[Y];
            break;
        case 0x0001:
            // 8XY1: Sets VX to VX or VY (Bitwise OR operation)
            fprintf(state->log, "8XY1: Sets VX to VX or VY (Bitwise OR operation)\n");
            state->V[X] = state->V[X] | state->V[Y];
            break;
        case 0x0002:
            // 8XY2: Sets VX to VX and VY (Bitwise AND operation)
            fprintf(state->log, "8XY2: Sets VX to VX and VY (Bitwise AND operation)\n");
            state->V[X] = state->V[X] & state->V[Y];
            break;
        case 0x0003:
            // 8XY3: Sets VX to VX xor VY
            fprintf(state->log, "8XY3: Sets VX to VX xor VY\n");
            state->V[X] = state->V[X] ^ state->V[Y];
            break;
        case 0x0004:
            // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
            fprintf(state->log, "8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't\n");
            if (state->V[Y] > (0xFF - state->V[X])) {
                state->V[CHIP8_REGISTER_CARRY] = 1; // carry
            } else {
                state->V[CHIP8_REGISTER_CARRY] = 0;
            }
            state->V[X] += state->V[Y];
            break;
        case 0x0005:
            // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            fprintf(state->log, "8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't\n");
            if (state->V[X] < state->V[Y]) {
                state->V[CHIP8_REGISTER_CARRY] = 0; // borrow
            } else {
                state->V[CHIP8_REGISTER_CARRY] = 1;
            }
            state->V[X] -= state->V[Y];
            break;
        case 0x0006:
            // 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1
            fprintf(state->log, "Stores the least significant bit of VX in VF and then shifts VX to the right by 1\n");
            state->V[X] >>= 1u;
            break;
        case 0x0007:
            // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            fprintf(state->log, "8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't\n");
            if (state->V[Y] < state->V[X]) {
                state->V[CHIP8_REGISTER_CARRY] = 0;
            } else {
                state->V[CHIP8_REGISTER_CARRY] = 1;
            }
            state->V[X] = state->V[Y] - state->V[X];
            break;
        case 0x000E:
            // 8XYE: Stores the most significant bit of VX in VF and shifts VX to the left by 1.
            fprintf(state->log, "8XYE: Stores the most significant bit of VX in VF and shifts VX to the left by 1.\n");
            state->V[X] <<= 1u;
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
            return Chip8_Decode_State_Invalid;
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0x9000(chip8State_t* state, uint16_t opcode) {
    // 9XY0: Skips the next instruction if VX doesn't equal VY
    fprintf(state->log, "9XY0: Skips the next instruction if VX doesn't equal VY\n");
    if (opcode & 0x000Fu) {
        fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
        return Chip8_Decode_State_Invalid;
    } else {
        uint8_t X = (opcode & 0x0F00u) >> 8u;
        uint8_t Y = (opcode & 0x00F0u) >> 4u;
        if (state->V[X] != state->V[Y]) {
            state->PC += 2;
        }
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xA000(chip8State_t* state, uint16_t opcode) {
    // ANNN: Sets I to the address NNN
    fprintf(state->log, "ANNN: Sets I to the address NNN\n");
    state->I = opcode & 0x0FFFu;
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xB000(chip8State_t* state, uint16_t opcode) {
    // BNNN: jumps to the address NNN plus V0
    fprintf(state->log, "BNNN: jumps to the address NNN plus V0\n");
    state->PC = (opcode & 0x0FFFu) + state->V[0];
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xC000(chip8State_t* state, uint16_t opcode) {
    // CXNN: Sets VX to the result of a bitwise AND operation on a random number (Typically: 0 to 255) and NN
    fprintf(state->log, "CXNN: Sets VX to the result of a bitwise AND operation on a random number (Typically: 0 to 255) and NN\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t NN = opcode & 0x00FFu;
    state->V[X] = rand() & NN;
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xD000(chip8State_t* state, uint16_t opcode) {
    // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
    // Each row of 8 pixels is read as bit-coded starting from memory location I
    // I value doesn't change during execution of this instruction
    // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if not
    fprintf(state->log, "DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.\n");
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    uint8_t Y = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    state->V[CHIP8_REGISTER_CARRY] = 0;
    uint8_t pixel;
    uint16_t position;
    // go row by row
    for (int yline = 0; yline < height; yline++) {
        pixel = state->memory[state->I + yline];
        // width of sprites is fixed at 8
        for (unsigned int xline = 0; xline < CHIP8_SPRITE_WIDTH; xline++) {
            // check for collision
            // if the pixel is set and if the graphics position is set then there's a collision
            if ((pixel & (0x80u >> xline)) != 0) {
                position = state->V[X] + xline + ((state->V[Y] + yline) * CHIP8_GRAPHICS_WIDTH);
                if (state->gfx[position] == 1) {
                    state->V[CHIP8_REGISTER_CARRY] = 1;
                }
                state->gfx[position] ^= 1u;
            }
        }
    }
    state->drawFlag = true;
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xE000(chip8State_t* state, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    switch (opcode & 0x00FFu) {
        case 0x009E: {
            // EX9E: Skips the next instruction if the key stored in VX is pressed
            fprintf(state->log, "EX9E: Skips the next instruction if the key stored in VX is pressed\n");
            if (state->V[X] >= 0 && state->V[X] < CHIP8_KEYS_SIZE && state->keys[state->V[X]] != 0) {
                state->PC += 2;
            }
            break;
        }
        case 0x00A1: {
            // EXA1: Skips the next instruction if the key stored in VX isn't pressed
            fprintf(state->log, "EXA1: Skips the next instruction if the key stored in VX isn't pressed\n");
            if (state->V[X] >= 0 && state->V[X] < CHIP8_KEYS_SIZE && state->keys[state->V[X]] == 0) {
                state->PC += 2;
            }
            break;
        }
        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
            return Chip8_Decode_State_Invalid;
    }
    state->PC += 2;
    return Chip8_Decode_State_Success;
}

enum chip8_decodeState chip8_decode0xF000(chip8State_t* state, uint16_t opcode) {
    uint8_t X = (opcode & 0x0F00u) >> 8u;
    switch(opcode & 0x00FFu) {
        case 0x0007:
            // FX07: Sets VX to the value of the delay timer
            fprintf(state->log, "FX07: Sets VX to the value of the delay timer\n");
            state->V[X] = state->delay;
            state->PC += 2;
            break;
        case 0x000A: {
            // FX0A: A key press is awaited, and then stored in VX.
            // (Blocking operation. All instruction halted until next key press).
            fprintf(state->log, "FX0A: A key press is awaited, and then stored in VX.\n");
            bool keyPressed = false;
            for (int i = 0; i < CHIP8_KEYS_SIZE; i++) {
                if (state->keys[i] != 0) {
                    state->V[X] = i;
                    keyPressed = true;
                }
            }
            // TODO So we don't increment the timers either?
            if (!keyPressed) {
                return Chip8_Decode_State_Blocking;
            }
            state->PC += 2;
            break;
        }
        case 0x0015:
            // FX15: Sets the delay timer to VX
            fprintf(state->log, "FX15: Sets the delay timer to VX\n");
            state->delay = state->V[X];
            state->PC += 2;
            break;
        case 0x0018:
            // FX18: Sets the sound timer to VX
            fprintf(state->log, "FX18: Sets the sound timer to VX\n");
            state->sound = state->V[X];
            state->PC += 2;
            break;
        case 0x001E:
            // FX1E: Adds VX to I. VF is not affected
            fprintf(state->log, "FX1E: Adds VX to I. VF is not affected\n");
            state->I += state->V[X];
            state->PC += 2;
            break;
        case 0x0029: {
            // FX29: Sets I to the location of the sprite for the character in VX.
            // Characters 0-F are represented by the font
            fprintf(state->log, "FX29: Sets I to the location of the sprite for the character in VX.\n");
            uint16_t location = state->V[X] * CHIP8_FONTSET_WIDTH;
            if (location > CHIP8_FONTSET_SIZE) {
                fprintf(stderr, "Accessing font out of bounds: %d\n", location);
                return Chip8_Decode_State_Invalid;
            } else {
                state->I = location;
            }
            state->PC += 2;
            break;
        }
        case 0x0033:
            // FX33: Stores the binary-coded decimal representation of VX, with the most significant of three
            // digits at the address in I, the middle digit at I plus 1, and the least significant digit at
            // I plus 2.
            // In other words, take the decimal representation of VX, place the hundreds digit in memory at
            // location in I, the tens digit at location I+1, and the ones digit at location I+2.
            fprintf(state->log, "FX33: Stores the binary-coded decimal representation of VX\n");
            state->memory[state->I] = state->V[X] / 100;            // 123 => 1
            state->memory[state->I + 1] = (state->V[X] / 10) % 10;  // 123 => 12 => 2
            state->memory[state->I + 2] = (state->V[X] % 100) % 10; // 123 => 23 => 3
            state->PC += 2;
            break;
        case 0x0055:
            // FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is
            // increased by 1 for each value written, but I itself is left unmodified
            fprintf(state->log, "FX55: Stores V0 to VX (including VX) in memory starting at address I\n");
            for (int i = 0; i <= X; i++) {
                state->memory[state->I + i] = state->V[i];
            }
            // TODO Original interpreter, when the operation is done, I = I + X + 1, do I do this?
            // I += X + 1;
            state->PC += 2;
            break;
        case 0x0065:
            // FX65: Fills V0 to VX (including VX) with values from memory starting at address I. The offset
            // from I is increased by 1 for each value written, but I itself is left unmodified.
            fprintf(state->log, "FX65: Fills V0 to VX (including VX) with values from memory starting at address I\n");
            for (int i = 0; i <= X; i++) {
                state->V[i] = state->memory[state->I + i];
            }
            // TODO Original interpreter, when the operation is done, I = I + X + 1, do I do this?
            // I += X + 1;
            state->PC += 2;
            break;
        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
            return Chip8_Decode_State_Invalid;
    }
    return Chip8_Decode_State_Success;
}

bool chip8_emulateCycle(chip8State_t* state, ALLEGRO_SAMPLE *soundEffect) {
    if (!state->isGameLoaded) {
        fprintf(stderr, "No game is loaded!\n");
        return false;
    }
    // Fetch Opcode
    uint16_t opcode = (state->memory[state->PC] << 8u) | state->memory[state->PC + 1];

    enum chip8_decodeState (*decodedOp)(chip8State_t*, uint16_t) = NULL;

    fprintf(state->log, "%d\t0x%x: ", state->PC, opcode);
    // Decode Opcode
    switch(opcode & 0xF000u) {
        case 0x0000:
            decodedOp = &chip8_decode0x0000;
            break;
        case 0x1000:
            decodedOp = &chip8_decode0x1000;
            break;
        case 0x2000:
            decodedOp = &chip8_decode0x2000;
            break;
        case 0x3000: {
            decodedOp = &chip8_decode0x3000;
            break;
        }
        case 0x4000: {
            decodedOp = &chip8_decode0x4000;
            break;
        }
        case 0x5000: {
            decodedOp = &chip8_decode0x5000;
            break;
        }
        case 0x6000: {
            decodedOp = &chip8_decode0x6000;
            break;
        }
        case 0x7000: {
            decodedOp = &chip8_decode0x7000;
            break;
        }
        case 0x8000: {
            decodedOp = &chip8_decode0x8000;
            break;
        }
        case 0x9000: {
            decodedOp = &chip8_decode0x9000;
            break;
        }
        case 0xA000:
            decodedOp = &chip8_decode0xA000;
            break;
        case 0xB000:
            decodedOp = &chip8_decode0xB000;
            break;
        case 0xC000: {
            decodedOp = &chip8_decode0xC000;
            break;
        }
        case 0xD000: {
            decodedOp = &chip8_decode0xD000;
            break;
        }
        case 0xE000: {
            decodedOp = &chip8_decode0xE000;
            break;
        }
        case 0xF000: {
            decodedOp = &chip8_decode0xF000;
            break;
        }
        default:
            fprintf(stderr, "Unknown opcode: 0x%X\n", opcode);
            break;
    }

    if (decodedOp == NULL) {
        return false;
    }
    enum chip8_decodeState decodeState = (*decodedOp)(state, opcode);
    // If the decoded state is false, then there was an issue processing the opcode and we should quit
    if (decodeState == Chip8_Decode_State_Invalid) {
        return false;
    } else if (decodeState == Chip8_Decode_State_Blocking) {
        // otherwise, if the decoded state is blocking then we should quit early before updating the timers
        return true;
    } else if (decodeState != Chip8_Decode_State_Success) {
        // otherwise, the decoded state should be success
        // this shouldn't run
        fprintf(stderr, "Invalid Decode State detected: %d\n", decodeState);
        return false;
    }

    // Update timers
    if (state->cycle == CHIP8_CYCLES_PER_TIMER_UPDATE) {
        if (state->delay > 0) {
            state->delay--;
        }

        if (state->sound > 0) {
            al_play_sample(soundEffect, 1.0f, 0.0f, 1.0f, ALLEGRO_PLAYMODE_ONCE, 0);
            state->sound--;
        }
        state->cycle = 0;
    } else {
        state->cycle++;
    }
    return true;
}

bool chip8_loadGame(chip8State_t* state, const char* filePath) {
    FILE *fptr;
    fptr = fopen(filePath, "rb");
    if (fptr == NULL) {
        fprintf(stderr, "Failed to open file: %d\n", errno);
        return false;
    }

    fseek(fptr, 0, SEEK_END);
    long fileSize = ftell(fptr);
    rewind(fptr);

    if (fileSize == 0) {
        fprintf(stderr, "File size is 0!\n");
        return false;
    }

    char* buffer = (char*)malloc(sizeof(char) * fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer for ROM\n");
        return false;
    }

    size_t result = fread(buffer, sizeof(uint8_t), (size_t)fileSize, fptr);
    if (result != fileSize) {
        fprintf(stderr, "Failed to read ROM\n");
        return false;
    }

    if (CHIP8_MEM_SIZE >= fileSize) {
        for (int i = 0; i < fileSize; ++i) {
            state->memory[i + CHIP8_PC_START] = (uint8_t)buffer[i];
        }
        free(buffer);
        buffer = NULL;
    } else {
        fprintf(stderr, "File too large!\n");
        return false;
    }

    fclose(fptr);
    fptr = NULL;
    state->isGameLoaded = true;

    uint16_t opcode;
    for (int i = 0; i < fileSize; i += 2) {
        opcode = (state->memory[i + CHIP8_PC_START] << 8u) | state->memory[i + CHIP8_PC_START + 1];
        fprintf(state->log, "%d: \t0x%x\n", i + CHIP8_PC_START, opcode);
    }

    return true;
}

void chip8_run(chip8State_t* state) {
    if (!state->isGameLoaded) {
        fprintf(stderr, "No game is loaded!\n");
        return;
    }

    chip8_draw(state);
}

void chip8_draw(chip8State_t* state) {
    al_init();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();

    ALLEGRO_SAMPLE *soundEffect = al_load_sample("..\\sounds\\beep.wav");
    if (soundEffect == NULL) {
        fprintf(stderr, "Failed to load sample!\n");
    }
    al_reserve_samples(1);

    ALLEGRO_TIMER* timer = al_create_timer(CHIP8_ALLEGRO_TIMER_SPEED_SECS);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(CHIP8_GRAPHICS_WIDTH * CHIP8_SCALED_PIXEL_SIZE,
                                              CHIP8_GRAPHICS_HEIGHT * CHIP8_SCALED_PIXEL_SIZE);
    ALLEGRO_FONT* font = al_create_builtin_font();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    ALLEGRO_EVENT event;

    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            if (chip8_emulateCycle(state, soundEffect) == false) {
                break;
            }
        } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            chip8_processKey(state, *al_keycode_to_name(event.keyboard.keycode), 1);
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            chip8_processKey(state, *al_keycode_to_name(event.keyboard.keycode), 0);
        } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        if (state->drawFlag && al_is_event_queue_empty(queue))
        {
            // clear screen
            al_clear_to_color(al_map_rgb(0, 0, 0));

            int posX = 0;
            int x = 0;
            int y = 0;
            ALLEGRO_COLOR color;
            // draw each pixel
            for (int i = 0; i < CHIP8_GRAPHICS_SIZE; i++) {
                // pixels are either on (white) or off (black)
                if (state->gfx[i]) {
                    color = al_map_rgb(255, 255, 255);
                } else {
                    color = al_map_rgb(0, 0, 0);
                }
                // Scale each pixel so it's easier to see
                for (int lineX = 0; lineX < CHIP8_SCALED_PIXEL_SIZE; lineX++) {
                    for (int lineY = 0; lineY < CHIP8_SCALED_PIXEL_SIZE; lineY++) {
                        al_draw_pixel(x + lineX, y + lineY, color);
                    }
                }
                x += CHIP8_SCALED_PIXEL_SIZE;
                posX++;
                if (posX == CHIP8_GRAPHICS_WIDTH) {
                    x = 0;
                    y += CHIP8_SCALED_PIXEL_SIZE;
                    posX = 0;
                }
            }

            al_flip_display();
            state->drawFlag = false;
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_sample(soundEffect);
    al_destroy_event_queue(queue);
}

void chip8_processKey(chip8State_t* state, int key, int value) {
    /*
     * Keypad:
     * +-+-+-+-+  ===  +-+-+-+-+
     * |1|2|3|C|       |1|2|3|4|
     * |4|5|6|D|       |Q|W|E|R|
     * |7|8|9|E|       |A|S|D|F|
     * |A|0|B|F|       |Z|X|C|V|
     */
    int lower = key;
    if (isalpha(key)) {
        lower = tolower(key);
    }
    switch (lower) {
        case '1':
            state->keys[1] = value;
            break;
        case '2':
            state->keys[2] = value;
            break;
        case '3':
            state->keys[3] = value;
            break;
        case '4':
            state->keys[12] = value;
            break;
        case 'q':
            state->keys[4] = value;
            break;
        case 'w':
            state->keys[5] = value;
            break;
        case 'e':
            state->keys[6] = value;
            break;
        case 'r':
            state->keys[13] = value;
            break;
        case 'a':
            state->keys[7] = value;
            break;
        case 's':
            state->keys[8] = value;
            break;
        case 'd':
            state->keys[9] = value;
            break;
        case 'f':
            state->keys[14] = value;
            break;
        case 'z':
            state->keys[10] = value;
            break;
        case 'x':
            state->keys[0] = value;
            break;
        case 'c':
            state->keys[11] = value;
            break;
        case 'v':
            state->keys[15] = value;
            break;
        default:
            break;
    }
}
