/*
 * usbFilesystem.h
 *
 *  Created on: 13-Jul-2014
 *      Author: vish
 */

#ifndef USBFILESYSTEM_H_
#define USBFILESYSTEM_H_

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
#include "fatfs/src/ff.h"
#include "usblib/usblib.h"
#include "usblib/usbmsc.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
#include "utils/ustdlib.h"
#include "utils/cmdline.h"
#include "utils/uartstdio.h"
#include "system.h"

//*****************************************************************************
// Defines the size of the buffers that hold the path, or temporary data from
// the memory card.  There are two buffers allocated of this size.  The buffer
// size must be large enough to hold the longest expected full path name,
// including the file name, and a trailing null character.
//*****************************************************************************
#define PATH_BUF_SIZE   80

//*****************************************************************************
// Defines the size of the buffer that holds the command line.
//*****************************************************************************
#define CMD_BUF_SIZE    64

//*****************************************************************************
// Defines the number of times to call to check if the attached device is
// ready.
//*****************************************************************************
#define USBMSC_DRIVE_RETRY      4

//*****************************************************************************
// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
//*****************************************************************************
static char g_cCwdBuf[PATH_BUF_SIZE] = "/";

//*****************************************************************************
// A temporary data buffer used when manipulating file paths, or reading data
// from the memory card.
//*****************************************************************************
static char g_cTmpBuf[PATH_BUF_SIZE];

//*****************************************************************************
// Current FAT fs state.
//*****************************************************************************
static FATFS 	g_sFatFs;
static DIR 		g_sDirObject;
static FILINFO 	g_sFileInfo;
static FIL 		g_sFileObject;
//*****************************************************************************
// A structure that holds a mapping between an FRESULT numerical code,
// and a string representation.  FRESULT codes are returned from the FatFs
// FAT file system driver.
//*****************************************************************************
typedef struct
{
    FRESULT fresult;
    char *pcResultStr;
} tFresultString;

//*****************************************************************************
// A macro to make it easy to add result codes to the table.
//*****************************************************************************
#define FRESULT_ENTRY(f)        { (f), (#f) }

//*****************************************************************************
// Hold the current state for the application.
//*****************************************************************************
typedef enum
{
    // No device is present.
    STATE_NO_DEVICE,
    // Mass storage device is being enumerated.
    STATE_DEVICE_ENUM,
    // Mass storage device is ready.
    STATE_DEVICE_READY,
    // An unsupported device has been attached.
    STATE_UNKNOWN_DEVICE,
    // A mass storage device was connected but failed to ever report ready.
    STATE_TIMEOUT_DEVICE,
    // A power fault has occurred.
    STATE_POWER_FAULT
} tState;

//*****************************************************************************
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.  In this application uDMA is only used for USB,
// so only the first 6 channels are needed.
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
static tDMAControlTable g_sDMAControlTable[6];
#elif defined(ccs)
#pragma DATA_ALIGN(g_sDMAControlTable, 1024)
static tDMAControlTable g_sDMAControlTable[6];
#else
static tDMAControlTable g_sDMAControlTable[6] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
// The class of the unknown device.
//*****************************************************************************
uint32_t g_ui32UnknownClass;

// Function declarations
uint32_t GetTicks( void );
const char *StringFromFresult(FRESULT fresult);
int Cmd_ls(int argc, char *argv[]);
int Cmd_cd(int argc, char *argv[]);
int Cmd_pwd(int argc, char *argv[]);
int Cmd_cat(int argc, char *argv[]);
int Cmd_help(int argc, char *argv[]);
void MSCCallback(tUSBHMSCInstance *ps32Instance, uint32_t ui32Event, void *pvData);
void initUsbFs( void );
void ReadLine( char *cmdBuf, uint16_t buffLen );
void updateUSBFsTick( void );

#endif /* USBFILESYSTEM_H_ */
