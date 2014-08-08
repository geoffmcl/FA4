
// Fa4Wild.h
#ifndef  _Fa4Wild_h
#define  _Fa4Wild_h

extern void	Process_Wilds( WS, char * lpwild );

extern uint32_t g_dwFoundFileCnt;
extern uint32_t g_dwFoundDirsCnt;
extern uint64_t g_ulTotalBytes;
extern uint64_t g_ulTotalBRej;
extern uint32_t g_dwFoundRejCnt;

#define g_dwFoundMatchCnt ( g_dwFoundFileCnt - g_dwFoundRejCnt ) // difference

#endif   // #ifndef  _Fa4Wild_h
// eof - Fa4Wild.h


