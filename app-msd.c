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

#include "app-msd.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

void ut_printf(const char *fmt, ...);


APP_DATA_MSD addData_msd;

/* This is the string that will written to the file */
uint8_t writeData[12] = "Hello World ";

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

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

void APP_Initialize_msd ( void )
{
    /* Place the App state machine in its initial state. */
    addData_msd.state = APP_STATE_BUS_ENABLE;
    addData_msd.deviceIsConnected = false;
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}

//USB_HOST_EVENT_RESPONSE APP_USBHostEventHandler (USB_HOST_EVENT event, void * eventData, uintptr_t context)
//{
//    ut_printf("APP_USBHostEventHandler start\n");//AAAAA
//    switch (event)
//    {
//        case USB_HOST_EVENT_DEVICE_UNSUPPORTED:
//            break;
//        default:
//            break;
//                    
//    }
//    
//    return(USB_HOST_EVENT_RESPONSE_NONE);
//}

void APP_SYSFSEventHandler(SYS_FS_EVENT event, void * eventData, uintptr_t context)
{
    ut_printf("APP_SYSFSEventHandler eventData=%s\n", (char*)eventData);//AAAAA
    switch(event)
    {
        case SYS_FS_EVENT_MOUNT:
            addData_msd.deviceIsConnected = true;
            
            break;
            
        case SYS_FS_EVENT_UNMOUNT:
            addData_msd.deviceIsConnected = false;
//AAAAA            LED1_Off();
            break;
            
        default:
            break;
    }
}
/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks_msd ( void )
{
    switch(addData_msd.state)
    {
        case APP_STATE_BUS_ENABLE:
            ut_printf("=== APP_STATE_BUS_ENABLE\n");//AAAAA
           /* Set the event handler and enable the bus */
            SYS_FS_EventHandlerSet((void *)APP_SYSFSEventHandler, (uintptr_t)NULL);
//            USB_HOST_EventHandlerSet(APP_USBHostEventHandler, 0);
            USB_HOST_BusEnable(0);
            addData_msd.state = APP_STATE_WAIT_FOR_BUS_ENABLE_COMPLETE;
            break;
            
        case APP_STATE_WAIT_FOR_BUS_ENABLE_COMPLETE:
            if(USB_HOST_BusIsEnabled(0))
            {
                ut_printf("=== APP_STATE_WAIT_FOR_MSD_DEVICE_ATTACH\n");//AAAAA
                addData_msd.state = APP_STATE_WAIT_FOR_DEVICE_ATTACH;
            }
            break;
       
        case APP_STATE_WAIT_FOR_DEVICE_ATTACH:

            /* Wait for device attach. The state machine will move
             * to the next state when the attach event
             * is received.  */
            if(addData_msd.deviceIsConnected)
            {
                ut_printf("=== APP_STATE_MSD_DEVICE_CONNECTED\n");//AAAAA
                addData_msd.state = APP_STATE_DEVICE_CONNECTED;
            }

            break;

        case APP_STATE_DEVICE_CONNECTED:

            /* Device was connected. We can try mounting the disk */
            if(addData_msd.deviceIsConnected==0)
            {
                ut_printf("*** APP_STATE_MSD_DEVICE_DISCONNECTED\n");//AAAAA
                addData_msd.state = APP_STATE_WAIT_FOR_DEVICE_ATTACH;
            }
            break;


        default:
            break;

    }
}
 

/*******************************************************************************
 End of File
 */
