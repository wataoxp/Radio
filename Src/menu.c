/*
 * menu.c
 *
 *  Created on: Aug 17, 2024
 *      Author: wataoxp
 */
#include "menu.h"
#include "main.h"
#include "lcd.h"
#include "ll_i2c.h"
#include "Radio.h"
#include <string.h>
#include <stdio.h>

//#define DEBUG_BOARD_ON
//#define SYSCLOCK_HSI

//画面上部の表示文字。DispUpdateからのみアクセスする
const char* const ModeStr[6] = {
		"SeekMode",
		"FM YOKOHAMA",
		"Freq:76.0MHz",
		"SeekRise","SeekFall",
};
//画面下部の表示文字。DispUpdateからのみアクセスする
const char* const PushStr[4] = {
		"Fall  View  Rise",
		"Prev  Enter Next",
		"-Hz   Enter  +Hz",
};
//TUNE時の上部表示文字
const char* const StationName[] = {
		"FM YOKOHAMA",
		"TOKYO FM",
		"bayfm",
		"interFM",
		"J-WAVE",
		"NHK FM TOKYO",
		"NACK5",
		"BunkaHousou",
		"NipponHousou",
};
//TUNE時に利用する周波数のリスト。Radio.hで定義
const uint16_t ChanStation[] = {
		FM_YOKOHAMA,
		FM_TOKYO,
		FM_CHIBA,
		FM_INTER,
		FM_JWAVE,
		FM_SHONAN,
		//FM_NHK_TOKYO,
		FM_NACK5,
		FM_BUNKA,
		FM_NIPPON,
};
static I2C_TypeDef *lcdi2c;
static I2C_TypeDef *radioi2c;
static TIM_TypeDef *TIMx;

const uint8_t StationNum = sizeof(ChanStation) / sizeof(uint16_t);

void SetHandle(I2C_TypeDef *lcdHandle, I2C_TypeDef *radioHandle,TIM_TypeDef *TIMHandle)
{
	lcdi2c = lcdHandle;
	radioi2c = radioHandle;
	TIMx = TIMHandle;
}

