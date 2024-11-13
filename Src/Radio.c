#include "Radio.h"
#include "main.h"
#include "ll_i2c.h"

void RadioInit(I2C_TypeDef *I2Cx)
{
	uint16_t config = FLG_DHIZ | FLG_DMUTE | FLG_BASS
				| FLG_NEW | FLG_ENABLE;
	uint8_t init[] = {
			(config >> 8), (config & 0xff),
			0x00,BAND_WORLD,
			(GPIO_INT_OFF >> 8), (GPIO_INT_OFF & 0xff),
			(VOLUME_DEFAULT >> 8), (VOLUME_DEFAULT & 0xff),};

	I2C_Master_Transmit(I2Cx, RADIO_ADDR_SEQ, init, sizeof(init));
}
void RadioTune(I2C_TypeDef *I2Cx,uint16_t chan)
{
	uint16_t tune = (chan << CHAN_SHIFT) | BAND_WORLD | FLG_TUNE;
	uint8_t tune_buf[2] = {(uint8_t)(tune >> 8 ),tune & 0xff};

	StreamI2C_Mem_Write(I2Cx, RADIO_ADDR_PUSH, tune_buf, REG_TUNING, I2C_MEMADD_SIZE_8BIT, sizeof(tune_buf));
}
void Seek(I2C_TypeDef *I2Cx ,uint16_t seekmode)
{
	uint8_t gpio_buf[2] = {(GPIO_INT_ON >> 8),GPIO_INT_ON & 0xff};
	uint16_t SeekConfig = FLG_DHIZ | FLG_DMUTE | FLG_BASS | seekmode | FLG_SEEK | FLG_NEW | FLG_ENABLE;
	uint8_t seek_buf[2] = {(uint8_t)(SeekConfig >> 8),SeekConfig & 0xff};

	__disable_irq();

	StreamI2C_Mem_Write(I2Cx, RADIO_ADDR_PUSH, gpio_buf, REG_GPIO, I2C_MEMADD_SIZE_8BIT, sizeof(gpio_buf));
	StreamI2C_Mem_Write(I2Cx, RADIO_ADDR_PUSH, seek_buf, REG_CONFIG, I2C_MEMADD_SIZE_8BIT, sizeof(seek_buf));

	while(LL_GPIO_IsInputPinSet(STC_GPIO_Port, STC_Pin) != RESET){}

	__enable_irq();
	//シーク後はGPIO2をハイZにしておく
	gpio_buf[1] = GPIO_INT_OFF & 0xff;
	StreamI2C_Mem_Write(I2Cx, RADIO_ADDR_PUSH, gpio_buf, REG_GPIO, I2C_MEMADD_SIZE_8BIT, sizeof(gpio_buf));
}
uint16_t GetChan(I2C_TypeDef *I2Cx)
{
	uint8_t Read[2];
	uint16_t ReadFreq;

	I2C_Mem_Read(I2Cx, RADIO_ADDR_PUSH, Read, REG_STATUS, I2C_MEMADD_SIZE_8BIT, sizeof(Read));
	ReadFreq = ((Read[0] << 8) | (Read[1] & 0xFF)) & CHAN_MASK;

	//算出式はRadio.hを参照
	//ReadFreq = (read_freq * 10 )+ 7600なので両辺を10で割ってReadFreq+760
	ReadFreq += 760;

	return ReadFreq;
}
uint8_t GetRSSI(I2C_TypeDef *I2Cx)
{
	uint8_t Rssi[2];
	uint16_t RssiValue;

	I2C_Mem_Read(I2Cx, RADIO_ADDR_PUSH, Rssi, REG_RSSI, I2C_MEMADD_SIZE_8BIT, sizeof(Rssi));
	RssiValue = ((Rssi[0] << 8) | (Rssi[1] & 0xFF)) & RSSI_MASK;
	RssiValue >>= RSSI_SHIFT;

	return (uint8_t)RssiValue;
}
