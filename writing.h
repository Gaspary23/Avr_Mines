#ifndef MINES_WRITING
#define MINES_WRITING

#include <stdint.h>

#include "board.h"

void write_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t sel_x, uint8_t sel_y,
	State game_state
);

void write_flag_count(
	uint8_t x, uint8_t y,
	uint8_t flags_placed, uint8_t max_flags
);

void write_timer(
	uint8_t x, uint8_t y, uint8_t min, uint8_t sec
);

void write_victory(uint8_t x, uint8_t y);
void write_defeat(uint8_t x, uint8_t y);
void write_menu();

#endif
