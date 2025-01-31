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

#define GOTO_HOME    "\x1B[H"
#define CLEAR_SCREEN "\x1B[2J"
#define CLEAR_LINE   "\x1B[0K"

#define HIDE_CURSOR  "\x1B[?25l"
#define SHOW_CURSOR  "\x1B[?25h"

#define SAVE_POS     "\x1B[s"
#define RETURN_2_POS "\x1B[u"

typedef struct _USER_DATA
{
    char    buff[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPos[MAX_FIELDS];
    char    fieldType[MAX_FIELDS];
}USER_DATA;

void initScreen(void);
void getsUart0(USER_DATA* data);
void parseFields(USER_DATA* data);
char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
int32_t getFieldInt(USER_DATA* data, uint8_t fieldNumber);
float getFieldFloat(USER_DATA* data, uint8_t fieldNumber);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);

void strcpy(char* to, const char* from);
void strcpyFill(char* to, const char* from, uint8_t len, char letter);
uint32_t strlen(const char *str);
bool strcmp(const char* s1, const char* s2);
int32_t atoi32(const char* str);
void itoa32(int32_t num, char *str);
void htoa(uint32_t num, char str[MAX_DIG_U32]);
void clearString(char* string, uint32_t len);
void newline(void);

#endif /* SHELL_H_ */
