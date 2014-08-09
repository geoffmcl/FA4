/*\
 * Fa4.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * primary single header that include all other headers
 *
\*/
#ifndef	_Fa4_h
#define	_Fa4_h

#ifndef	FA4
// should be SUPPLIED by compiler !!!
#define		FA4
#endif	/* !FA4 */

#ifdef	NDEBUG
#undef	FA4DBG
#else
#define	FA4DBG
#endif	/* NDEBUG y/n */

#include	"Fa4Vers.h"		// include our VERSION file = MAJ, MIN & BLD_VERS
#include    <sys/types.h>
#include    <sys/stat.h>
#include   "Fa4CWin.h"  // compatible with windows
#ifdef WIN32
#include	"FastComp.h"
#include	<windows.h>
#include	<conio.h>
#endif // WIN32
#include	<stdlib.h>
#include    <stdint.h>
#include	<stdio.h>
#include	<setjmp.h>
#include	<math.h>
#include	<time.h>
#ifdef   ADD_REGEX
#include    <pcre.h>
#endif   // ADD_REGEX
#include    <dirent.h>

//#include	<TimeDF.h> // Include my TIME macros // Macros to UNPACK DOS FILE DATE/TIME
//#include "D:/mssdk/include/TimeDF.h"
#define DFHour( a )	(( a & 0xf800 ) >> 11 )
#define DFMins( a )	(( a & 0x07e0 ) >> 5  )
#define DFSecs( a )	(( a & 0x001f ) *  2  )
#define DFYear( a )	((( a & 0xfe00 ) >> 9  ) + 80)
#define DFMonth( a )	(( a & 0x01e0 ) >> 5  )
#define DFDay( a )	( a & 0x001f )
// wFatDate = (mth << 5) + (day & 0x0f) + ((yr+1980) << 9);
// wFatTime = (hr << 11) + (min << 5) + ((sec / 2) & 0x0f);
#define DOSFT( hr, min, sec )	((hr << 11) + (min << 5) + ((sec / 2) & 0x0f))
#define DOSFD( yr, mth, day )	((mth << 5) + (day & 0x0f) + ((yr+1980) << 9))

#define  PLE   PLIST_ENTRY
#define  VFH(a)   ( a && ( a != INVALID_HANDLE_VALUE ) )

typedef  struct   tagMWL {
   LIST_ENTRY   wl_List;
   uint32_t     wl_dwRank; // for ordering later
   uint32_t     wl_dwItems; // items to be found
   uint32_t     wl_dwFound; // find count in this file
   uint64_t     wl_DateTime64; // age is important
   uint64_t     wl_FileSize64; // TODO: Add the SIZE of the FAIL
   char         wl_cName[264];
}MWL, * PMWL;

#include "Fa4Work.h"		// global data structure
#include "Fa4Help.h"
#include "Fa4List.h"    // just copy of YAHUList.h
// #include	"Fa4Con.h"		// CONSOLE entry point
#include "Fa4Util.h"    // general utility module - geting quite COMMON
#include "Fa4Out.h" // out list
#include "Fa4Wild.h" // handle WILD file names

// OTHER includes ...
#include	"grmLib.h"
//#ifndef	NDEBUG
//#ifdef	FA4DBG
// then the prt(char *) is extenal, so
#include	"grmOut.h"
//#else
// provide our own CONSOLE i/o
// could just feed it to print(char *), or not, as desired
//void	prt( char * lps );
// and define a valid handle as 
//#define		VH(a)		( ( a ) && ( a != INVALID_HANDLE_VALUE ) )
//#endif	// !NDEBUG
//#include	"F:\GTools32\Utils\GmZList.h"
#ifndef PATH_SEP
#ifdef WIN32
#define PATH_SEP "\\"
#define PATH_CHAR '\\'
#else
#define PATH_SEP "/"
#define PATH_CHAR '/'
#endif
#endif // PATH_SEP

extern	jmp_buf mark;			// Address for long jump to jump to
extern	void	Pgm_Exit( WS );	// ALL should come here to exit
extern   void	Error_Exit( WS, int val );
extern   void	Find_In_File( WS );
#ifdef USE_EXCLUDE_LIST
extern int	InExcludeD( char * lpact );
#endif

extern char *g_ProgramName;

#define	Err_Exit( a )	Error_Exit( pWS, a )
#define CHKMEM(a) if( !a ) { prt("C:ERROR: MEMORY FAILED!"MEOR ); exit(-1); }


#endif		// _Fa4_h
// eof - Fa4.h
