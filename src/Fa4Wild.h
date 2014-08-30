/*\
 * Fa4Wild.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * handle wild card file name input ...
 * Scan a directory, and do finds in files matching the input mask
 *
\*/
#ifndef  _Fa4Wild_h
#define  _Fa4Wild_h

extern void	Process_Wilds( WS, char * lpwild ); // FIX20140830: Slight miss naming - now does ALL files/dirs

extern uint32_t g_dwFoundFileCnt;
extern uint32_t g_dwFoundDirsCnt;
extern uint64_t g_ulTotalBytes;
extern uint64_t g_ulTotalBRej;
extern uint32_t g_dwFoundRejCnt;

#define g_dwFoundMatchCnt ( g_dwFoundFileCnt - g_dwFoundRejCnt ) // difference

#endif   // #ifndef  _Fa4Wild_h
// eof - Fa4Wild.h


