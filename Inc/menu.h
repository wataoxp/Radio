/*
 * menu.h
 *
 *  Created on: Aug 17, 2024
 *      Author: wataoxp
 */
#ifndef MENU_H
#define MENU_H

#include "main.h"

#define MYUSE_PIN

#ifndef MYUSE_PIN
#define CENTER_PUSH 0x10
#define LEFT_PUSH 0x08
#define RIGHT_PUSH 0x04
#define BACK_PUSH 0x02
#define STOP_PUSH 0x01
#else
#define CENTER_PUSH CENTER_Pin
#define LEFT_PUSH LEFT_Pin
#define RIGHT_PUSH RIGHT_Pin
#define BACK_PUSH BACK_Pin
#define STOP_PUSH STOP_Pin
#endif
#define PUSH_ALL CENTER_PUSH | LEFT_PUSH | RIGHT_PUSH | BACK_PUSH | STOP_PUSH
#define CHECK_ALL_PIN CENTER_Pin | LEFT_Pin | RIGHT_Pin | BACK_Pin | STOP_Pin

/* *** スイッチ入力を判別する関数 ***
 * LL関数の引数を適宜変更してください。
 * GPIOA->GPIOポート
 * CENRER_Pin->GPIOピン
 * 例えばPA0ピンを使うなら
 * LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0)
 **/
static inline uint8_t ReadInput(void)
{
#ifndef MYUSE_PIN
	uint8_t Push =
			LL_GPIO_IsInputPinSet(GPIOA, CENTER_Pin) << 4 |
			LL_GPIO_IsInputPinSet(GPIOA, LEFT_Pin) << 3 |
			LL_GPIO_IsInputPinSet(GPIOA, RIGHT_Pin) << 2 |
			LL_GPIO_IsInputPinSet(GPIOA, BACK_Pin) << 1 |
			LL_GPIO_IsInputPinSet(GPIOA, STOP_Pin) << 0 ;
#else
	uint8_t Push = (GPIOA->IDR) & (CHECK_ALL_PIN);
#endif
	Push ^= PUSH_ALL;

	return Push;
}

#define DEBOUNCE_COUNT 20

//FREQモード
//表示開始位置
#define FREQ_START_POSTION 8
//入力桁数
#define FREQ_START_CNT 0
#define FREQ_MAX_COUNT 3
//初期値
#define FREQ_DEFAULT_VALUE 7
#define FREQ_DEFAULT_ASCI '7'

#define LCD_MAXLINE_16 16

//LCD表示の切り替え用
#define CONFIG 3
typedef enum LCD_Type{
	SEEK,
	TUNE,
	FREQ,
	SkUp,
	SkDn,
	Skwait,
	SeekChan,
}Draw;
typedef struct{
	uint8_t cnt;
	uint8_t currentFreq;
	uint8_t position;
	uint8_t mode;
	uint8_t freq[FREQ_MAX_COUNT+1];
	char freqAsci;
}FreqTypedef;

//LL_LPM_EnableSleep()と同じ
static inline void LL_LPM_DisableDeepSleep(void)
{
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}
void SetHandle(I2C_TypeDef *lcdHandle, I2C_TypeDef *radioHandle,TIM_TypeDef *TIMHandle);
//メニュー、各モード制御関数
void SeekMenu(void);
void TuneMenu(void);
void FreqMenu(void);

void InitTypedef(FreqTypedef *obj);
/*
 * FreqMenuで用いる構造体の初期化を行う
 */
void FreqUpdate(FreqTypedef *obj);
/*
 * Freqの入力値の処理と分岐を行う
 */
void DispUpdate(uint8_t mode);
/*
 * 画面の更新をする関数
 * 処理分岐はenum Drawの値が引数modeに渡される
 * mode < CONFIG:各モード間の更新
 * mode == SeekChan:受信周波数の表示
 * else:シーク実行後の画面更新
 */
void ChannelDisp(void);
/*
 * 受信周波数と強度(RSSI)を取得し、文字列に変換し出力する関数
 */
uint8_t InputMenu(void);
/*
 * 入力されたボタンを判別する
 * 入力がある間は1ms毎にカウント。10回分の入力がなければチャタリングと判定する
 * また50回分のカウントがある場合は長押しと判定して抜ける
 */
void EnterSleepMode(void);
/*
 * SLEEPモードに入る
 */
void EnterStopMode(void);
/*
 * STOPモードに入る関数
 * LCDの動作電圧を下げ、バックライトを消してからSTOPモードに入る
 */
void DebugMode(I2C_TypeDef *I2Cx,GPIO_TypeDef *GPIOx,uint32_t GPIO_Pin);
void SystemClock_ReConfig(void);
/*
 * クロックの設定関数
 * 中身はmain.cのSystemClock_Configとまったく同じ
 */

/* *** ChannelDisp 古い内容 ***

void ChannelDisp(I2C_HandleTypeDef *lcdi2c, I2C_HandleTypeDef *radioi2c)
{
	static const char mhz[] = "MHz ";
	static const char dbm[] = "dBm";
	char channel[17];
	char freq[5+1];		//xxx.x\0
	char rssi[2+1];		//xx\0

	volatile uint32_t tick1,tick2,tick3,tick4;		//処理サイクルカウント保存用

	LL_TIM_EnableCounter(TIM16);				//プリスケーラ0、64MHｚ

	TIM16->CNT = 0;
	tick1 = TIM16->CNT;
	memset(freq,' ',sizeof(freq));
	memset(channel,'\0',sizeof(channel));
	memset(rssi,'0',sizeof(rssi));				//memset3つで200サイクル

	ItoS(freq,GetChan(radioi2c),4);
	ItoS(rssi,GetRSSI(radioi2c),2);				//この2つのItoSで約18000サイクル使っている

	freq[4] = freq[3];
	freq[3] = '.';
	freq[5] = '\0';

	memcpy(channel,freq,strlen(freq));
	memcpy(channel+strlen(freq),mhz,4);
	memcpy(channel+strlen(channel),rssi,2);
	memcpy(channel+strlen(channel),dbm,3);		//memcpy4つで500サイクル

	tick2 = TIM16->CNT;

	TIM16->CNT = 0;
	tick3 = TIM16->CNT;
	uint16_t chan = GetChan(radioi2c);
	char str[17];
	sprintf(str,"%3d.%dMHz %2ddBm",chan/10,chan%10,GetRSSI(radioi2c));
	tick4 = TIM16->CNT;							//21000サイクル

	PointClear(lcdi2c);
	SetCusor(lcdi2c, 1, 0);
	HAL_Delay(10);
	StringLCD(lcdi2c, channel, strlen(channel));
}

* TIM16を使って計測した結果、ItoSとsprintfに大きな差にはなりませんでした。
* Itosを使う場合の総処理サイクル数は19587。約310us(0.015us*20000)
* sprintfを使う場合の総処理サイクル数は21842。約340us
*
* またsprintfのみを記述してテストした結果が13000サイクル。
* いずれにしてもより読みやすいsprintfを今回は使うことにしています。
*
* ItoSは別のファイルに移しました。
*/

#endif
