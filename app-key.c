/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "app-key.h"
//#include "system/time/sys_time.h"

void ut_printf(const char *fmt, ...);
void APP_Tasks_msd ( void );
void APP_Mouse_Tasks ( void );

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/* Usage ID to Key map table */

const short KeyTable_lower[256]=
{
                          0,		// 0x00
                          0,
                          0,
                          0,
                          'a',
                          'b',
                          'c',
                          'd',
                          'e',		// 0x08
                          'f',
                          'g',
                          'h',
                          'i',
                          'j',
                          'k',
                          'l',
                          'm',		// 0x10
                          'n',
                          'o',
                          'p',
                          'q',
                          'r',
                          's',
                          't',
                          'u',		// 0x18
                          'v',
                          'w',
                          'x',
                          'y',
                          'z',
                          '1',
                          '2',
                          '3',		// 0x20
                          '4',
                          '5',
                          '6',
                          '7',
                          '8',
                          '9',
                          '0',
                          '\n',		// enter		// 0x28
                          0x1b,		// escape
                          '\b',		// backspace
                          '\t',		// tab
                          ' ',		// space
                          '-',
                          '^',
                          '@',
                          '[',						// 0x30
                          0,
                          ']',
                          ';',
                          ':',
                          0,
                          ',',
                          '.',
                          '/',						// 0x38
                          0,
                          F1,
                          F2,
                          F3,
                          F4,
                          F5,
                          F6,
                          F7,		// 0x40
                          F8,
                          F9,
                          F10,
                          F11,
                          F12,
                          0,
                          0,
                          0,		// 0x48
                          0,
                          0,
                          0,
                          DEL,
                          0,
                          0,
                          RIGHT,	// RIGHT
                          LEFT,		// LEFT     // 0x50
                          DOWN,		// DOWN
                          UP,		// UP
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x58
                          END,
                          DOWN,
                          PGDN,
                          LEFT,
                          0,
                          RIGHT,
                          HOME,
                          UP,		// 0x60
                          PGUP,
                          0,
                          DEL,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x68
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x70
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x78
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x80
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          '\\',
                          0,		// 0x88
                          '\\',
                          0,
                          0,
                          0,
                          0,
                          0,
                          '\\',
                          0,		// 0x90
};

