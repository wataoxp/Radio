/*
 * ll_i2c.h
 *
 *  Created on: Nov 5, 2024
 *      Author: wataoxp
 */

#ifndef INC_LL_I2C_H_
#define INC_LL_I2C_H_
#include "main.h"



//1になるまで待つ
#define TXE_READY 1
#define RXNE_READY 1
#define STOPF_COMPLETE 1
#define TC_COMPLETE 1

//1の間待つ
#define BUSY_FLAG 1

//I2Cスレーブメモリアドレス幅(byte)
#define I2C_MEMADD_SIZE_8BIT UINT8_C(1)
#define I2C_MEMADD_SIZE_16BIT UINT8_C(2)

/**
 * I2C_CR2 Register Config
 * @param I2Cx...I2C Instance
 * @param address...SlaveAddress
 * @param direction...I2C Transmission Direction
 * 	@arg LL_I2C_REQUEST_WRITE
 * 	@arg LL_I2C_REQUEST_READ
 * @param length...Transmission Size
 * @param EndMode...REPEAT STRATorSTOPF
 * 	@arg LL_I2C_MODE_SOFTEND...ReStart
 * 	@arg LL_I2C_MODE_AUTOEND...STOPF
 **/
static inline void CR2SetUP(I2C_TypeDef *I2Cx,uint8_t address,uint16_t direction,uint8_t length,uint32_t EndMode)
{
	uint32_t tmp = 0;
	tmp = (address & I2C_CR2_SADD)  | (direction & I2C_CR2_RD_WRN) | I2C_CR2_START | (length << I2C_CR2_NBYTES_Pos) | (EndMode & I2C_CR2_AUTOEND);

	MODIFY_REG(I2Cx->CR2,I2C_CR2_SADD|I2C_CR2_RD_WRN|I2C_CR2_NBYTES|I2C_CR2_START|I2C_CR2_AUTOEND,tmp);
}

void PushI2C_Mem_Write(I2C_TypeDef *I2Cx,uint8_t address,uint8_t data,uint16_t Reg,uint8_t RegSize);
/**
 * Send single data
 * @param I2Cx...I2C Instance
 * @param address...SlaveAddress
 * @param data...SendData
 * @param Reg...SlaveRegisterAddress
 * @param RegSize...RegisterSize
 * 		@arg I2C_MEMADD_SIZE_8BIT
 * 		@arg I2C_MEMADD_SIZE_16BIT
 **/
void StreamI2C_Mem_Write(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *data,uint16_t Reg,uint8_t RegSize,uint8_t length);
/**
 * Send Stream data
 * @param I2Cx...I2C Instance
 * @param address...SlaveAddress
 * @param data...Databuffer Address
 * @param Reg...SlaveRegisterAddress
 * @param RegSize...RegisterSize
 * 		@arg I2C_MEMADD_SIZE_8BIT
 * 		@arg I2C_MEMADD_SIZE_16BIT
 * @param length...Send Datasize
 **/
void I2C_Master_Transmit(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *data,uint8_t length);
/**
 * Send Sequential data. No Register pointing
 * @param I2Cx...I2C Instance
 * @param address...SlaveAddress
 * @param data...Databuffer Address
 * @param length...Send Datasize
 */
void I2C_Mem_Read(I2C_TypeDef *I2Cx,uint8_t address,uint8_t *buffer,uint16_t Reg,uint8_t RegSize,uint8_t length);
/**
 * Read Data
 * @param I2Cx...I2C Instance
 * @param address...SlaveAddress
 * @param buffer...ReadDatabuffer Address
 * @param Reg...SlaveRegisterAddress
 * @param RegSize...RegisterSize
 * 		@arg I2C_MEMADD_SIZE_8BIT
 * 		@arg I2C_MEMADD_SIZE_16BIT
 * @param length...Read Datasize
 **/




#endif /* INC_LL_I2C_H_ */
