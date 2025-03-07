#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef enum{
    convert_success,
    buffer_over_run,
}boolItoS;

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
        digit++;
    }
    return digit;
}

boolItoS ItoS(char *buffer,uint16_t value)
{
    char *pbuf;
    uint8_t digit;

    digit = CheckDigit(value);
    pbuf = buffer + digit;

    *pbuf = '\0';
    pbuf--;
    if(value == 0)
    {
        *pbuf = '0';
        return convert_success;
    }

    while(value > 0 && pbuf >= buffer)
    {
        *pbuf = (value % 10) + '0';
        value /= 10;
        pbuf--;
    }
    if (pbuf < buffer)
    {
        return buffer_over_run;
    }

    return convert_success;
}