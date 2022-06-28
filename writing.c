#include <avr/io.h>

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "chars.h"
#include "nokia5110.h"

/**
 * Write the board to the screen.
 */
void write_board(
	uint8_t board_width, uint8_t board_height,
	Field board[board_height][board_width],
	uint8_t sel_x, uint8_t sel_y,
	State game_state
) {
	for (int row = 0; row < board_height; row++) {
		// Set the cursor to the start of the line.
		nokia_lcd_set_cursor(0, row * 8);

		for (int col = 0; col < board_width; col++) {
			if (
				row == sel_y && col == sel_x
				&& (game_state == START || game_state == PLAYING)
			) {
				nokia_lcd_write_string("\003", 1);
				continue;
			}

			Field field = board[row][col];

			if (!field.revealed) {
				if (field.flagged) {
					nokia_lcd_write_string("\004", 1);
				}
				else {
					nokia_lcd_write_string("\002", 1);
				}
			}
			else if (field.mine) {
				nokia_lcd_write_string("\005", 1);
			}
			else {
				if (field.num_mines > 0) {
					char value[3];
					sprintf(value, "%d", field.num_mines);
					nokia_lcd_write_string(value, 1);
				} else {
					nokia_lcd_write_string(" ", 1);
				}
			}
		}
	}
}

/**
 * Write the amount of flags used by the player to the screen.
 */
void write_flag_count(
	uint8_t x, uint8_t y,
	uint8_t flags_placed, uint8_t max_flags
) {
	char flags[7];
	sprintf(flags, "%02d/%02d\004", flags_placed, max_flags);
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string(flags, 1);
}

/**
 * Write the timer in MM:SS format to the screen.
 */
void write_timer(
	uint8_t x, uint8_t y, uint8_t min, uint8_t sec
) {
	char time_display[7];
	sprintf(time_display, "\001%02d:%02d", min, sec);
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string(time_display, 1);
}

/**
 * Write the victory message to the screen.
 */
void write_victory(uint8_t x, uint8_t y)
{
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string("B) Press FLAG ", 1);
}

/**
 * Write the defeat message to the screen.
 */
void write_defeat(uint8_t x, uint8_t y)
{
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string("xO Press FLAG ", 1);
}

/**
 * Write the start menu to the screen.
 */
void write_menu()
{
	nokia_lcd_write_string(
		"              "
		" AVR \005 Mines! "
		"              "
		" Press CHECK! "
		"			   ",
		1
	);
}