const short KeyTable_upper[256]=
{
                          0,		// 0x00
                          0,
                          0,
                          0,
                          'A',
                          'B',
                          'C',
                          'D',
                          'E',		// 0x08
                          'F',
                          'G',
                          'H',
                          'I',
                          'J',
                          'K',
                          'L',
                          'M',		// 0x10
                          'N',
                          'O',
                          'P',
                          'Q',
                          'R',
                          'S',
                          'T',
                          'U',		// 0x18
                          'V',
                          'W',
                          'X',
                          'Y',
                          'Z',
                          '!',
                          '\"',
                          '#',		// 0x20
                          '$',
                          '%',
                          '&',
                          '\'',
                          '(',
                          ')',
                          0,
                          '\n',		// enter		// 0x28
                          0x1b,		// escape
                          '\b',		// backspace
                          '\t',		// tab
                          ' ',		// space
                          '=',
                          '~',
                          '`',
                          '{',						// 0x30
                          0,
                          '}',
                          '+',
                          '*',
                          0,
                          '<',
                          '>',
                          '?',						// 0x38
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x40
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x48
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          RIGHT,	// RIGHT
                          LEFT,		// LEFT     // 0x50
                          DOWN,		// DOWN
                          UP,		// UP
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x58
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x60
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x68
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x70
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x78
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x80
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          '_',
                          0,		// 0x88
                          '|',
                          0,
                          0,
                          0,
                          0,
                          0,
                          '\\',
                          0,		// 0x90
};
const short KeyTable_kana_lower[256]=
{
                          0,		// 0x00
                          0,
                          0,
                          0,
                  0xc1,//  'a',
                  0xba,//  'b',
                  0xbf,//  'c',
                  0xbc,//  'd',
                  0xb2,//  'e',		// 0x08
                  0xca,//  'f',
                  0xb7,//  'g',
                  0xb8,//  'h',
                  0xc6,//  'i',
                  0xcf,//  'j',
                  0xc9,//  'k',
                  0xd8,//  'l',
                  0xd3,//  'm',		// 0x10
                  0xd0,//  'n',
                  0xd7,//  'o',
                  0xbe,//  'p',
                  0xc0,//  'q',
                  0xbd,//  'r',
                  0xc4,//  's',
                  0xb6,//  't',
                  0xc5,//  'u',		// 0x18
                  0xcb,//  'v',
                  0xc3,//  'w',
                  0xbb,//  'x',
                  0xdd,//  'y',
                  0xc2,//  'z',
                  0xc7,//  '1',
                  0xcc,//  '2',
                  0xb1,//  '3',		// 0x20
                  0xb3,//  '4',
                  0xb4,//  '5',
                  0xb5,//  '6',
                  0xd4,//  '7',
                  0xd5,//  '8',
                  0xd6,//  '9',
                  0xdc,//  '0',
                          '\n',		// enter		// 0x28
                          0x1b,		// escape
                          '\b',		// backspace
                          '\t',		// tab
                          ' ',		// space
                  'Î',//  '-',
                  'Í',//  '^',
                  'Þ',//  '@',
                  'ß',//  '[',						// 0x30
                          0,
                  0xd1,//  ']',
                  0xda,//  ';',
                  0xb9,//  ':',
                          0,
                  0xc8,//  ',',
                  0xd9,//  '.',
                  0xd2,//  '/',						// 0x38
                          0,
                          F1,
                          F2,
                          F3,
                          F4,
                          F5,
                          F6,
                          F7,		// 0x40
                          F8,
                          F9,
                          F10,
                          F11,
                          F12,
                          0,
                          0,
                          0,		// 0x48
                          0,
                          0,
                          0,
                          DEL,
                          0,
                          0,
                          RIGHT,	// RIGHT
                          LEFT,		// LEFT     // 0x50
                          DOWN,		// DOWN
                          UP,		// UP
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x58
                          END,
                          DOWN,
                          PGDN,
                          LEFT,
                          0,
                          RIGHT,
                          HOME,
                          UP,		// 0x60
                          PGUP,
                          0,
                          DEL,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x68
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x70
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x78
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x80
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                 0xdb,//  '\\',
                          0,		// 0x88
                 0xdb,//  '\\',
                          0,
                          0,
                          0,
                          0,
                          0,
                 0xdb,//  '\\',
                          0,		// 0x90
};
const short KeyTable_kana_upper[256]=
{
                          0,		// 0x00
                          0,
                          0,
                          0,
                 0,//     'a',
                 0,//     'b',
                 0,//     'c',
                 0,//     'd',
                 0xa8,//  'e',		// 0x08
                 0,//     'f',
                 0,//     'g',
                 0,//     'h',
                 0,//     'i',
                 0,//     'j',
                 0,//     'k',
                 0,//     'l',
                 0,//     'm',		// 0x10
                 0,//     'n',
                 0,//     'o',
                 0,//     'p',
                 0,//     'q',
                 0,//     'r',
                 0,//     's',
                 0,//     't',
                 0,//     'u',		// 0x18
                 0,//     'v',
                 0,//     'w',
                 0,//     'x',
                 0,//     'y',
                 0xaf,//   'z',
                 0,//     '1',
                 0,//     '2',
                 0xa7,//   '3',		// 0x20
                 0xa9,//   '4',
                 0xaa,//   '5',
                 0xab,//   '6',
                 0xac,//   '7',
                 0xad,//   '8',
                 0xae,//   '9',
                 0xa6,//   '0',
                          '\n',		// enter		// 0x28
                          0x1b,		// escape
                          '\b',		// backspace
                          '\t',		// tab
                          ' ',		// space
                 0,//     '-',
                 0,//     '^',
                 0,//     '@',
                 '¢',//   '[',						// 0x30
                          0,
                 '£',//   ']',
                 0,//     ';',
                 0,//     ':',
                          0,
                 '¤',//   ',',
                 '¡',//   '.',
                 '¥',//   '/',						// 0x38
                          0,
                          F1,
                          F2,
                          F3,
                          F4,
                          F5,
                          F6,
                          F7,		// 0x40
                          F8,
                          F9,
                          F10,
                          F11,
                          F12,
                          0,
                          0,
                          0,		// 0x48
                          0,
                          0,
                          0,
                          DEL,
                          0,
                          0,
                          RIGHT,	// RIGHT
                          LEFT,		// LEFT     // 0x50
                          DOWN,		// DOWN
                          UP,		// UP
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x58
                          END,
                          DOWN,
                          PGDN,
                          LEFT,
                          0,
                          RIGHT,
                          HOME,
                          UP,		// 0x60
                          PGUP,
                          0,
                          DEL,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x68
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x70
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x78
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,		// 0x80
                          0,
                          0,
                          0,
                          0,
                          0,
                          0,
                 0xdb,//  '\\',
                          0,		// 0x88
                 0xdb,//  '\\',
                          0,
                          0,
                          0,
                          0,
                          0,
                 0xdb,//  '\\',
                          0,		// 0x90
};



