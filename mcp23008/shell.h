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
    for(c = 0; from[c]; c++)
        to[c] = from[c];
    to[c] = '\0';
}
#else 
#include <string.h>
#endif

void _printArgs(const char str[], UartArgs uArgs);
void prepareTerminal();
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
