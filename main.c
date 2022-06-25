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
#define TIMER_FREQ 01
// Specify the minefield's dimensions in lines and columns.
#define FIELD_WIDTH 14
#define FIELD_HEIGHT 05
#define MINE_AMOUNT 14
#define UP PIND &(1 << PD1)
#define LEFT PIND &(1 << PD2)
#define DOWN PIND &(1 << PD3)
#define RIGHT PIND &(1 << PD4)
#define FLAG PIND &(1 << PD6)
#define CHECK PIND &(1 << PD7)

typedef struct field
{
	int mine;
	int flagged;
	int revealed;
}
Field;

int playing = 0;
int lost = 0;
int min, sec = 0;
int num_flags = 0;
int seed = 0;
Field board[FIELD_HEIGHT][FIELD_WIDTH];
uint8_t sel_x = 0;
uint8_t sel_y = 0;

void reveal_board();
void generate_mines();
void handle_buttons(Field *sel_field);
void handle_movement();
int move_overflowing(uint8_t sel, int x, int amount);
void reset_board();
void setup();
void write_board();
void write_flag_count(uint8_t x, uint8_t y);
void write_start();
void write_timer(uint8_t x, uint8_t y);

int main()
{
	setup();

	while (1)
	{
		while (!playing)
		{
			nokia_lcd_clear();

			if (!lost)
			{
				write_start();
				seed++;
			}
			else
			{
				reveal_board();
				write_board();
				nokia_lcd_set_cursor(0, FIELD_HEIGHT * 8);
				nokia_lcd_write_string("  Press FLAG  ", 1);
			}

			nokia_lcd_render();
		}

		srand(seed);
		reset_board();

		while (playing)
		{
			nokia_lcd_clear();
			write_board();
			write_timer(0, FIELD_HEIGHT * 8);
			write_flag_count(FIELD_WIDTH * 5 - 22, FIELD_HEIGHT * 8);
			nokia_lcd_render();
		}
	}
}

/**
 * Increment the timer.
 */
ISR(TIMER1_COMPA_vect)
{
	sec++;

	if (sec >= 60)
	{
		sec = 0;
		min++;
	}
}

/**
 * Handle button interruptions.
 */
ISR(PCINT2_vect)
{
	if (playing)
	{
		handle_movement();
	}

	handle_buttons(&board[sel_y][sel_x]);
}

void handle_buttons(Field *sel_field)
{
	if (CHECK)
	{
		if (!playing && !lost)
		{
			sec = 0;
			min = 0;
			playing = 1;
		}
		else
		{
			sel_field->revealed = 1;

			if (sel_field->mine)
			{
				lost = 1;
				playing = 0;
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
		if (!playing && lost)
		{
			lost = 0;
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
		sel_y = move_overflowing(sel_y, 0, -1);
	}
	else if (DOWN)
	{
		sel_y = move_overflowing(sel_y, 0, 1);
	}
	else if (LEFT)
	{
		sel_x = move_overflowing(sel_x, 1, -1);
	}
	else if (RIGHT)
	{
		sel_x = move_overflowing(sel_x, 1, 1);
	}
}

/**
 * Move the cursor alongside the x or y axis,
 * overflowing from one border to the other if necessary.
 */
int move_overflowing(uint8_t sel, int x, int amount)
{
	if (amount < 0 && sel < abs(amount))
	{
		return x ? FIELD_WIDTH - 1 : FIELD_HEIGHT - 1;
	}

	sel += amount;

	if (x)
	{
		if (sel >= FIELD_WIDTH)
		{
			return 0;
		}
	}
	else
	{
		if (sel >= FIELD_HEIGHT)
		{
			return 0;
		}
	}

	return sel;
}

/*
 * Resets the field and the view to its initial state.
 * Mines are then regenerated.
 */
void reset_board()
{
	for (int i = 0; i < FIELD_HEIGHT; i++)
	{
		for (int j = 0; j < FIELD_WIDTH; j++)
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
	for (int i = 0; i < FIELD_HEIGHT; i++)
	{
		for (int j = 0; j < FIELD_WIDTH; j++)
		{
			board[i][j].revealed = 1;
		}
	}
}

/*
 * Fill the field with mines.
 */
void generate_mines()
{
	for (int mines = 0; mines < MINE_AMOUNT;)
	{
		int i = rand() % FIELD_HEIGHT;
		int j = rand() % FIELD_WIDTH;

		if (!board[i][j].mine)
		{
			board[i][j].mine = 1;
			mines++;
		}
	}
}

/**
 * Write the minefield matrix to the screen.
 */
void write_board()
{
	for (int i = 0; i < FIELD_HEIGHT; i++)
	{
		// Set the cursor to the start of the line.
		nokia_lcd_set_cursor(0, i * 8);

		for (int j = 0; j < FIELD_WIDTH; j++)
		{
			if (i == sel_y && j == sel_x && !lost)
			{
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
 * Write the start menu to the screen.
 */
void write_start()
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
