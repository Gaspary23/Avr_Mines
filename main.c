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

#include "nokia5110.h"

uint8_t clock[] = {
	0b00011100,
	0b00100010,
	0b01001111,
	0b00101010,
	0b00011100
};

uint8_t flag[] = {
	0b00000000,
	0b11111111,
	0b00011111,
	0b00001110,
	0b00000100
};

uint8_t mine[] = {
	0b00101010,
	0b0011100,
	0b0111110,
	0b0011100,
	0b0101010
};

uint8_t space[] = {
	0b0000000,
	0b0011100,
	0b0011100,
	0b0011100,
	0b0000000
};

uint8_t selected[] = {
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110
};

int start = 1;
int min, sec = 0;
char field[5][14];

#define TIMER_CLK		F_CPU / 1024
// Update the timer once per second.
#define TIMER_FREQ		01
// Specify the minefield's dimensions in lines and columns.
#define FIELD_WIDTH		14
#define FIELD_HEIGHT	05
#define MINE_AMOUNT		14

enum Tile {
	Clock = 'a',
	Blank = 'b',
	Selected = 'c',
	Flag = 'd',
	Mine = 'e',
};

void write_start();
void write_timer(uint8_t x, uint8_t y);
void write_flag_count(int num_flags, uint8_t x, uint8_t y);
void write_field();

int main(void)
{
	cli();

	// Set up the timer.
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;
	OCR1A = (TIMER_CLK / TIMER_FREQ) - 1;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12) | (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);
	DDRD |= (1 << PD5);

	sei();

    nokia_lcd_init();
    nokia_lcd_custom(1, clock);
    nokia_lcd_custom(2, space);
    nokia_lcd_custom(3, selected);
    nokia_lcd_custom(4, flag);
    nokia_lcd_custom(5, mine);
    int num_flags = 0;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 14; j++) {
            field[i][j] = Blank;
        }
    }

	while (1) {
		write_field();
		write_timer(0, FIELD_HEIGHT * 8);
		write_flag_count(num_flags, FIELD_WIDTH * 5 - 16, FIELD_HEIGHT * 8);
		nokia_lcd_render();
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
void write_flag_count(int num_flags, uint8_t x, uint8_t y)
{
    char flags[6];
    sprintf(flags, "%d/%d\004", num_flags, MINE_AMOUNT);
    nokia_lcd_set_cursor(x, y);
    nokia_lcd_write_string(flags, 1);
}

/**
 * Write the minefield matrix to the screen.
 */
void write_field()
{
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        nokia_lcd_set_cursor(0, i * 8);

        for (int j = 0; j < FIELD_WIDTH; j++) {
            switch (field[i][j]) {
				case Clock:
					nokia_lcd_write_string("\001", 1);
					break;
				case Blank:
					nokia_lcd_write_string("\002", 1);
					break;
				case Selected:
					nokia_lcd_write_string("\003", 1);
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


/**
 * Write the start menu to the screen.
 */
void write_start() {
	nokia_lcd_write_string(
		" AVR \005 Mines! "
		"              "
		" \0041\002\00232   11\002 "
		" 1 2\005\002\0021 1\002\002\002 "
		"              "
		" Press CHECK! ",
		1
	);
}

ISR(TIMER1_COMPA_vect)
{
    if (start) {
        sec++;

        if (sec >= 60) {
            sec = 0;
            min++;
        }
    } else {
		sec = 0;
		min = 0;
	}
}
