/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "app.h"

void spi_init();
void ut_printf(const char *fmt, ...);
void f_mgr();
void timer_init();
uint GetTickCount();
void cursor_set(int x, int y);
void display_xn(int x, int y, const char * str, int n);
int get_APP_chA();


//typedef int (*func1)(void);
//
//struct osv{
//	func1 f1;
//	func1 f2;
//	func1 f3;
//	func1 f4;
//	func1 f5;
//	func1 f6;
//	func1 f7;
//	func1 f8;
//	func1 f9;
//	func1 f10;
//	func1 f11;
//	func1 f12;
//	func1 f13;
//	func1 f14;
//	func1 f15;
//	func1 f16;
//	func1 f17;
//	func1 f18;
//	func1 f19;
//	func1 f20;
//};
//
//struct osv __attribute__((section(".mySection2"))) osv = {
//	(func1)disp_str,            // 0xa0006400
//	(func1)ut_gets,				// 0xa0006404
//	(func1)ut_getc,				// 0xa0006408
//	(func1)ut_putc,				// 0xa000640c
//	(func1)wait_ms,				// 0xa0006410
//	(func1)Pset,				// 0xa0006414
//	(func1)Pget,				// 0xa0006418
//	(func1)GetTickCount,		// 0xa000641c
//	(func1)cursor_set,			// 0xa0006420
//	(func1)display_xn,			// 0xa0006424
//	(func1)ut_printf,			// 0xa0006428
//};

uint TickCount;
volatile int bz_1msec;

// *****************************************************************************
//
//		CPU: 80MHz, IO: 40MHz
//
// *****************************************************************************

int main ( void )
{
    int i;
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    timer_init();
	spi_init();
    for(i=0; i<3000; i++){
        wait_ms(1);
        SYS_Tasks();
    }
    
	f_mgr();
	
	

//    while ( true )
//    {
//        /* Maintain state machines of all polled MPLAB Harmony modules. */
//        SYS_Tasks ( );
//    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

/********************************************************************************/
/*		timer_init																*/
/********************************************************************************/
void timer_init()
{
	T2CON = 0;								// 1ms TickCount 用
	PR2 = 40000-1;							// 40MHz/40000 = 1KHz
	TMR2 = 0;
	IPC2bits.T2IP = 6;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;
	T2CONSET=0x8000;						// TIMER2 ON

	T4CON = 0;								// buzzer melody 用
	T4CONbits.TCKPS = 2;					// 1:4 prescaler
	IPC4bits.T4IP = 6;
	IFS0bits.T4IF = 0;
	IEC0bits.T4IE = 1;
	T4CONSET=0x8000;						// TIMER4 ON
}

//	1ms 割り込み TickCount
void __ISR(_TIMER_2_VECTOR, IPL6AUTO) T2Interrupt()
{
	++TickCount;
	IFS0bits.T2IF = 0;
	
	/***	buzzer 処理		***/
	if(bz_1msec){
		bz_1msec--;
	}
}

//	Buzzer 割り込み
void __ISR(_TIMER_4_VECTOR, IPL6AUTO) T4Interrupt()
{
	IFS0bits.T4IF = 0;
	if(bz_1msec){
		LATDINV = 0x0001;				// RD0: ブザーを鳴らす
	}
}

//	ブザーの設定を上書きする
void buzzer(int Hz, int msec)
{
	if(Hz < 100){
		ut_printf("\n*** bz_melody error, Hz must >=100Hz\n");
		return;
	}
	PR4 = 40000000/4/2/Hz-1;				// PBCLK/prescaler/2/Hz-1
	TMR4 = 0;
	bz_1msec = msec;
}

//	鳴り終わるまで待ってからブザーを鳴らす

void buzzer_wait(int Hz, int msec)
{
	while(bz_1msec) ;
	buzzer(Hz, msec);
}

uint GetTickCount()
{
	return TickCount;
}