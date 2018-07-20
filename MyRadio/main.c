/*
* MyRadio.c
*
* Created: 2018-07-13 오후 6:34:03
* Author : kiki
*/

#ifndef F_CPU
#define F_CPU	16000000L
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "u8g.h"
#include "SI4703.h"
#include "IrReceiver.h"

#define PRESET1	89.9
#define PRESET2	95.7
#define PRESET3	97.5
#define PRESET4	101.5
#define PRESET5	105.7

u8g_t u8g;
uint8_t IRCommand;
char OledMsg[20];
bool irRcv = false;
bool err = false;

void HandleOLED();
void HandleIRCommand();
void IrRcvCallback(uint32_t code);

int main(void)
{	
	IR_Init(IrRcvCallback);
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x64_i2c, U8G_I2C_OPT_NONE);
	
	while(!SI4703_Init())
	{
		err = true;
	}
		
	if(!SI4703_SetFreq(PRESET1))
	{
		err = true;
	}
		
	SI4703_UpdateRadioInfo();	
	
	/* Replace with your application code */
	while (1)
	{		
		 HandleOLED();
		 HandleIRCommand();
	}
}

void HandleOLED()
{
	u8g_FirstPage(&u8g);
	do
	{
		//Outer rectangle
		u8g_DrawFrame(&u8g, 0, 0, 128, 64);
		
		//Corner rectangles
		u8g_DrawFrame(&u8g,  0,  0, 50, 20);
		u8g_DrawFrame(&u8g, 77,  0, 50, 20);
		u8g_DrawFrame(&u8g,  0, 43, 50, 20);
		u8g_DrawFrame(&u8g, 77, 43, 50, 20);
		
		//STERO, MONO indicator
		memset(OledMsg, 0, 20);
		sprintf(OledMsg, "%s", radioInfo.Mono? "MONO" : "STEREO");
		u8g_SetFont(&u8g, u8g_font_helvB08);
		u8g_DrawStr(&u8g, 5, 14, OledMsg);
		
		//SIG indicator
		memset(OledMsg, 0, 20);
		sprintf(OledMsg, "SIG:%02d", radioInfo.RSSI);
		u8g_SetFont(&u8g, u8g_font_helvB08);
		u8g_DrawStr(&u8g, 85, 14, OledMsg);
		
		//MUTE & ERROR indicator
		if(err)
		{
			memset(OledMsg, 0, 20);
			sprintf(OledMsg, "%s", "ERROR");
		}
		else
		{
			memset(OledMsg, 0, 20);
			sprintf(OledMsg, "%s", radioInfo.DSmute ? "MUTE" : "");
		}
		
		u8g_SetFont(&u8g, u8g_font_helvB08);
		u8g_DrawStr(&u8g, 5, 57, OledMsg);
		
		//VOLUME indicator
		memset(OledMsg, 0, 20);
		sprintf(OledMsg, "VOL:%02d", radioInfo.Volume);
		u8g_SetFont(&u8g, u8g_font_helvB08);
		u8g_DrawStr(&u8g, 85, 57, OledMsg);
		
		//FREQ indicator
		memset(OledMsg, 0, 20);
		sprintf(OledMsg, "%5.1fMHz", SI4703_GetFreq());
		u8g_SetFont(&u8g, u8g_font_helvB14);
		u8g_DrawStr(&u8g, 25, 38, OledMsg);
		
	} while ( u8g_NextPage(&u8g) );
}

void HandleIRCommand()
{
	/* Check IR receive flag */
	if(!irRcv) return;	
	
	/* Toggle flag */
	irRcv = false;
	err = false;
	
	switch(IRCommand)
	{
		case 0x40:	/* Volume Up */
		if(!SI4703_SetVolume(++radioInfo.Volume)) err = true;
		break;
		case 0xc0:	/* Volume Down */
		if(!SI4703_SetVolume(--radioInfo.Volume)) err = true;
		break;
		case 0x00:	/* Channel Up */
		if(!SI4703_SeekUp()) err = true;
		break;
		case 0x80:	/* Channel Down */
		if(!SI4703_SeekDown()) err = true;
		break;
		case 0x90: /* Mute */
		if(!SI4703_SetMute(!radioInfo.Dmute)) err = true;
		break;
		case 0x4A: /* Mono, Stereo */
		if(!SI4703_SetMono(!radioInfo.Mono)) err = true;
		break;
		case 0x88:	/* Number#1 */
		if(!SI4703_SetFreq(PRESET1)) err = true;
		break;
		case 0x48:	/* Number#2 */
		if(!SI4703_SetFreq(PRESET2)) err = true;
		break;
		case 0xc8:	/* Number#3 */
		if(!SI4703_SetFreq(PRESET3)) err = true;
		break;
		case 0x28:	/* Number#4 */
		if(!SI4703_SetFreq(PRESET4)) err = true;
		break;
		case 0xa8:	/* Number#5 */
		if(!SI4703_SetFreq(PRESET5)) err = true;
		break;
		default:
		err = true;
		break;
	}	
	
	SI4703_UpdateRadioInfo();		
}

void IrRcvCallback(uint32_t code)
{			
	/* Ignore duplicated code */
	if(irRcv || !code) return;
	
	//IRAddress = (code >> 16) & 0xFFFF;
	IRCommand = (code >> 8) & 0xFF;
	irRcv = true;
}
