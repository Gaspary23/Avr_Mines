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

#include "board.h"
#include "chars.h"
#include "nokia5110.h"
#include "usart.h"
#include "writing.h"

// Specify the board's dimensions in lines and columns.
#define BOARD_WIDTH 14
#define BOARD_HEIGHT 5
#define UP PIND &(1 << PD1)
#define LEFT PIND &(1 << PD2)
#define DOWN PIND &(1 << PD3)
#define RIGHT PIND &(1 << PD4)
#define FLAG PIND &(1 << PD6)
#define CHECK PIND &(1 << PD7)

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

void handle_buttons(Field *sel_field);
void handle_movement();
void setup();

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
		reset_board(BOARD_WIDTH, BOARD_HEIGHT, g_board, MINE_AMOUNT);

		while (g_game_state == START || g_game_state == PLAYING)
		{
			nokia_lcd_clear();

			write_board(
				BOARD_WIDTH, BOARD_HEIGHT, g_board,
				g_sel_x, g_sel_y, g_game_state
			);

			write_timer(0, BOARD_HEIGHT * 8, g_min, g_sec);

			write_flag_count(
				BOARD_WIDTH * 5 - 22, BOARD_HEIGHT * 8,
				g_flags_placed, MINE_AMOUNT
			);

			nokia_lcd_render();
		}

		nokia_lcd_clear();
		reveal_board(BOARD_WIDTH, BOARD_HEIGHT, g_board);

		write_board(
			BOARD_WIDTH, BOARD_HEIGHT, g_board,
			g_sel_x, g_sel_y, g_game_state
		);

		if (g_game_state == DEFEAT)
		{
			write_defeat(0, BOARD_HEIGHT * 8);
		}
		else
		{
			write_victory(0, BOARD_HEIGHT * 8);
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
	if (CHECK) {
		if (g_game_state == MENU) {
			g_sec = 0;
			g_min = 0;
			g_game_state = START;
		} else if (g_game_state == START || g_game_state == PLAYING) {
			sel_field->revealed = 1;
			g_game_state=PLAYING;

			if (sel_field->mine) {
				g_game_state = DEFEAT;
			} else {
				g_flags_placed = sel_field->flagged ? g_flags_placed - 1 : g_flags_placed;
				sel_field->flagged = 0;
				g_fields_left--;

				if (g_fields_left == 0) {
					g_game_state = VICTORY;
				}
			}
		}
	} else if (FLAG) {
		if (g_game_state == DEFEAT || g_game_state == VICTORY) {
			g_game_state = MENU;
			g_sel_y = 0;
			g_sel_x = 0;
			g_flags_placed = 0;
		} else if (!sel_field->revealed) {
			sel_field->flagged ^= 1;
			g_flags_placed = sel_field->flagged ? g_flags_placed + 1 : g_flags_placed - 1;
		}
	}
}

void handle_movement()
{
	if (UP) {
		g_sel_y = move_wrapping(g_sel_y, -1, BOARD_HEIGHT);
	} else if (DOWN) {
		g_sel_y = move_wrapping(g_sel_y, 1, BOARD_HEIGHT);
	} else if (LEFT) {
		g_sel_x = move_wrapping(g_sel_x, -1, BOARD_WIDTH);
	} else if (RIGHT) {
		g_sel_x = move_wrapping(g_sel_x, 1, BOARD_WIDTH);
	}
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
	nokia_lcd_custom(2, UNREVEALED_GLYPH);
	nokia_lcd_custom(3, SELECTED_GLYPH);
	nokia_lcd_custom(4, FLAG_GLYPH);
	nokia_lcd_custom(5, MINE_GLYPH);

	// Initialize USART for debugging purposes.
	USART_Init();
}
