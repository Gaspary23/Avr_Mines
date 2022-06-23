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

uint8_t selected_space[] = {
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110,
	0b0111110
};

int start = 1;
int min, sec = 0;
char field[5][14];

#define TIMER_CLK	F_CPU / 1024
// Update the timer once per second.
#define TIMER_FREQ  1

void print_start();
void print_timer();

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
    nokia_lcd_custom(1, clock);          // Cod: a
    nokia_lcd_custom(2, space);          // Cod: b
    nokia_lcd_custom(3, selected_space); // Cod: c
    nokia_lcd_custom(4, flag);           // Cod: d
    nokia_lcd_custom(5, mine);           // Cod: e
    int num_flags = 0;

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 14; j++)
        {
            field[i][j] = 'b';
        }
    }

	while (1) {
		print_field();
		print_timer();
		print_flags(num_flags);
		nokia_lcd_render();
	}
}

ISR(TIMER1_COMPA_vect)
{
    if (start) {
        sec++;

        if (sec >= 60) {
            sec = 0;
            min++;
        }
    }
}

/**
 * Prints the timer in MM:SS format to the screen.
 */
void print_timer()
{
	char time_display[7];
	sprintf(time_display, "\001%02d:%02d", min, sec);
	nokia_lcd_set_cursor(0, 40);
	nokia_lcd_write_string(time_display, 1);
	nokia_lcd_render();
}

//Metodo que imprime as bandeiras usadas pelo jogador
void print_flags(int num_flags) {
    char flags[6];
    sprintf(flags,"%d",num_flags);
    strcat(flags, "/14");
    strcat(flags,"\004");

    nokia_lcd_set_cursor(54,40);
    nokia_lcd_write_string(flags, 1);
    nokia_lcd_render();
}

// Metodo que imprime a matriz no display
void print_field()
{
    for (int i = 0; i < 5; i++)
    {
        nokia_lcd_set_cursor(0, i * 8);
        for (int j = 0; j < 14; j++)
        {
            switch (field[i][j])
            {
            case 'a':
                nokia_lcd_write_string("\001", 1);
                break; // clock
            case 'b':
                nokia_lcd_write_string("\002", 1);
                break; // blank_space
            case 'c':
                nokia_lcd_write_string("\003", 1);
                break; // selected_space
            case 'd':
                nokia_lcd_write_string("\004", 1);
                break; // flag
            case 'e':
                nokia_lcd_write_string("\005", 1);
                break; // mine
            }
        }
    }
}

void print_start() {
	nokia_lcd_write_string(
			" AVR \005 Mines! "
			"              "
			" \0041\002\00232   11\002 "
			" 1 2\005\002\0021 1\002\002\002 "
			"              "
			" Press CHECK! ",
			1
		);
	nokia_lcd_render();
}
