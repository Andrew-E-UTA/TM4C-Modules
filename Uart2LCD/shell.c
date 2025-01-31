/*
 * shell.c
 *
 *  Created on: Sep 6, 2024
 *      Author: kyojin
 */

//-----------------------------------------------------------------------------
// Device includes
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "uart0.h"
#include "shell.h"

void initShell(uint32_t baudRate, uint32_t fcyc)
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();
    _delay_cycles(3);

    initUart0();
    setUart0BaudRate(baudRate, fcyc);
}

void getsUart0(USER_DATA* data)
{
    uint8_t count = 0;
    char c = 0;
    while(1)
    {
        c = getcUart0();
        switch(c)
        {
        case (8):
        case (127):         //backspace
        {
            if(count > 0)
                count--;
            continue;
        }
        case (13):
        case (10):          //enter
        {
            data->buff[count] = '\0';
            return;
        }
        }
        if(c >= 32)         // character
        {
            data->buff[count] = c;
            count++;
            if(count == MAX_CHARS)
            {
                data->buff[MAX_CHARS] = '\0';
                return;
            }
        }
    }
}

void parseFields(USER_DATA* data)
{
    uint32_t prevField = DELIMETER;
    uint32_t i;
    char c = '\0';

    data->fieldCount = 0;
    for(i = 0; i < MAX_CHARS; i++)
    {
        c = data->buff[i];
        if((c >= 65 && c <= 90) || (c >= 97 && c <= 127) ) // ALPHA
        {
            if(prevField == DELIMETER && data->fieldCount < 6)
            {
                data->fieldPos[data->fieldCount] = i;
                data->fieldType[data->fieldCount] = 'a';
                data->fieldCount++;
                prevField = ALPHA;
            }
        }
        else if((c >= 48 && c <= 57) || c == '-' || c == '.') // NUMERIC
        {
            if(prevField == DELIMETER && data->fieldCount < 6)
            {
                data->fieldPos[data->fieldCount] = i;
                data->fieldType[data->fieldCount] = 'n';
                data->fieldCount++;
                prevField = NUMERIC;
            }
        }
        else if(c == '\0')
            return;
        else //DELIMITER
        {
            data->buff[i] = '\0';
            prevField = DELIMETER;
        }
    }
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    return &(data->buff[data->fieldPos[fieldNumber]]);
}

int32_t getFieldInt(USER_DATA* data, uint8_t fieldNumber)
{
    return atoi32(&(data->buff[data->fieldPos[fieldNumber]]));
}

float getFieldFloat(USER_DATA* data, uint8_t fieldNumber){
    return atof(&(data->buff[data->fieldPos[fieldNumber]]));
}


bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    char* Command = &(data->buff[data->fieldPos[0]]);
    bool match = strcmp(Command, strCommand);
    if(!match) return false;
    if(data->fieldCount-1 == minArguments) return true;
    else return false;
}

void strcpy(char* to, const char* from)
{
    uint8_t c;
    for(c = 0; from[c]; c++)
        to[c] = from[c];
    to[c] = '\0';
}

void strcpyFill(char* to, const char* from, uint8_t len, char letter)
{
    uint8_t c;
    bool fill = false;
    for(c = 0; (c < len); c++)
    {
        if(!from[c]) fill = true;
        if(!fill)
            to[c] = from[c];
        else
            to[c] = letter;
    }

    to[len-1] = '\0';
}

uint32_t strlen(const char *str){
    uint32_t len  = 0;
    while(*(str + len++));
    return len;
}

bool strcmp(const char* s1, const char* s2)
{
    uint8_t i;
    for(i = 0; i < MAX_CHARS; i++)
    {
        if(s1[i] != s2[i])
            return false;
        if(s1[i] == '\0')
            return true;
    }
    //We reached the end of the buffer and it didnt exit false
    //  therefore all the letters matched and we can assume strings match
    return true;
}

int32_t atoi32(const char* str)
{
    char c = 0;
    int32_t sum = 0;
    uint8_t i = 0, base = 0;
    bool n = false;

    //determine the base of the integer
    if(*(str+0)=='0' && *(str+1)=='x')
    {
        base = 16;
        i = 2;
    }
    else if(*(str+0)=='0' && *(str+1)=='b')
    {
        base = 2;
        i = 2;
    }
    else
    {
        base = 10;
        if(n = (*str == '-')) i = 1;
        else i = 0;
    }

    while((c = str[i++]) != '\0')
    {
        sum *= base;
        if(c >= '0' && c <= '9')
            sum += c - '0';
        else if('a' <= c && c <= 'z')
            sum += c - 'a' + 10;
        else if('A' <= c && c <= 'Z')
            sum += c - 'A' + 10;
        else return -1;
    }
    if(n) sum *= -1;
    return sum;
}

void itoa32(int32_t num, char *str)
{
    uint8_t digits = 0, i;
    char buffer[12];
    bool negative;

    if(!num)
    {
        *str = '0';
        *(str+1) = '\0';
        return;
    }

    negative = num < 0;

    num *= -1 * negative + 1 * !negative;

    while(num % 10)
    {
        buffer[digits] = num % 10;
        digits++;
        num /= 10;
    }
    if(negative)
        *str = '-';

    for(i = 0; i <= digits - 1; i++)
    {
        str[i + negative] = buffer[digits - 1- i];
    }
    str[i + negative] = '\0';
}

void htoa(uint32_t num, char str[MAX_DIG_U32])
{
    uint8_t i = 0, j = 0, temp = 0;
    char buff[MAX_DIG_U32] = {};
    str[0] = '0';
    str[1] = 'x';

    while(num)
    {
        temp = num % 16;
        if(temp >= 10)
            buff[i++] = temp-10 + 'a';
        else
            buff[i++] = temp + '0';
        num /= 16;
    }
    for(j = 0; j < i; j++)
        str[j+2] = buff[(i-1) - j];
    str[i+2] = '\0';
}

void clearString(char* string, uint32_t len){
    uint32_t i;
    for(i = 0; i < len; i++) string[i] = 0;
}
