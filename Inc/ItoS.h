/* *** Dec 2, 2024 ***
 * 数値を文字列に変換する関数
 * 本ファイルはメモ用に近いので、実際に使用する際はこのまま利用しないこと
 */


typedef enum{
    convert_success,
    buffer_over_run,
}boolItoS;
/*
 * ItoSの戻り値
 * 格納先の配列のサイズは求めない為、エラー処理は必須
*/

static inline uint8_t CheckDigit(uint16_t value)
{
    uint8_t digit = 0;

    while (value > 0)
    {
        value /= 10;
        digit++;
    }
    if(value == 0)
    {
        digit++;        //0のとき、上のwhile文が実行されないため
    }
    return digit;
}
/*
 * ItoSの構成関数
 * 変換する整数valueのdigit(桁数)を求める
 * digitの数値分格納先の配列にアクセスする
 * 
 * valueが0であるとき、vscodeではエラーになる。
 * gccでコンパイルしているようなので、何で試しても同じ結果になるだろう
 */

/*
 * 引数
 * *buffer 格納先の配列の先頭アドレス
 * value 変換する整数
 * ここでは16ビットまでを受け付ける形にしているが、32ビットでも問題なく変換できる
 */
boolItoS ItoS(char *buffer,uint16_t value)
{
    char *pbuf;         //bufferのアドレスを受け取るポインタ
    uint8_t digit;      //valueの桁数

    digit = CheckDigit(value);  //valueの桁数を取得
    pbuf = buffer + digit;      //ポインタpbufにbuffer[digit]のアドレスを渡す

    *pbuf = '\0';               //①pbufはbuffer[digit]のアドレスを持っているので、終端文字を入れる
    pbuf--;                     //pbuf = buffer[digit-1]
    if(value == 0)              
    {
        *pbuf = '0';  
        return convert_success;
    }

    while(value > 0 && pbuf >= buffer)      //②valueが1以上かつpbufがbuffer[0]を指すまで
    {
        *pbuf = (value % 10) + '0';
        value /= 10;
        pbuf--;
    }
    if (pbuf < buffer)                      //pbufがbuffer[0]より若いアドレス番地を指してしまったとき
    {
        return buffer_over_run;
    }

    return convert_success;
}
/* *** メモ ***
* ①valueが589とする。このときdigitは3
* 期待されるbufferの値は以下のようになる。
* buffer[0] = '5'
* buffer[1] = '8'
* buffer[2] = '9'
* buffer[3] = '\0'
* 以上のように、buffer[digit]が終端文字の格納先となる。
* 数値を格納する配列であれば[digit-1]が正しい終端だが、文字列ではヌル文字を入れるために[digit]になる。
* 
* ②while(value > 0 && pbuf >= buffer)について
* valueが1以上のとき という条件はもはや解説不要だろう。
* pbuf >= buffer、これはアドレスの比較演算をしている。なぜこんなことができるのか？
* 
* まず前提としてpbufはbufferの先頭アドレスからdigit個先の要素を示している。
* 
* メモリ
* +---+---+---+---+---+---+---+
* |   |   |   |   |   |   |   |
* +---+---+---+---+---+---+---+
*  ^                       ^
*  buffer                  pbuf==(buffer+digit)
*   
* pbufはループの中でデクリメントされるのでchar型サイズずつ前の要素を指していくようになる。
* 上記のイメージ図で言うと1つのブロックがchar型、つまり1バイトに相当する。
* 仮にbufferのアドレスが0x10とする。
* この時pbufは0x10からdigitバイト先のアドレスを指す。つまり0x13である。
* pbufはデクリメントされるので、自身が指すアドレスは0x12,0x11,0x10とさかのぼっていく。
* 0x10を指した時がpbuf == bufferであり、それよりさかのぼると0x09となって偽になる。
* 
* もっとも大事なのが「アドレス」と言った所で所詮ただの数値でしかないということだ。
* なのでポインタの比較演算と言ってもただの数値の大小の比較でしかない。
* その数値を「アドレスとして扱いますよ」というだけであって、それは演算後の話なのでまったく別の話なのだ
*/
