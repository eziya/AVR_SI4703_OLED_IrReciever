/*
 * SI4703.c
 *
 * Created: 2018-05-29 오전 10:43:04
 *  Author: kiki
 */ 

#ifndef F_CPU
#define F_CPU	16000000L
#endif

#include <util/delay.h>

#include "SI4703.h"
#include "328P_TWI.h"

_radioInfo radioInfo;
uint16_t SI4703_Regs[16] = {0,};

static bool SI4703_Wait(void);
static bool SI4703_RxRegs(void);
static bool SI4703_TxRegs(void);
static void SI4703_Reset(void);

bool SI4703_Init()
{			
	/* OUTPUT SDIO, RST */	
	DDRC |= _BV(SI4703_SDIO) | _BV(SI4703_SCLK);
	DDRD |= _BV(SI4703_RST);
				
	/* SDIO Low */	
	PORTC &= ~_BV(SI4703_SDIO);
				
	/* SI4703 Reset */
	SI4703_Reset();
	
	/* Init TWI(I2C) */
	TWI_Init();
			
	if(!SI4703_RxRegs()) return false;
	
	/* Enable Oscillator  */
	SI4703_Regs[REG_TEST1] = 0x8100;		/* AN230 page 12, why 0x8100 ??? */		
	if(!SI4703_TxRegs()) return false;
	
	/* Delay minimum 500 ms, AN230 page 12*/
	_delay_ms(500);
	
	if(!SI4703_RxRegs()) return false;
	
	/* Power-up sequence */
	SI4703_Regs[REG_POWERCFG] |= (1 << IDX_DMUTE);
	SI4703_Regs[REG_POWERCFG] |= (1 << IDX_ENABLE);
		
	/* Set Force Mode for single speaker */
	//SI4703_Regs[REG_POWERCFG] |= (1 << IDX_MONO);
	
	/* Set Seek Mode as Stop at band limit */
	//SI4703_Regs[REG_POWERCFG] |= (1 << IDX_SKMODE);
	
	/* Enable RDS */
	SI4703_Regs[REG_SYSCONFIG1] |= (1 << IDX_RDS);
	
	/* Set De-Emphasis 75us (Korea) */
	SI4703_Regs[REG_SYSCONFIG1] &= ~(1 << IDX_DE);
	
	/* Set Band as 00 (Korea) */
	SI4703_Regs[REG_SYSCONFIG2] &= ~((1 << IDX_BAND0)|(1 << IDX_BAND1));
	
	/* Set Space as 00 (Korea) */
	SI4703_Regs[REG_SYSCONFIG2] &= ~((1 << IDX_SPACE0)|(1 << IDX_SPACE1));
			
	/* Set Volume as 0x0F */
	SI4703_Regs[REG_SYSCONFIG2] &= 0xFFF0;
	SI4703_Regs[REG_SYSCONFIG2] |= 0x0F;
	
	/* Set Seek Threshold, Recommended 0x19 */
	SI4703_Regs[REG_SYSCONFIG2] &= ~(MASK_SEEKTH);
	SI4703_Regs[REG_SYSCONFIG2] |= (0x0C << 8);
	
	/* Set SKSNR, Recommended 0x04 */
	SI4703_Regs[REG_SYSCONFIG3] &= ~(MASK_SKSNR);	
	SI4703_Regs[REG_SYSCONFIG3] |= (0x04 << 4);	
	
	/* Set SKCNT, Recommended 0x08 */
	SI4703_Regs[REG_SYSCONFIG3] &= ~(MASK_SKCNT);	
	SI4703_Regs[REG_SYSCONFIG3] |= 0x0F;	
	
	if(!SI4703_TxRegs()) return false;
	
	/* Wait Powerup Time(110ms), Datasheet page 13 */
	_delay_ms(110);
	
	return true;
}

bool SI4703_SetVolume(uint8_t volume)
{
	if(volume > 15) volume = 15;
	
	if(!SI4703_RxRegs()) return false;
	
	/* Set Volume */
	SI4703_Regs[REG_SYSCONFIG2] &= 0xFFF0;
	SI4703_Regs[REG_SYSCONFIG2] |= volume;
	
	if(!SI4703_TxRegs()) return false;
	
	return true;
}

