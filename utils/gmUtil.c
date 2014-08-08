

// gmUtil.c
#ifdef WIN32
#ifndef  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#endif // #ifndef  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#include <windows.h>
#include <direct.h>
#else // !WIN32
#include <string.h> // fro strlen(), ...
#endif // WIN32
#include <stdio.h>
#include "gmUtils.h"		// Exposes service of module

#ifdef USE_GMUTIL_MODULE

#ifndef	BYTES_PER_READ
#define	BYTES_PER_READ		1024
#endif	// BYTES_PER_READ

// This module contains
//int	DVGetCwd( char * lpb, uint32_t siz );
//int    GetRootDir( char * lpf );
//int CenterWindow(HWND hwndChild, HWND hwndParent);
//void Hourglass (int bDisplay);
//int	EnsureCrLf( char * lpd, char * lps );
//char *	DVf2s( double source );
//#ifdef	WIN32
//uint32_t	GetTextExtent( HDC hdc, char * lpS, int len );
//#endif	// WIN32
// but see GmUtils.h for the complete list exposed
// ===============================================

char	szCWD[MAX_PATH+16] = { "\0" };

#ifdef   ADD_STGBUF

#define		MXSTGS		32
#define		MXONE		   256
#define		MXONEX	   (MXONE + 8)
//#define  GetNxtBuf   _sGetSStg
char *	GetStgBuf( void )
{
	char *	lprs;
	static char szP2S[ (MXSTGS * MXONEX) ];
	static LONG  iNP2S = 0;
	// NOTE: Can be called only MXSTGS times before repeating
	lprs = &szP2S[ (iNP2S * MXONEX) ];	// Get 1 of ? buffers
	iNP2S++;
	if( iNP2S >= MXSTGS )
		iNP2S = 0;
	return lprs;
}

#endif   // #ifdef   ADD_STGBUF

#ifdef   ADD_SPRTF

extern   void DiagMsg( char * lpd );
void MCDECL sprtf( char * lpf, ... )
{
   static char _s_sprtfbuf[1024];
   char *      lpb = &_s_sprtfbuf[0];
   int         i;
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   i = vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   //sprtf(lpb);
   //prt(lpb);
   DiagMsg(lpb);
}

#ifdef   ADD_CHKME
extern   uint32_t InStr( char * lps, char * lpi );
#endif   // ADD_CHKME
void MCDECL chkme( char * lpf, ... )
{
   static char _s_chkmebuf[1024];
   static int _s_incheckme = 0;
   char *      lpb = &_s_chkmebuf[0];
   int         i;
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   i = vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   //sprtf(lpb);
   //prt(lpb);
#ifdef   ADD_CHKME
#ifndef  BVTEST
   // add some 'visible' noise if string small
   if( i < 200 )
   {
      char *   lps = GetStgBuf();
      *lps = 0;
      if( !InStr(lpb,"CHECKME") )
         strcpy(lps,"CHECKME: ");
      if( !InStr(lpb,"WARNING") )
         strcat(lps,"WARNING: ");
      strcat(lps,lpb);
      DiagMsg(lps);
   }
   else
#endif // #ifndef  BVTEST
#endif   // ADD_CHKME
   {
      DiagMsg(lpb);
   }
   if( !_s_incheckme )
   {
      _s_incheckme = 1;
      if(( i > 2         ) &&
         ( lpb[0] == 'C' ) &&
         ( lpb[1] == ':' ) )
      {
         i = MessageBox(NULL,
            lpb,
            "CRITICAL ERROR: CHECK ME",
            (MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_SYSTEMMODAL) );
         if( i == IDABORT )
         {
            //DestroyWindow(ghwndFrame);
            exit(-1);
         }
      }
      _s_incheckme = 0;
   }
}

#endif   // #ifdef   ADD_SPRTF

#ifdef   ADD_RECT2STG
char *	Rect2Stg( PRECT lpr )
{
	char *	lps = GetStgBuf();
   sprintf( lps,
			"(%d,%d,%d,%d)",
			lpr->left,
			lpr->top,
			lpr->right,
			lpr->bottom );
	return lps;
}

