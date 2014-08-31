/*\
 * Fa4Comp.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * Full compare of FIND ...
 *
\*/
#include "Fa4.h"

// static const char *module = "Fa4Comp";

int	NotAsciiRng( char c )
{
	int	flg = TRUE;

	if( ( ( c >= 'a' ) && ( c <= 'z' ) ) ||
		( ( c >= 'A' ) && ( c <= 'Z' ) ) ||
		( ( c >= '0' ) && ( c <= '9' ) ) )
	{
		flg = FALSE;
	}
	return flg;
}

#define  ISSPACEY(a)    ( a <= ' ' )

#ifdef USE_EXCLUDE_LIST
// FIX20101007 - exclude directories in excluded list, when doing find in directory list (-d)
int Check4ExcludedDir(void)
{
    int suc = TRUE;
    char * pdir = g_cDirBuf;
    size_t i;
    char c;
    int res;
    char * pbgn = pdir;
    size_t len = strlen(pdir);
    for (i = 0; i < len; i++)
    {
        c = pdir[i];
        if (IS_PATH_SEP_CHAR(c)) {
            pdir[i] = 0;
            res = InExcludeD( pbgn );
            pdir[i] = c;
            pbgn = &pdir[i+1];
            if (res) {
                suc = FALSE;
                break;
            }
        }
    }
    return suc;
}
#endif // #ifdef USE_EXCLUDE_LIST


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : FullComp
// Return type: int 
// Arguments  : WS
//            : char * lpf
//            : uint32_t dwoff
//            : uint32_t dwmax
//            : char * lpc
//            : uint32_t len
//            : Puint32_t pdw
// Description: Found JUST the first LETTER match
//              Now check it match the WHOLE find request
///////////////////////////////////////////////////////////////////////////////
int	FullComp( WS,
				 char * lpf, uint32_t dwoff, uint32_t dwmax,
				 char * lpc, uint32_t flen) // , Puint32_t pdw )
{
    uint32_t    i;
	int	   c, cm;
	int	   suc, fC, fP;
	char *   pbgn = &lpf[dwoff];        // pointer to FIRST LETTER
    uint32_t    dwrem = (dwmax - dwoff);

	suc   = FALSE;       // assume NOT the same
	i  = 0;
	fC = g_fCase;         // extract CASE flag
	fP = gfParity;       // extract PARITY flga

	if( fP )
		cm = 0x7f;
	else
		cm = 0xff;

    // can only find if LENGTH of (copy of) find string,
    // is LESS than remainder of file!
    if( flen == 1 ) {
      suc = TRUE;
    } else if( flen < dwrem ) {
        if( gfSpacey ) {   // -I to ignore SPACE chars in COMPARE
            lpc++;
            pbgn++;  // skip past first letter on both
            flen--;  // reduce find length
            i = dwoff;
            i++;
            while( *lpc && flen && ( i < dwrem ) ) {
                if( *lpc <= ' ' ) {
                    if( *pbgn <= ' ' ) {
                        while( *lpc && ( *lpc <= ' ' ) ) {
                            flen--;
                            lpc++;
                        }
                        if( *lpc ) {
                            while( *pbgn && ( *pbgn <= ' ' ) && ( i < dwrem ) ) {
                                i++;
                                pbgn++;
                            }
                            continue;
                        } else {
                            suc = TRUE;
                            break;
                        }
                    } else
                        break;   // space in find, NOT in file = FAILED
                } else {
                    if(fC) {
                        if( *lpc != (*pbgn & cm) )
                            break;
                    } else {
                        if( *lpc != toupper(*pbgn & cm) )
                            break;
                    }
                }
                lpc++;   // next FIND string char
                pbgn++;  // next FILE char
                i++;     // bump the offset into the buffer
                flen--;  // and reduce the length of finding
            }  // for a while
            if( (flen == 0) &&
                (*lpc == 0) )
                suc = TRUE;
        } else { // no ignore space chars
            i = 1;
            if( fC ) {
                for( ; i < flen; i++ ) {
                    c = ( pbgn[i] & cm );
                    if( lpc[i] != c )
                        break;
                }
            } else {
                for( ; i < flen; i++ ) {
                    c = toupper( ( pbgn[i] & cm ) );
                    if( lpc[i] != c )
                        break;
                }
            }
            if( i == flen )
                suc = TRUE;
        }
    }
	if( suc ) {
		suc = TRUE;		// We have a FIND
		if( gfWhole ) {
            if( dwoff ) {
                c = lpf[dwoff-1]; // get PREVIOUS character
                if( fP )
                    c = c & 0x7f;
                if( g_fWhole2 )   // FIX20010413 - add -w2 to force SPACE delimited
                    suc = (int)ISSPACEY( c );
                else
                    suc = (int)NotAsciiRng( (char)c );
            }
			if( suc ) {
                if( (dwoff + i) < dwmax ) {
                    c = lpf[dwoff+i]; // get FOLLOWING character
					if( fP )
						c = c & 0x7f;
                    if( g_fWhole2 )   // FIX20010413 - add -w2 to force SPACE delimited
                        suc = (int)ISSPACEY( c );
                    else
                        suc = (int)NotAsciiRng( (char)c );
                }
            }
		}
        // FIX20101007 - exclude directories in excluded list
#ifdef USE_EXCLUDE_LIST
        if (suc && g_dwExclDCnt && gfDirLst) {
            suc = Check4ExcludedDir();
        }
#endif
		if( suc ) {
			// All appears OK
			// ==============
			gdwFinds++;		// successful FIND
            g_dwOffBgn = dwoff;
            g_dwOffEnd = dwoff + i;
		}
    }
    // UNREFERENCED_PARAMETER(pdw);
    return suc;
}

// eof - Fa4Comp.c
