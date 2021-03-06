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
 * ISR of Card detect interrupt for card insert
 *---------------------------------------------------------------------------*/
void isr_card_insert()
{
    UINT32 result;
    sysprintf("--- ISR: card inserted on SD port 0 ---\n\n");
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
 * ISR of Card detect interrupt for card remove
 *---------------------------------------------------------------------------*/
void isr_card_remove()
{
    sysprintf("--- ISR: card removed on SD port 0 ---\n\n");
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
    init_UART();
    init_timer();

    sysprintf("\n=====> W55FA92 Non-OS SIC/SD Driver Sample Code [tick %d] <=====\n", sysGetTicks(0));

    sysprintf("\n");
    sysprintf("===============================================================\n");
    sysprintf("=== NOTE: The SD card detect interrupt feature is DISABLED  ===\n");
    sysprintf("===       by default in SIC driver.                         ===\n");
    sysprintf("===       Please make sure the keyword \"_SIC_USE_INT_\" is   ===\n");
    sysprintf("===       defined in fmi.h and rebuild SIC driver to ENABLE ===\n");
    sysprintf("===       SD card detect interrupt feature.                 ===\n");
    sysprintf("===============================================================\n");
    sysprintf("\n");

    //--- Initial system clock
    sicIoctl(SIC_SET_CLOCK, sysGetPLLOutputHz(eSYS_UPLL, sysGetExternalClock())/1000, 0, 0);    // clock from PLL

    //--- Enable AHB clock for SIC/SD/NAND, interrupt ISR, DMA, and FMI engineer
    sicOpen();

    //--- Initial callback function for card detection interrupt
    sicIoctl(SIC_SET_CALLBACK, FMI_SD_CARD, (INT32)isr_card_remove, (INT32)isr_card_insert);

    //--- Enable SD card detection feature
    sicIoctl(SIC_SET_CARD_DETECT, TRUE, 0, 0);  // MUST call sicIoctl() BEFORE sicSdOpen0()

    //--- Initial SD card on port 0
    sicSdOpen0();

    sysprintf("Please insert or remove SD card to trigger SD card detect interrupt ...\n");
    while(1);
}
