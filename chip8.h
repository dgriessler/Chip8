#ifndef CHIP_8_CHIP8_H
#define CHIP_8_CHIP8_H

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define CHIP8_REGISTERS_SIZE 16
#define CHIP8_STACK_SIZE 16
#define CHIP8_PC_START 0x200
#define CHIP8_MEM_SIZE 4096
#define CHIP8_GRAPHICS_WIDTH 64
#define CHIP8_GRAPHICS_HEIGHT 32
#define CHIP8_GRAPHICS_SIZE CHIP8_GRAPHICS_WIDTH * CHIP8_GRAPHICS_HEIGHT
#define CHIP8_KEYS_SIZE 16
#define CHIP8_REGISTER_CARRY 0xF
#define CHIP8_SPRITE_WIDTH 8
#define CHIP8_SCALED_PIXEL_SIZE 8
#define CHIP8_ALLEGRO_TIMER_SPEED_SECS 1.0 / 360.0
#define CHIP8_CYCLES_PER_TIMER_UPDATE 10

#define CHIP8_FONTSET_HEIGHT 16
#define CHIP8_FONTSET_WIDTH 5
#define CHIP8_FONTSET_SIZE CHIP8_FONTSET_WIDTH * CHIP8_FONTSET_HEIGHT

enum chip8_decodeState{ Chip8_Decode_State_Invalid, Chip8_Decode_State_Blocking, Chip8_Decode_State_Success };

typedef struct {
    uint8_t *V;           // Registers V0-VF
    uint16_t I;           // Index register
    uint16_t SP;          // Stack pointer
    uint16_t *stack;      // Stack for call stacks
    uint16_t PC;          // Program counter
    uint8_t delay;        // Delay timer
    uint8_t sound;        // Sound timer
    uint8_t *memory;      // Memory of system
    uint8_t *gfx;         // Graphics - each index is a pixel
    uint8_t *keys;        // Input keys
    bool drawFlag;        // Whether the screen needs to be drawn
    bool isGameLoaded;    // Whether there is a game loaded to chip8_run
    FILE* log;            // Log file for debugging
    int cycle;            // Cycle number - should stay between 1 and 10 inclusive
} chip8State_t;

/**
 * Initializes and returns a chip 8 state struct
 * @return A pointer to the created chip8State_t struct
 */
chip8State_t* chip8_init(void);

/**
 * Deallocates and frees a chip 8 state struct
 * It also nulls the pointer to the object during deletion
 * @param state A pointer to the pointer to be freed of type chip8State_t**
 */
void chip8_del(chip8State_t** state);

/**
 * Decodes the given opcode which is of the form 0x0NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x0000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x1NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x1000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x2NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x2000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x3NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x3000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x4NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x4000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x5NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x5000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x6NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x6000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x7NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x7000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x8NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x8000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0x9NNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0x9000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xANNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xA000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xBNNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xB000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xCNNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xC000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xDNNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xD000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xENNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xE000(chip8State_t* state, uint16_t opcode);

/**
 * Decodes the given opcode which is of the form 0xFNNN
 * @param state A pointer to the state for chip 8
 * @param opcode The opcode to be decoded
 * @return Chip8_Decode_State_Invalid if the opcode is invalid,
 * Chip8_Decode_State_Blocking if the instruction is blocking,
 * or Chip8_Decode_State_Success for success
 */
enum chip8_decodeState chip8_decode0xF000(chip8State_t* state, uint16_t opcode);

/**
 * Emulate a cycle of the chip 8 machine
 * @param state A pointer to the state for chip 8
 * @param soundEffect The sample to be played if the sound timer is counting down
 * @return If the emulation cycle was successful
 */
bool chip8_emulateCycle(chip8State_t* state, ALLEGRO_SAMPLE *soundEffect);

/**
 * Load a rom into the chip 8 machine
 * @param state A pointer to the state for chip 8
 * @param filePath The file path to the file to be loaded
 * @return If the chip 8 machine was able to load the file specified
 */
bool chip8_loadGame(chip8State_t* state, const char* filePath);

/**
 * Runs the chip 8 machine. Returns early if no rom is loaded in the chip 8 machine.
 * @param state A pointer to the state for chip 8
 */
void chip8_run(chip8State_t* state);

/**
 * Uses allegro to run the chip 8 machine drawing the output and emulating the machine
 * @param state A pointer to the state for chip 8
 */
void chip8_draw(chip8State_t* state);

/**
 * Processes the key press and sets the corresponding key in the chip 8 machine to the given value
 * @param state A pointer to the state for chip 8
 * @param key The character for the key that was pressed
 * @param value The value to assign to the corresponding key in the chip 8 machine
 */
void chip8_processKey(chip8State_t* state, int key, int value);

#endif //CHIP_8_CHIP8_H
