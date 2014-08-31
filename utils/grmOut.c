/*\
 * grmOut.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * file and console io ...
 *
\*/
#ifdef WIN32
#define	WIN32_MEAN_AND_LEAN
#include	<windows.h>
#endif // WIN32
#include	"grmOut.h"

// FIX20140831: Make diag log file PERMANENT
#define		ADDDIAGT

//#define		MXIO		256
#define		MXIO		1024
#define		MXSTG		32		// In PutThous( )

#ifdef	ADDDIAGT
extern	void	WriteDiagFile( char * lps );
#endif	// ADDDIAGT

int	bEnsureTail = FALSE;
int	gbCheckCrLf = TRUE;
#if	!(defined FC4W)
int	   fOnce    = TRUE;
HANDLE	hOut     = 0;
HANDLE   hErrOut  = 0;
int     bRedirON = FALSE;
#endif	/* !FC4W */

HANDLE	hUserOut;
//	char * buf = _s_goutbuf; [MXIO + 4];
char _s_goutbuf[MXIO + 4];

// NOTE: In here VH(a) is defined
//#ifndef  VFH
//#define  VFH(a)   ( a && ( a != INVALID_HANDLE_VALUE ) )
//#endif   // VFH

// extracted from FixF32 prohect
// HOW TO DETECT if the CONSOLE stdout is REDIRECTED to a file?
// ============= fprintf() fopen() ????
// Found a way! Presently using handle to get mode. If this get
// fails, then we are being redirected!!

// General Services
void	oi( char * lps )    // do output - hOut(stdout), hUserOut if valid, WriteDiagFile(lps)
{
	int		i;
	uint32_t	dw;
#ifndef	FC4W
#ifdef WIN32
	if( fOnce )
	{
		hOut    = GetStdHandle( STD_OUTPUT_HANDLE );
      hErrOut = GetStdHandle( STD_ERROR_HANDLE  );   // error out
      if( VH(hOut) )
      {
         uint32_t dwm;
         if( !GetConsoleMode( hOut, &dwm ) )
            bRedirON = TRUE;
         // g_bRedirON = TRUE;
      }
		fOnce = FALSE;
	}
#else // !WIN32
	if ( fOnce )
	{
		hOut = stdout;
		hErrOut = stderr;
		fOnce = 0;
		// printf("Set hOut=%p, and hErrOut=%p\n", hOut, hErrOut );
	}
#endif // WIN32 y/n
#endif	/* !FC4W */

   i = 0;
	if( lps )
      i = strlen( lps );
   if(i)
	{
#ifdef WIN32
#ifndef	FC4W
//		WriteAFile( hOut, lps, i );
		WriteFile( hOut, lps, i, &dw, NULL );
#endif	/* !FC4W */

		if( VH(hUserOut) )
			WriteFile( hUserOut, lps, i, &dw, NULL );
#else // !WIN32
        dw = fwrite( lps, 1, i, hOut );
		if( VH(hUserOut) )
			dw = fwrite( lps, 1, i, hUserOut );
#endif // WIN32 y/n

#ifdef	ADDDIAGT
		// write diagnostic text file
		WriteDiagFile( lps );
		// ==========================
#endif	// ADDDIAGT

	}
}

void	doi( char * lps )
{
#ifdef	ADDDIAGT
		WriteDiagFile( lps );
#endif	// ADDDIAGT
}

void	SetEnsureCrLf( int i )
{
	if( i )
		gbCheckCrLf = TRUE;
	else
		gbCheckCrLf = FALSE;
}

int	GetEnsureCrLf( void )
{
	return( gbCheckCrLf ? TRUE : FALSE );
}