int kana_flag;
int APP_ch;
int APP_chA;
uint key_time;
APP_DATA_KEY appData_key;
USB_HID_KEYBOARD_KEYPAD old_keyCode;

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Routines
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************
 * USB HOST Layer Events - Host Event Handler
 *******************************************************/

USB_HOST_EVENT_RESPONSE APP_USBHostEventHandler (USB_HOST_EVENT event, void * eventData, uintptr_t context)
{
	ut_printf("APP_USBHostEventHandler start\n");//AAAAA
    switch(event)
    {
        case USB_HOST_EVENT_DEVICE_UNSUPPORTED:
            break;
        default:
            break;
    }
    return USB_HOST_EVENT_RESPONSE_NONE;
}

/*******************************************************
 * USB HOST HID Layer Events - Application Event Handler
 *******************************************************/

void APP_USBHostHIDKeyboardEventHandler(USB_HOST_HID_KEYBOARD_HANDLE handle, 
        USB_HOST_HID_KEYBOARD_EVENT event, void * pData)
{   
//	ut_printf("APP_USBHostHIDKeyboardEventHandler start\n");//AAAAA
    switch ( event)
    {
        case USB_HOST_HID_KEYBOARD_EVENT_ATTACH:
            appData_key.handle = handle;
            appData_key.state =  APP_STATE_DEVICE_ATTACHED;
            appData_key.nBytesWritten = 0;
            appData_key.stringReady = false;
            memset(&appData_key.string, 0, sizeof(appData_key.string));
            memset(&appData_key.lastData, 0, sizeof(appData_key.lastData));
            appData_key.stringSize = 0;
            appData_key.capsLockPressed = false;
            appData_key.scrollLockPressed = false;
            appData_key.numLockPressed = false;
            appData_key.outputReport = 0;
            break;

        case USB_HOST_HID_KEYBOARD_EVENT_DETACH:
            appData_key.handle = handle;
            appData_key.state = APP_STATE_DEVICE_DETACHED;
            appData_key.nBytesWritten = 0;
            appData_key.stringReady = false;
            appData_key.usartTaskState = APP_USART_STATE_CHECK_FOR_STRING_TO_SEND;
            memset(&appData_key.string, 0, sizeof(appData_key.string));
            memset(&appData_key.lastData, 0, sizeof(appData_key.lastData));
            appData_key.stringSize = 0;
            appData_key.capsLockPressed = false;
            appData_key.scrollLockPressed = false;
            appData_key.numLockPressed = false;
            appData_key.outputReport = 0;
            break;

        case USB_HOST_HID_KEYBOARD_EVENT_REPORT_RECEIVED:
            appData_key.handle = handle;
            appData_key.state = APP_STATE_READ_HID;
            /* Keyboard Data from device */
            memcpy(&appData_key.data, pData, sizeof(appData_key.data));
            break;

        default:
            break;
    }
    return;
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary local functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize_key ( void )
{
    /* Place the App state machine in its initial state. */
    memset(&appData_key, 0, sizeof(appData_key));
    appData_key.state = APP_STATE_INIT;
    appData_key.usartTaskState = APP_USART_STATE_DRIVER_OPEN;
}


void APP_USART_Tasks_key(void)
{
    if(appData_key.stringReady){ //AAAAA
        ut_printf((char*)appData_key.string);//AAAAA
        appData_key.stringReady = false;//AAAAA
    }
}



void APP_Driver_key ( void )
{
    int ch;
    
    appData_key.currentOffset = 0;
   	USB_HID_KEYBOARD_KEYPAD keyCode;
    
    /* Check the application's current state. */
    switch ( appData_key.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
//        	ut_printf("APP_STATE_INIT\n");//AAAAA
            USB_HOST_EventHandlerSet(APP_USBHostEventHandler, 0);
            USB_HOST_HID_KEYBOARD_EventHandlerSet(APP_USBHostHIDKeyboardEventHandler);
            
			USB_HOST_BusEnable(0);
			appData_key.state = APP_STATE_WAIT_FOR_HOST_ENABLE;
            break;
			
		case APP_STATE_WAIT_FOR_HOST_ENABLE:
//			ut_printf("APP_STATE_WAIT_FOR_HOST_ENABLE\n");//AAAAA
            /* Check if the host operation has been enabled */
            if(USB_HOST_BusIsEnabled(0))
            {
                /* This means host operation is enabled. We can
                 * move on to the next state */
                appData_key.state = APP_STATE_HOST_ENABLE_DONE;
            }
            break;
        case APP_STATE_HOST_ENABLE_DONE:
            appData_key.stringSize = 64;
            memcpy(&appData_key.string[0], "\r\n***Connect Keyboard***\r\n",
                        sizeof("\r\n***Connect Keyboard***\r\n"));
            appData_key.stringReady = true;
            /* The test was successful. Lets idle. */
            appData_key.state = APP_STATE_WAIT_FOR_DEVICE_ATTACH;
            break;

        case APP_STATE_WAIT_FOR_DEVICE_ATTACH:

            /* Wait for device attach. The state machine will move
             * to the next state when the attach event
             * is received.  */

            break;

        case APP_STATE_DEVICE_ATTACHED:
            
            /* Wait for device report */
            memcpy(&appData_key.string[0], "---Keyboard Connected---\r\n",
                    sizeof("---Keyboard Connected---\r\n"));
            appData_key.stringReady = true;
            appData_key.stringSize = 64;
            appData_key.state = APP_STATE_READ_HID;
            break;

        case APP_STATE_READ_HID:
//            for(count = 0; count < appData_key.data.nNonModifierKeysData; count++)
//            {
                if(appData_key.data.nonModifierKeysData[0].event == USB_HID_KEY_PRESSED)
                {
                    /* scan code to ascii charactor */
                    keyCode = appData_key.data.nonModifierKeysData[0].keyCode;
                    // if upper case
					if(kana_flag){
						if(appData_key.data.modifierKeysData.leftShift || appData_key.data.modifierKeysData.rightShift)
							ch = KeyTable_kana_upper[(unsigned char)keyCode];
						// if lower case
						else
    						ch = KeyTable_kana_lower[(unsigned char)keyCode];
    				}
    				else{
						if(appData_key.data.modifierKeysData.leftShift || appData_key.data.modifierKeysData.rightShift)
							ch = KeyTable_upper[(unsigned char)keyCode];
						// if lower case
						else
    						ch = KeyTable_lower[(unsigned char)keyCode];
					}
//ut_printf("keyCode=%x ch=%x\n", keyCode, ch);
					if(old_keyCode != keyCode)
					{
						key_time = GetTickCount();
						if(keyCode==0x35){		// ”¼Šp/‘SŠp
							kana_flag = 1-kana_flag;
						}
						if(APP_ch==0)
							APP_ch = ch;
						old_keyCode = keyCode;
					}
					else{
						if(GetTickCount()-key_time >= 700){
							if((GetTickCount()-key_time)%100==0){
                                key_time--;
								APP_ch = ch;
							}
						}
					}
					APP_chA = ch;
//                    ut_printf("APP_chA = ch %x\n", ch);
                }
                else{
                    old_keyCode = 0xff;
                    APP_chA = 0;
//                    ut_printf("APP_chA = 0\n");
                }
//            }
            break;

        case APP_STATE_DEVICE_DETACHED:
            appData_key.state = APP_STATE_HOST_ENABLE_DONE;
            break;

        case APP_STATE_ERROR:

            /* The application comes here when the demo
             * has failed. Provide LED indication .*/

//            LED1_On();
            break;

        default:
            break;
    }

}

//
//      APP_Driver inputed data display
//
#if 0
void APP_MAIN(void)
{
    char ch;
    static char buf[80];
    static int ptr=0;
    SYS_FS_HANDLE fileHandle;
    
    if(APP_ch){                 // exist new key data
        ch = APP_ch;
        APP_ch = 0;

        if(ptr < 80){
			buf[ptr++] = ch;
		}
        ut_printf("%c", ch);

        if(ch=='\n'){
            /* Try opening the file for append */
            fileHandle = SYS_FS_FileOpen("/mnt/myDrive1/file-test1.txt", (SYS_FS_FILE_OPEN_APPEND_PLUS));
            if(fileHandle != SYS_FS_HANDLE_INVALID)
            {
                ut_printf("=== file open sucess\n");//AAAAA
                /* Try writing to the file */
                ut_printf("ptr=%d\n", ptr);
                if (SYS_FS_FileWrite( fileHandle, buf, ptr ) != -1)
                {
                    ut_printf("=== file write success\n");
                    /* Close the file */
                    SYS_FS_FileClose(fileHandle);
                }
                else{
                    ut_printf("*** file write error\n");
                }
            }
            else{
                ut_printf("*** file open error\n");
            }
		}
    }
}
#endif


void APP_Tasks ( void )
{
    APP_Driver_key();
    APP_USART_Tasks_key();
	APP_Tasks_msd();
//	APP_MAIN();
//	APP_Mouse_Tasks();
}

/*******************************************************************************
 End of File
 */
