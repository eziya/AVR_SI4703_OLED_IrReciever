/*
 * _328P_USART.c
 *
 * Created: 2018-02-06 오후 10:27:09
 *  Author: kiki
 */ 

#ifndef F_CPU
#define F_CPU	16000000
#endif

#include "328P_USART.h"
#include <util/delay.h>

volatile usart * const USART = (void*)USART_REG;

void USART_Init()
{
	/* 2배속 모드 */
	USART->ucsr_a |= _BV(U2X0);
	
	/* Baud rate 설정, UBRR = (fosc / (8 * BAUD)) - 1 */
	/* 16MHz / (8 * BAUDRATE) - 1 = 16 */
	uint16_t baudrate = F_CPU / 8 / USART_BAUDRATE - 1;
	USART->ubrr_h = (baudrate >> 8) & 0xFF;
	USART->ubrr_l = baudrate & 0xFF;
	
	/* 비동기, 8-N-1 설정 */		
	USART->ucsr_c |= _BV(UCSZ01) | _BV(UCSZ00);
	
	/* 송수신 Enable */
	USART->ucsr_b |= _BV(RXEN0) | _BV(TXEN0);	
}

int8_t USART_RxByte(uint8_t *data)
{	
	uint16_t loop = USART_TIMEOUT;
	do 
	{
		if(USART->ucsr_a & _BV(RXC0))
		{
			*data = USART->udr;			
			return 0;
		}
	} while (--loop);
	
	return -1;
}

void USART_TxByte(uint8_t data)
{
	while((USART->ucsr_a & _BV(UDRE0)) == 0);			
	USART->udr = data;	
}

int8_t USART_RxBuffer(uint8_t *buffer, uint16_t len)
{
	for(int i=0; i < len; i++)
	{
		if(USART_RxByte(&buffer[i]) < 0)
		{
			return -1;
		}
	}	
	
	return 0;
}

void USART_TxBuffer(uint8_t *buffer, uint16_t len)
{
	for(int i=0; i < len; i++)
	{
		USART_TxByte(buffer[i]);
	}
}
