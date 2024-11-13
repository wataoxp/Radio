/*
 * RDA5807FP用
 * Seek完了フラグをGPIO割り込みで感知するので、他ICで使う際はI2C受信で該当ビットをチェックする
 */

#ifndef RADIO_H
#define RADIO_H

#include "main.h"

/*STM32では左シフト(LSBビットでW/Rを識別)*/
#define RADIO_ADDR_PUSH (0x11 << 1)
#define RADIO_ADDR_SEQ (0x10 << 1)

#define BAND_JAPAN (0x1 << 2)	//76~91Mhz
#define BAND_WORLD (0x08)	//76~108Mhz
#define BAND_EASTERN_EUROPA (0x3 << 2)
/*レジスタアドレス*/
#define REG_CONFIG 0x02	//主に動作モードやクロックを設定
#define REG_TUNING 0x03	//選局(TUNE)の設定
#define REG_GPIO 0x04	//(RDA5807FP用)GPIOの設定
#define REG_VOLUME 0x05	//ボリューム、およびGPIO割り込み信号の設定
#define REG_STATUS 0x0A	//シーク完了フラグ、受信周波数の格納先
#define REG_RSSI 0x0B //RSSIの格納先

/*Writeコマンド*/
/*CONFIG*/
#define FLG_DHIZ 0x8000		//ハイZの無効
#define FLG_DMUTE 0x4000	//ミュートの無効
#define FLG_BASS 0x1000		//低音ブーストの有効
#define FLG_SEEKUP 0x0200	//シークアップ
#define FLG_SEEKDOWN 0x0000	//シークダウン時は単に9ビット目を立てない
#define FLG_SEEK 0x0100		//シークの開始(完了すると0になる)
#define FLG_SKMODE UINT16_C(0x0000)	//上限・下限で折り返す。デフォルト値
#define FLG_NEW UINT16_C(0x0004)
#define FLG_RESET UINT16_C(0x0002)
#define FLG_ENABLE UINT16_C(0x0001)
#define CHAN_SHIFT 6		//周波数は0x03レジスタの6～15ビットなのでこれでシフトする
/*TUNING*/
#define FLG_TUNE UINT16_C(0x0010)	//TUNE時は1を立てる。完了後は0になる

/*GPIOレジスタ*/
//割り込みの有効、ディエンファシス50μs(日欧豪、75usは米)、ソフトミュートの有効。GPIO2割り込みモード
#define GPIO_INT_ON 0x4A04
//GPIO2をハイZに
#define GPIO_INT_OFF 0x4A00
/*VOLUMEレジスタ*/
//5msGPIO2割り込みの有効化、SeekTH(しきい値の設定)、音量MAX
//上位バイトの数値を小さくすれば電波の弱い局でも受信しようとする。当然聞けないけど
#define VOLUME_DEFAULT 0x048F

/*Readコマンド*/
#define CHAN_MASK 0x03FF	//CHANは0～9ビット目までの10ビットの値
//RSSIは0Bレジスタの15~9ビット。しかし試した限り7ビット分の数字が有効とは思えないので6ビットのみ読んでいる
#define RSSI_MASK 0xFF00
#define RSSI_SHIFT 9


/*FM Freq*/
//周波数f = チャンネル間隔(100KHz)*CHAN+76MHz
#define FM_YOKOHAMA UINT16_C(0x0057)
#define FM_TOKYO UINT16_C(0x0028)
#define FM_CHIBA UINT16_C(0x0014)
#define FM_INTER UINT16_C(0x0005)
#define FM_JWAVE UINT16_C(0x0035)
#define FM_SHONAN UINT16_C(0x0047)
#define FM_NACK5 UINT16_C(0x0023)
#define FM_BUNKA UINT16_C(0x009c)
#define FM_NIPPON UINT16_C(0x00aa)

//局数-1
#define STATION_NUM 8

void RadioInit(I2C_TypeDef *I2Cx);
void RadioTune(I2C_TypeDef *I2Cx,uint16_t chan);
/*
 * 直接放送局を指定する
 * 引数chanは周波数を規定の値に変換したもの
 */
void Seek(I2C_TypeDef *I2Cx ,uint16_t seekmode);
/*
 * シーク(自動選局)を行う
 * RDA5807FPはシーク完了時にGPIO2からLowを出力できる
 * これを利用してGPIO_PIN_RESET(0)になるまで待つ。シーク完了後Rise,Fallを表示
 * この動作には0x04、0x05レジスタの設定が必要
 */
uint16_t GetChan(I2C_TypeDef *I2Cx);
/*
 * 受信している周波数を取得して返す
 */
uint8_t GetRSSI(I2C_TypeDef *I2Cx);
/*
 * 今の受信強度を取得して返す
 */

#endif
