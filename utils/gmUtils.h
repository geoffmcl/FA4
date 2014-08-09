
// GmUtils.h
#ifndef	_GmUtils_h
#define	_GmUtils_h

//#include	<windows.h>
//#include	<direct.h>
#include <stdint.h>
#include "gmFile.h"

//#ifndef	BYTES_PER_READ
//#define	BYTES_PER_READ		1024
//#endif	// BYTES_PER_READ

// This module exposes
//int	DVGetCwd( char * lpb, uint32_t siz );
//int    GetRootDir( char * lpf );
//int CenterWindow(HWND hwndChild, HWND hwndParent);
//void Hourglass (int bDisplay);
//int	EnsureCrLf( char * lpd, char * lps );
//char *	DVf2s( double source );
//#ifdef	WIN32
//uint32_t	GetTextExtent( HDC hdc, char * lpS, int len );
//#endif	// WIN32

//char	szCWD[MAX_PATH+16] = { "\0" };

//
// Get current work directory
//
extern	int	DVGetCwd( char * lpb, uint32_t siz );
//
// int    GetRootDir( char * lpf )
//
// Purpose: Get the ROOT RUNTIME Directory
//			This could be the current work directory,
//			that is the directory "Windows" has as
//			the "current",
//			*** OR ***
//			The directory where this EXE was run
//			from.
// Input:	char * lpf - Buffer for results
// Output:	int - TRUE if OK
//
extern	int    GetRootDir( char * lpf );

#ifdef WIN32
//
//  FUNCTION: CenterWindow(HWND, HWND)
//
//  PURPOSE:  Center one window over another.
//
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//
//  RETURN VALUE:
//
//    TRUE  - Success
//    FALSE - Failure
//
//  COMMENTS:
//
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
//

extern	int CenterWindow(HWND hwndChild, HWND hwndParent);
//---------------------------------------------------------------------
//
// Function:   Hourglass
//
// Purpose:    Displays or hides the hourglass during lengthy operations.
//
// Parms:      bDisplay == TRUE to display, false to put it away.
//
// History:	Date			Reason
//
//			6/1/91			Created.
//			17 June, 1997	Moved to DvUtil.c             
//---------------------------------------------------------------------

extern	void Hourglass (int bDisplay);
// Include the service TransparentBlt( HDC, HBITMAP,
//	DestX, DestY, COLORREF )
// ===============================================

//#include	"DvTrans.h"
#endif // WIN32

// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
extern	int	EnsureCrLf( char * lpd, char * lps );
//#define	MXFSB	32
//#define	MXFSBS	8
//char	szFSBufs[MXFSB*MXFSBS];
//int		iFSBuf = 0;
//int	bFilNPlace = FALSE; //TRUE;	// Put a SPACE for the NEG sign
//int	bAddSig3 = FALSE; //TRUE;	// Always min. 3 digits before decimal
//int	bFilNeg10 = FALSE; //TRUE;
//int	bFilPos10 = FALSE; //TRUE;

extern	char *	GetABuf( void );
#define		MXONE		   256
//#define		MXONEX	   (MXONE + 8)
//#define  GetNxtBuf   _sGetSStg
extern   char *	GetStgBuf( void );

#ifdef   ADD_SPRTF
extern void MCDECL sprtf( char * lpf, ... );
#endif   // #ifdef   ADD_SPRTF
#ifdef   ADD_RECT2STG
extern   char *	Rect2Stg( PRECT lpr );
#endif // ADD_RECT2STG
#ifdef   ADD_POINT2STG
extern   char *	Pt2Stg( PPOINT ppt );
#endif // ADD_POINT2STG

//
// Convert "double" to string
// with a little bit of conditioning
// 
extern	char *	DVf2s( double source );

#if defined(WIN32) && defined(NEED_STG_FUNCS)
extern	uint32_t DVWrite(int fh, char * pv, uint32_t ul );
extern	HFILE	DVOpenFile( char * lpf, LPOFSTRUCT pof, uint32_t uStyle );
extern	uint32_t DVSeekEnd( HFILE hf );
extern	HFILE DVlclose( HFILE hf );
#endif // #if defined(WIN32) && defined(NEED_STG_FUNCS)

/* Given a BUFFER, the Current Direcory and a File name */
/* Build a complete PATH into the Buffer */
extern	void	SRSetupPath( char * lpb, char * lpd, char * lpf );
#ifdef	WIN32
extern	uint32_t	GetTextExtent( HDC hdc, char * lpS, int len );
#endif	// WIN32
extern	char	GMUpper( char c );
extern	int		GMInStr( char * lpsrc, char * lpfind );
// Specialised GMInStr for checking a files EXTENSION
extern	int	GMIsExt( char * lpsrc, char * lpfind );

// **********************************************************
// Added 1998 May 17
// =================
/* =====================================================================
 *
 * GetFPath( lpFullPath, lpDrive, lpPath, lpFile, lpExtent )
 *
 * Purpose: Split the supplied FULL PATH into four components if
 * the buffers are supplied. Those components NOT required may be
 * NULL. This is a FAR PTR implementation of _splitpath in STDLIB.H.
 * The supplied buffers must be at least equal to the manifest contants
 * of _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT in stdlib.h ...
 * The Drive will include the ':'; The Directory will include both the
 * leading and trailing '\' or '/'; The file name will be exactly that;
 * the Extent will include the '.' character; Such that a recombination
 * by say strcat will put it all back together ...
 * Any component requested that does not exist in the FULL PATH will 
 * contain a nul string.
 *
 * ===================================================================== */
//extern	void	GMGetFPath( char * lpc, char * lpd, char * lpp, char * lpf, char * lpe );
extern   void	GMGetFPath( char * lpFull, char * lpDrive, char * lpPath,
                          char * lpFileName, char * lpExtension ); // splitfilename

extern	int	GMGetFName( char * lpfull, char * lpname );
extern	int	GMRTrimStg( char * lps );
#ifndef  ALGN
#define  ALGN     4
#endif   // ALGN

typedef  struct tagSPLITFILENAME {
   char szDrive[ _MAX_DRIVE + ALGN ];
   char szPath[ _MAX_DIR + ALGN ];
   char szFN[ _MAX_FNAME + ALGN ];
   char szExt[ _MAX_EXT + ALGN ];
}SPLITFILENAME, * PSLITFILENAME;

#define  GMGFP(a,b)  GMGetFPath( a, (*b).szDrive, (*b).szPath, (*b).szFN, (*b).szExt )

#ifndef ECVT
#ifdef WIN32
#define ECVT _ecvt
#else
#define ECVT ecvt
#endif
#endif // ECVT

#endif	// _GmUtils.h

// **********************************************************
// eof - GMUtils.h
