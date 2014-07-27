/*
 * cgiHandlers.c
 *
 *  Created on: 12-Jul-2014
 *      Author: vish
 *
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cgiHandlers.h"

//*****************************************************************************
//
// This CGI handler is called whenever the web browser requests iocontrol.cgi.
// This function will set LED and PIN status
//
//*****************************************************************************
char*
hCGI_ioControl(int32_t iIndex, int32_t i32NumParams, char *pcParam[],
                  char *pcValue[])
{
	int32_t i32DevId;
	int32_t i32Cmd;
	bool bParamError;
	int8_t i8Scope = 0x00;
	int8_t i8Protocol = 0x00;
	int8_t i8Id = 0x00;

	i32DevId 	= 0x00000000;
	i32Cmd 		= 0x00000000;
	bParamError = false;
    // Get deviceId
    i32DevId 	= GetCGIParam( "id", pcParam, pcValue, i32NumParams, &bParamError );
    i32Cmd		= GetCGIParam( "val", pcParam, pcValue, i32NumParams, &bParamError );

    // Was there any error reported by the parameter parser?
    if( bParamError ) {
        return( PARAM_ERROR_RESPONSE );
    }

    // Identify which device and command
    i32DevId = i32DevId & 0x0000FFFF;			// we need only 16bits
    i8Scope = (i32DevId & 0x3000) >> 10;
    i8Protocol = ( i32DevId & 0x0300) >> 8;
    i8Id = ( i32DevId & 0x00FF);

    if( i8Scope == 0x00 ) {	// internal device
    	if( i8Protocol == 0x00 ) {	//	no special protocol
    		switch( i8Id ) {
    			case DEV_INTERNAL_LED1:
    			    UARTprintf( "Triggering LED1.\n" );
    			    LEDWrite( CLP_D1, (i32Cmd)?CLP_D1:0 );
    			    break;
    			case DEV_INTERNAL_LED2:
    			    UARTprintf( "Triggering LED2.\n" );
    			    LEDWrite( CLP_D2, (i32Cmd)?CLP_D2:0 );
    			    break;
    			case DEV_INTERNAL_LED3:
    			    UARTprintf( "Triggering LED3.\n" );
    			    LEDWrite( CLP_D3, (i32Cmd)?CLP_D3:0 );
    			    break;
    			case DEV_INTERNAL_LED4:
    			    UARTprintf( "Triggering LED4.\n" );
    			    LEDWrite( CLP_D4, (i32Cmd)?CLP_D4:0 );
    			    break;

    			default:
    				UARTprintf( "Unkown device ID.\n");
    				break;
    		}
    	} else {
    		UARTprintf( "Protocol not supported.\n");
    	}
    } else {
    	UARTprintf( "Device scope not supported.\n");
    }

    return( DEFAULT_CGI_RESPONSE );
}
