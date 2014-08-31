/*\
 * grmLib.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * general functions ...
 *
\*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#ifdef WIN32
#if (defined(_MSC_VER) && (_MSC_VER > 1300))
#pragma warning(disable:4996)
#endif  // MSVC8 plus
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32
#include <stdlib.h>
#include "grmLib.h"
#include "grmOut.h"

// #define ADD_REJ_DBG

#ifdef	FC4W
#define		NOGMUTIL
#endif	/* FC4W */

// FIX20140831: Make diag log file PERMANENT
#define  SETDIAG

#ifndef	NOGMFILETYPE
// =================================

typedef	struct	{
	uint32_t	ft_Type;
	char *	ft_Ext;
}FTEXT, *LPFTEXT;

FTEXT	FtExt[] = {
	{ FT_LIBRARY, "LIB" },
	{ FT_DYNALIB, "DLL" },
	{ FT_EXE,     "EXE" },
	{ FT_BAT,	  "BAT" },
	{ FT_COM,	  "COM" },
	{ FT_BMP,	  "BMP" },
	{ FT_C,		  "C"   },
	{ FT_CPP,	  "CPP" },
	{ FT_CXX,	  "CXX" },
	{ FT_H,		  "H"   },
	{ FT_ASM,	  "ASM" },
	{ FT_TXT,	  "TXT" },
	{ FT_GIF,	  "GIF" },
	{ FT_JPG,	  "JPG" },
	{ FT_OCX,     "OCX" },
	{ FT_OBJ,	  "OBJ" },
	{ FT_DSW,	  "DSW" },
	{ FT_DSP,	  "DSP" },
	{ FT_MAK,	  "MAK" },
	{ FT_PLG,	  "PLG" },
	{ FT_OPT,	  "OPT" },
	{ FT_OLD,     "OLD" },
	{ FT_BAK,	  "BAK" },
	{ FT_IDB,	  "IDB" },
	{ FT_PDB,	  "PDB" },
	{ FT_PCH,	  "PCH" },
	{ FT_ILK,	  "ILK" },
	{ FT_ICO,     "ICO" },
	// NOTE: LAST ENTRY
	{ FT_UNKNOWN, "UNK" }
};

// =================================
#endif	/* !NOGMFILETYPE */

#ifndef	GMALTOUT

extern	void	chkchk(void);
#ifdef USE_DT4_MODULE
extern	char *	GetDT4( int Typ );
#endif // USE_DT4_MODULE

char	szDTxt[MAX_PATH+4] = { "TEMPDIAG.TXT" };
static HFILE	hDFile = 0;

#endif	/* !GMALTOUT */


#ifndef	NOGMFILETYPE
// =================================

int	MatchExt( char * lpe, int len, uint32_t *lpT )
{
	int	fOk;
	LPFTEXT	lpft;
	uint32_t	typ;
	char *	lpt;

	fOk = FALSE;
	lpft = &FtExt[0];
	typ = lpft->ft_Type;
	while( typ != FT_UNKNOWN )
	{
		lpt = lpft->ft_Ext; 
		if(lpt)
		{
			if( strcmpi( lpt, lpe ) == 0 )
			{
				*lpT = typ;
				fOk = TRUE;
				break;
			}
		}
		lpft++;
   	typ = lpft->ft_Type;
	}
   UNREFERENCED_PARAMETER(len);

	return fOk;
}

int	SetFileType( char * lpInFil, uint32_t *pType )
{
	int		i, j, k;
	char	   c = 0;
	char	   ebuf[MAX_EXT];
	char *	   lpe;
	int	   fGotT;

	fGotT = FALSE;
	*pType = FT_UNKNOWN;
	lpe = &ebuf[0];

	// HERE we will ONLY reply on the EXTENT
	// if there is ONE!!!
	i = k = 0;
   if(lpInFil)
      i = strlen(lpInFil);
	if( i )
	{
		// Back up to the FULL STOP
		for( j = (i - 1); j >= 0; j-- )
		{
			c = lpInFil[j];
			if( c == '.' )
				break;
		}
		// If we FOUND a FULL STOP
		if( c == '.' )
		{
			j++;
			for( ; j < i; j++ )
			{
				if( k < MAX_EXT )
					lpe[k++] = lpInFil[j];
				else
					break;
			}
			lpe[k] = 0;		// Zero TERMINATE it
			fGotT = MatchExt( lpe, k, pType );
		}
		else
		{
			*pType = FT_NONE;
			fGotT = TRUE;
		}
	}
	return	fGotT;
}


// =================================
#endif	/* !NOGMFILETYPE */


// ==========================================================
// int	SplitFN( char * lpdir, char * lpfil, char * lpext )
//
// INPUT: lpdir - Destination of PATH.
//        lpfil - Destination of FILENAME portion.
//        lpext - The combined PATH\FILNAME source.
//
// OUTPUT: The lpext string is moved to the lpdir buffer,
//		noting the last instance of ':', '\' or '/',
//		which is then used to copy the traling FILENAME
//		to lpfil, and zero terminating lpdir AFTER this
//		character.
//
// RETURN:	Length of DIRECTORY (if any)
//			0 if no PATH
// ==========================================================
int	SplitFN( char * lpdir, char * lpfil, char * lpext )
{
	int		i, j, k, l;
	char	c;

	k = 0;		// Start NO DIRECTORY
	i = strlen( lpext ); 
	if(i)
	{
		if( lpdir )
			*lpdir = 0;
		if( lpfil )
			*lpfil = 0;
		for( j = 0; j < i; j++ )
		{
			c = lpext[j];
			if( ( c == ':' ) || ( c == '\\' ) || ( c == '/' ) )
			{
				k = j + 1;
			}
			if( lpdir )
				lpdir[j] = c;
		}
		if( lpdir )
			lpdir[j] = 0;	// ZERO termination
		if( k )
		{
			j = 0;	// Start at ZERO
			for( l = k; l < i; l++ )
			{
				// Move the tail in as the FILE NAME
				c = lpext[l];
				if( lpfil )
					lpfil[j] = c;
				j++;
			}
			if( lpfil )
				lpfil[j] = 0;	// Zero terminate FILE NAME
			if( lpdir )
				lpdir[k] = 0;	// Remove TAIL of DIRECTORY
		}
		else	// No PATH
		{
			// Take whole name as the FILE
			if( lpfil && lpdir )
				strcpy( lpfil, lpdir );
			if( lpdir )
				*lpdir = 0;	// and remove any PATH!
		}
	}
	return k;
}

