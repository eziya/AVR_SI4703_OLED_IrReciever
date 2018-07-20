/*
 * _328P_USART.h
 *
 * Created: 2018-02-06 오후 10:26:52
 *  Author: kiki
 */ 


#ifndef _328P_USART_H_
#define _328P_USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>

#define USART_REG		0xC0
#define USART_TIMEOUT	1000	/* loop */
#define USART_BAUDRATE	9600

typedef struct _usart
{
	uint8_t ucsr_a;		/* USART Control & Status Register A */
	uint8_t ucsr_b;		/* USART Control & Status Register B */
	uint8_t ucsr_c;		/* USART Control & Status Register C */
	uint8_t reserved;
	uint8_t ubrr_l;		/* USART Baud Rate Register Low */
	uint8_t ubrr_h;		/* USART Baud Rate Register High */
	uint8_t udr;		/* USART Data Register */
} usart;

void USART_Init();
int8_t USART_RxByte(uint8_t *data);
void USART_TxByte(uint8_t data);
int8_t USART_RxBuffer(uint8_t *buffer, uint16_t len);
void USART_TxBuffer(uint8_t *buffer, uint16_t len);

extern volatile usart * const USART;
extern volatile uint8_t USART_rx_flag;

#endif /* _328P_USART_H_ */