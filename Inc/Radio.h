/*
 * Radio.h
 *
 *  Created on: Noc 30, 2024
 *      Author: wataoxp
 */

#ifndef RADIO_H
#define RADIO_H

#include "main.h"

typedef enum{
	VolDown,
	VolUp,
	Mute,
	UnMute,
}Choices;

typedef enum{
	Success,
	Failed,
}Radio_bool;


/* I2C Address */
#define RADIO_ADDR_PUSH (0x11 << 1)		//I2C_Mem_Write
#define RADIO_ADDR_SEQ (0x10 << 1)		//I2C_Master_Transmit

#define BAND_EUROPA (0x0 << 2)			//87~108Mhz
#define BAND_JAPAN (0x1 << 2)			//76~91Mhz	(JP)ワイドFM使用時はWORLDにしてください
#define BAND_WORLD (0x2 << 2)			//76~108Mhz
#define BAND_EASTERN_EUROPA (0x3 << 2)	//50~65MHz
/*レジスタアドレス*/
#define REG_CONFIG 0x02				//Config,Clock,Seek
#define REG_TUNING 0x03				//Tune,Band,Space,FrequencySpace
#define REG_GPIO 0x04				//De-Emphasis.GPIOinterrupt(RDA5807FP)
#define REG_VOLUME 0x05				//5msInterrupt,Volume
#define REG_STATUS 0x0A				//RDS,Tune,SeekFlag,ReadChannel
#define REG_RSSI 0x0B 				//ReadRSSI

/*Write & Read*/
/*CONFIG*/
#define FLG_DHIZ 0x8000				//Audio Output Hi-Z(SetBit Hi-Z Disable)
#define FLG_DMUTE 0x4000			//Audio Mute(SetBit Mute Disable)
#define FLG_MONO 0x2000				//Audio Monaural(SetBit Monaural)
#define FLG_BASS 0x1000				//Bass Boost(SetBit Bass Boost Enable)
#define FLG_SEEKUP 0x0200			//SetBit SeekUp
#define FLG_SEEKDOWN 0x0000			//ClearBit SeekDown
#define FLG_SEEK 0x0100				//Seek Start
#define FLG_SKMODE UINT16_C(0x0000)	//設定バンド値周波数の上限・下限で折り返す。デフォルト値
#define FLG_NEW UINT16_C(0x0004)
#define FLG_RESET UINT16_C(0x0002)	//SoftReset
#define FLG_ENABLE UINT16_C(0x0001)	//SetBit PowerON

//Clock Source
#define CLOCK_32_K 0x0
#define CLOCK_12_M 0x1
#define CLOCK_24_M 0x5
#define CLOCK_38_M 0x7

#define RDA_CLOCK CLOCK_24_M
#define CLOCK_SET (RDA_CLOCK << 4)

/*TUNING*/
#define CHAN_SHIFT 6				//CHAN->6~15bit
#define CHAN_WRITE_MASK 0xFFC0
#define FLG_TUNE UINT16_C(0x0010)	//SetBit StartTune
#define BAND_MASK UINT16_C(0x000C)	//2~3Bit Default0b00
#define BAND_SHIFT 2
#define SPACE_MASK UINT16_C(0x0003)	//0~1Bit Default0b00
#define SPACE_SHIFT 0

/*GPIO*/
#define FLG_DE	0x0800 			//De-Emphasis SetBit50us(JP,EU,AUS) ClearBit75us(USA)
#define FLG_SOFT_MUTE 0x0200	//SetBit EnableSoftMute 受信状況が悪い時に自動的に出力を抑える

//RDA5807FP GPIO Settings
#define FLG_STC 0x4000 			//SetBit GPIOinterrupt Enable
#define GPIO2_INT_ON 0x0004		//SetBit GPIO2 FallingEgde Enable
#define GPIO2_INT_OFF 0x0000	//ClearBit GPIO2 Hi-Z
#define GPIO2_INT_MASK 0xC

/*VOLUME*/
#define FLG_INT_ENABLE 0x0000		//Default.ClearBit 5msInterrupt
#define FLG_INT_DISABLE 0x8000		//SetBit interrupt Disable

#define FLG_LNA 0x0080
#define FLG_LNA_MAX 0x00C0