int	SplitExt( char * lpb, char * lpe, char * lpf )
{
	int	flg = FALSE;
	int	i, j, k, l;

	i = j = k = l = 0;
    if(lpf) {
        i = strlen(lpf);
    }
	if(i) {
      // FIX20041205 - this appear WRONG with *.htm* par example
		// j = HasDot( lpf ); // this is WRONG because it returns the FIRST
      char * p = strrchr(lpf,'.');
      if(p) {
         if( lpb ) {
            strcpy(lpb,lpf);
            lpb[ (p - lpf) ] = 0;
         }
         if( lpe ) {
            strcpy(lpe,p);
         }
      } else {
         // *** NO DOT = NO EXTENT ***
         // FIX20060126 - An input of say cmptidy* should be the same as cmptidy*.*,
         // but perhaps NOT cmptidy*. != cmptidy*.* - 
         if( lpb ) {
				strcpy( lpb, lpf );
         }
         if( lpe ) {
				*lpe = 0;
            // FIX20060126 - but if it ENDS IN AN ASTERIX, then make the extention
            // also an asterix!!!
            if( lpf[i - 1] == '*' ) {
               strcpy(lpe,"*");
            }
         }
      }
	}
	return	flg;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : WildComp2
// Return type: int 
// Arguments  : char * lp1
//            : char * lp2
// Description: Compare two components of a file name
//              FIX20010503 - Should NOT return TRUE on "c" with "cab"
//
///////////////////////////////////////////////////////////////////////////////
int	WildComp2( char * lp1, char * lp2 )
{
	int	   flg;
	int		i1, i2, j1, j2, ilen;
	int      c, d;

	flg = FALSE;
	i1 = strlen(lp1);
	i2 = strlen(lp2);
	if( i1 && i2 )    // if BOTH have lengths
	{
      ilen = i1;
      if( i2 > i1 )
         ilen = i2;
		j2 = 0;
		for( j1 = 0; j1 < ilen; j1++ )
		{
			c = toupper(lp1[j1]);   // extract next char from each
			d = toupper(lp2[j2]);
         if( c == d )
         {
            j2++;
            continue;
         }

			// they are NOT equal, but some things are allowed
         if( c == 0 ) {
            if( d == '*' ) { // if this wild card is the last char
               j2++;
               if( lp2[j2] == 0 )   // get NEXT
               {
                  // ended with this asteric, so
                  flg = TRUE; // this matches NO rest of 2
               }
            }
            break;
         }
         if( d == 0 ) {
            if( c == '*' ) {
               j1++;
               if( lp1[j1] == 0 )   // get NEXT
               {
                  // ended with this asteric, so
                  flg = TRUE; // this matches NO rest of 2
               }
            }
            break;
         }
			{
				if( c == '*' )
				{
               j1++;
               if( lp1[j1] == 0 )   // get NEXT
               {
                  // ended with this asteric, so
                  flg = TRUE; // this matches all the rest of 2
					   break;   // out of here with a MATCH
               }
               // else we have somehting like *abc, which mean the asteric
               // matched what ever was in 2, up until this letter encountered
               c = toupper(lp1[j1]);
               j2++;    // asteric matched at least this one
      			if( lp2[j2] == 0 )   // 2 ended, but 1 has more
                  break;   // so no MATCH
               for( ; j2 < ilen; j2++ )
               {
         			d = toupper(lp2[j2]);
                  if( c == d )
                     break;
                  if( d == 0 )
                     break;
               }
               if( c == d )
               {
                  // found next of 1 in 2
                  j2++;
                  continue;
               }
               // else the char in 1 not present in two;
               break;   // no MATCH
				}

				if( d == '*' )
				{
               j2++;
               if( lp2[j2] == 0 )
               {
                  // 2 ends with asteric, so matches all rest in 1
                  flg = TRUE;
                  break;
               }
               d = toupper(lp2[j2]);
               j1++;    // asteric matched at least this one
      			if( lp1[j1] == 0 )   // 1 ended, but 2 has more
                  break;   // so no MATCH
               for( ; j1 < ilen; j1++ )   // find the 2 in 1
               {
         			c = toupper(lp1[j1]);
                  if( c == d )   // found it?
                     break;
                  if( c == 0 )   // or ran out of chars
                     break;
               }
               if( c == d )
               {
                  // found next of 2 in 1
                  j2++;
                  continue;
               }
               // else the char in 2 is not present in 1
					break;
				}

            if( ( c == '?' ) || ( d == '?' ) )
				{
					// One match char ok.
				}
				else
				{
					if( toupper( c ) != toupper( d ) )
						break;
				}
			}
			j2++;
		}
		if( !flg && ( j1 == ilen ) )
			flg = TRUE;
   } else {
      // FIX20060605 
      // one or the other is BLANK
      // this is a MATCH if one is an ASTERIX!!!
      if( i1 == 0 ) {
         if(( i2 == 1 ) && ( *lp2 == '*' )) {
            flg = TRUE;
         } else if(( i2 == 2 ) && (lp2[0] == '.') && (lp2[1] == '*')) {
            flg = TRUE;
         }
      } else if( i2 == 0 ) {
         if(( i1 == 1 ) && ( *lp1 == '*' )) {
            flg = TRUE;
         } else if(( i1 == 2 ) && ( lp1[0] == '.' ) && ( lp1[1] == '*' )) {
            flg = TRUE;
         }
      }
   }
	return flg;
}

static char	_s_body1[MAX_PATH+16];
static char	_s_ext1[MAX_PATH+16];
static char	_s_body2[MAX_PATH+16];
static char	_s_ext2[MAX_PATH+16];
int chk_rej( char * lp1, char * lp2 )
{
   int i;
#ifdef ADD_REJ_DBG
    int len1 = (int)strlen(lp1);
    int len2 = (int)strlen(lp2);
    fprintf(stderr,"Rejected compare of '%s'(%d) with '%s'(%d)\n", lp1, len1, lp2, len2 );
#endif // ADD-REJ_DBG
   i = 0;
   return i;
}

int	MatchFiles( char * lp1, char * lp2 )
{
	int	flg;
   char * body1 = _s_body1;
   char * body2 = _s_body2;
   char * ext1 = _s_ext1;
   char * ext2 = _s_ext2;
	flg = FALSE;
	if( lp1 && lp2 &&
		*lp1 && *lp2 )
	{
		if( !GotWild( lp1 ) &&
			 !GotWild( lp2 ) )
		{
			if( strcmpi( lp1, lp2 ) == 0 )
				flg = TRUE;
		}
		else
		{
			// One of the other HAS WILD CHAR(S)
			//SplitExt( &body1[0], &ext1[0], lp1 );
			//SplitExt( &body2[0], &ext2[0], lp2 );
			//if( ( WildComp2( &body1[0], &body2[0] ) ) &&
			//	( WildComp2( &ext1[0], &ext2[0] ) ) )
			SplitExt( body1, ext1, lp1 );
			SplitExt( body2, ext2, lp2 );
			if(( WildComp2( body1, body2 ) ) &&
				( WildComp2( ext1, ext2 ) ) )
			{
				flg = TRUE;
			}
		}
	}
   if( !flg ) {
      chk_rej(lp1,lp2);
   }
	return flg;
}

int	GotWild( char * lps )
{
	int	flg = FALSE;
	int		i, j;
	char	c;
	i = strlen( lps ); 
	if(i)
	{
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( ( c == '*' ) || ( c == '?' ) )
			{
				flg = TRUE;
				break;
			}
		}
	}
	return flg;
}

