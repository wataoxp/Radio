/*
 * Radio.h
 *
 *  Created on: Noc 30, 2024
 *      Author: wataoxp
 */
#include "Radio.h"
#include "main.h"
#include "ll_i2c.h"

typedef struct{
	uint16_t Config;
	uint16_t Tuning;
	uint16_t GpioSet;
	uint16_t VolumeSet;
}RDA_Typedef;

static RDA_Typedef RadioReg = {0};

static inline void SetRegister(I2C_TypeDef *I2Cx,uint8_t reg,uint16_t value)
{
	uint8_t WriteBuffer[2] ={
			(value >> 8),(value & 0xFF),
	};
	StreamI2C_Mem_Write(I2Cx, RADIO_ADDR_PUSH, WriteBuffer, reg, I2C_MEMADD_SIZE_8BIT, sizeof(WriteBuffer));
}
void RadioInit(I2C_TypeDef *I2Cx)
{
	RadioReg.Config = FLG_DHIZ | FLG_DMUTE | FLG_BASS | FLG_NEW | FLG_ENABLE;
	RadioReg.Tuning = BAND_WORLD;
	RadioReg.GpioSet = FLG_DE | FLG_STC | FLG_SOFT_MUTE | GPIO2_INT_OFF;
	RadioReg.VolumeSet = FLG_LNA | FLG_INT_ENABLE | (0x4 << THRESHOLD_SHIFT) | VOLUME_MASK;

	uint8_t init[] = {
			(RadioReg.Config >> 8),(RadioReg.Config & 0xFF),
			(RadioReg.Tuning >> 8),(RadioReg.Tuning & 0xFF),
			(RadioReg.GpioSet >> 8),(RadioReg.GpioSet & 0xFF),
			(RadioReg.VolumeSet >> 8),(RadioReg.VolumeSet & 0xFF),
	};
	I2C_Master_Transmit(I2Cx, RADIO_ADDR_SEQ, init, sizeof(init));
}
void Seek(I2C_TypeDef *I2Cx ,uint16_t seekmode)
{
	RadioReg.GpioSet = (RadioReg.GpioSet & ~GPIO2_INT_MASK) | GPIO2_INT_ON;
	RadioReg.Config = (RadioReg.Config & ~(FLG_SEEKUP | FLG_SEEK)) | FLG_SEEK | seekmode;

	SetRegister(I2Cx, REG_GPIO,RadioReg.GpioSet);
	SetRegister(I2Cx, REG_CONFIG,RadioReg.Config);

	__disable_irq();

	while(LL_GPIO_IsInputPinSet(STC_GPIO_Port, STC_Pin) != RESET);

	__enable_irq();

	//After the seek is completed GPIO2 Reset
	RadioReg.GpioSet = (RadioReg.GpioSet & ~GPIO2_INT_MASK) | GPIO2_INT_OFF;
	SetRegister(I2Cx, REG_GPIO, RadioReg.GpioSet);
}
void RadioTune(I2C_TypeDef *I2Cx,uint16_t chan)
{
	RadioReg.Tuning = (RadioReg.Tuning & ~(CHAN_WRITE_MASK | FLG_TUNE)) | (chan << CHAN_SHIFT) | FLG_TUNE;

	SetRegister(I2Cx, REG_TUNING, RadioReg.Tuning);
}
uint16_t GetRegister(I2C_TypeDef *I2Cx,uint8_t Reg)
{
	uint8_t Read[2];
	uint16_t ReadValue;

	I2C_Mem_Read(I2Cx, RADIO_ADDR_PUSH, Read, Reg, I2C_MEMADD_SIZE_8BIT, sizeof(Read));

	ReadValue = (Read[0] << 8) | (Read[1] & 0xFF);
	return ReadValue;
}
uint16_t GetChan(I2C_TypeDef *I2Cx)
{
	uint16_t ReadFreq;

	//実数ではなく整数として取得する
	ReadFreq = GetRegister(I2Cx, REG_STATUS);
	ReadFreq &= CHAN_READ_MASK;
	ReadFreq += 760;

	return ReadFreq;
}
uint8_t GetRSSI(I2C_TypeDef *I2Cx)
{
	uint16_t ReadRssi;

	ReadRssi = GetRegister(I2Cx, REG_RSSI);
	ReadRssi &= RSSI_MASK;
	ReadRssi >>= RSSI_SHIFT;

	return (uint8_t)ReadRssi;
}
