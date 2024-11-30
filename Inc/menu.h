#ifndef MENU_H
#define MENU_H

#include "main.h"
/*
 * F446REの場合
 * XOR...PORTCの値は0x83(1000 0011)なので同じ値でXORすると常に0となる。
 * ここで入力があるとそのビットが0となり「一緒は嫌」によってそのビットのみが1となる。
 * 実際のレジスタ値は32ビットだが8ビットまで読む
 */

//G030F6P6の場合
//ピンとビットの関係はMX_GPIO_Init()内から目的のピンを探し、ctrl+クリックで辿っていく
#define CENTER_PUSH CENTER_Pin
#define LEFT_PUSH LEFT_Pin
#define RIGHT_PUSH RIGHT_Pin
#define BACK_PUSH BACK_Pin
#define STOP_PUSH STOP_Pin
//すべてのポートA入力ピンをマスクするのでFFで良い
//#define PORTA_MASK (CENTER_PUSH | BACK_PUSH | LEFT_PUSH | RIGHT_PUSH | STOP_Pin | STC_Pin | RESET_Pin | NC_Pin)

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

static inline uint8_t ReadInput(void)
{
	return (GPIOA->IDR) ^ 0xFF;
}
//LL_LPM_EnableSleep()と同じ
static inline void LL_LPM_DisableDeepSleep(void)
{
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}

//メニュー、各モード制御関数
void SeekMenu(I2C_TypeDef *lcdi2c, I2C_TypeDef *radioi2c);
void TuneMenu(I2C_TypeDef *lcdi2c, I2C_TypeDef *radioi2c);
void FreqMenu(I2C_TypeDef *lcdi2c, I2C_TypeDef *radioi2c);

void InitTypedef(FreqTypedef *obj);
/*
 * FreqMenuで用いる構造体の初期化を行う
 */
void FreqUpdate(FreqTypedef *obj);
/*
 * Freqの入力値の処理と分岐を行う
 */
void DispUpdate(I2C_TypeDef *lcdi2c, I2C_TypeDef *radioi2c, uint8_t mode);
/*
 * 画面の更新をする関数
 * 処理分岐はenum Drawの値が引数modeに渡される
 * mode < CONFIG:各モード間の更新
 * mode == SeekChan:受信周波数の表示
 * else:シーク実行後の画面更新
 */
void ChannelDisp(I2C_TypeDef *lcdi2c, I2C_TypeDef *radioi2c);
/*
 * 受信周波数と強度(RSSI)を取得し、文字列に変換し出力する関数
 * 実行サイクル数を短縮するため、ItoSとmemcpyを組み合わせている
 * やっている内容としては下記の通り。sprintfだとだいたい3倍のサイクル数になる
 * sprint(channel,"%.1fMHz RSSI:%03d",(float)GetChan()/10,GetRSSI());
 */
void ItoS(char *s, uint16_t num, uint8_t size);
/*
 * 整数を文字列に変換する関数
 * 有効な数字の桁数分のsizeを渡す。
 * freqであればドットを抜いた3ケタ。RSSIは0を含んだ3ケタ
 */
uint8_t InputMenu(I2C_TypeDef *lcdi2c);
/*
 * 入力されたボタンを判別する
 * 入力がある間は1ms毎にカウント。10回分の入力がなければチャタリングと判定する
 * また50回分のカウントがある場合は長押しと判定して抜ける
 */
void EnterSleepMode(void);
/*
 * SLEEPモードに入る
 */
void EnterStopMode(I2C_TypeDef *lcdi2c);
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