#ifdef   USE_OLD_HASDOT
int	HasDot( char * lps )
{
	int	flg = 0;
	int		i, j;
	char	c;
	i = strlen( lps ); 
	if(i)
	{
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c == '.' )
			{
				//flg = TRUE;
				flg = j + 1;	// Including the DOT
				break;
			}
		}
	}
	return flg;
}
#else // #ifdef   USE_OLD_HASDOT

int	HasDot( char * lps )
{
   char * p = strchr(lps,'.');
   if(p)
      return( (p - lps) + 1 );
   return 0;
}

#endif // #ifdef   USE_OLD_HASDOT

int	IsDot( char * lps )
{
	int	flg = FALSE;
	int		i = strlen( lps ); 
	if(i)
	{
		if( ( i == 1 ) &&
			( *lps == '.' ) )
		{
			flg = TRUE;
		}
	}
	return flg;
}

int	IsDDot( char * lps )
{
	int	flg = FALSE;
	int		i = strlen( lps ); 
	if(i)
	{
		if( ( i == 2 ) &&
			( lps[0] == '.' ) &&
			( lps[1] == '.' ) )
		{
			flg = TRUE;
		}
	}
	return flg;
}

// ***************************************************
// Diagnostic File Output
HANDLE	CreateUserFile( char * lpf )
{
	HANDLE	hf = INVALID_HANDLE_VALUE;
	if( ( lpf ) &&
		( *lpf ) )
	{
#if WIN32
		// CreateFile
		hf = CreateFile( lpf,	// pointer to name of the file
			GENERIC_READ | GENERIC_WRITE,	// access (read-write) mode
			0,					// share mode
			NULL,				// pointer to security attributes
			CREATE_ALWAYS,		// how to create
			FILE_ATTRIBUTE_NORMAL,	// file attributes
			NULL );		// handle to file with attributes to copy
#else
        FILE *fp = fopen(lpf,"w");
        if (fp)
            hf = fp;
#endif
	}
	return hf;
}

#ifndef	GMALTOUT

void	SetUserHandle( HANDLE hf )
{
	hUserOut = hf;
}

void	CloseUserFile( HANDLE hf )
{
	if( VH(hf) ) {
#ifdef WIN32
		CloseHandle(hf);
#else
        fclose(hf);
#endif
    }
}

#define		VLD		( hDFile && ( hDFile != HFILE_ERROR ) )

int CreateDiagFile( void )
{
    int iret = 0;   // set FAILED
#if (defined(WIN32) && defined(USE_WIN32_API))
	OFSTRUCT	of;
	hDFile = OpenFile( &szDTxt[0], &of, OF_CREATE | OF_READWRITE );
#else
    FILE *fp = fopen( szDTxt, "w" );
    if (fp) {
        hDFile = (HFILE)fp;
    } else {
        hDFile = (HFILE)INVALID_HANDLE_VALUE;
    }
#endif
    if (!VLD) {
        fprintf(stderr,"\nWARNING: FAILED to open diag file '%s'!\n", szDTxt);
    } else {
#ifdef ADD_DIAG_DEBUG
        fprintf(stderr,"open diag file '%s'\n", szDTxt);
#endif // ADD_DIAG_DEBUG
        iret = 1;
    }
    return iret;
}


