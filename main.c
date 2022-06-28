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

// Specify the board's dimensions in lines and columns.
#define BOARD_WIDTH 14
#define BOARD_HEIGHT 5
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
	unsigned char mine;
	unsigned char flagged;
	unsigned char revealed;
	unsigned char num_mines;
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

const int MINE_AMOUNT = 14;
const int TIMER_CLK = F_CPU / 1024;
// Update the timer once per second.
const int TIMER_FREQ = 1;

static State g_game_state = MENU;
static Field g_board[BOARD_HEIGHT][BOARD_WIDTH];
// This is the amount of empty fields left to be revealed
// until victory is achieved.
static int g_fields_left = BOARD_HEIGHT * BOARD_WIDTH - MINE_AMOUNT;
static int g_flags_placed = 0;
// Store the coordinates of the currently selected field.
static uint8_t g_sel_x = 0;
static uint8_t g_sel_y = 0;
// Track elapsed time.
static int g_min = 0;
static int g_sec = 0;

void reveal_board();
void generate_mines();
void handle_buttons(Field *sel_field);
void handle_movement();
void increment_neighbours(int i, int j);
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
	int seed;

	while (1)
	{
		seed = 0;

		while (g_game_state == MENU)
		{
			nokia_lcd_clear();
			write_menu();
			// Obtain a random seed from the time taken to start gameplay.
			seed++;
			nokia_lcd_render();
		}

		srand(seed);
		reset_board();

		while (g_game_state == START || g_game_state == PLAYING)
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

		if (g_game_state == DEFEAT)
		{
			write_defeat();
		}
		else
		{
			write_victory();
		}

		while (g_game_state != MENU)
		{
			nokia_lcd_render();
		}
	}
}

/**
 * Increment the timer.
 */
ISR(TIMER1_COMPA_vect)
{
	if (g_game_state == PLAYING)
	{
		g_sec++;

		if (g_sec >= 60)
		{
			g_sec = 0;
			g_min++;
		}
	}
}

/**
 * Handle button interruptions.
 */
ISR(PCINT2_vect)
{
	if (g_game_state == START || g_game_state == PLAYING)
	{
		handle_movement();
	}

	handle_buttons(&g_board[g_sel_y][g_sel_x]);
}

void handle_buttons(Field *sel_field)
{
	if (CHECK)
	{
		if (g_game_state == MENU)
		{
			g_sec = 0;
			g_min = 0;
			g_game_state = START;
		}
		else if (g_game_state == START || g_game_state == PLAYING)
		{
			sel_field->revealed = 1;
			g_game_state=PLAYING;

			if (sel_field->mine)
			{
				g_game_state = DEFEAT;
			}
			else
			{
				g_flags_placed = sel_field->flagged ? g_flags_placed - 1 : g_flags_placed;
				sel_field->flagged = 0;
				g_fields_left--;

				if (g_fields_left == 0)
				{
					g_game_state = VICTORY;
				}
			}
		}
	}
	else if (FLAG)
	{
		if (g_game_state == DEFEAT || g_game_state == VICTORY)
		{
			g_game_state = MENU;
			g_sel_y = 0;
			g_sel_x = 0;
			g_flags_placed = 0;
		}
		else if (!sel_field->revealed)
		{
			sel_field->flagged ^= 1;
			g_flags_placed = sel_field->flagged ? g_flags_placed + 1 : g_flags_placed - 1;
		}
	}
}

void handle_movement()
{
	if (UP)
	{
		g_sel_y = move_wrapping(g_sel_y, 0, -1);
	}
	else if (DOWN)
	{
		g_sel_y = move_wrapping(g_sel_y, 0, 1);
	}
	else if (LEFT)
	{
		g_sel_x = move_wrapping(g_sel_x, 1, -1);
	}
	else if (RIGHT)
	{
		g_sel_x = move_wrapping(g_sel_x, 1, 1);
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
			g_board[i][j] = (Field) {0, 0, 0, 0};
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
			g_board[i][j].revealed = 1;
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
void generate_mines()
{
	int mines_generated = 0;
	int fields_left = BOARD_HEIGHT * BOARD_WIDTH - 1;

	for (int i=0; i < BOARD_HEIGHT; i++)
	{
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			float rand_res = (float) rand() / (float) RAND_MAX;

			if (fields_left * rand_res < MINE_AMOUNT - mines_generated)
			{
				g_board[i][j].mine = 1;
				mines_generated++;
				increment_neighbours(i,j);

				if (mines_generated == MINE_AMOUNT)
				{
					break;
				}
			}

			fields_left--;
		}

		if (mines_generated == MINE_AMOUNT)
		{
			break;
		}
	}
}

/*
 * Count number of neigbour mines
 */
void increment_neighbours(int i, int j)
{
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			int a = i+x;
			int b = j+y;
			if (
				a >= 0 &&
				a < BOARD_HEIGHT &&
				b >= 0 &&
				b < BOARD_WIDTH
			) {
				g_board[i+x][j+y].num_mines++;
			}
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
				i == g_sel_y && j == g_sel_x
				&& (g_game_state == START || g_game_state == PLAYING)
			) {
				nokia_lcd_write_string("\003", 1);
				continue;
			}

			Field field = g_board[i][j];

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
void write_flag_count(uint8_t x, uint8_t y)
{
	char flags[7];
	sprintf(flags, "%02d/%02d\004", g_flags_placed, MINE_AMOUNT);
	nokia_lcd_set_cursor(x, y);
	nokia_lcd_write_string(flags, 1);
}

/**
 * Write the timer in MM:SS format to the screen.
 */
void write_timer(uint8_t x, uint8_t y)
{
	char time_display[7];
	sprintf(time_display, "\001%02d:%02d", g_min, g_sec);
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
