/*\
 * WinErr.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * convert windows error values to a string ...
 *
\*/
#ifndef	_WinErr_H
#define	_WinErr_H

typedef struct {
	long	we_lgVal;
	char *	we_lpStg;
	char *	we_lpEStg;
}WER;
typedef WER * LPWER;
#ifdef WIN32
extern void OutWin32Err( long lErr );
#endif // WIN32

#endif	/* _WinErr_H */
// eof - WinErr.h