void	CloseDiagFile( void )
{
	if( VLD )
	{
#if (defined(WIN32) && defined(USE_WIN32_API))
		_lclose( hDFile );
#else
        fclose( (FILE *)hDFile );
#endif
	}
	hDFile = 0;
}

void	WriteDiagFile( char * lps )
{
	static int _sdninit = 0;
	int		i, wtn;
	// if given a pointer, and there is data
   i = 0;
   if(lps)
      i = strlen(lps);
	if(i)
	{
		if( ( hDFile == 0 ) &&
			( !_sdninit   ) )
		{
			CreateDiagFile();
			_sdninit = 1;
		}
		if( VLD )
		{
#if (defined(WIN32) && defined(USE_WIN32_API))
			wtn = _lwrite( hDFile, lps, i );
#else
            wtn = fwrite(lps,1,i,(FILE *)hDFile);
#endif
			if( wtn != i )
			{
				CloseDiagFile();
				hDFile = HFILE_ERROR;
                fprintf(stderr,"Warning: write diag file '%s' FAILED! req %d, got %d. CLOSED\n", szDTxt, i, wtn);

			}
		}
	}
}

char *GetDiagFile() { return szDTxt; } 

int	SetDiagFile( char * lpf )
{
	char	buf[264];
	int	flg = FALSE;
	char *	lpb = &buf[0];

#ifdef   SETDIAG
	if( ( lpf         ) &&
		( *lpf        ) &&
		( hDFile == 0 ) )
	{
		if( strcmpi( &szDTxt[0], lpf ) )
		{
			if( VLD )
			{
				sprintf( lpb, "New File: %s\r\n", lpf );
				WriteDiagFile( lpb );
				CloseDiagFile();
			}
			hDFile = 0;
		}

		strcpy( &szDTxt[0], lpf );
		CreateDiagFile();
		if( VLD )
			flg = TRUE;
	}

#endif   // SETDIAG

	return flg;
}

void	DisableDiagFile( void )
{
	if( VLD )
	{
		CloseDiagFile();
	}
	hDFile = HFILE_ERROR;
}

void	EnableDiagFile( void )
{
	if( hDFile == HFILE_ERROR )
		hDFile = 0;
}

#endif // #ifndef	GMALTOUT
// End - Diagnostic File Output
// ***************************************************

// ========================================================
// Place a FLOATING POINT Conversion to ASCII into a
// buffer, aligning the DECIMAL point, using the
// "precision" to fill out in from of the decimal,
// and after the decimal point.
void Buffer2Lps2( char * lps, char * lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;
	strcpy( lps, "0.0" );	// fill in something
	j = strlen( lpb );
	k = 0;
	cad = 0;	// Count AFTER the decimal
	lps[k++] = '[';
	if( sign )
		lps[k++] = '-';
	else
		lps[k++] = ' ';
	l = decimal;
	if( l < 0 )
	{	// A NEGATIVE decimal position
		for( i = 0; i < precision; i++ )
		{
			if( ((precision - i) % 3) == 0 )
				lps[k++] = ' ';
			lps[k++] = ' ';
		}
		k--;
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		if( decimal == 0 )
			j = precision - 1;
		else
			j = precision;
		for( i = 0; i < j; i++ )
		{
			sig = precision - decimal;
			if( ((precision - i) % 3) == 0 )
				lps[k++] = ' ';
			if( i == sig )
			{
				break;
			}
			lps[k++] = ' ';
		}
	}
	sig = 0;	// Significant character counter
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];
		if( i == decimal )
		{
			if( i == 0 )
			{
				lps[k++] = '0';
			}
			lps[k++] = '.';
			cad++;
		}
		if( ( decimal > 0 ) &&
			sig &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';
		}
		lps[k++] = c;
		if( sig )
		{
			sig++;
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}
	if( cad )
		cad--;
	if( cad < precision )
	{
		sig = precision - cad;
		for( i = 0; i < sig; i++ )
			lps[k++] = ' ';
	}
	lps[k++] = ']';
	lps[k] = 0;
}

#ifdef WIN32
//void Buffer2Stg( char * lps, char * lpb, int decimal,
//				 int sign, int precision );
// float to string trimmed - not decimal alligned
void	Float2Stg( char * lps, float factor, int precision )
{
	int		decimal, sign;
	char *	buffer;
	buffer = ECVT( factor, precision, &decimal, &sign ); // WIN32 guard
	Buffer2Stg( lps, buffer, decimal, sign, precision );
}

// float to string DECIMAL ALLIGNED between braces, like [ 2.34 ]
void	AFloat2Stg( char * lps, float factor, int precision )
{
	int		decimal, sign;
	char *	buffer;
	buffer = ECVT( factor, precision, &decimal, &sign ); // WIN32 guard
	Buffer2Lps2( lps, buffer, decimal, sign, precision );
}

#endif // #ifdef WIN32

float Str2Float( char * lps )
{
	double	d;
	float	f;

	d = atof(lps);
	f = (float)d;

	return f;
}

// ================================================
// Given a BLOCK of DATA from the beginning of
// the file attempt to DETERMINE its TYPE,
// and return that TYPE.
// ================================================

#ifdef WIN32
// MINIMUM BITMAP
// File Header + Info header + 1 24-bit colour of a 1x1 BMP
#define	MINBMP	(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3)

