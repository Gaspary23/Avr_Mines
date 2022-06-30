/**
 * Nokia 5110 LCD screen writing utilities
 * for the AVR Mines game.
 */

#ifndef MINES_WRITING
#define MINES_WRITING

#include <stdint.h>

#include "board.h"

/**
 * Write the board to the screen.
 *
 * @sel_x: horizontal position of the selected field
 * @sel_y: vertical position of the selected field
 */
void write_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t sel_x, uint8_t sel_y,
	State game_state
);

/**
 * Write the amount of flags placed compared to
 * the number of mines in the board to the screen.
 *
 * @x: horizontal position
 * @y: vertical position
 */
void write_flag_count(
	uint8_t x, uint8_t y,
	uint8_t flags_placed, uint8_t mine_amount
);

/**
 * Write the timer in MM:SS format to the screen.
 *
 * @x: horizontal position
 * @y: vertical position
 */
void write_timer(
	uint8_t x, uint8_t y, uint8_t min, uint8_t sec
);

/**
 * Write the victory message to the screen.
 *
 * @x: horizontal position
 * @y: vertical position
 */
void write_victory(uint8_t x, uint8_t y);

/**
 * Write the defeat message to the screen.
 *
 * @x: horizontal position
 * @y: vertical position
 */
void write_defeat(uint8_t x, uint8_t y);

/**
 * Write the start menu to the screen.
 */
void write_menu();

#endif