void SeekMenu(void)
{
	uint8_t skmode;
	uint8_t flag_sk = 1;
	uint8_t type = SEEK;

	DispUpdate(SEEK);
	while(flag_sk)
	{
		skmode = InputMenu();

		switch(skmode)
		{
		case CENTER_PUSH:
			type = SeekChan;
			break;
		case LEFT_PUSH:
			DispUpdate(Skwait);
			Seek(radioi2c, FLG_SEEKDOWN);
			type = SkDn;
			break;
		case RIGHT_PUSH:
			DispUpdate(Skwait);
			Seek(radioi2c, FLG_SEEKUP);
			type = SkUp;
			break;
		case BACK_PUSH:
			flag_sk = 0;
			type = SEEK;
			break;
		default:
			continue;
		}
		DispUpdate(type);
	}
}
void TuneMenu(void)
{
	uint8_t tnmode;
	uint8_t flag_tn = 1;
	uint8_t list = 0;

	DispUpdate(TUNE);

	while(flag_tn)
	{
		tnmode = InputMenu();

		switch(tnmode)
		{
		case CENTER_PUSH:
			RadioTune(radioi2c, ChanStation[list]);
			break;
		case LEFT_PUSH:
			list = (list > 0)? list-1 : StationNum-1;
			break;
		case RIGHT_PUSH:
			list = (list < StationNum)? list+1 : 0;
			break;
		case BACK_PUSH:
			flag_tn = 0;
			break;
		default:
			continue;
		}
		PointClear(lcdi2c);
		StringLCD(lcdi2c, StationName[list], strlen(StationName[list]));
	}
}
void FreqMenu(void)
{
	static FreqTypedef obj;
	static const char CheckStr[] = "OK??";
	static const char SelectStr[] = "NO     OK     NO";
	uint8_t fqmode;
	uint8_t flag_fq = 1;
	uint16_t value = 0;	//ここ初期化しないと正しい値にならないぞ！

	InitTypedef(&obj);
	DispUpdate(FREQ);
	CMDSend(lcdi2c, (DISP_CMD | CUSOR_ON));	//カーソルを表示
	LL_mDelay(1);
	SetCusor(lcdi2c, obj.position,0);

	while(flag_fq)
	{
		fqmode = InputMenu();

		switch(fqmode)
		{
		case CENTER_PUSH:
			if(obj.cnt == 1)obj.position++;		//2文字目のときは次にドットを入れるため、次の文字をスキップさせる
			obj.freq[obj.cnt] = obj.currentFreq;
			obj.currentFreq = 0;
			obj.mode = 0;
			obj.position++;
			obj.cnt++;
			break;
		case LEFT_PUSH:
			obj.mode = LEFT_PUSH;
			break;
		case RIGHT_PUSH:
			obj.mode = RIGHT_PUSH;
			break;
		case BACK_PUSH:
			flag_fq = 0;
			CMDSend(lcdi2c, DISP_CMD);		//カーソルはこのモードでのみ表示する
			LL_mDelay(1);
			break;
		default:
			continue;
		}
		if(obj.cnt < FREQ_MAX_COUNT)
		{
			FreqUpdate(&obj);
			obj.freqAsci = obj.currentFreq + '0';
			SetCusor(lcdi2c, obj.position,0);
			StringLCD(lcdi2c, &obj.freqAsci, 1);
			SetCusor(lcdi2c, obj.position,0);
		}
		else if(obj.cnt == FREQ_MAX_COUNT)
		{
			SetCusor(lcdi2c, HOME_CUSOR,0);
			StringLCD(lcdi2c, CheckStr, strlen(CheckStr));
			SetCusor(lcdi2c, 0,1);
			StringLCD(lcdi2c, SelectStr,strlen(SelectStr));
			//NOが押されたときは元に戻す
			if(obj.mode == LEFT_PUSH || obj.mode == RIGHT_PUSH)
			{
				InitTypedef(&obj);
				DispUpdate(FREQ);
				SetCusor(lcdi2c, FREQ_START_POSTION,0);
			}
		}
		else //cntが4になったとき(CENTERが押された)のみここに来る
		{
			DispUpdate(FREQ);
			for(uint8_t i = 0; i < 3; i++)
			{
				value = value * 10 + obj.freq[i];
			}
			value = (value - 760);
			RadioTune(radioi2c, value);
			//各変数の初期化。ここで初期化しないと次の入力で再びRadio_Tuneを実行してしまう
			InitTypedef(&obj);
			value = 0;
		}
	}
}
void FreqUpdate(FreqTypedef *obj)
{
	if(obj->cnt == 0)
	{
		if(obj->mode == LEFT_PUSH){
			obj->currentFreq = (obj->currentFreq > 7)? obj->currentFreq-1 : 9;}

		else if(obj->mode == RIGHT_PUSH){
			obj->currentFreq = (obj->currentFreq < 9)? obj->currentFreq+1 : 7;}
	}
	else if(obj->cnt == 1 && obj->freq[0] != 8)
	{
		if(obj->freq[0] == 7)
		{
			if(obj->currentFreq < 6) {obj->currentFreq = 6;}	//Freqが6未満の時、これが無いと減るのみになってしまう

			if(obj->mode == LEFT_PUSH){
				obj->currentFreq = (obj->currentFreq > 6)? obj->currentFreq-1 : 9;}

			else if(obj->mode == RIGHT_PUSH){
				obj->currentFreq = (obj->currentFreq < 9)? obj->currentFreq+1 : 6;}
		}
		else if(obj->freq[0] == 9)
		{
			if(obj->mode == LEFT_PUSH){
				obj->currentFreq = (obj->currentFreq > 0)? obj->currentFreq-1 : 4;}

			else if(obj->mode == RIGHT_PUSH){
				obj->currentFreq = (obj->currentFreq < 4)? obj->currentFreq+1 : 0;}
		}
	}
	else //cntが2のとき、またはfreq[0]が8であるとき
	{
		if(obj->mode == LEFT_PUSH){
			obj->currentFreq = (obj->currentFreq > 0)? obj->currentFreq-1 : 9;}

		else if(obj->mode == RIGHT_PUSH){
			obj->currentFreq = (obj->currentFreq < 9)? obj->currentFreq+1 : 0;}
	}
}
void InitTypedef(FreqTypedef *obj)
{
	obj->cnt = FREQ_START_CNT;
	obj->currentFreq = FREQ_DEFAULT_VALUE;
	obj->position = FREQ_START_POSTION;
	obj->freqAsci = FREQ_DEFAULT_ASCI;
}
//基本的には下記の関数で画面を更新する
void DispUpdate(uint8_t mode)
{

	static const char waitSeek[] = "SeekNow";

	if(mode < CONFIG)	//ホーム、各メニュー初期画面
	{
		ClearLCD(lcdi2c);
		SetCusor(lcdi2c, HOME_CUSOR,0);
		StringLCD(lcdi2c, ModeStr[mode], strlen(ModeStr[mode]));
		SetCusor(lcdi2c, 0,1);
		StringLCD(lcdi2c, PushStr[mode], strlen(PushStr[mode]));
	}
	else if(mode == SeekChan)	//シークモード、周波数表示
	{
		ChannelDisp();
	}
	else if(mode == Skwait)
	{
		PointClear(lcdi2c);
		StringLCD(lcdi2c, waitSeek, strlen(waitSeek));
	}
	else			//シークアップ、ダウンの表示
	{
		for (uint8_t i = 0; i < 2; i++)
		{
			PointClear(lcdi2c);
			LL_mDelay(100);
			StringLCD(lcdi2c, ModeStr[mode], strlen(ModeStr[mode]));
			LL_mDelay(100);
		}
	}
}