#define THRESHOLD_MASK 0xF			//8~11bit Default0b1000
#define THRESHOLD_SHIFT 8
#define VOLUME_MASK 0xF				//0~3bit Default0b1111
#define VOLUME_SHIFT 0

/*Read*/
/*STATUS*/
#define FLG_STC 0x4000			//Seek,Tune CompleteFlag
#define STC_SHIFT 14
#define CHAN_READ_MASK 0x03FF		//0~9bit Default 0x80?

/*RSSI*/
#define RSSI_MASK 0xFE00		//0b1111 1110
#define RSSI_SHIFT 9			//9~15bit

/*FM CHAN*/
#define FM_YOKOHAMA UINT16_C(0x0057)
#define FM_TOKYO UINT16_C(0x0028)
#define FM_CHIBA UINT16_C(0x0014)
#define FM_INTER UINT16_C(0x0005)
#define FM_JWAVE UINT16_C(0x0035)
#define FM_NHK_TOKYO UINT16_C(0x0041)
#define FM_NACK5 UINT16_C(0x0023)
#define FM_BUNKA UINT16_C(0x009c)
#define FM_NIPPON UINT16_C(0x00aa)


void RadioInit(I2C_TypeDef *I2Cx);
/*
 * RDA5807 initialize.
 * #ifdefで5807M用にしています。
 * 特に5807FPのGPIO割り込みの設定項目はMには無いので、安全の為両者を別関数としています。
 * これはSeek()も同様です。
 */
void Seek(I2C_TypeDef *I2Cx ,uint16_t seekmode);
/*
 * シーク(自動選局)を行います。引数はFLG_SEEKUPまたはDOWNです。
 * REG_STATUSの14ビット目を読み込むことでシークの完了をチェックします...が、どうもいつ読み込んでもビットがセットされています。
 * 手動でクリアする必要があるのかもしれません。
 */
void RadioTune(I2C_TypeDef *I2Cx,uint16_t chan);
/*
 * 聞きたい放送局を直接指定します。
 * あらかじめ放送局とその周波数をCHAN値に変換、リスト化しています。
 */
Radio_bool SetVolume(I2C_TypeDef *I2Cx,uint8_t dir);
/*
 * 音量を調整します。
 * 0x1~0xFまでの4段階です。無音にしたい場合はSetMuteを使用してください。
 */
void SetMute(I2C_TypeDef *I2Cx,uint8_t muteselect);
uint16_t GetRegister(I2C_TypeDef *I2Cx,uint8_t Reg);
/*
 * Regレジスタの今の値を読み込んで返します。
 */
uint16_t GetChan(I2C_TypeDef *I2Cx);
/*
 * 受信している周波数を取得して返します。
 * 本来周波数fは0.1(MHz)*CHAN+76(MHz)によって求められます。
 * しかし当然のことながら、この計算で求められるのは実数(小数点を含む)になります。
 * Cortex-M0系など、小数用の演算ユニットが無いものもあります。
 * (あとなんかCubeIDEがfloat用の設定しろって言ってきて面倒っす)
 *
 * ですので、あえて整数として出力させて後の文字列変換で実数「っぽく」見せています。
 * 84.7 = 0.1(MHz)*CHAN+76(MHz) ***両辺に10を乗算
 * 10 * 84.7 = (0.1 * 87 + 76) * 10
 * 847 = 87 + 760
 */
uint8_t GetRSSI(I2C_TypeDef *I2Cx);
/*
 * 今の受信強度を取得して返します。
 */

/* ***周波数fと設定値CHANの関係 ***
 * 周波数f = チャンネル間隔(100KHz)*CHAN+下限周波数(76MHz)
 * データシートP10参照。
 * 下限周波数はREG_TUNING(03H)のビット3:2のBANDの設定値に依存。
 * チャンネル間隔は同じく03Hレジスタのビット1:0の設定値に依存する。
 *
 * ここで上記の式において100Kと76Mで桁を合わせると
 * 0.1(MHz)*CHAN+76(MHz)
 * CHANを0x0057(87U)とすると
 * 0.1*87+76により84.7MHzとなる。
 *
 * CHAN = (周波数f - 76) / 0.1
 * 上記の式を変形させると設定値CHANが導ける。
 * 周波数fを81.3とするとCHANは以下の値になる。
 * (81.3 - 76) / 0.1 = 53U(0x0035)
 *
 */

#endif
