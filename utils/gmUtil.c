/*\
 * gmUtil.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * general utility functions ...
 *
\*/
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