#endif   // #ifdef   ADD_RECT2STG

#ifdef   ADD_POINT2STG
char *	Pt2Stg( PPOINT ppt )
{
	char *	lps = GetStgBuf();
   sprintf( lps,
			"(%d,%d)",
			ppt->x,
			ppt->y );
	return lps;
}
#endif // #ifdef   ADD_POINT2STG

//
// Get current work directory
//
int	DVGetCwd( char * lpb, uint32_t siz )
{
	char *	lpd;
	int		i;

	i = 0;
	lpd = &szCWD[0];
	if( *lpd == 0 )
	{
		_getcwd( lpd, MAX_PATH );
		if( i = strlen( lpd ) )
		{
			if( lpd[i-1] != '\\' )
				strcat( lpd, "\\" );
		}
	}
	if( lpb && siz )	// Check the CALLER
	{
		*lpb = 0;
		if( i = strlen( lpd ) )
		{
			if( (uint32_t)i < siz )
			{
				strcpy( lpb, lpd );
			}
			else
			{
				for( i = 0; (uint32_t)i < siz; i++ )
				{
					lpb[i] = lpd[i];
				}
				i--;
				lpb[i] = 0;
			}
		}
		i = strlen( lpb );
	}

	return i;
}

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
int    GetRootDir( char * lpf )
{
	int    flg;
	uint32_t   dwL, dwO, dwI;
	char	buf[260];
	char    c;
	int		i;
	char *	lpb;

	flg    = FALSE;		// Assume FAILED
	if( lpf )	// If given a buffer
	{
		*lpf = 0;		// Start with NOTHING
		lpb = &buf[0];	// Set up a pointer
		buf[0] = 0;		// Init it to zero also
		//_getcwd( &buf[0], 256 );	// Get DIRECTORY
		if( i = DVGetCwd( lpb, 256 ) )	// Get DIRECTORY
		{
			// Simple - Copy it to CALLER
			strcpy( lpf, lpb );	// Copy it, and
			flg = TRUE;				// Set OK
		}
		else if( dwL = GetModuleFileName( NULL, lpf, MAX_PATH ) )
		{
			// This method has the BAD effect of returning
			// like D:\fff\fff\DEBUG\ or RELEASE,
			// or other MS Studio folders where the EXE
			// is written.
			// BUT, the _getcwd should NEVER fail!!!
			dwO = 0;
			// Move only the ROOT without module name
			for( dwI = 0; dwI < dwL; dwI++ )
			{
				c = lpf[dwI];
				if( (c == '\\') ||
					(c == '/') ||
					(c == ':') )
				{
					// Keep LAST of any of these
					dwO = dwI + 1;
				}
			}
			lpf[dwO] = 0;	// Zero terminate it
			if( dwO )	// if length
				flg = TRUE;
		}
	}
	return( flg );
}

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

int CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the Height and Width of the parent window
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

    // Get the display limits
    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
         xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
}

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

void Hourglass (int bDisplay)
{
   static HCURSOR hOldCursor = NULL;
   static int     nCount     = 0;

	if( bDisplay )
	{
		// Check if we already have the hourglass up and increment
		//  the number of times Hourglass (TRUE) has been called.
		if( nCount++ )
			return;

		hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

		// If this machine doesn't have a mouse, display the
		//  hourglass by calling ShowCursor(TRUE) (if it does
		//  have a mouse this doesn't do anything much).
		ShowCursor( TRUE );
	}
	else
	{
		// If we haven't changed the cursor, return to caller.
		if( !nCount )
			return;
		// If our usage count drops to zero put back the cursor
		//  we originally replaced.
		if( !(--nCount) )
		{
			SetCursor( hOldCursor );
			hOldCursor = NULL;
			ShowCursor( FALSE );
		}
	}
}
// Include the service TransparentBlt( HDC, HBITMAP,
//	DestX, DestY, COLORREF )
// ===============================================
//#include	"DvTrans.h"
#endif // WIN32

// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
int	EnsureCrLf( char * lpd, char * lps )
{
	size_t	i, j, k;
	char	c, d;

	k = 0;
	if( lpd && lps &&
		(i = strlen( lps )) )
	{
		d = 0;
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c == '\r' )
			{	// If a CR
				if( (j+1) < i )
				{	// and there is more
					if( lps[j+1] != '\n' )
					{	// but NOT a LF
						lpd[k++] = c;	// Add the CR
						c = '\n';		// and set LF
					}
				}
				else
				{	// no more
					lpd[k++] = c;	// Add the CR
					c = '\n';		// and set LF
				}
			}
			else if( c == '\n' )
			{	// If a LF
				if( d != '\r' )
				{	// If no previous CR
					lpd[k++] = '\r';	// Add a CR
				}
			}
			lpd[k++] = c;	// Add char
			d = c;			// keep last put
		}
		if( c != '\n' )
		{	// If it does NOT end in a Cr/Lf
			lpd[k++] = '\r';	// then ADD THEM NOW
			lpd[k++] = '\n';
		}
	}
	return k;	// Return the NEW length
}

//#define	MXFSB	32
//#define	MXFSBS	8
#define	MXFSB	   264
#define	MXFSBS	32
char    szFSBufs[MXFSB*MXFSBS];
int		iFSBuf = 0;
int	   bFilNPlace = FALSE; //TRUE;	// Put a SPACE for the NEG sign
int	   bAddSig3 = FALSE; //TRUE;	// Always min. 3 digits before decimal
int	   bFilNeg10 = FALSE; //TRUE;
int	   bFilPos10 = FALSE; //TRUE;

char *	GetABuf( void )
{
	char *	lpb;
	lpb = &szFSBufs[ (iFSBuf * MXFSB) ];   // select NEXT buffer
	iFSBuf++;      // bump buffer index
	if( iFSBuf >= MXFSBS )  // check against max
		iFSBuf = 0; // roll back to zero
	return lpb;
}

#if defined(WIN32) && defined(NEED_STG_FUNCS)
//
// Convert "double" to string
// with a little bit of conditioning
// 
char *	DVf2s( double source )
{
	int     decimal,   sign;
	char    *buffer;
	char	wbuf[MAX_PATH+8];
	int     precision = 10;
	char    *lpb, *lpwb;
	int		i, j, k, l, sd, fd;
	char	c, d;

	// Do the CONVERSION to ASCII at precision 10
	// Returns buffer pointer to digits, and
	// a decimal position counter, and
	// the sign.
	buffer = _ecvt( source, precision, &decimal, &sign ); // #if defined(WIN32) && defined(NEED_STG_FUNCS) guard
	lpb = GetABuf();	// Get a (next) text buffer
	lpwb = &wbuf[0];	// and a work buffer (if reqd)
	i = strlen( buffer );	// Get length of digits.
	k = 0;
	if( sign )
	{
		lpb[k++] = '-';		// Add in the Minus
	}
	else
	{
		if( bFilNPlace )
		{
			lpb[k++] = ' ';		// Else a SPACE
		}
	}

	if( decimal < 0 )
	{
		// We have just a DECIMAL
		sd = 0;
		if( bAddSig3 )
		{
			lpb[k++] = ' ';
			lpb[k++] = ' ';
		}
		// Now start the DECIMAL
		lpb[k++] = '0';
		lpb[k++] = '.';
		while( decimal < 0 )
		{
			if( decimal < (int)-10 )
			{
				sprintf( &lpb[k], "0+%d+0s", -(decimal) );
				k = strlen( lpb );
			}
			else
			{
				lpb[k++] = '0';
				sd++;		// Counted as significant
				decimal++;
			}
		}
		d = 0;
		for( j = 0; j < i; j++ )
		{
			c = buffer[j];
			if( j && ( c == '0' ) )
			{
				if( d == 0 )
				{
					d = c;
					l = j;
				}
			}
			else
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = c;
				sd++;
			}
		}
		if( bFilNeg10 &&
			( sd < 10 ) )
		{
			fd = 10 - sd;
			while( fd-- )
			{
				lpb[k++] = ' ';
			}
		}
	}
	else	// Decimal is positive
	{
		if( decimal == 0 )
		{
			lpb[k++] = ' ';
			lpb[k++] = ' ';
			lpb[k++] = '0';
			lpb[k++] = '.';
			decimal--;
		}
		else if( ( decimal < 3 ) &&
			bAddSig3 )
		{
			int	tdec;
			tdec = 3 - decimal;
			while( tdec-- )
			{
				lpb[k++] = ' ';
			}
		}
		d = 0;
		sd = 0;
		for( j = 0; j < i; j++ )
		{
			c = buffer[j];
			if( j == decimal )
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = '.';
			}
			if( j && ( c == '0' ) )
			{
				if( d == 0 )
				{
					d = c;
					l = j;
				}
			}
			else
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = c;
				sd++;
			}
		}
		if( bFilPos10 &&
			( sd < 10 ) )
		{
			fd = 10 - sd;
			while( fd-- )
			{
				lpb[k++] = ' ';
			}
		}
	}
	if( k )
	{
		if( lpb[k-1] == '.' )
			lpb[k++] = '0';
	}
	lpb[k] = 0;
	return lpb;
}

