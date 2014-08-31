/*\
 * grmOut.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * file and console io ...
 *
\*/
#ifndef	_GrmOut_h
#define	_GrmOut_h
#include <stdint.h>
#include "grmLib.h"

#ifndef MCDECL

#ifdef WIN32
#define MCDECL _cdecl
#else
#define MCDECL
#endif // WIN32

#endif // MDECL

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (HANDLE)-1
#endif

#ifndef HFILE_ERROR
#define HFILE_ERROR (HANDLE)-1
#endif

#define		VH(a)		( ( a ) && ( a != INVALID_HANDLE_VALUE ) )

extern	void	oi( char * lps );
extern	void	prt( char * lps );

extern	void	PutThous( char * lpb, int len, uint32_t size );
extern	char *	ShortName( char * lps, char * lpd, int siz );

extern	void	   SetEnsureCrLf( int i );
extern	int	   GetEnsureCrLf( void );

extern	HANDLE	hUserOut;
extern   HANDLE   hErrOut; // cosole ERROR out - only valid after call to oi()
extern   int     bRedirON;   // TRUE if standard out is being REDIRECTED
// NOTE: This bRedirON is only VALID AFTER the first oi() call

#endif	// _GrmOut_h
// eof - grmOut.h
