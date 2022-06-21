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

// Relogio desenhado em binario
uint8_t clock[] = {0b00011100,
                   0b00100010,
                   0b01001111,
                   0b00101010,
                   0b00011100};

uint8_t flag[] = { 0b00000000,
                   0b11111111,
                   0b00011111,
                   0b00001110,
                   0b00000100};

uint8_t mine[] = { 0b00101010,
                    0b0011100,
                    0b0111110,
                    0b0011100,
                    0b0101010};

uint8_t space[] = { 0b0000000,
                    0b0011100,
                    0b0011100,
                    0b0011100,
                    0b0000000};
                             
uint8_t selected_space[] = { 0b0111110,
                             0b0111110,
                             0b0111110,
                             0b0111110,
                             0b0111110};                             
                    
int start = 0;
int reset = 0;

int minu, seg=0;

TIMER_CLK = F_CPU / 1024;
IRQ_FREQ = 100;


// Botao de cima
// Inicia ou pausa o cronometro
ISR(INT0_vect)
{
    if (start == 0)
    {
        start = 1;
    }
    else
    {
        start = 0;
    }
    _delay_ms(1);
}

// Botao de baixo
// Reseta o cronometro se ele estiver parado
ISR(INT1_vect)
{
    if (reset == 0 && start == 0)
    {
        reset = 1;
    }
    _delay_ms(1);
}

ISR(TIMER1_COMPA_vect){
    if (start == 1) {
        seg++;
        if (seg >= 60)
        {
            seg = 0;
            minu++;
        }
    }
}

// Metodo que imprime o relogio na tela
void print_crono(char *min, char *sec, char *crono)
{
    // Printa no modelo XX:XX
    sprintf(min, "%d", minu);
    sprintf(sec, "%d", seg);
    if (minu < 10)
        strcat(crono, "0");
    strcat(crono, min);
    strcat(crono, ":");
    if (seg < 10)
        strcat(crono, "0");
    strcat(crono, sec);

    //Alterar
    nokia_lcd_set_cursor(7, 23);
    nokia_lcd_write_string(crono, 2);
    nokia_lcd_set_cursor(65, 32);
    crono[0] = '\0';
    nokia_lcd_render();
}

int main(void)
{
	cli();
    // TIMER1
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1  = 0;

	OCR1A = (TIMER_CLK / IRQ_FREQ) - 1;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS12) | (1 << CS10);  
	TIMSK1 |= (1 << OCIE1A);
	DDRD |= (1 << PD5);

    // botoes para iniciar/parar e resetar    
    DDRD &= ~((1 << PD2) | (1 << PD3));
    PORTD = ((1 << PD2) | (1 << PD3));

    EICRA = ((1 << ISC01) | (1 << ISC00));
    EICRA |= ((1 << ISC11) | (1 << ISC10));
    EIMSK |= ((1 << INT1) | (1 << INT0));
    sei();

    nokia_lcd_init();
    nokia_lcd_custom(1, clock);
    nokia_lcd_custom(2, space);
    nokia_lcd_custom(3, selected_space);
    nokia_lcd_custom(4, flag);
    nokia_lcd_custom(5, mine);
    char min[4], sec[3], crono[6];

    nokia_lcd_write_string("\002\002\005\002\005\002\002\002\005\002\005\002\002\002", 1);
    nokia_lcd_set_cursor(0,8);
    nokia_lcd_write_string("\005\002\0022\002\005\002\002222\005\003\002", 1);
    nokia_lcd_set_cursor(0,16);
    nokia_lcd_write_string("\0052\002\002\002\0024\0052 13\002\002", 1);
    nokia_lcd_set_cursor(0,24);
    nokia_lcd_write_string("\002\002\002\0021\002\005\00531 1\005\002", 1);
    nokia_lcd_set_cursor(0,32);
    nokia_lcd_write_string("\002\002\002\002\002\005\002\002\0051 1\002\002", 1);
    nokia_lcd_set_cursor(0,40);
    nokia_lcd_write_string("\00107:30   08/15", 1);
    nokia_lcd_render();
    _delay_ms(1);
    
}