#endif

// **************************************************************
//
// FUNCTION   : DVWrite(int fh, void MLPTR pv, uint32_t ul)
//
// PURPOSE    : Writes data in steps of 32k
//				till all the data is written.
//
// RETURNS    : 0 - If write did not proceed correctly.
//		 number of bytes written otherwise.
//
// **************************************************************
uint32_t DVWrite(int fh, char * pv, uint32_t ul )
{
	uint32_t	ulT = ul;
	char *	hp = pv;

	while( ul > BYTES_PER_READ )
	{
		if( _lwrite(fh, (char *)hp, (WORD)BYTES_PER_READ) != BYTES_PER_READ )
			return 0;
		ul -= BYTES_PER_READ;
		hp += BYTES_PER_READ;
	}
	if( ul )
	{
		if( _lwrite(fh, (char *)hp, (WORD)ul) != (WORD)ul )
			return 0;
	}

	return ulT;
}

//HFILE OpenFile(
//    LPCSTR lpFileName,	// pointer to filename 
//    LPOFSTRUCT lpReOpenBuff,	// pointer to buffer for file information  
//    uint32_t uStyle	// action and attributes 
HFILE	DVOpenFile( char * lpf, LPOFSTRUCT pof, uint32_t uStyle )
{
	HFILE	hf;
	hf = OpenFile( lpf, pof, uStyle );
	return hf;
}


uint32_t	DVSeekEnd( HFILE hf )
{
#ifdef	WIN32
	uint32_t	dw;
	HANDLE	hFile;
	dw = (uint32_t)-1;
	if( ( hFile = (HANDLE)hf ) &&
		( hf != HFILE_ERROR ) )
	{
		dw = SetFilePointer( hFile,	// handle of file 
			0,		// number of bytes to move file pointer
			NULL,	// address of high-order word of distance to move
			FILE_END );	// // how to move
	}
	return( dw );
#else	// !WIN32
	return( _llseek( hf, 0, 2 ) );	// Ensure at EOF ... 
#endif	// WIN32 y/n
}

HFILE DVlclose( HFILE hf )
{
	HFILE hFile = HFILE_ERROR;
	if( hf && (hf != HFILE_ERROR) )
	{
		hFile = _lclose( hf );
	}
	return( hFile );
}

/* Given a BUFFER, the Current Direcory and a File name */
/* Build a complete PATH into the Buffer */
void	SRSetupPath( char * lpb, char * lpd, char * lpf )
{
WORD i;
char	c;
	i = 0;
	if( lpb && lpf && lpf[0] )
	{
		if( lpd && lpd[0] )
		{
			strcpy( lpb, lpd );
			if( i = strlen( lpb ) )
			{
				c = lpb[i-1];
				if( !(( c == ':') || ( c == '\\')) )
					strcat( lpb, "\\" );
			}
		}
		strcat( lpb, lpf );
	}
}

#ifdef	WIN32
void	chkte( void )
{
	int	i;
	i = 0;
}

