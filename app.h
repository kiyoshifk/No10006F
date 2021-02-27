/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef _APP_H
#define _APP_H


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <xc.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <setjmp.h>
#include <ctype.h>
#include "configuration.h"
#include "definitions.h"
//#include "configuration.h"
//#include "definitions.h"


#define UP    0x112
#define DOWN  0x113
#define RIGHT 0x114
#define LEFT  0x115
#define ESC   0x1b
#define F1    0x180
#define F2    0x181
#define F3    0x182
#define F4    0x183
#define F5    0x184
#define F6    0x185
#define F7    0x186
#define F8    0x187
#define F9    0x188
#define F10   0x189
#define F11   0x18a
#define F12   0x18b
#define HOME  0x18c
#define END   0x18d
#define PGUP  0x18e
#define PGDN  0x18f
#define DEL   0x190


//	disp.c
void Pset(int x, int y, int c);
int Pget(int x, int y);
void ut_printf(const char *fmt, ...);
void disp_1char(unsigned char c);
void disp_L();
void disp_str(const char *str);
void display_xn(int x, int y, const char * str, int n);
void cursor_set(int x, int y);

//	f_mgr.c
void ut_error_msg(const char *head);
int ut_getc();
void ut_gets(char *buf, int maxlen);
void ut_putc(char c);

//	asm32.c
int asm32(const char *filename);

//	prep.c
int prep(const char *filename);

//		eeprom.c
void ee_write(void *datax, int len, int startaddr);
void ee_read(void *datax, int len, int startaddr);

//		utl.c
void wait_ms(int ms);

//		main.c
uint GetTickCount();
void buzzer(int Hz, int msec);
void buzzer_wait(int Hz, int msec);

void APP_Initialize_key ( void );
void APP_Initialize_msd ( void );
void APP_Initialize ( void );


//extern struct osv osv;

#endif	/* _APP_H	*/