bool SI4703_SetMono(bool mono)
{
	if(!SI4703_RxRegs()) return false;
	
	/* Set Mono or Stereo */
	if(mono)
	{
		SI4703_Regs[REG_POWERCFG] |= (1 << IDX_MONO);		
	}
	else
	{
		SI4703_Regs[REG_POWERCFG] &= ~(1 << IDX_MONO);
	}
	
	if(!SI4703_TxRegs()) return false;
	
	return true;
}

bool SI4703_SetMute(bool dmute)
{
	if(!SI4703_RxRegs()) return false;
	
	/* Set Mute */
	if(!dmute)
	{
		SI4703_Regs[REG_POWERCFG] &= ~(1 << IDX_DMUTE);		
	}
	else
	{
		SI4703_Regs[REG_POWERCFG] |= (1 << IDX_DMUTE);
	}
	
	if(!SI4703_TxRegs()) return false;
	
	return true;
}

float SI4703_GetFreq()
{
	float freq;
	
	if(!SI4703_RxRegs()) return false;
	
	uint16_t channel = SI4703_Regs[REG_READCHAN] & MASK_READCHAN;
	
	/* F = (S x C) + L */
	freq = (0.2 * channel) + 87.5;	/* Band rage is between 87.5 and 108MHz, Channel Space is 0.2MHz for Korea */
		
	return freq;
}

bool SI4703_SetFreq(float freq)
{
	if(freq < 87.5) freq = 87.5;
	if(freq > 108.0) freq = 108.0;
	
	/* C = (F - L) / S */
	uint16_t channel = (freq - 87.5) / 0.2;
	
	if(!SI4703_RxRegs()) return false;
	
	/* Update frequency */
	SI4703_Regs[REG_CHANNEL] &= 0xFE00;
	SI4703_Regs[REG_CHANNEL] |= channel;
	SI4703_Regs[REG_CHANNEL] |= (1 << IDX_TUNE);
	
	if(!SI4703_TxRegs()) return false;	
	
	/* Wait STC bit set & clear */
	if(!SI4703_Wait()) return false;
	
	return true;
}

bool SI4703_SeekUp()
{
	if(!SI4703_RxRegs()) return false;
	
	/* Set SEEKUP + SEEK bit */
	SI4703_Regs[REG_POWERCFG] |= (1 << IDX_SEEKUP);
	SI4703_Regs[REG_POWERCFG] |= (1 << IDX_SEEK);
	
	if(!SI4703_TxRegs()) return false;
	
	/* Wait STC bit set & clear */
	if(!SI4703_Wait()) return false;
	
	return true;
}

bool SI4703_SeekDown()
{
	if(!SI4703_RxRegs()) return false;
	
	/* Clear SEEKUP + SEEK bit */
	SI4703_Regs[REG_POWERCFG] &= ~(1 << IDX_SEEKUP);
	SI4703_Regs[REG_POWERCFG] |= (1 << IDX_SEEK);
	
	if(!SI4703_TxRegs()) return false;
	
	/* Wait STC bit set & clear */
	if(!SI4703_Wait()) return false;
	
	return true;
}

bool SI4703_CheckRDSReady()
{
	if(!SI4703_RxRegs()) return false;
	
	return (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_RDSR) >> 15);
}

static bool SI4703_Wait(void)
{
	uint8_t timeout = 0;
	
	while(1)
	{
		if(!SI4703_RxRegs()) return false;		
		if((SI4703_Regs[REG_STATUSRSSI] & MASK_STC) != 0) break;
		_delay_ms(60);	/* Seek or Tune Time Delay */
		
		timeout++;
		if(timeout > 40) return false;
	}
	
	if(SI4703_Regs[REG_STATUSRSSI] & MASK_SFBL)
	{
		/* Clear Error */
		SI4703_Regs[REG_POWERCFG] &= ~(1 << IDX_SEEK);
		SI4703_Regs[REG_CHANNEL] &= ~(1 << IDX_TUNE);	
		if(!SI4703_TxRegs()) return false;	
		return false;
	}
	
	timeout = 0;
	
	while(1)
	{
		SI4703_Regs[REG_POWERCFG] &= ~(1 << IDX_SEEK);
		SI4703_Regs[REG_CHANNEL] &= ~(1 << IDX_TUNE);		
		if(!SI4703_TxRegs()) return false;
		
		if(!SI4703_RxRegs()) return false;
		if((SI4703_Regs[REG_STATUSRSSI] & MASK_STC) == 0) break;
		_delay_ms(60);	/* Seek or Tune Time Delay */
		
		timeout++;
		if(timeout > 40) return false;
	}
	
	return true;
}