void	prt( char * lps )
{
	// char	buf[MXIO + 4];
	char * buf = _s_goutbuf; // [MXIO + 4];
	char *	lpb;
	int		i, j, k;
	char	c, d;

   i = strlen(lps);
	if(i)
	{
		k = 0;
		d = 0;
		lpb = &buf[0];
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c == 0x0d )
			{
				if( (j+1) < i )
				{
					if( lps[j+1] != 0x0a )
					{
						lpb[k++] = c;
						c = 0x0a;
					}
				}
				else
				{
					lpb[k++] = c;
					c = 0x0a;
				}
			}
			else if( c == 0x0a )
			{
				if( d != 0x0d )
				{
					lpb[k++] = 0x0d;
				}
			}
			lpb[k++] = c;
			d = c;
			if( k >= MXIO )
			{
				lpb[k] = 0;
				oi( lpb );  // reached MXIO
				k = 0;
			}
		}	// for length of string
		if( k )
		{
			if( ( gbCheckCrLf ) &&
				( d != 0x0a ) )
			{
				// add Cr/Lf pair
				lpb[k++] = 0x0d;
				lpb[k++] = 0x0a;
				lpb[k] = 0;
			}
			lpb[k] = 0;
			oi( lpb );  // out remainder
		}
	}
}

void	dout( char * lps )
{
//	char	buf[MXIO + 4];
 static char _s_buf[MXIO + 4];
	char * buf = _s_buf; // [MXIO + 4];
	char *	lpb;
	int		i, j, k;
	char	c, d;

	i = strlen( lps );
	if(i)
	{
		k = 0;
		d = 0;
		lpb = &buf[0];
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c == 0x0d )
			{
				if( (j+1) < i )
				{
					if( lps[j+1] != 0x0a )
					{
						lpb[k++] = c;
						c = 0x0a;
					}
				}
				else
				{
					lpb[k++] = c;
					c = 0x0a;
				}
			}
			else if( c == 0x0a )
			{
				if( d != 0x0d )
				{
					lpb[k++] = 0x0d;
				}
			}
			lpb[k++] = c;
			d = c;
			if( k >= MXIO )
			{
				lpb[k] = 0;
				doi( lpb );
				k = 0;
			}
		}	// for length of string
		if( k )
		{
			if( bEnsureTail )
			{
				if( d != 0x0a )
				{
					lpb[k++] = 0x0d;
					lpb[k++] = 0x0a;
					lpb[k] = 0;
				}
			}
			lpb[k] = 0;
			doi( lpb );
		}
	}
}

#define		MXSTG		32

void	PutThous( char * lpb,
				 int	len,
				 uint32_t	size )
{
	int		i, j, k, l;
	char	szform[16];	// For FORM string
	char	sztmp[MXSTG+4];	// For initial results
	char	c;

	if( len )
	{
		if( len < MXSTG )
			i = len;
		else
			i = MXSTG;
	}
	else
	{
		i = 9;
	}
	sprintf( &szform[0], "%%%uu", i );	// Get a format string
	sprintf( &sztmp[0], &szform[0], size );	// and initial string

	i = strlen( &sztmp[0] ); 
	if(i)
	{
		strcpy( lpb, &sztmp[0] );	// Just COPY first
		k = 0;
		l = i - 1;
		for( j = l; j >= 0; j-- )
		{
			c = sztmp[j];
			if( c > ' ' )
			{
				if( k == 3 )
				{
					lpb[l--] = ',';
					k = 0;
					if( l < 0 )
						break;
				}
			}
			lpb[l--] = c;
			k++;
			if( l < 0 )
				break;
		}
	}
	else
	{
		sprintf( lpb, "%u", size );
	}
}

char *	ShortName( char * lps, char * lpd, int siz )
{
	int		i, i2, i3, k, j;

	k = 0;
	if( (siz > 10) &&
		((i = strlen( lps )) > (siz+3) ) )
	{
		i2 = (siz-1) / 2;
		i3 = i2 + (i - (i2 * 2));
		for( j = 0; j < i; j++ )
		{
			if( j < i2 )
			{
				lpd[k++] = lps[j];
			}
			else if( j == i2 )
			{
				lpd[k++] = '.';
				lpd[k++] = '.';
				lpd[k++] = '.';
			}
			else if( j > i3 )
			{
				lpd[k++] = lps[j];
			}
		}
		lpd[k] = 0;
	}
	else
	{
		strcpy( lpd, lps );
	}
	return	lpd;
}

// eof - GrmOut.c
