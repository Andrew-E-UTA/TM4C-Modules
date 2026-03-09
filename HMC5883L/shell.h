/*
 * shell.h
 *
 *  Created on: Sep 6, 2024
 *      Author: kyojin
 */

#ifndef SHELL_C_
#define SHELL_C_


#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"


#define MAX_CHARS       80
#define MAX_FIELDS      5
#define MAX_DIG_U32     10

#define DELIMETER       0
#define ALPHA           1
#define NUMERIC         2

#define GOTO_HOME       "\x1B[H"
#define PREV_LINE       "\x1B[F"
#define SAVE_POS        "\x1B[s"
#define RETURN_2_POS    "\x1B[u"

#define CLEAR_SCREEN    "\x1B[2J"
#define CLEAR_LINE      "\x1B[0K"
#define CLEAR_COLOR     "\x1b[0m"

#define SET_FG_BL    "\x1b[30m"
#define SET_FG_W     "\x1b[97m"
#define SET_FG_R     "\x1b[31m"
#define SET_FG_G     "\x1b[32m"
#define SET_FG_Y     "\x1b[33m"
#define SET_FG_B     "\x1b[34m"

#define SET_BG_BL       "\x1b[40m"
#define SET_BG_W        "\x1b[107m"
#define SET_BG_R        "\x1b[41m"
#define SET_BG_G        "\x1b[42m"
#define SET_BG_Y        "\x1b[43m"
#define SET_BG_B        "\x1b[44m"

#define HIDE_CURSOR     "\x1B[?25l"
#define SHOW_CURSOR     "\x1B[?25h"

#define INFO            0
#define WARNING         1
#define ERROR           2

//Funny Characters ∈ ° ± σ Σ ≠ ∀ ∃ √ &

enum Color{
    White = 0,
    Black,
    Red,
    Green,
    Blue,
    Yellow
};

typedef struct _uartArgs
{
    enum Color bg;
    enum Color fg;
    char end;
}UartArgs;

#define Print(s, ...) (_printArgs(s, (UartArgs){\
    .bg = Black,\
    .fg = White,\
    .end = '\n',\
    __VA_ARGS__\
}))

typedef struct _USER_DATA
{
    char    buff[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPos[MAX_FIELDS];
    char    fieldType[MAX_FIELDS];
}USER_DATA;

#ifdef _MEM_CONSTRAINED
bool strcmp(const char* s1, const char* s2)
{
     while( *(str1++) == *(str2++))
        if (!(*str1) && !(*str2)) return true;
    return false;
}

void strcpy(char* to, const char* from)
{
    uint8_t c;
    for(c = 0; from[c]; ++c)
        to[c] = from[c];
    to[c] = '\0';
}

void memset(char* s, char c, size_t n)
{
    size_t i = 0;
    for(;i < n;++i) s[i] = c;
}

#else 
#include <string.h>
#endif

void _printArgs(const char str[], UartArgs uArgs);
void prepareTerminal();
void log(uint8_t logType, const char* str);
void getsUart0(USER_DATA* data);
void parseFields(USER_DATA* data);
inline char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
inline int32_t getFieldInt(USER_DATA* data, uint8_t fieldNumber);
inline float getFieldFloat(USER_DATA* data, uint8_t fieldNumber);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);

void strcpyFill(char* to, const char* from, uint8_t len, char letter);
uint32_t strlen(const char *str);
int32_t atoi32(const char* str);
void itoa32(int32_t num, char *str);
void htoa(uint32_t num, char str[MAX_DIG_U32]);
void clearString(char* string, uint32_t len);

#endif /* SHELL_H_ */

#ifdef SHELL_IMPLEMENTATION

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "uart0.h"
#include <stdarg.h>