static bool SI4703_RxRegs()
{
	uint8_t buffer[32];
	
	if(!TWI_RxBuffer(SI4703_DEVICEADDR, buffer, sizeof(buffer))) return false;
	
	/* Cautions!!! Si4703 returns data from register 0x0A */
	for(int i=0; i < 6; i++)
	{
		SI4703_Regs[10+i] = (buffer[i*2] << 8) | buffer[i*2+1];		
	}
	
	for(int i=0; i < 10; i++)
	{
		SI4703_Regs[i] = (buffer[12+2*i] << 8) | buffer[13+2*i];	
	}		
	
	return true;
}

static bool SI4703_TxRegs()
{
	uint8_t buffer[12];
		
	buffer[0] = SI4703_Regs[REG_POWERCFG] >> 8;
	buffer[1] = SI4703_Regs[REG_POWERCFG] & 0xFF;
	buffer[2] = SI4703_Regs[REG_CHANNEL] >> 8;
	buffer[3] = SI4703_Regs[REG_CHANNEL] & 0xFF;
	buffer[4] = SI4703_Regs[REG_SYSCONFIG1] >> 8;
	buffer[5] = SI4703_Regs[REG_SYSCONFIG1] & 0xFF;
	buffer[6] = SI4703_Regs[REG_SYSCONFIG2] >> 8;
	buffer[7] = SI4703_Regs[REG_SYSCONFIG2] & 0xFF;
	buffer[8] = SI4703_Regs[REG_SYSCONFIG3] >> 8;
	buffer[9] = SI4703_Regs[REG_SYSCONFIG3] & 0xFF;
	buffer[10] = SI4703_Regs[REG_TEST1] >> 8;
	buffer[11] = SI4703_Regs[REG_TEST1] & 0xFF;
	
	if(!TWI_TxBuffer(SI4703_DEVICEADDR, buffer, sizeof(buffer))) return false;
	
	return true;
}

static void SI4703_Reset(void)
{
	PORTD &= ~(1 << SI4703_RST);
	_delay_ms(10);							/* Min 100us */
	PORTD |= (1 << SI4703_RST);
	_delay_ms(10);							/* Min 100us */
}

