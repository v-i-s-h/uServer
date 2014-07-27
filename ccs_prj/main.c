//*****************************************************************************
// uServer : Embedded Server for TM4C1294XL
// Tested on EK-TM4C1294XL with TI CC v5.1.5
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/udma.h"
#include "usblib/usblib.h"
#include "usblib/usbmsc.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/cmdline.h"
#include "drivers/pinout.h"
#include "drivers/io.h"
#include "drivers/devices.h"
#include "httpserver/httpd.h"
#include "httpserver/cgiHandlers.h"
#include "fatfs/src/ff.h"
#include "usbFilesystem/usbFilesystem.h"
#include "system.h"

#define APP_BUILD_STRING 	"uServer with USB for TM4C1294XL\n"
#define APP_LOCATOR_STRING 	"EK-TM4C1294XL uServer USB"

// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

// External Application references.
extern void httpd_init(void);

// Timeout for DHCP address request (in seconds).
#ifndef DHCP_EXPIRE_TIMER_SECS
#define DHCP_EXPIRE_TIMER_SECS  45
#endif

// The current IP address.
uint32_t g_ui32IPAddress;
// The system clock frequency.
uint32_t g_ui32SysClock;

// The buffer that holds the command line.
char g_cCmdBuf[CMD_BUF_SIZE];

// This is the table that holds the command names, implementing functions, and
// brief description.
tCmdLineEntry g_psCmdTable[] = {
    { "help",   Cmd_help,      " : Display list of commands" },
    { "h",      Cmd_help,   "    : alias for help" },
    { "?",      Cmd_help,   "    : alias for help" },
    { "ls",     Cmd_ls,      "   : Display list of files" },
    { "chdir",  Cmd_cd,         ": Change directory" },
    { "cd",     Cmd_cd,      "   : alias for chdir" },
    { "pwd",    Cmd_pwd,      "  : Show current working directory" },
    { "cat",    Cmd_cat,      "  : Show contents of a text file" },
    { 0, 0, 0 }
};

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// The interrupt handler for the SysTick interrupt.
void
SysTickIntHandler(void)
{
    // Update our tick counter.
    updateUSBFsTick();
    // Call the lwIP timer handler.
    lwIPTimer(SYSTICKMS);
}

// Display an lwIP type IP Address.
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    // Convert the IP Address into a string.
    usprintf( pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
              (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    // Display the string.
    UARTprintf(pcBuf);
}




// Required by lwIP library to support any host-related timer functions.
void
lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    // Get the current IP address.
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    // See if the IP address has changed.
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        // See if there is an IP address assigned.
        if(ui32NewIPAddress == 0xffffffff)
        {
            // Indicate that there is no link.
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            // There is no IP address, so indicate that the DHCP process is
            // running.
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            // Display the new IP address.
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTprintf("\n");
            UARTprintf("Open a browser and enter the IP address.\n");
            // Turn off the LEDs as we have acquired IP now.
            LEDWrite( CLP_D1|CLP_D2|CLP_D3|CLP_D4, 0 );
        }

        // Save the new IP address.
        g_ui32IPAddress = ui32NewIPAddress;
    }
}



int
main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];
    int iStatus;

    // ------------------------------------------------------------------------
	// For log file
	UINT logId = 0;
	FRESULT logResult;
	char logLine[64];
	WORD logLength = 0;
	UINT bytesWritten = 0;
	ULONG historySize = 0;
	static FIL logObject;
	// ------------------------------------------------------------------------

    //------------------------------- START BOARD SETUP ----------------------------------
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins.  The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    // Run from the PLL at 120 MHz.
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    // Initialize IO controls
    // This will set leds D1-D4 for user access and enable USB
    io_init();
    // ----------------------------------------------------------------------------------

    //------------------------------ START UART -----------------------------------------
    // Configure debug port for internal use.
    UARTStdioConfig(0, 115200, g_ui32SysClock);
    // Print application banner
    UARTprintf( APP_BUILD_STRING );
    //-----------------------------------------------------------------------------------

    //---------------------------- SETUP INTERRUPTS -------------------------------------
    // Configure SysTick for a periodic interrupt.
    MAP_SysTickPeriodSet(g_ui32SysClock / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);
    //-----------------------------------------------------------------------------------

    // ----------------------------- SETUP USBMSC FILESYSTEM ----------------------------
    initUsbFs();
	//-----------------------------------------------------------------------------------

    //--------------------------- SETUP SERVER ------------------------------------------
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        // Let the user know there is no MAC address
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }
    // Tell the user what we are doing just now.
    UARTprintf("Waiting for IP.\n");
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
    // MAC address needed to program the hardware registers, then program
    // the MAC address into the Ethernet Controller registers.
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);
    // Initialze the lwIP library, using DHCP.
    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);
    // Setup the device locator service.
    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet( APP_LOCATOR_STRING );
    // Initialize a sample httpd server.
    httpd_init();
    // Pass our CGI handlers to the HTTP server.
    http_set_cgi_handlers(g_psConfigCGIURIs, NUM_CONFIG_CGI_URIS);
    //-----------------------------------------------------------------------------------

    // -------------------------------LOOP FOR EVER -------------------------------------
    while(1)
    {
    	// Get a line of text from the user.
    	ReadLine( g_cCmdBuf, sizeof(g_cCmdBuf) );

    	// ------------------------------------------------------------------------
   		// log the cmd
   		logResult = f_open( &logObject, "/cmd.log", FA_OPEN_ALWAYS|FA_WRITE );
   		if( logResult != FR_OK ) {
   			UARTprintf( "Logging failed :: %s\n",
   						 StringFromFresult(logResult) );
   		}
   		else {
   			historySize = (&logObject)->fsize;
   			// Seek to end
   			f_lseek(&logObject, historySize );
   			logId++;
   			usprintf( logLine, "%3d %s\r\n", logId, g_cCmdBuf);
   			logLength = ustrlen( logLine );
   			logResult = f_write( &logObject, logLine, logLength, &bytesWritten ) ;
   			if( logResult != FR_OK ) {
   				UARTprintf( "Writing to file failed :: %s\n",
   							 StringFromFresult(logResult) );
   			}
   			else {
   				// UARTprintf( "Logged %2dB of %2dB  :: %s\n", bytesWritten, logLength, logLine );
   			}
   			f_close( &logObject );
   		}
   		// ------------------------------------------------------------------------


    	if(g_cCmdBuf[0] == '\0') {
    		continue;
    	}
    	// Pass the line from the user to the command processor.
    	// It will be parsed and valid commands executed.
    	iStatus = CmdLineProcess(g_cCmdBuf);
    	// Handle the case of bad command.
    	if(iStatus == CMDLINE_BAD_CMD) {
    		UARTprintf("Bad command!\n");
    	}
        // Handle the case of too many arguments.
        else if(iStatus == CMDLINE_TOO_MANY_ARGS) {
        	UARTprintf("Too many arguments for command processor!\n");
        }
    	// Otherwise the command was executed.  Print the error
    	// code if one was returned.
        else if(iStatus != 0) {
        	UARTprintf("Command returned error code %s\n",
        				StringFromFresult((FRESULT)iStatus));
        }
    }
}
