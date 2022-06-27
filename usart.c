#include "usart.h"
#include <stdio.h>
#include <stdarg.h>

void USART_Init(void)
{
    // Seta taxa de transmissão/recepção (baud rate)
    UBRR0H = (uint8_t)(USART_UBBR_VALUE >> 8);
    UBRR0L = (uint8_t)USART_UBBR_VALUE;
    // Seta formato do frame de transmissão: 8 bits de dados, sem paridade, 1 stop bit
    UCSR0C = (0 << USBS0) | (3 << UCSZ00);
    // Habilita receptor e transmissor
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
}

void USART_SendByte(uint8_t u8Data)
{
    // Espera se um byte estiver sendo transmitido
    while ((UCSR0A & (1 << UDRE0)) == 0)
        ;
    // Transmite o byte
    UDR0 = u8Data;
}

uint8_t USART_ReceiveByte(void)
{
    // Espera até um byte ter sido recebido
    while ((UCSR0A & (1 << RXC0)) == 0)
        ;
    return UDR0;
}

void USART_puts(const char *str)
{
    while (*str)
        USART_SendByte(*str++);
}

void USART_printf(const char *format, ...)
{
    char str[40];
    va_list arg;

    va_start(arg, format);
    vsnprintf(str, 40, format, arg);
    USART_puts(str);
    va_end(arg);
}