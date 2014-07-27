//*****************************************************************************
//
// io.c - I/O routines
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "utils/ustdlib.h"
#include "drivers/pinout.h"
#include "drivers/devices.h"
#include "io.h"

//*****************************************************************************
//
// Initialize the IO used in this demo
//
//*****************************************************************************
void
io_init(void)
{
	//
	// Configure the device pins.
	//
	PinoutSet( false, true );

    // @TODO : Set up leds here

	// @TODO : Initialize LEDs here.
	// Set all LEDs ON untill acuiring IP, these LEDs will be turned OFF by EthHandlers
	LEDWrite( CLP_D1|CLP_D2|CLP_D3|CLP_D4, CLP_D1|CLP_D2|CLP_D3|CLP_D4 );

	// @TODO : Enable TIMER peripherals

	// @TODO : Configure TIMER

	// @TODO : Enable Interrupts

}

//*****************************************************************************
// Get device state
//*****************************************************************************
void
io_getDeviceStatus( char *pcBuf, int iLen ) {

	uint32_t devStatus = 0x00000000;

	LEDRead( &devStatus );

	usnprintf( pcBuf, iLen,
				"%1d,%1d,%1d,%1d,0,0,0,0,0,0,0,0,0,0",
				(devStatus&CLP_D1)?1:0,
				(devStatus&CLP_D2)?1:0,
				(devStatus&CLP_D3)?1:0,
				(devStatus&CLP_D4)?1:0 );

}
