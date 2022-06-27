/*
 * Authors: Bernardo Barzotto Zomer (21103639)
 *          Lucas Marchesan Cunha   (21101060)
 *          Pedro da Cunha Gaspary  (21101429)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chars.h"
#include "nokia5110.h"
#include "usart.h"

#define TIMER_CLK F_CPU / 1024
// Update the timer once per second.
#define TIMER_FREQ 1
// Specify the board's dimensions in lines and columns.
#define BOARD_WIDTH 14
#define BOARD_HEIGHT 5
#define MINE_AMOUNT 14
#define UP PIND &(1 << PD1)
#define LEFT PIND &(1 << PD2)
#define DOWN PIND &(1 << PD3)
#define RIGHT PIND &(1 << PD4)
#define FLAG PIND &(1 << PD6)
#define CHECK PIND &(1 << PD7)

/*
 * Represent a field in the board.
 */
typedef struct field
{
	int mine;
	int flagged;
	int revealed;
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

Field board[BOARD_HEIGHT][BOARD_WIDTH];
uint8_t sel_x = 0;
uint8_t sel_y = 0;
State game_state = MENU;
int min, sec = 0;
int seed = 0;
int num_flags = 0;

void reveal_board();
void generate_mines();
void handle_buttons(Field *sel_field);
void handle_movement();
int move_wrapping(uint8_t sel, int x, int amount);
void reset_board();
void setup();
void write_board();
void write_defeat();
void write_flag_count(uint8_t x, uint8_t y);
void write_menu();
void write_timer(uint8_t x, uint8_t y);
void write_victory();

int main()
{
	setup();

	while (1)
	{
		while (game_state == MENU)
		{
			nokia_lcd_clear();
			write_menu();
			// Obtain a random seed from the time taken to start gameplay.
			seed++;
			nokia_lcd_render();
		}

		srand(seed);
		reset_board();

		while (game_state == START || game_state == PLAYING)
		{
			nokia_lcd_clear();
			write_board();
			write_timer(0, BOARD_HEIGHT * 8);
			write_flag_count(BOARD_WIDTH * 5 - 22, BOARD_HEIGHT * 8);
			nokia_lcd_render();
		}

		nokia_lcd_clear();
		reveal_board();
		write_board();

		if (game_state == DEFEAT)
		{
			write_defeat();
		}
		else
		{
			write_victory();
		}

		while (game_state != MENU) {
			nokia_lcd_render();
		}
	}
}

/**
 * Increment the timer.
 */
ISR(TIMER1_COMPA_vect)
{
	if (game_state == PLAYING)
	{
		sec++;

		if (sec >= 60)
		{
			sec = 0;
			min++;
		}
	}
}

/**
 * Handle button interruptions.
 */
ISR(PCINT2_vect)
{
	if (game_state == START || game_state == PLAYING)
	{
		handle_movement();
	}

	handle_buttons(&board[sel_y][sel_x]);
}

void handle_buttons(Field *sel_field)
{
	if (CHECK)
	{
		if (game_state == MENU)
		{
			sec = 0;
			min = 0;
			game_state = START;
		}
		else if (game_state == START || game_state == PLAYING)
		{
			sel_field->revealed = 1;

			if (sel_field->mine)
			{
				game_state = DEFEAT;
			}
			else
			{
				num_flags = sel_field->flagged ? num_flags - 1 : num_flags;
				sel_field->flagged = 0;
			}
		}
	}
	else if (FLAG)
	{
		if (game_state == DEFEAT || game_state == VICTORY)
		{
			game_state = MENU;
			sel_y = 0;
			sel_x = 0;
			num_flags = 0;
		}
		else if (!sel_field->revealed)
		{
			sel_field->flagged ^= 1;
			num_flags = sel_field->flagged ? num_flags + 1 : num_flags - 1;
		}
	}
}

void handle_movement()
{
	if (UP)
	{
		sel_y = move_wrapping(sel_y, 0, -1);
	}
	else if (DOWN)
	{
		sel_y = move_wrapping(sel_y, 0, 1);
	}
	else if (LEFT)
	{
		sel_x = move_wrapping(sel_x, 1, -1);
	}
	else if (RIGHT)
	{
		sel_x = move_wrapping(sel_x, 1, 1);
	}
}

/**
 * Move the cursor alongside the x or y axis,
 * wrapping from one screen border to the other if necessary.
 */
int move_wrapping(uint8_t sel, int x, int amount)
{
	if (amount < 0 && sel < abs(amount))
	{
		return x ? BOARD_WIDTH - 1 : BOARD_HEIGHT - 1;
	}

	sel += amount;

	if (x)
	{
		if (sel >= BOARD_WIDTH)
		{
			return 0;
		}
	}
	else
	{
		if (sel >= BOARD_HEIGHT)
		{
			return 0;
		}
	}

	return sel;
}

/*
 * Resets the board to its initial state.
 * Mines are then regenerated.
 */
void reset_board()
{
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i][j] = (Field) {0, 0, 0};
		}
	}

	generate_mines();
}

