/*
 * _328P_TWI.c
 *
 * Created: 2018-07-03 오후 11:10:36
 *  Author: kiki
 */ 
#ifndef F_CPU
#define F_CPU	16000000L
#endif

#include "328P_TWI.h"

static bool TWI_Start(uint8_t deviceAddr);
static bool TWI_Stop(void);
static bool TWI_TxData(uint8_t data);
static bool TWI_RxAck(uint8_t *data);
static bool TWI_RxNack(uint8_t *data);

void TWI_Init(void)
{
	/* Pin Configuration */
	TWI_DDR |= _BV(SCL_OUT)|_BV(SDA_OUT);
			
	/* Configure Speed */
	TWBR = (((F_CPU / SCL_FREQ) - 16 ) / 2);
	TWSR = 0;
}

bool TWI_RxBuffer(uint8_t deviceAddr, uint8_t *data, uint8_t length)
{
	/* Send start bit */
	if(!TWI_Start((deviceAddr << 1)|TWI_READ)) return false;
	
	/* Send ACK or NACK at the end of data */
	for(uint8_t i = 0; i < length; i++)
	{
		if(i < length -1)
		{
			if(!TWI_RxAck(data + i)) return false;
		}
		else
		{
			if(!TWI_RxNack(data + i)) return false;
		}
	}
	
	/* Send stop bit */
	if(!TWI_Stop()) return false;
	
	return true;
}

bool TWI_TxBuffer(uint8_t deviceAddr, uint8_t *data, uint8_t length)
{
	/* Send start bit */
	if(!TWI_Start((deviceAddr << 1)|TWI_WRITE)) return false;	
	
	/* Send data */
	for(uint8_t i = 0; i < length; i++)
	{
		if(!TWI_TxData(*(data+i))) return false;
	}
	
	/* Send stop bit */
	if(!TWI_Stop()) return false;
	
	return true;
}

static bool TWI_Start(uint8_t deviceAddr)
{
	//TWCR = 0;
	
	/* Send START condition */
	TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
	
	/* Wait for TWINT flag */
	while((TWCR & _BV(TWINT)) == 0);
	
	/* Check value of TWI Status Register */
	if(((TWSR & TWI_TWS_MASK) != TWI_START) && ((TWSR & TWI_TWS_MASK) != TWI_RESTART) ) 
	{		
		TWI_Stop();
		return false;
	}
	
	/* Load Data into TWDR Register */
	TWDR = deviceAddr;
	TWCR = _BV(TWINT) | _BV(TWEN);
	
	/* Wait for TWINT Flag set */
	while((TWCR & _BV(TWINT)) == 0);
	
	uint8_t status = TWSR & TWI_TWS_MASK;
	if((status != TWI_MR_SLA_ACK) && (status != TWI_MT_SLA_ACK))
	{
		TWI_Stop();
		return false;
	}	
	
	return true;
}

static bool TWI_Stop()
{		
	/* Send STOP condition */
	TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
	while((TWCR & _BV(TWSTO)) == 0);
	
	return true;
}

static bool TWI_TxData(uint8_t data)
{		
	/* Load Data into TWDR Register */
	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN);
	
	/* Wait for TWINT Flag set */
	while((TWCR & _BV(TWINT)) == 0);
	
	/* Check value of TWI Status Register */
	if((TWSR & TWI_TWS_MASK) != TWI_MT_DATA_ACK) 
	{
		TWI_Stop();
		return false;
	}
	
	return true;
}

static bool TWI_RxAck(uint8_t *data)
{		
	/* Send ACK */
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	
	/* Wait for TWINT Flag set */
	while((TWCR & _BV(TWINT)) == 0);
	
	/* Check value of TWI Status Register */
	if((TWSR & TWI_TWS_MASK) != TWI_MR_DATA_ACK) return false;
	
	/* return data */
	*data = TWDR;
	return true;
}

static bool TWI_RxNack(uint8_t *data)
{		
	/* Send NACK */
	TWCR = _BV(TWINT) | _BV(TWEN);
	
	/* Wait for TWINT Flag set */
	while((TWCR & _BV(TWINT)) == 0);
	
	/* Check value of TWI Status Register */
	if((TWSR & TWI_TWS_MASK) != TWI_MR_DATA_NACK) return false;
	
	/* return data */
	*data = TWDR;
	return true;
}