uint32_t	IsBmpType( char * lpName, uint32_t dwSize,
					char * lpBuf, uint32_t dwLen )
{
	uint32_t	rtyp = 0;
	LPWORD	lpw;
	if( dwLen >= 2 )
	{
		lpw = (LPWORD)lpBuf;
		if( *lpw == 19778 )		// = "BM" for BITMAP;
		{
			BITMAPFILEHEADER * pbmfh;
			BITMAPINFOHEADER * pbmih;
			if( dwLen < MINBMP )
				return( FT_BMP );
			pbmfh = (BITMAPFILEHEADER *)lpBuf;
			pbmih = (BITMAPINFOHEADER *)((char *)lpBuf + sizeof(BITMAPFILEHEADER));
			if( ( pbmfh->bfSize == dwSize   ) &&
				( pbmfh->bfOffBits < dwSize ) )
			{
				return( FT_BMP );
			}
		}
	}
   UNREFERENCED_PARAMETER(lpName);
	return rtyp;
}

#endif // WIN32

//#if	( defined(GMALTOUT) && defined(NOGMUTIL) ) - Added FA4 - 17FEB00
#if	( ( defined(GMALTOUT) && defined(NOGMUTIL) ) || defined(FA4) )
// GMALTOUT is since 1999 recovery into Grmlib.c
// and some of these also in GMUtil.c

char    GMUpper( char c )
{
    char    d;
    if( (c >= 'a') && (c <= 'z') )
        d = (char)(c & 0x5f);
    else
        d = c;
    return( d );
}

