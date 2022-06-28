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

#endif
