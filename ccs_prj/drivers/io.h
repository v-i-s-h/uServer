//*****************************************************************************
//
// io.h - Prototypes for I/O routines.
//
//*****************************************************************************

#ifndef __IO_H__
#define __IO_H__

#ifdef __cplusplus
extern "C"
{
#endif

void io_init( void );
void io_getDeviceStatus( char *pcBuf, int iLen );
void io_getAnalogData( char *pcBuff, int iLen );

#ifdef __cplusplus
}
#endif

#endif // __IO_H__
