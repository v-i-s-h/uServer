/*
 * cgiHandlers.h
 *
 *  Created on: 12-Jul-2014
 *      Author: vish
 *
 *  This module will define the CGI handlers.
 */

#ifndef __CGIHANDLERS_H__
#define __CGIHANDLERS_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "httpserver/httpd.h"
#include "drivers/pinout.h"
#include "drivers/io.h"
#include "drivers/devices.h"
#include "httpserver/cgifuncs.h"

//*****************************************************************************
//
// Prototypes for the various CGI handler functions.
//
//*****************************************************************************
char* hCGI_ioControl(int32_t iIndex, int32_t i32NumParams,
                               char *pcParam[], char *pcValue[]);

//*****************************************************************************
//
// CGI URI indices for each entry in the g_psConfigCGIURIs array.
//
//*****************************************************************************
#define CGI_INDEX_IOCONTROL       0

//*****************************************************************************
//
// This array is passed to the HTTPD server to inform it of special URIs
// that are treated as common gateway interface (CGI) scripts.  Each URI name
// is defined along with a pointer to the function which is to be called to
// process it.
//
//*****************************************************************************
static const tCGI g_psConfigCGIURIs[] =
{
    { "/io_control.cgi", (tCGIHandler)hCGI_ioControl }, // CGI_INDEX_CONTROL
};

//*****************************************************************************
//
// The number of individual CGI URIs that are configured for this system.
//
//*****************************************************************************
#define NUM_CONFIG_CGI_URIS     (sizeof(g_psConfigCGIURIs) / sizeof(tCGI))

//*****************************************************************************
//
// The file sent back to the browser by default following completion of any
// of our CGI handlers.  Each individual handler returns the URI of the page
// to load in response to it being called.
//
//*****************************************************************************
#define DEFAULT_CGI_RESPONSE    "/getDeviceStatus.php"

//*****************************************************************************
//
// The file sent back to the browser in cases where a parameter error is
// detected by one of the CGI handlers.  This should only happen if someone
// tries to access the CGI directly via the broswer command line and doesn't
// enter all the required parameters alongside the URI.
//
//*****************************************************************************
#define PARAM_ERROR_RESPONSE    "/perror.htm"

#endif
