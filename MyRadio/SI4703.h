/*
 * SI4703.h
 *
 * Created: 2018-05-29 오전 10:42:54
 *  Author: kiki
 */ 


#ifndef SI4703_H_
#define SI4703_H_

#include <avr/io.h>
#include <stdbool.h>

#define SI4703_SCLK		PC5
#define SI4703_SDIO		PC4
#define SI4703_RST		PD2		

#define SI4703_DEVICEADDR	0x10

#define REG_DEVICEID		0x00
#define REG_CHIPID			0x01
#define REG_POWERCFG		0x02
#define REG_CHANNEL			0x03
#define REG_SYSCONFIG1		0x04
#define REG_SYSCONFIG2		0x05
#define REG_SYSCONFIG3		0x06
#define REG_TEST1			0x07
#define REG_TEST2			0x08
#define REG_BOOTCONFIG		0x09
#define REG_STATUSRSSI		0x0A
#define REG_READCHAN		0x0B
#define REG_RDSA			0x0C
#define REG_RDSB			0x0D
#define REG_RDSC			0x0E
#define REG_RDSD			0x0F

#define MASK_PN				0xF000
#define MASK_MFGID			0x0FFF
#define MASK_REV			0xFC00
#define MASK_DEV			0x03C0
#define MASK_FIRMWARE		0x003F
#define MASK_DSMUTE			0x8000
#define MASK_DMUTE			0x4000
#define MASK_MONO			0x2000
#define MASK_RDSM			0x0800
#define MASK_SKMODE			0x0400
#define MASK_SEEKUP			0x0200
#define MASK_SEEK			0x0100
#define MASK_DISABLE		0x0040
#define MASK_ENABLE			0x0001
#define MASK_TUNE			0x8000
#define MASK_CHAN			0x03FF
#define MASK_RDSIEN			0x8000
#define MASK_STCIEN			0x4000
#define MASK_RDS			0x1000
#define MASK_DE				0x0800
#define MASK_AGCD			0x0400
#define MASK_BLNDADJ		0x00C0
#define MASK_GPIO3			0x0030
#define MASK_GPIO2			0x000C
#define MASK_GPIO1			0x0003
#define MASK_SEEKTH			0xFF00
#define MASK_BAND			0x00C0
#define MASK_SPACE			0x0030
#define MASK_VOLUME			0x000F
#define MASK_SMUTER			0xC000
#define MASK_SMUTEA			0x3000
#define MASK_VOLEXT			0x0100
#define MASK_SKSNR			0x00F0
#define MASK_SKCNT			0x000F
#define MASK_XOSCEN			0x8000
#define MASK_AHIZEN			0x4000
#define MASK_RDSR			0x8000
#define MASK_STC			0x4000
#define MASK_SFBL			0x2000
#define MASK_AFCRL			0x1000
#define MASK_RDSS			0x0800
#define MASK_BLERA			0x0600
#define MASK_ST				0x0100
#define MASK_RSSI			0x00FF
#define MASK_BLERB			0xC000
#define MASK_BLERC			0x3000
#define MASK_BLERD			0x0C00
#define MASK_READCHAN		0x03FF

#define IDX_DSMUTE			15
#define IDX_DMUTE			14
#define IDX_MONO			13
#define IDX_RDSM			11
#define IDX_SKMODE			10
#define IDX_SEEKUP			9
#define IDX_SEEK			8
#define IDX_DISABLE			6
#define IDX_ENABLE			0
#define IDX_TUNE			15
#define IDX_RDSIEN			15
#define IDX_STCIEN			14
#define IDX_RDS				12
#define IDX_DE				11
#define IDX_AGCD			10
#define IDX_BAND0			6			
#define IDX_BAND1			7
#define IDX_SPACE0			4
#define IDX_SPACE1			5
#define IDX_SMUTER0			14
#define IDX_SMUTER1			15
#define IDX_SMUTEA0			12
#define IDX_SMUTEA1			13
#define IDX_VOLEXT			8
#define IDX_XOSCEN			15
#define IDX_AHIZEN			14
#define IDX_RDSR			15
#define IDX_STC				14
#define IDX_SFBL			13
#define IDX_AFCRL			12
#define IDX_RDSS			11
#define IDX_ST				8

typedef struct
{
	uint8_t PartNumber;
	uint16_t ManufactID;
	uint8_t Revision;
	uint8_t Device;
	uint8_t Firmware;
	bool DSmute;
	bool Dmute;
	bool Mono;
	bool RDSMode;
	bool SeekMode;
	bool SeekUp;
	bool Seek;
	bool Disable;
	bool Enable;
	bool Tune;
	uint16_t Channel;
	bool RDSIntEnable;
	bool STCIntEnable;
	bool RDS;
	bool DeEmpahsis;
	bool AGCDisable;
	uint8_t BlendLevelAdjust;
	uint8_t GPIO3;
	uint8_t GPIO2;
	uint8_t GPIO1;
	uint16_t SeekThreshold;
	uint8_t BandSelect;
	uint8_t ChannelSpace;	
	uint8_t SoftmuteRate;
	uint8_t SoftmuteAttenuation;
	uint8_t Volume;
	bool ExtendedVolumeRange;
	uint8_t SNRThreshold;
	uint8_t ImpulseThreshold;
	bool XOSCEnable;
	bool AudioHiZEnable;
	bool RDSReady;
	bool SeekTuneComplete;
	bool SeekFailBandLimit;
	bool AFCRail;
	bool RDSSynchronized;	
	bool Stereo;
	uint8_t RSSI;
	uint8_t RDSBlockAError;
	uint8_t RDSBlockBError;
	uint8_t RDSBlockCError;
	uint8_t RDSBlockDError;
	uint16_t ReadChannel;
	uint16_t RDSA;
	uint16_t RDSB;
	uint16_t RDSC;
	uint16_t RDSD;
} _radioInfo;

extern _radioInfo radioInfo;

bool SI4703_Init(void);
bool SI4703_SetVolume(uint8_t volume);
bool SI4703_SetMono(bool mono);
bool SI4703_SetMute(bool dmute);
float SI4703_GetFreq();
bool SI4703_SetFreq(float freq);
bool SI4703_SeekUp(void);
bool SI4703_SeekDown(void);
bool SI4703_UpdateRadioInfo(void);
bool SI4703_CheckRDSReady(void);

#endif /* SI4703_H_ */