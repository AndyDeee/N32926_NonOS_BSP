/***************************************************************************
 * Copyright (c) 2013 Nuvoton Technology. All rights reserved.
 *
 * FILENAME
 *     main.c
 * DESCRIPTION
 *     The main file for SIC/SD demo code.
 **************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"

#include "w55fa92_reg.h"
#include "w55fa92_sic.h"

/*-----------------------------------------------------------------------------
 * For system configuration
 *---------------------------------------------------------------------------*/
// Define DBG_PRINTF to sysprintf to show more information about testing
#define DBG_PRINTF    sysprintf
//#define DBG_PRINTF(...)

VOID fmiSD_Show_info(int sdport);

/*-----------------------------------------------------------------------------
 * For global variables
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Function of Card detect for card insert
 *---------------------------------------------------------------------------*/
void func_card_insert()
{
    UINT32 result;
    sysprintf("--- func_card_insert(): card inserted on SD port 0 ---\n\n");
    result = sicSdOpen0();
    if (result < FMI_ERR_ID)
    {
        sysprintf("    Detect card on port %d.\n", 0);
        fmiSD_Show_info(0);
        sysprintf("\n");
    }
    else if (result == FMI_NO_SD_CARD)
    {
        sysprintf("WARNING: Don't detect card on port %d !\n", 0);
    }
    else
    {
        sysprintf("WARNING: Fail to initial SD/MMC card %d, result = 0x%x !\n", 0, result);
    }
    return;
}


/*-----------------------------------------------------------------------------
 * Function of Card detect for card remove
 *---------------------------------------------------------------------------*/
void func_card_remove()
{
    sysprintf("--- func_card_remove: card removed on SD port 0 ---\n\n");
    sicSdClose0();
    return;
}


/*-----------------------------------------------------------------------------
 * Initial UART.
 *---------------------------------------------------------------------------*/
void init_UART()
{
    UINT32 u32ExtFreq;
    WB_UART_T uart;

    u32ExtFreq = sysGetExternalClock();
    sysUartPort(1);
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    uart.uart_no = WB_UART_1;
    sysInitializeUART(&uart);
}


/*-----------------------------------------------------------------------------
 * Initial Timer0 interrupt for system tick.
 *---------------------------------------------------------------------------*/
void init_timer()
{
    sysSetTimerReferenceClock(TIMER0, sysGetExternalClock());   // External Crystal
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);                  // 100 ticks/per sec ==> 1tick/10ms
    sysSetLocalInterrupt(ENABLE_IRQ);
}


/*-----------------------------------------------------------------------------*/

int main(void)
{
    UINT32 card_current_status = FALSE, card_pre_status;

    init_UART();
    init_timer();

    sysprintf("\n=====> W55FA92 Non-OS SIC/SD Driver Sample Code [tick %d] <=====\n", sysGetTicks(0));

    //--- Initial system clock
    sicIoctl(SIC_SET_CLOCK, sysGetPLLOutputHz(eSYS_UPLL, sysGetExternalClock())/1000, 0, 0);    // clock from PLL

    //--- Enable AHB clock for SIC/SD/NAND, interrupt ISR, DMA, and FMI engineer
    sicOpen();

    //--- Enable SD card detection feature
    sysprintf("SD0 card detect feature is DISABLED by default.\n");
    sysprintf("The card status will always be INSERTED if card detect feature disabled.\n");
    sysprintf("Now, enable SD0 card detect feature !\n");
    sysprintf("\n");
    // Second parameter: TRUE to enable, FALSE to disable card detection feature.
    sicIoctl(SIC_SET_CARD_DETECT, TRUE, 0, 0);  // MUST call sicIoctl() BEFORE sicSdOpen0()

    //--- Initial SD card on port 0
    sicSdOpen0();

    sysprintf("Please insert or remove SD card to trigger SD card detect interrupt ...\n");

    card_pre_status = 0xFF;
    while(1)
    {
        //--- Got SD card insert/remove status
        sicIoctl(SIC_GET_CARD_STATUS, (INT32)&card_current_status, 0, 0);

        if (card_current_status != card_pre_status)
        {
            sysprintf("SD0 card status change to %s\n", card_current_status ? "INSERTED" : "REMOVED");
            if (card_current_status == TRUE)
                func_card_insert();
            else
                func_card_remove();

            card_pre_status = card_current_status;
        }
    }
}
