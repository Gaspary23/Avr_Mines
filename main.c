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
#include <string.h>

#include "chars.h"
#include "nokia5110.h"

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

int playing = 0;
int min, sec = 0;
int num_flags = 0;
int seed = 0;
char field[FIELD_HEIGHT][FIELD_WIDTH];
uint8_t sel_x = 0;
uint8_t sel_y = 0;

enum Tile
{
	Clock = 'a',
	Blank = 'b',
	Selected = 'c',
	Flag = 'd',
	Mine = 'e',
};

void write_start();
void write_timer(uint8_t x, uint8_t y);
void write_flag_count(uint8_t x, uint8_t y);
void write_field();
int move_overflowing(uint8_t sel, int x, int amount);

int main(void)
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

	for (int i = 0; i < FIELD_HEIGHT; i++)
	{
		for (int j = 0; j < FIELD_WIDTH; j++)
		{
			field[i][j] = Blank;
		}
	}

	write_start();

	while (1)
	{
		while (!playing)
		{
			seed++;
			nokia_lcd_render();
		}

		nokia_lcd_clear();
		srand(seed);
		generate_mines();

		write_field();
		write_timer(0, FIELD_HEIGHT * 8);
		write_flag_count(FIELD_WIDTH * 5 - 22, FIELD_HEIGHT * 8);
		nokia_lcd_render();
	}
}

/*
 * Fill the field with mines 
 */
void generate_mines()
{
	for (int x = 0; x < MINE_AMOUNT; x++)
	{
		int i = rand() % FIELD_HEIGHT;
		int j = rand() % FIELD_WIDTH;

		if(field[i][j] != Mine)
		{
			field[i][j] = Mine;
			x++;
		}
	}
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
 * Write the minefield matrix to the screen.
 */
void write_field()
{
	for (int i = 0; i < FIELD_HEIGHT; i++)
	{
		nokia_lcd_set_cursor(0, i * 8);
		for (int j = 0; j < FIELD_WIDTH; j++)
		{
			if (i == sel_y && j == sel_x)
			{
				nokia_lcd_write_string("\003", 1);
			}
			else
			{
				switch (field[i][j])
				{
				case Clock:
					nokia_lcd_write_string("\001", 1);
					break;
				case Blank:
					nokia_lcd_write_string("\002", 1);
					break;
				case Flag:
					nokia_lcd_write_string("\004", 1);
					break;
				case Mine:
					nokia_lcd_write_string("\005", 1);
					break;
				}
			}
		}
	}
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
		1);
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
	// Handle movement.
	if (playing)
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

	// Handle CHECK and FLAG.
	if (CHECK)
	{
		if (!playing)
		{
			playing = 1;
		}
	}

	if (FLAG)
	{
		if (playing)
		{
			if (field[sel_y][sel_x] == Blank)
			{
				field[sel_y][sel_x] = Flag;
				num_flags += 1;
			}
			else if (field[sel_y][sel_x] == Flag)
			{
				field[sel_y][sel_x] = Blank;
				num_flags -= 1;
			}
		}
	}
}
