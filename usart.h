#ifndef _USART_H
#define _USART_H

#include <stdint.h>
#include <avr/io.h>

// VALUE = F_FPU / (16 * BAUD) - 1
#define USART_UBBR_VALUE (((unsigned long)F_CPU / (unsigned long)(16 * (unsigned long)USART_BAUD)) - 1)

void USART_Init(void);
void USART_SendByte(uint8_t u8Data);
uint8_t USART_ReceiveByte(void);
void USART_puts(const char *str);
void USART_printf(const char *format, ...);

#endif