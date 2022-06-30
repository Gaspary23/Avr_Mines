/**
 * Board manipulation utilities
 * for the AVR Mines game.
 */

#ifndef MINES_BOARD
#define MINES_BOARD

#include <stdint.h>

/**
 * Represent a field in the board.
 */
typedef struct field {
	uint8_t mine;
	uint8_t flagged;
	uint8_t revealed;
	// Number of neighbouring mines.
	uint8_t num_mines;
} Field;

/**
 * Represent a game state.
 */
typedef enum state {
	// Initial state. represents the start menu.
	MENU,
	// Gameplay before the first field is revealed.
	START,
	// The rest of gameplay until victory or defeat.
	PLAYING,
	DEFEAT,
	VICTORY
} State;

/**
 * Resets the board to its initial state.
 * Mines are then regenerated.
 *
 * @mine_amount: the amount of mines to generate
 */
void reset_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t mine_amount
);

/**
 * Reveal the whole board.
 */
void reveal_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
);

/**
 * Randomly distribute mines across the board.
 *
 * The last field is always skipped in case the
 * first field revealed turns out to be a mine.
 * In that case, it may safely be moved to that corner.
 */
void generate_mines(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t amount
);

/**
 * Reveal a field and its neighbours, unless it has neigbouring mines.
 * In that case, reveal the origin field only.
 * It is assumed to be unrevealed and not a mine.
 *
 * @fields_revealed: the number of fields revealed during the
 *	execution of this function will be returned in this pointer
 * @flags_removed: the number of flags removed during the
 *	execution of this function will be returned in this pointer.
 *	A flag is removed when its field is revealed
 * @row_orig: the origin row of this section
 * @col_orig: the origin column of this section
 */
void reveal_section(
	uint8_t *fields_revealed, uint8_t *flags_removed,
	uint8_t row_orig, uint8_t col_orig,
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
);

/**
 * Move a mine from one field to another,
 * updating the number of neighbouring mines accordingly.
 * The movement will fail if the origin field is not a mine
 * or if the destination already contains one.
 *
 * @return: 0 on a failure, 1 on a success.
 */
int move_mine(
	uint8_t row_orig, uint8_t col_orig, uint8_t row_dest, uint8_t col_dest,
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
);

/**
 * Increment the number of neighbouring mines on the coordinates given.
 * This may be used when adding or removing a mine.
 */
void increment_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t row_center, uint8_t col_center, int8_t increment
);

/**
 * Move the cursor alongside the x or y axis,
 * wrapping from one screen border to the other if necessary.
 *
 * @limit: the boundary for movement,
 *	such as the width or height of the board
 *
 * @return: the new position on the given axis
 */
int move_wrapping(uint8_t sel, int8_t amount, uint8_t limit);

#endif
