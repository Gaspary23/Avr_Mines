#ifndef MINES_BOARD
#define MINES_BOARD

#include <stdint.h>

/*
 * Represent a field in the board.
 */
typedef struct field
{
	uint8_t mine;
	uint8_t flagged;
	uint8_t revealed;
	uint8_t num_mines;
}
Field;

/*
 * Represent a game state.
 */
typedef enum state
{
	// Initial state. represents the start menu.
	MENU,
	// Represents gameplay before the first field is revealed.
	START,
	// Represents the rest of gameplay until victory or defeat.
	PLAYING,
	DEFEAT,
	VICTORY
}
State;

void reset_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t mine_amount
);

void reveal_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
);

void generate_mines(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t amount
);

void increment_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t i, uint8_t j
);

int move_wrapping(uint8_t sel, int amount, uint8_t limit);

#endif