bool SI4703_UpdateRadioInfo()
{
	if(!SI4703_RxRegs()) return false;
	
	radioInfo.PartNumber = (SI4703_Regs[REG_POWERCFG] & MASK_PN) >> 12;
	radioInfo.ManufactID = (SI4703_Regs[REG_POWERCFG] & MASK_MFGID);
	radioInfo.Revision = (SI4703_Regs[REG_CHIPID] & MASK_REV) >> 10;
	radioInfo.Device = (SI4703_Regs[REG_CHIPID] & MASK_DEV) >> 6;
	radioInfo.Firmware = (SI4703_Regs[REG_CHIPID] & MASK_FIRMWARE);
	radioInfo.DSmute = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_DSMUTE) >> 15);
	radioInfo.Dmute = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_DMUTE) >> 14);
	radioInfo.Mono = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_MONO) >> 13);
	radioInfo.RDSMode = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_RDSM) >> 11);
	radioInfo.SeekMode = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_SKMODE) >> 10);
	radioInfo.SeekUp = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_SEEKUP) >> 9);
	radioInfo.Seek = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_SEEK) >> 8);
	radioInfo.Disable = (bool)((SI4703_Regs[REG_POWERCFG] & MASK_DISABLE) >> 6);
	radioInfo.Enable = (bool)(SI4703_Regs[REG_POWERCFG] & MASK_ENABLE);
	radioInfo.Tune = (bool)((SI4703_Regs[REG_CHANNEL] & MASK_TUNE) >> 15);
	radioInfo.Channel = (SI4703_Regs[REG_CHANNEL] & MASK_CHAN);
	radioInfo.RDSIntEnable = (bool)((SI4703_Regs[REG_SYSCONFIG1] & MASK_RDSIEN) >> 15);
	radioInfo.STCIntEnable = (bool)((SI4703_Regs[REG_SYSCONFIG1] & MASK_STCIEN) >> 14);
	radioInfo.RDS = (bool)((SI4703_Regs[REG_SYSCONFIG1] & MASK_RDS) >> 12);
	radioInfo.DeEmpahsis = (bool)((SI4703_Regs[REG_SYSCONFIG1] & MASK_DE) >> 11);
	radioInfo.AGCDisable = (bool)((SI4703_Regs[REG_SYSCONFIG1] & MASK_AGCD) >> 10);
	radioInfo.BlendLevelAdjust = (SI4703_Regs[REG_SYSCONFIG1] & MASK_BLNDADJ) >> 6 ;
	radioInfo.GPIO3 = (SI4703_Regs[REG_SYSCONFIG1] & MASK_GPIO3) >> 4;
	radioInfo.GPIO2 = (SI4703_Regs[REG_SYSCONFIG1] & MASK_GPIO2) >> 2;
	radioInfo.GPIO1 = (SI4703_Regs[REG_SYSCONFIG1] & MASK_GPIO1);
	radioInfo.SeekThreshold = (SI4703_Regs[REG_SYSCONFIG2] & MASK_SEEKTH) >> 8;
	radioInfo.BandSelect = (SI4703_Regs[REG_SYSCONFIG2] & MASK_BAND) >> 6;
	radioInfo.ChannelSpace = (SI4703_Regs[REG_SYSCONFIG2] & MASK_SPACE) >> 4;
	radioInfo.Volume = (SI4703_Regs[REG_SYSCONFIG2] & MASK_VOLUME);
	radioInfo.SoftmuteRate = (SI4703_Regs[REG_SYSCONFIG3] & MASK_SMUTER) >> 14;
	radioInfo.SoftmuteAttenuation = (SI4703_Regs[REG_SYSCONFIG3] & MASK_SMUTEA) >> 12;
	radioInfo.ExtendedVolumeRange = (bool)((SI4703_Regs[REG_SYSCONFIG3] & MASK_VOLEXT) >> 8);
	radioInfo.SNRThreshold = (SI4703_Regs[REG_SYSCONFIG3] & MASK_SKSNR) >> 4;
	radioInfo.ImpulseThreshold = (SI4703_Regs[REG_SYSCONFIG3] & MASK_SKCNT);
	radioInfo.XOSCEnable = (bool)((SI4703_Regs[REG_TEST1] & MASK_XOSCEN) >> 15);
	radioInfo.AudioHiZEnable = (bool)((SI4703_Regs[REG_TEST1] & MASK_AHIZEN) >> 14);
	radioInfo.RDSReady = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_RDSR) >> 15);
	radioInfo.SeekTuneComplete = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_STC) >> 14);
	radioInfo.SeekFailBandLimit = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_SFBL) >> 13);
	radioInfo.AFCRail = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_AFCRL) >> 12);
	radioInfo.RDSSynchronized = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_RDSS) >> 11);
	radioInfo.RDSBlockAError = (SI4703_Regs[REG_STATUSRSSI] & MASK_BLERA) >> 9;
	radioInfo.Stereo = (bool)((SI4703_Regs[REG_STATUSRSSI] & MASK_ST) >> 8);
	radioInfo.RSSI = (SI4703_Regs[REG_STATUSRSSI] & MASK_RSSI);
	radioInfo.RDSBlockBError = (SI4703_Regs[REG_READCHAN] & MASK_BLERB) >> 14;
	radioInfo.RDSBlockCError = (SI4703_Regs[REG_READCHAN] & MASK_BLERC) >> 12;
	radioInfo.RDSBlockDError = (SI4703_Regs[REG_READCHAN] & MASK_BLERD) >> 10;
	radioInfo.ReadChannel = (SI4703_Regs[REG_READCHAN] & MASK_READCHAN);
	radioInfo.RDSA = SI4703_Regs[REG_RDSA];
	radioInfo.RDSB = SI4703_Regs[REG_RDSB];
	radioInfo.RDSC = SI4703_Regs[REG_RDSC];
	radioInfo.RDSD = SI4703_Regs[REG_RDSD];
	
	return true;
}