void ChannelDisp(void)
{
	char channel[17];
	uint16_t chan = GetChan(radioi2c);

	snprintf(channel,sizeof(channel),"%3d.%dMHz %2ddBm",chan/10,chan%10,GetRSSI(radioi2c));

	PointClear(lcdi2c);
	SetCusor(lcdi2c, 1, 0);
	LL_mDelay(10);
	StringLCD(lcdi2c, channel, strlen(channel));
}

uint8_t InputMenu(void)
{
	uint8_t input = 0;
	uint8_t count = 0;

#ifdef DEBUG_BOARD_ON
	do
	{
		input = ReadInput();
		LL_mDelay(100);
		if(!input)
		{
			continue;
		}
 	}while(!input);
#else
	LL_EXTI_EnableEvent_0_31(LL_EXTI_LINE_4|LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);
	input = ReadInput();
	while(ReadInput())
	{
		LL_mDelay(1);
		count++;
		if(count > 120)	//長押し時。これより短くしても更新が早すぎる
		{
			break;
		}
	}
	if(count < DEBOUNCE_COUNT)
	{
		input = 0;
		EnterSleepMode();
	}
	LL_EXTI_DisableEvent_0_31(LL_EXTI_LINE_4|LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);
#endif
	if(input == STOP_PUSH)
	{
		EnterStopMode();
	}
	return input;
}
void EnterSleepMode(void)
{
	//入力があるまでスリープ。各ボタンはEXTIモードに。WFEで復帰
	LL_TIM_EnableIT_UPDATE(TIMx);		//タイマーのセット
	TIMx->CNT = 0;
	LL_TIM_EnableCounter(TIMx);

	LL_LPM_EnableSleep();
	__SEV();
	__WFE();
	__WFE();

	if(LL_TIM_IsActiveFlag_UPDATE(TIMx))	//割り込みがGPIOではなくタイマーなら
	{
		LL_TIM_ClearFlag_UPDATE(TIMx);
		EnterStopMode();				//STOPモードへ
	}
}
void EnterStopMode(void)
{
	LL_GPIO_ResetOutputPin(BackLight_GPIO_Port, BackLight_Pin);

	CMDSend(lcdi2c, FUNCTION_SET_ON);
	LL_mDelay(1);
	CMDSend(lcdi2c, CONTRAST_HIGH_OFF);
	LL_mDelay(1);
	CMDSend(lcdi2c, CONTRAST_LOW_OFF);
	LL_mDelay(1);
	CMDSend(lcdi2c, LOW_VOLTAGE_SET);
	LL_mDelay(200);
	CMDSend(lcdi2c, FUNCTION_SET_OFF);
	LL_mDelay(1);

	//STOP以外の入力を無効にする
	LL_EXTI_DisableEvent_0_31(LL_EXTI_LINE_4|LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);

	LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
	LL_LPM_EnableDeepSleep();
	__SEV();
	__WFE();
	__WFE();

	LL_mDelay(100);
	SystemClock_ReConfig();
	LL_mDelay(10);

	//入力を有効にする
	LL_EXTI_EnableEvent_0_31(LL_EXTI_LINE_4|LL_EXTI_LINE_5|LL_EXTI_LINE_6|LL_EXTI_LINE_7);
	LL_GPIO_SetOutputPin(BackLight_GPIO_Port, BackLight_Pin);
	LL_LPM_DisableDeepSleep();

	CMDSend(lcdi2c, FUNCTION_SET_ON);
	LL_mDelay(1);
	CMDSend(lcdi2c, ICON_CONTRAST_CMD);
	LL_mDelay(1);
	CMDSend(lcdi2c, CONTRAST_CMD);
	LL_mDelay(1);
	CMDSend(lcdi2c, VOLTAGE_CMD);
	LL_mDelay(200);
	CMDSend(lcdi2c, FUNCTION_SET_OFF);
	LL_mDelay(10);
}
void DebugMode(I2C_TypeDef *I2Cx,GPIO_TypeDef *GPIOx,uint32_t GPIO_Pin)
{
	LL_mDelay(3000);
	CMDSend(I2Cx, RETURN_HOME);
	LL_mDelay(10);
	StringLCD(I2Cx, "DEBUG MODE", 10);
	if(!(LL_GPIO_IsInputPinSet(GPIOx, GPIO_Pin)) )
	{
	  LL_mDelay(1000);
	  while(1)
	  {
		  if(!LL_GPIO_IsInputPinSet(GPIOx, GPIO_Pin))
		  {
			  break;
		  }
	  }
	}
}
//HSI
#ifdef SYSCLOCK_HSI
void SystemClock_ReConfig(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  }

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(64000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(64000000);
}
//HSE
#else
void SystemClock_ReConfig(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  }

  /* HSE configuration and activation */
  LL_RCC_HSE_EnableBypass();
  LL_RCC_HSE_Enable();
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 32, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the main PLL */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(64000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(64000000);
}
#endif