/**
 * Reveal the whole board.
 */
void reveal_board()
{
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i][j].revealed = 1;
		}
	}
}

/*
 * Fill the board with mines.
 */
void generate_mines()
{
	for (int mines = 0; mines < MINE_AMOUNT;)
	{
		int i = rand() % BOARD_HEIGHT;
		int j = rand() % BOARD_WIDTH;

		if (!board[i][j].mine)
		{
			board[i][j].mine = 1;
			mines++;
		}
	}
}

/**
 * Write the board to the screen.
 */
void write_board()
{
	for (int i = 0; i < BOARD_HEIGHT; i++)
	{
		// Set the cursor to the start of the line.
		nokia_lcd_set_cursor(0, i * 8);

		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			if (
				i == sel_y && j == sel_x
				&& (game_state == START || game_state == PLAYING)
			) {
				nokia_lcd_write_string("\003", 1);
				continue;
			}

			Field field = board[i][j];

			if (!field.revealed)
			{
				if (field.flagged)
				{
					nokia_lcd_write_string("\004", 1);
				}
				else
				{
					nokia_lcd_write_string("\002", 1);
				}
			}
			else if (field.mine)
			{
				nokia_lcd_write_string("\005", 1);
			}
			else
			{
				// TODO: write number of neighbouring bombs.
				nokia_lcd_write_string(" ", 1);
			}
		}
	}
}

/**
 * Write the amount of flags used by the player to the screen.
 */
void write_flag_count(uint8_t x, uint8_t y)
{
	char flags[7];
	sprintf(flags, "%02d/%02d\004", num_flags, MINE_AMOUNT);
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string(flags, 1);
}

/**
 * Write the timer in MM:SS format to the screen.
 */
void write_timer(uint8_t x, uint8_t y)
{
	char time_display[7];
	sprintf(time_display, "\001%02d:%02d", min, sec);
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string(time_display, 1);
}

/**
 * Write the victory message to the screen.
 */
void write_victory()
{
	nokia_lcd_set_cursor(0, BOARD_HEIGHT * 8);
	nokia_lcd_write_string("B) Press FLAG ", 1);
}

/**
 * Write the defeat message to the screen.
 */
void write_defeat()
{
	nokia_lcd_set_cursor(0, BOARD_HEIGHT * 8);
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

/**
 * Sets up the timer, inputs, interruptions and custom glyphs.
 */
void setup()
{
	cli();

	// Set up the timer.
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = (TIMER_CLK / TIMER_FREQ) - 1;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);

	// Set ports as input.
	// These will be mapped to the buttons.
	DDRD &= ~(1 << PD1) & ~(1 << PD2) & ~(1 << PD3) & ~(1 << PD4) & ~(1 << PD6) & ~(1 << PD7);

	// Toggle interruption vector for PD7, ..., PD0.
	PCICR |= (1 << PCIE2);
	// Toggle interruptions for every button.
	PCMSK2 |= (1 << PD1) | (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD6) | (1 << PD7);

	sei();

	nokia_lcd_init();
	nokia_lcd_custom(1, CLOCK_GLYPH);
	nokia_lcd_custom(2, SPACE_GLYPH);
	nokia_lcd_custom(3, SELECTED_GLYPH);
	nokia_lcd_custom(4, FLAG_GLYPH);
	nokia_lcd_custom(5, MINE_GLYPH);

	// Initialize USART for debugging purposes.
	USART_Init();
}
