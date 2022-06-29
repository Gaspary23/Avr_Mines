#include <stdint.h>
#include <stdlib.h>

#include "board.h"

/*
 * Use this structure to queue fields when
 * performing BFS on the field graph.
 */
typedef struct queued_field {
	uint8_t row;
	uint8_t col;
	struct queued_field *next;
} QueuedField;

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
					board_width, board_height, board, row, col, 1
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
 * Reveal a field and then from there,
 * all adjacent fields that can be revealed.
 */
void reveal_section(
	int *fields_revealed, int *flags_removed,
	uint8_t row_orig, uint8_t col_orig,
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
) {
	QueuedField u = (QueuedField) { row_orig, col_orig, NULL };
	board[row_orig][col_orig].revealed = 1;

	(*fields_revealed)++;

	if (board[row_orig][col_orig].flagged) {
		board[row_orig][col_orig].flagged = 0;
		(*flags_removed)++;
	}

	if (!board[row_orig][col_orig].num_mines) {
		do {
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					int row = u.row + dy;
					int col = u.col + dx;

					if (
						row >= 0 && row < board_height &&
						col >= 0 && col < board_width &&
						!board[row][col].revealed &&
						!board[row][col].mine
					) {
						board[row][col].revealed = 1;
						(*fields_revealed)++;

						if (board[row][col].flagged) {
							board[row][col].flagged = 0;
							(*flags_removed)++;
						}

						if (board[row][col].num_mines) {
							continue;
						}

						QueuedField tail = u;

						while (tail.next) {
							tail = *tail.next;
						}

						tail.next = &(QueuedField) { row, col, NULL };
					}
				}
			}

			if (u.next) {
				u = *u.next;
			}
		} while(u.next);
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

	increment_neighbours(
		board_width, board_height, board, row_orig, col_orig, -1
	);

	dest->mine = 1;

	increment_neighbours(
		board_width, board_height, board, row_dest, col_dest, 1
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
	uint8_t row_center, uint8_t col_center, uint8_t increment
) {
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			int row = row_center + dy;
			int col = col_center + dx;

			if (
				row >= 0 && row < board_height &&
				col >= 0 && col < board_width
			) {
				board[row][col].num_mines+=increment;
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
