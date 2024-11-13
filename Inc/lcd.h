/*
 * SPLC792A用
 * ST7032互換だが、Internal OSC freq(内部周波数設定)がない
 * また電圧値はデフォルトを使用するので、Initでは送らない
 */
#ifndef LCD_H
#define LCD_H

#include "main.h"
#include "ll_i2c.h"

#define LCD_ADDRESS (0x3E << 1)
#define CMD_CTRL 0x00
#define DATA_CTRL 0x40	//CGRAMへのアクセス。CGRAM内に文字コードがある

//LCD更新コマンド
#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02
#define SET_CUSOR 0x80
//描画位置の指定用マクロ
#define ENTER_CUSOR 0x40	//16*2モードでは0x40が2行目の左端。2行使用する場合は5*8ドット固定？
#define HOME_CUSOR 0x03

//LCD表示コマンド
#define DISP_ON 0x04
#define CUSOR_ON 0x02
#define BLINK_CUSOR 0x01
#define DISP_CMD 0x08 | DISP_ON

//自作パターン用
#define CGRAM_ACCESS 0x40	//0x40(01xxx.xxxx)以下の6ビットで自作パターン格納先を指定する
//DDRAMアクセス。描画位置が記憶される
#define DDRAM_ACCESS 0x80	//0x80以下の7ビットで次の描画位置を指定する

//表示の設定、拡張モードの制御
#define FUNCTION_SET_ON 0x39	//8bitモード、2行表示、5*7ドット文字、拡張モードを利用
#define FUNCTION_SET_OFF 0x38	//拡張モードの設定を終了

//拡張モード
//アイコン表示オフ。コントラスト設定上位2ビット
#define CONTRAST_HIGH 0x03
#define ICON_CONTRAST_CMD 0x50 | CONTRAST_HIGH
//コントラスト最大値
#define CONTRAST_LOW 0x0f
#define CONTRAST_CMD 0x70 | CONTRAST_LOW
//駆動電圧の設定。コントラスト最大値なら0x63。これはデフォルト値
#define VOLTAGE_SET 0x03
#define VOLTAGE_CMD 0x60 | VOLTAGE_SET

//省電力モード用。コントラストや電圧をすべて0にする
#define CONTRAST_HIGH_OFF 0x50
#define CONTRAST_LOW_OFF 0x70
#define LOW_VOLTAGE_SET 0x60 | 0x00

static inline void StringLCD(I2C_TypeDef *I2Cx, const char *str, uint8_t size)
{
	//StreamI2C_Mem_Write(I2Cx, LCD_ADDRESS, (uint8_t *)str, DATA_CTRL, size);
	StreamI2C_Mem_Write(I2Cx, LCD_ADDRESS, (uint8_t*)str, DATA_CTRL, I2C_MEMADD_SIZE_8BIT, size);
}
static inline void SetCusor(I2C_TypeDef *I2Cx, uint8_t pos)
{
	//PushI2C_Mem_Write(I2Cx,LCD_ADDRESS,(DDRAM_ACCESS | pos), CMD_CTRL);
	PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, (DDRAM_ACCESS | pos), CMD_CTRL, I2C_MEMADD_SIZE_8BIT);
}
static inline void ClearLCD(I2C_TypeDef *I2Cx)
{
	//PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, CLEAR_DISPLAY, CMD_CTRL);
	PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, CLEAR_DISPLAY, CMD_CTRL, I2C_MEMADD_SIZE_8BIT);
}
static inline void CMDSend(I2C_TypeDef *I2Cx, uint8_t cmd)
{
	//PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, cmd, CMD_CTRL);
	PushI2C_Mem_Write(I2Cx, LCD_ADDRESS, cmd, CMD_CTRL, I2C_MEMADD_SIZE_8BIT);
}

void LCDInit(I2C_TypeDef *I2Cx);
void PointClear(I2C_TypeDef *I2Cx);
/*
 * LCDの1行のみを消す関数
 * コマンド0x02を送信し、カーソルを0に戻した上でスペース(LCD内文字コードを参照)となる0xA0を送信して上書きする
 * 仮に2行目を消したいときはSetCusorを使うと良い
 */
//void StringLCD(I2C_TypeDef *I2Cx, const char *str, uint8_t size);

#endif
