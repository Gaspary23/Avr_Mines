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
	for (int i = 0; i < board_height; i++) {
		for (int j = 0; j < board_width; j++) {
			board[i][j] = (Field) {0, 0, 0, 0};
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
	for (int i = 0; i < board_height; i++) {
		for (int j = 0; j < board_width; j++) {
			board[i][j].revealed = 1;
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

	for (int i=0; i < board_height; i++) {
		for (int j = 0; j < board_width; j++) {
			float rand_res = (float) rand() / (float) RAND_MAX;

			if (fields_left * rand_res < amount - mines_generated) {
				board[i][j].mine = 1;
				mines_generated++;
				increment_neighbours(board_width, board_height, board, i, j);

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
 * Count the number of neigbouring mines.
 */
void increment_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t i, uint8_t j
) {
	for (int row = -1; row <= 1; row++) {
		for (int col = -1; col <= 1; col++) {
			int dy = i + row;
			int dx = j + col;

			if (
				dy >= 0 && dy < board_height &&
				dx >= 0 && dx < board_width
			) {
				board[dy][dx].num_mines++;
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
