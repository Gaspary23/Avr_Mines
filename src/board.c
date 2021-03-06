#include <stdint.h>
#include <stdlib.h>

#include "board.h"
#include "usart.h"

void reset_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t mine_amount
) {
	for (uint8_t row = 0; row < board_height; row++) {
		for (uint8_t col = 0; col < board_width; col++) {
			board[row][col] = (Field) {0, 0, 0, 0};
		}
	}

	generate_mines(board_width, board_height, board, mine_amount);
}

void reveal_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
) {
	for (uint8_t row = 0; row < board_height; row++) {
		for (uint8_t col = 0; col < board_width; col++) {
			board[row][col].revealed = 1;
		}
	}
}

void generate_mines(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t amount
) {
	int mines_generated = 0;
	int fields_left = board_height * board_width - 1;

	for (uint8_t row = 0; row < board_height; row++) {
		for (uint8_t col = 0; col < board_width; col++) {
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

void reveal_section(
	uint8_t *fields_revealed, uint8_t *flags_removed,
	uint8_t row_orig, uint8_t col_orig,
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width]
) {
	Field *field = &board[row_orig][col_orig];
	field->revealed = 1;
	(*fields_revealed)++;

	if (field->flagged) {
		field->flagged = 0;
		(*flags_removed)++;
	}

	if (field->num_mines) {
		return;
	}

	for (int8_t dy = -1; dy <= 1; dy++) {
		for (int8_t dx = -1; dx <= 1; dx++) {
			int8_t row = row_orig + dy;
			int8_t col = col_orig + dx;

			if (
				row < 0 || row >= board_height ||
				col < 0 || col >= board_width
			) {
				continue;
			}

			field = &board[row][col];

			if (field->revealed) {
				continue;
			}

			field->revealed = 1;
			(*fields_revealed)++;

			if (field->flagged) {
				field->flagged = 0;
				(*flags_removed)++;
			}
		}
	}
}

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

void increment_neighbours(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t row_center, uint8_t col_center, int8_t increment
) {
	for (int8_t dy = -1; dy <= 1; dy++) {
		for (int8_t dx = -1; dx <= 1; dx++) {
			int8_t row = row_center + dy;
			int8_t col = col_center + dx;

			if (
				row >= 0 && row < board_height &&
				col >= 0 && col < board_width
			) {
				board[row][col].num_mines += increment;
			}
		}
	}
}

int move_wrapping(uint8_t sel, int8_t amount, uint8_t limit) {
	if (amount < 0 && sel < abs(amount)) {
		return limit - 1;
	}

	sel += amount;
	return sel < limit ? sel : 0;
}
