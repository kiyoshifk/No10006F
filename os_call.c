#include "app.h"


typedef int (*func1)(void);


int get_APP_chA();

struct osv{
	func1 f1;
	func1 f2;
	func1 f3;
	func1 f4;
	func1 f5;
	func1 f6;
	func1 f7;
	func1 f8;
	func1 f9;
	func1 f10;
	func1 f11;
	func1 f12;
	func1 f13;
	func1 f14;
	func1 f15;
	func1 f16;
	func1 f17;
	func1 f18;
	func1 f19;
	func1 f20;
};

struct osv __attribute__((section(".mySection2"))) osv = {
	(func1)disp_str,            // 0xa0006400
	(func1)ut_gets,				// 0xa0006404
	(func1)ut_getc,				// 0xa0006408
	(func1)ut_putc,				// 0xa000640c
	(func1)wait_ms,				// 0xa0006410
	(func1)Pset,				// 0xa0006414
	(func1)Pget,				// 0xa0006418
	(func1)GetTickCount,		// 0xa000641c
	(func1)cursor_set,			// 0xa0006420
	(func1)display_xn,			// 0xa0006424
	(func1)ut_printf,			// 0xa0006428
	(func1)SYS_FS_FileOpen,		// 0xa000642c
	(func1)SYS_FS_FileClose,	// 0xa0006430
	(func1)SYS_FS_FileRead,		// 0xa0006434
	(func1)SYS_FS_FileWrite,	// 0xa0006438
	(func1)ut_error_msg,		// 0xa000643c
	(func1)get_APP_chA,			// 0xa0006440
	(func1)buzzer,				// 0xa0006444
	(func1)buzzer_wait,			// 0xa0006448
};