uint32_t	GetTextExtent( HDC hdc, char * lpS, int len )
{
	TEXTMETRIC	tm;
	SIZE		sz, szt;
	uint32_t		dwS;
	int			i;

	GetTextExtentPoint32( hdc,	// handle of device context
		lpS,	// address of text string
		len,	// number of characters in string
		&sz ); 	// address of structure for string size
	dwS = (uint32_t)MAKELONG( sz.cx, sz.cy );
	//dwS =  (((WORD)sz.cy << 16) | ((WORD)sz.cx) );
	if( GetTextMetrics( hdc, &tm ) )
	{
		// FIX981227 Can NOT now understand WHY I have ADDED this
		// tmInternalLeading to the HEIGHT. If ANYTHING is added
		// it SHOULD be tmExternalLeading, as any additional space
		// between ROWS of text.
		//szt.cy = (tm.tmHeight + tm.tmInternalLeading);
		// BUT actually WHY NOT JUST USE HEIGHT!!!!
		szt.cy = tm.tmHeight;	// FIX981227 - Just use HEIGHT
		szt.cx = (tm.tmAveCharWidth * len);
		i = 0;
		if( szt.cx > sz.cx )
		{
			i++;
			sz.cx = szt.cx;
		}
		if( szt.cy > sz.cy )
		{
			i++;
			sz.cy = szt.cy;
		}
		if( i )
		{
			chkte();	// we are ALTERING the SIZE!!!
			// ***************************************
			dwS = (uint32_t)MAKELONG( sz.cx, sz.cy );
			// ***************************************
		}
	}
	return( dwS );
}

#endif	// WIN32


int	GMIsExt( char * lpsrc, char * lpfind )
{
	int	flg;
	int		i, j, k, l;

	flg = FALSE;
	if( ( lpsrc ) &&
		( i = strlen(lpsrc) ) &&
		( lpfind ) &&
		( j = strlen(lpfind) ) &&
		( l = GMInStr( lpsrc, "." ) ) )
	{
//		if( ( l + 1 ) == k )
		if( k = GMInStr( &lpsrc[l], lpfind ) )
		{
			flg = TRUE;
		}
	}
	return flg;
}

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
 * (see grmlib.c for same/similar SplitFN - splitfilename into components)
 * ===================================================================== */
