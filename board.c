#include <stdint.h>
#include <stdlib.h>

#include "board.h"

/*
 * Resets the board to its initial state.
 * Mines are then regenerated.
 */
void reset_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t mine_amount
) {
	for (int row = 0; row < board_height; row++) {
		for (int col = 0; col < board_width; col++) {
			board[row][col] = (Field) {0, 0, 0, 0};
		}
	}

	generate_mines(board_width, board_height, board, mine_amount);
}

/**
 * Reveal the whole board.
 */
void reveal_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
) {
	for (int row = 0; row < board_height; row++) {
		for (int col = 0; col < board_width; col++) {
			board[row][col].revealed = 1;
		}
	}
}

/*
 * Fill the board with mines, skipping the last field.
 *
 * If the first field the player reveals turns out to be a mine,
 * it will be moved there. This way, the game can never be lost
 * on the first button press.
 */
void generate_mines(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t amount
) {
	int mines_generated = 0;
	int fields_left = board_height * board_width - 1;

	for (int row = 0; row < board_height; row++) {
		for (int col = 0; col < board_width; col++) {
			float rand_res = (float) rand() / (float) RAND_MAX;

			if (fields_left * rand_res < amount - mines_generated) {
				board[row][col].mine = 1;
				mines_generated++;

				increment_neighbours(
					board_width, board_height, board, row, col
				);

				if (mines_generated == amount) {
					break;
				}
			}

			fields_left--;
		}

		if (mines_generated == amount) {
			break;
		}
	}
}

/*
 * Move a mine from one field to another.
 */
int move_mine(
	uint8_t row_orig, uint8_t col_orig, uint8_t row_dest, uint8_t col_dest,
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
) {
	Field *orig = &board[row_orig][col_orig];
	Field *dest = &board[row_dest][col_dest];

	if (!orig->mine || dest->mine) {
		return 0;
	}

	orig->mine = 0;

	decrement_neighbours(
		board_width, board_height, board, row_orig, col_orig
	);

	dest->mine = 1;

	increment_neighbours(
		board_width, board_height, board, row_dest, col_dest
	);

	return 1;
}

/*
 * Assuming the field at i, j is a mine,
 * increments the neigbouring mines counter
 * for every neighbour field.
 */
void increment_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t row_center, uint8_t col_center
) {
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			int row = row_center + dy;
			int col = col_center + dx;

			if (
				row >= 0 && row < board_height &&
				col >= 0 && col < board_width
			) {
				board[row][col].num_mines++;
			}
		}
	}
}

/*
 * Assuming the field at i, j is no longer a mine,
 * decrements the neigbouring mines counter
 * for every neighbour field.
 */
void decrement_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t row_center, uint8_t col_center
) {
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			int row = row_center + dy;
			int col = col_center + dx;

			if (
				row >= 0 && row < board_height &&
				col >= 0 && col < board_width
			) {
				board[row][col].num_mines--;
			}
		}
	}
}

/**
 * Move the cursor alongside the x or y axis,
 * wrapping from one screen border to the other if necessary.
 */
int move_wrapping(uint8_t sel, int amount, uint8_t limit) {
	if (amount < 0 && sel < abs(amount)) {
		return limit - 1;
	}

	sel += amount;
	return sel < limit ? sel : 0;
}