int		GMInStr( char * lpsrc, char * lpfind )
{
	int		iAt, islen, iflen, i, j, k;
	char	c, d, b;

	iAt = 0;	// NOT FOOUND yet
	islen = strlen( lpsrc );
	iflen = strlen( lpfind );
	if( islen && iflen )
	{
		d = GMUpper( lpfind[0] );
		for( i = 0; i < islen; i++ )
		{
			c = GMUpper( lpsrc[i] );
			if( c == d )
			{
				if( iflen == 1 )
				{
					// The FIND location ***PLUS*** ONE (1)!!!
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
							// FIX981106 - This should be
							//iAt = k + 1;
							// The FIRST char FIND location
							// ***PLUS*** ONE (1)!!!
							iAt = i + 1;
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

// ================================
#endif	/* GMALTOUT and NOGMUTIL */


#ifdef	GMALTOUT

#ifndef  DISKDB     // has its own copy in DDBUtil.c

/* Oct 99 update - retreived from DDBData.c */

// ===========================================================
// void Buffer2Stg( char * lps, char * lpb, int decimal,
//				 int sign, int precision )
//
// Purpose: Convert the string of digits from the ecvt
//			function to a nice human readbale form.
//
// 1999 Sept 7 - Case of removing ?.?0000 the zeros
//
// ===========================================================
void Buffer2Stg( char * lps, char * lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;

	k = 0;					// Start at output beginning
	cad = 0;				// Count AFTER the decimal
	j = strlen( lpb );		// Get LENGTH of buffer digits

	if( sign )				// if the SIGN flag is ON
		lps[k++] = '-';		// Fill in the negative

	l = decimal;
	if( l < 0 )
	{
		// A NEGATIVE decimal position
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		// Possible shortened use of the digit string
		// ie possible LOSS of DIGITS to fit the precision requested.
		if( decimal == 0 )
		{
			if( ( precision - 1 ) < j )
			{
				//chkme( "NOTE: precision -1 is LT digits! Possible LOSS!!" );
				j = precision - 1;
			}
		}
		else
		{
			if( precision < j )
			{
//				chkme( "NOTE: precision is LT digits! Possible LOSS!!" );
				j = precision;
			}
		}
	}

	sig = 0;	// Significant character counter
	// Process each digit of the digit list in the buffer
	// or LESS than the list if precision is LESS!!!
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];		// Get a DIGIT
		if( i == decimal )	// Have we reached the DECIMAL POINT?
		{
			// At the DECIMAL point
			if( i == 0 )	
			{
				// if no other digits BEFORE the decimal
				lps[k++] = '0';	// then plonk in a zero now
			}
			lps[k++] = '.';	// and ADD the decimal point
			cad++;
		}
		// Check for adding a comma for the THOUSANDS
		if( ( decimal > 0 ) &&
			( sig ) &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';	// Add in a comma
		}
		lps[k++] = c;	// Add this digit to the output
		if( sig )		// If we have HAD a significant char
		{
			sig++;		// Then just count another, and another etc
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}	// while processing the digit list

	// FIX980509 - If digit length is LESS than decimal position
	// =========================================================
	if( ( decimal > 0 ) &&
		( i < decimal ) )
	{
		c = '0';
		while( i < decimal )
		{
			if( ( decimal > 0 ) &&
				( sig ) &&
				( i < decimal ) )
			{
				m = decimal - i;
				if( (m % 3) == 0 )
					lps[k++] = ',';	// Add in a comma
			}
			lps[k++] = c;	// Add this digit to the output
			i++;
		}
	}
	// =========================================================
	if( cad )
		cad--;
	lps[k] = 0;		// zero terminate the output
	// FIX990907 - Remove unsightly ZEROs after decimal point
    for( i = 0; i < k; i++ )
    {
        if( lps[i] == '.' )
            break;
    }
    if( ( i < k ) &&
        ( lps[i] == '.' ) )
    {
        i++;
        if( lps[i] == '0' )
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
            if( k > i )
            {
                // we have backed to a not '0' value so STOP
            }
            else
            {
                // we backed all the way, so remove the DECIMAL also
                i--;
                lps[i] = 0;
            }
        }
        else
        {
            while( k > i )
            {
                k--;
                if( lps[k] == '0' )
                    lps[k] = 0;
                else
                    break;
            }
        }
    }

}
#endif   // !DISKDB     // has its own copy in DDBUtil.c

#else	/* !GMALTOUT - prior Oct 1999 */

// ===========================================================
// void Buffer2Stg( char * lps, char * lpb, int decimal,
//				 int sign, int precision )
//
// Purpose: Convert the string of digits from the ecvt
//			function to a nice human readbale form.
//
// ===========================================================
void Buffer2Stg( char * lps, char * lpb, int decimal,
				 int sign, int precision )
{
	int		i, j, k, l, m, sig, cad;
	char	c;

	k = 0;					// Start at output beginning
	cad = 0;				// Count AFTER the decimal
	j = strlen( lpb );		// Get LENGTH of buffer digits

	if( sign )				// if the SIGN flag is ON
		lps[k++] = '-';		// Fill in the negative

	l = decimal;
	if( l < 0 )
	{
		// A NEGATIVE decimal position
		lps[k++] = '0';
		lps[k++] = '.';
		cad++;
		while( l < 0 )
		{
			lps[k++] = '0';
			l++;
			cad++;
		}
	}
	else if( ( decimal >= 0 ) &&
		( decimal < precision ) )
	{
		// Possible shortened use of the digit string
		// ie possible LOSS of DIGITS to fit the precision requested.
		if( decimal == 0 )
		{
			if( ( precision - 1 ) < j )
			{
				//chkme();
				j = precision - 1;
			}
		}
		else
		{
			if( precision < j )
			{
				chkchk();
				j = precision;
			}
		}
	}

	sig = 0;	// Significant character counter
	// Process each digit of the digit list in the buffer
	// or LESS than the list if precision is LESS!!!
	for( i = 0; i < j; i++ )
	{
		c = lpb[i];		// Get a DIGIT
		if( i == decimal )	// Have we reached the DECIMAL POINT?
		{
			// At the DECIMAL point
			if( i == 0 )	
			{
				// if no other digits BEFORE the decimal
				lps[k++] = '0';	// then plonk in a zero now
			}
			lps[k++] = '.';	// and ADD the decimal point
			cad++;
		}
		// Check for adding a comma for the THOUSANDS
		if( ( decimal > 0 ) &&
			( sig ) &&
			( i < decimal ) )
		{
			m = decimal - i;
			if( (m % 3) == 0 )
				lps[k++] = ',';	// Add in a comma
		}
		lps[k++] = c;	// Add this digit to the output
		if( sig )		// If we have HAD a significant char
		{
			sig++;		// Then just count another, and another etc
		}
		else if( c > '0' )
		{
			sig++;	// First SIGNIFICANT character
		}
		if( cad )
			cad++;
	}	// while processing the digit list

	// FIX980509 - If digit length is LESS than decimal position
	// =========================================================
	if( ( decimal > 0 ) &&
		( i < decimal ) )
	{
		c = '0';
		while( i < decimal )
		{
			if( ( decimal > 0 ) &&
				( sig ) &&
				( i < decimal ) )
			{
				m = decimal - i;
				if( (m % 3) == 0 )
					lps[k++] = ',';	// Add in a comma
			}
			lps[k++] = c;	// Add this digit to the output
			i++;
		}
	}
	// =========================================================
	if( cad )
		cad--;
	lps[k] = 0;		// zero terminate the output
}

#ifdef	NOGMUTIL

char    GMUpper( char c )
{
    char    d;
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
		( islen = strlen( lpsrc ) ) &&
		( iflen = strlen( lpfind ) ) )
	{
		d = GMUpper( lpfind[0] );
		for( i = 0; i < islen; i++ )
		{
			c = GMUpper( lpsrc[i] );
			if( c == d )
			{
				if( iflen == 1 )
				{
					// The FIND location ***PLUS*** ONE (1)!!!
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
							// FIX981106 - This should be
							//iAt = k + 1;
							// The FIRST char FIND location
							// ***PLUS*** ONE (1)!!!
							iAt = i + 1;
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

#endif	/* NOGMUTIL */

#endif	/* GMALTOUT y/n - Oct 1999 */

void	RTrimDecimal1( char * lpr )
{
	int		i, j, k, dlen;
	char	c, d;
	char *	lpend;

	c = 0;
   i = k = j = 0;
   if(lpr)
   {
      i = strlen(lpr);
      if(i)
      {
         k = GMInStr(lpr, ".");
         j = ( i - k );
      }
   }
	if( ( k       ) &&
		 ( i < 128 ) &&
		 ( j > 2   ) )
	{
		// Also look for runs, especially say 0.799999997
		dlen = j - 1;

		c = lpr[i - 1];	// Get last char of string
		if( ( c >= '0' ) &&
			( c <= '9' ) )
		{

			lpend = &lpr[ i - 1 ];
			*lpend = 0;		// kill end
			lpend--;
			//k = i - k;
			if( c >= '5' )
			{
				//if( k )
				//	k--;
				d = c;
				c = *lpend;
				if( ( c >= '0' ) &&
					( c <= '9' ) )
				{
					if( c < '9' )
					{
						c++;
						*lpend = c;
					}
					else
					{
				//		if( k )
				//			k--;
				//		while( k-- )
						while( dlen-- )
						{
							*lpend = '0';
							lpend--;
							d = c;
							c = *lpend;
							if( ( c >= '0' ) &&
								( c <= '9' ) )
							{
								if( c < '9' )
								{
									c++;
									*lpend = c;
									break;
								}
								else
								{
									if( k )
									{
										*lpend = '0';
										lpend--;
									}
								}
							}
							else
							{
								break;
							}
						}	// backing up
					}
				}	// back one is a number
			}	// we are removing a 5 or more
		}
	}
}

void	RTrimDecimal( char * lpr )
{
	size_t	i, j, k, l;
	char	c;
	char	szless[128];
	char *	lpl;

   i = 0;
   if(lpr)
      i = strlen(lpr);
	if(i)
	{
		k = GMInStr( lpr, "." ); 
		if(k)
		{
			// Returns LOCATION of DECIMAL
			k--;	// back to char BEFORE decimal
			for( j = (i - 1); j >= k; j-- )
			{
				c = lpr[j];		// Get END (after DECIMAL POINT!)
				if( c == '0' )
				{
					lpr[j] = 0;
				}
				else
				{
					if( c == '.' )
						lpr[j] = 0;
					break;
				}
			}
		}	// if it contains a DECIMAL point

		// ==============================
		// 2nd processing ==============
		// ==============================
		i = strlen( lpr );
        k = GMInStr( lpr, "." );
		if( ( i             ) &&
			 ( k             ) &&
			 ( i < 128       ) &&
			 ( ( i - k ) > 2 ) )
		{
			// Also look for runs, especially say 0.799999997
			j = ( k + 1 );
			k = i - j;	// length of decimal number
			lpl = &szless[0];
			strcpy( lpl, lpr );
			RTrimDecimal1( lpl );
			if( ( strlen( lpl ) ) &&
				( strlen( lpl ) < i ) &&
				( GMInStr( lpl, "." ) ) )
			{
				i = strlen( lpl );
				k = GMInStr( lpl, "." );
				for( j = (i - 1); j > k; j-- )
				{
					c = lpl[j];
					if( c == '0' )
					{
						lpl[j] = 0;
					}
					else
					{
						break;
					}
				}
				if( (strlen(lpl) + 2 ) < strlen(lpr) )
				{
					strcpy( lpr, lpl );
				}
			}
			j = ( GMInStr( lpr, "." ) + 1 );

		}
		// ==============================
		// 3rd processing ==============
		// ==============================
		i = strlen( lpr );
      k = GMInStr( lpr, "." );
		if( ( i && k        ) &&
			 ( i < 128       ) &&
			 ( ( i - k ) > 6 ) )
		{
			// Ok, I specifically want to avoid such things as
			// 48.000000082 and 47.999999992, etc
			// Returns LOCATION of DECIMAL
			k++;	// to char AFTER decimal
			lpl = &szless[0];
			j = k;
			l = 0;		// Repeat counter
			*lpl = 0;
			for( ; j < i; j++ )
			{
				c = lpr[j];		// Get chars (after DECIMAL POINT)
				if( c == *lpl )
				{
					l++;		// Count SAMENESS
				}
				else
				{
					if( l > 5 )
					{
						// we have HAD 5 of these chars
						// ============================
						l++;	// Bump for FIRST of these
						lpl = &lpr[ ( j - l ) ];	// Get FIRST
						c = *lpl;	// Get FIRST
						*lpl = 0;
						lpl--;		// Back one more
						l++;		// count one more back before the zero created
						if( *lpl == '.' )
						{
							l++;	// Count one more
							*lpl = 0;
							lpl--;
						}
						if( c >= '5' )
						{
							for( j = (j - l); j >= 0; j-- )
							{
								if( ( j ) && ( lpr[j] == '.' ) )
								{
									// backed up to the DECIMAL
									lpr[j] = 0;		// Kill it
									j--;			// back one
									if( lpr[j] < '9' )
									{
										c = lpr[j];
										c++;
										lpr[j] = c;
										break;		// all done here
									}
									else if( lpr[j] == '9' )
									{
										lpr[j] = '0';	// bump to next
									}
									else
									{
										break;	// a NON-NUMBER or "."!!!
									}
								}
								else if( lpr[j] < '9' )
								{
									c = lpr[j];
									c++;
									lpr[j] = c;
									break;	// All done here
								}
								else if( lpr[j] == '9' )
								{
									lpr[j] = '0';	// bump to next
								}
								else
								{
									break;
								}
							}	// for a BACKWARDS count
							// SPECIAL CASE
							// ============
							if( j < 0 )
							{
								// Ok, we must INSERT a "1"
								lpl = &szless[0];
								strcpy( lpl, lpr );
								strcpy( lpr, "1" );
								strcat( lpr, lpl );
							}
						}	// if ROUNDING-UP required.
						break;
					}
					l = 0;
				}
				*lpl = c;
			}

		}	// if it contains ".", and is greater than 6 places
	}
}

// Dbl2Str Dbl2Stg DbltoStg
void	Double2Stg( char * lps, double factor )
{
	int		decimal, sign, precision;
	char *	buffer;

	precision = 16;
	buffer = ECVT( factor, precision, &decimal, &sign ); // WIN32 guard
	Buffer2Stg( lps, buffer, decimal, sign, precision );
}

#ifndef  DISKDB     // has its own copy in DDBUtil.c
/*	=======================================================
	void	Dbl2Stg( char * lps, double factor, int prec )
	======================================================= */

void    Dbl2Stg( char * lps, double factor, int prec )
{
    int             decimal, sign, precision;
    char *  buffer;

    if( prec )
        precision = prec;
    else
        precision = 16;

    buffer = ECVT( factor, precision, &decimal, &sign ); // WIN32 guard

    Buffer2Stg( lps, buffer, decimal, sign, precision );
}
#endif   // !DISKDB     // has its own copy in DDBUtil.c


void	Double2TStg( char * lps, double factor )
{
	Double2Stg( lps, factor );
	RTrimDecimal( lps );
}

uint32_t	Dbl2TStg( char * lps, double ds )
{
	uint32_t	dws = 0;
	double	div, dres;
	if( lps )
	{
		if( ds < (double)1000.0 )
		{
			dws = (uint32_t)ds;
			if( dws == 1 )
				strcpy( lps, "1 Byte" );
			else
				sprintf( lps, "%u Bytes", dws );
		}
		else if( ds < (double)(1000*1000) )
		{
			div = (double)1000.0;
			dres = ds / div;
			Dbl2Stg( lps, dres, 4 );
			strcat( lps, "KB" );
		}
		else if( ds < (double)(1000*1000*1000) )
		{
			div = (double)(1000*1000);
			dres = ds / div;
			Dbl2Stg( lps, dres, 4 );
			strcat( lps, "MB" );
		}
		else
		{
			div = (double)(1000*1000*1000);
			dres = ds / div;
			Dbl2Stg( lps, dres, 4 );
			strcat( lps, "GB" );
		}
		dws = strlen(lps);
	}
	return dws;
}

uint32_t	DW2TStg( char * lps, uint32_t dw )
{
	double ds;

	ds = (double)dw;

	return( Dbl2TStg( lps, ds ) );
}

/*	================================================
	Added Oct 1999
	================================================	*/
#define		MXRPTS		4
#define		MX1BUF		64
#ifdef WIN32
char *	Float2Str( float fNum )
{
	static int	_siNxt;
	static char _ssBuf[ ( MXRPTS * MX1BUF ) ];
	char	sBuf[MX1BUF];
	char *	cp;
	char *	cp2;
	int		i;

	_siNxt++;
	if( _siNxt >= MXRPTS )
		_siNxt = 0;
	cp2 = &sBuf[0];
	*cp2 = 0;
	Float2Stg( cp2, fNum, 5 );
	cp = &_ssBuf[ _siNxt * MX1BUF ];
	while( ( *cp2 != 0 ) && ( ( *cp2 == '[' ) || ( *cp2 == ' ' ) )  )
	{
		cp2++;
	}
	i = strlen(cp2); 
	if(i)
	{
		while( ( i ) &&
			( ( cp2[i-1] == ' ' ) || ( cp2[i-1] == ']' ) ) )
		{
			i--;
			cp2[i] = 0;
		}
	}
	RTrimDecimal( cp2 );
	strcpy( cp, cp2 );

	return cp;
}
#endif // WIN32

// #ifdef	FC4W - Added FA4 - 17FEB00
#if	( defined(FC4W) || defined(FA4) )

char *   GetDT4( int i )
{
        SYSTEMTIME      st;
        char *   lps;
        static char sszdt4[64];

        lps = &sszdt4[0];
//        GetSystemTime( &st );
        GetLocalTime( &st );
        sprintf( lps,
                "%4d-%02d-%02d %02d:%02d:%02d",
                (st.wYear & 0xffff),
                (st.wMonth & 0xffff),
                (st.wDay & 0xffff),
                (st.wHour & 0xffff),
                (st.wMinute & 0xffff),
                (st.wSecond & 0xffff) );
        UNREFERENCED_PARAMETER(i);
        return lps;
}

#endif	/* FC4W or FA4 */
// Added FA4 - 17FEB00

#ifdef WIN32
#ifndef	_CONSOLE	/* but NOT for CONAPPS !!! - 17FEB00 */
// ADDED FROM GMUtils.c - December, 1999
// =====================================
//
//  FUNCTION: CenterDialog(HWND, HWND)
//	(was CenterWindow in GMUtils)

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

int CenterDialog( HWND hChild, HWND hParent )
{
	int	bret = FALSE;
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;
	HWND	hwndChild, hwndParent;

	if( ( hwndChild = hChild   ) &&
		( hwndParent = hParent ) )
	{

		// Get the Height and Width of the child window
		if( GetWindowRect( hwndChild, &rcChild ) )
		{
			cxChild = rcChild.right - rcChild.left;
			cyChild = rcChild.bottom - rcChild.top;

			// Get the Height and Width of the parent window
			if( GetWindowRect( hwndParent, &rcParent ) );
			{
				cxParent = rcParent.right - rcParent.left;
				cyParent = rcParent.bottom - rcParent.top;

				// Get the display limits
				if( hdc = GetDC(hwndChild) )
				{
					cxScreen = GetDeviceCaps(hdc, HORZRES);
					cyScreen = GetDeviceCaps(hdc, VERTRES);
					ReleaseDC( hwndChild, hdc );

					// Calculate new X position,
					// then adjust for screen
					xNew = rcParent.left +
						( (cxParent - cxChild) / 2 );
					if( xNew < 0 )
					{
						xNew = 0;
					}
					else if( (xNew + cxChild) > cxScreen )
					{
						xNew = cxScreen - cxChild;
					}
					// Calculate new Y position,
					// then adjust for screen
					yNew = rcParent.top  +
						( (cyParent - cyChild) / 2 );
					if( yNew < 0 )
					{
						yNew = 0;
					}
					else if( (yNew + cyChild) > cyScreen )
					{
						yNew = cyScreen - cyChild;
					}

					// Set it, and return
					bret = SetWindowPos( hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER );
				}
			}
		}
	}
	return bret;
}
// END CenterDialog(HWND,HWND) ADDED FROM GMUtils.c
// December, 1999
// =====================================
#endif	/* !_CONSOLE = NOT for CONAPPS !!! - 17FEB00 */
#endif // WIN32

////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#define STAT64 _stat64
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#define STAT64 stat
#endif

static struct STAT64 buf64;
int is_file_or_directory64( char *path )
{
    if (!path)
        return NOT_VALID;
	if (STAT64(path,&buf64) == 0)
	{
		if (buf64.st_mode & M_IS_DIR)
			return VALID_DIR;
		else
			return VALID_FILE;
	}
	return NOT_VALID;
}
uint64_t get_last_file_size64()  { return buf64.st_size; }
uint64_t get_last_file_mtime64() { return buf64.st_mtime; }
uint64_t get_last_file_atime64() { return buf64.st_atime; }
uint64_t get_last_file_ctime64() { return buf64.st_ctime; }

int  IsValidFile( char * lpf )
{
    if (is_file_or_directory64(lpf) == VALID_FILE)
        return 1;
    return 0;
}

int  IsValidDir( char * lpf )
{
    if (is_file_or_directory64(lpf) == VALID_DIR)
        return 1;
    return 0;
}




// eof - GrmLib.c