void	GMGetFPath( char * lpc, char * lpd, char * lpp, char * lpf, char * lpe )
{
	int	i, j, k;
	char *	lps;
	char	c;

	lps = lpc;
	if( lpd )
		lpd[0] = 0;
	if( lpp )
		lpp[0] = 0;
	if( lpf )
		lpf[0] = 0;
	if( lpe )
		lpe[0] = 0;

	if( ( lps ) &&
		( i = strlen( lps ) ) )
	{
		if( lps[1] == ':' )	/* If a DRIVE ... */
		{	
			k = 2;
			if( lpd )	/* If a DRIVE requested ... */
			{
				lpd[0] = lps[0];
				lpd[1] = lps[1];
				lpd[2] = 0;
			}
			lps += 2;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			c = lps[0];
			k = 0;
			if( (c == '\\') || (c == '/') )
			{
				for( j = 0; j < i; j++ )
				{
					c = lps[j];
					if( (c == '\\') || (c == '/') )
						k = j + 1;	/* include this char in move ... */
					if( lpp )
					{
						if( j < _MAX_DIR )
							lpp[j] = c;
					}
				}
				if( lpp )
				{
					if( k < _MAX_DIR )
						lpp[k] = 0;	/* Put the NUL at the correct place ... */
					else
						lpp[_MAX_DIR - 1] = 0;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( c == '.' )
				{
					k = j;
					break;
				}
				if( lpf )
				{
					if( j < _MAX_FNAME )
						lpf[j] = c;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
			if( lpf )
			{
				if( j < _MAX_FNAME )
					lpf[j] = 0;
				else
					lpf[_MAX_FNAME - 1] = 0;
			}
		}
		if( i && (c == '.') )
		{
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( lpe )
				{	
					if( j < _MAX_EXT )
						lpe[j] = c;
				}
			}
			if( lpe )
			{
				if( j < _MAX_EXT )
					lpe[j] = 0;
				else
					lpe[_MAX_EXT - 1] = 0;
			}
		}
	}
}

int	GMGetFName( char * lpfull, char * lpname )
{
	int		i = 0;
	char	nm[_MAX_FNAME];
	char	ext[_MAX_EXT];
	if( ( lpfull ) &&
		( lpname ) )
	{
		GMGetFPath( lpfull, 0, 0, &nm[0], &ext[0] );
		strcpy( lpname, &nm[0] );
		strcat( lpname, &ext[0] );
		i = strlen( lpname );
	}
	return i;
}


int	GMRTrimStg( char * lps )
{
	int		i, j, k;

	k = 0;
	if( ( lps ) &&
		( i = strlen(lps) ) )
	{
		for( j = ( i - 1 ); j > 0; j-- )
		{
			if( lps[j] <= ' ' )
			{
				lps[j] = 0;
				k++;
			}
			else
			{
				break;
			}
		}
	}
	return k;
}


#define		MXNCNUM		128

// ===================================================
// void	GetNiceNum( char * lpdest, uint32_t num )
//
// Return NICE LOOKING large number
// ie 8123456 comes back as 8,123,456
// in the supplied buffer
//
// ===================================================
void	GetNiceNum( char * lpdest, uint32_t num )
{
	char *lpn, *lpr;
	char	buf[MXNCNUM];
	int		i, j, k;

	lpn = &buf[0];
	lpr = lpdest;
	wsprintf( lpn, "%u", num );
	if( i = strlen( lpn ) )
	{
		if( i > 3 )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				if( ( (i - j) % 3 ) == 0 )
					lpr[k++] = ',';
				lpr[k++] = lpn[j];
			}
			lpr[k] = 0;
		}
		else
		{
			strcpy( lpr, lpn );
		}
	}
}

// =====================================================
// char *	GetNNumSStg( uint32_t num )
//
// Return a static buffer containing the nice number
//
// Note use of just TWO buffers before over-write
//
// =====================================================
char *	GetNNumSStg( uint32_t num )
{
	char *	lpr;
	static	char	_numbuf[MXNCNUM*2];
	static	int		_innum;
	if( _innum )
	{
		lpr = &_numbuf[0];
		_innum = 0;
	}
	else
	{
		lpr = &_numbuf[MXNCNUM];
		_innum++;
	}
	GetNiceNum( lpr, num );
	return lpr;
}

#endif // #ifdef USE_GMUTIL_MODULE

///////////////////////////////////////////////////////////////////////////////////////
// USED utilties
char	GMUpper( char c )
{
	char	d;
	if( (c >= 'a') && (c <= 'z') )
		d = c & 0x5f;
	else
		d = c;
	return( d );
}

int		GMInStr( char * lpsrc, char * lpfind )
{
	int		iAt, islen, iflen, i, j, k;
	char	c, d, b;

	iAt = 0;	// NOT FOOUND yet
	if( lpsrc && lpfind &&
		( (islen = strlen( lpsrc )) > 0 ) &&
		( (iflen = strlen( lpfind )) > 0 ) )
	{
		d = GMUpper( lpfind[0] );
		for( i = 0; i < islen; i++ )
		{
			c = GMUpper( lpsrc[i] );
			if( c == d )
			{
				if( iflen == 1 )
				{
					iAt = i+1;
					break;
				}
				else
				{
					if( (islen - i) >= iflen )
					{
						// ok, we have the length
						k = i + 1;	// Get to NEXT char
						for( j = 1; j < iflen; j++ )
						{
							c = GMUpper( lpsrc[k] );	// Get next
							b = GMUpper( lpfind[j] );
							if( c != b )
								break;
							k++;
						}
						if( j == iflen )
						{
							iAt = k + 1;
							break;
						}
					}
					else
					{
						// not enough length left
						break;
					}
				}
			}
		}
	}
	return iAt;
}

//////////////////////////////////////////////////////////////////////////

// **********************************************************
// eof - gmUtil.c