void _printArgs(const char str[], UartArgs uArgs)
{
    switch(uArgs.fg) {
        case White: { putsUart0(SET_FG_W); }break;
        case Black: { putsUart0(SET_FG_BL); }break;
        case Red: { putsUart0(SET_FG_R); }break;
        case Green: { putsUart0(SET_FG_G); }break;
        case Blue: { putsUart0(SET_FG_B); }break;
        case Yellow: { putsUart0(SET_FG_Y); }break;
    }

    switch(uArgs.bg) {
        case White: { putsUart0(SET_BG_W); }break;
        case Black: { putsUart0(SET_BG_BL); }break;
        case Red: { putsUart0(SET_BG_R); }break;
        case Green: { putsUart0(SET_BG_G); }break;
        case Blue: { putsUart0(SET_BG_B); }break;
        case Yellow: { putsUart0(SET_BG_Y); }break;
    }

    putsUart0(str);

    putcUart0(uArgs.end);

    putsUart0(CLEAR_COLOR);
}

void prepareTerminal()
{
    putsUart0(CLEAR_SCREEN);
    putsUart0(GOTO_HOME);
    putsUart0(HIDE_CURSOR);
}

void log(uint8_t logType, const char* str)
{
    switch(logType)
    {
    case INFO:{
        putsUart0(SET_FG_G);
        putsUart0("[INFO]: ");
        putsUart0(str);
    }break;
    case WARNING:{
        putsUart0(SET_FG_Y);
        putsUart0("[WARNING]: ");
        putsUart0(str);
    }break;
    case ERROR:{
        putsUart0(SET_FG_R);
        putsUart0("[ERROR]: ");
        putsUart0(str);
    }break;
    }
    putsUart0(CLEAR_COLOR);
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

inline char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    return &(data->buff[data->fieldPos[fieldNumber]]);
}

inline int32_t getFieldInt(USER_DATA* data, uint8_t fieldNumber)
{
    return atoi32(&(data->buff[data->fieldPos[fieldNumber]]));
}

inline float getFieldFloat(USER_DATA* data, uint8_t fieldNumber){
    return atof(&(data->buff[data->fieldPos[fieldNumber]]));
}


bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    char* Command = &(data->buff[data->fieldPos[0]]);
    bool match = !strcmp(Command, strCommand);
    if(!match) return false;
    if(data->fieldCount-1 == minArguments) return true;
    else return false;
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

static void reverse(char *start, char *end)
{
    while (start < end)
    {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

static int itoa_simple(int value, char *buf)
{
    char *p = buf;
    int is_negative = 0;

    if (value == 0)
    {
        *p++ = '0';
        *p = 0;
        return 1;
    }

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    while (value)
    {
        *p++ = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative)
        *p++ = '-';

    *p = 0;
    reverse(buf, p - 1);

    return (p - buf);
}

int usprintf(char *out, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char *str = out;

    while (*fmt)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt++;
            continue;
        }
        if (*fmt == '%' && *(fmt+1) == '%')
        {
            fmt++;
            *str++ = *fmt++;
            continue;
        }

        fmt++;  // skip %

        int left_align = 0;
        int force_sign = 0;
        int zero_pad = 0;
        int width = 0;

        // flags
        int parsing = 1;
        while (parsing)
        {
            switch (*fmt)
            {
                case '-': left_align = 1; fmt++; break;
                case '+': force_sign = 1; fmt++; break;
                case '0': zero_pad = 1; fmt++; break;
                default: parsing = 0; break;
            }
        }

        // width
        while (*fmt >= '0' && *fmt <= '9')
        {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        if (*fmt == 'd')
        {
            fmt++;

            int value = va_arg(args, int);
            char numbuf[16];
            int len = itoa_simple(value, numbuf);

            int negative = (numbuf[0] == '-');

            char sign_char = 0;
            if (negative)
            {
                sign_char = '-';
                len--;              // exclude '-' from digit count
            }
            else if (force_sign)
            {
                sign_char = '+';
            }

            int total_len = len + (sign_char ? 1 : 0);
            int pad = (width > total_len) ? (width - total_len) : 0;

            if (!left_align)
            {
                while (pad--)
                    *str++ = zero_pad ? '0' : ' ';
            }

            if (sign_char)
                *str++ = sign_char;

            char *digits = negative ? &numbuf[1] : numbuf;
            int i = 0;
            for (; i < len; i++)
                *str++ = digits[i];

            if (left_align)
            {
                while (pad--)
                    *str++ = ' ';
            }
        }
    }

    *str = 0;

    va_end(args);

    return (str - out);
}

#endif
