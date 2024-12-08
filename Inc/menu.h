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
 * 実行サイクル数を短縮するため、ItoSとmemcpyを組み合わせている
 * やっている内容としては下記の通り。sprintfだとだいたい3倍のサイクル数になる
 * sprint(channel,"%.1fMHz RSSI:%03d",(float)GetChan()/10,GetRSSI());
 */
void ItoS(char *buffer,uint16_t value,uint8_t digit);
/*
 * 整数を文字列に変換する関数
 * 周波数99.9MHzまでを有効な変換対象とします。
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
void SystemClock_ReConfig(void);
/*
 * クロックの設定関数
 * 中身はmain.cのSystemClock_Configとまったく同じ
 */

#endif
