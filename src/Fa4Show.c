/*\
 * Fa4Show.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
\*/
#include "Fa4.h"
#undef  EXTRA_DEBUG2

// FIX20120407 - add -S - output ONLY alpha-numeric character if (gfStrip) 
#define ISUPPER(a) ((a >= 'A')&&(a <= 'Z'))
#define ISLOWER(a) ((a >= 'a')&&(a <= 'z'))
#define ISNUMBER(a) ((a >= '0')&&(a <= '9'))
#define ISALPHANUMERIC(a) (ISUPPER(a) || ISLOWER(a) || ISNUMBER(a))

extern uint32_t GettheLine( WS, uint32_t dwoff, uint32_t dwmax, uint32_t *pdw, uint32_t *pln, uint32_t dwmxvb );
extern int HasStg( WS, char * pn, char * ps );
extern void	OutALine( WS, char * lpv, uint32_t *pln, int bflg, uint32_t dwoff );
extern void  DoOutFileName( WS );
#ifdef   ADDCVSDATE
//   if( g_bCVSDate )
extern char * IsValidEntries( char * lpv );
#endif // #ifdef   ADDCVSDATE
extern uint32_t ChkMake2( WS, char * lpv, uint32_t dwk, uint32_t dwo );

static int fDoWrp;

// =====================================================================
//   void	ShowLine( WS, uint32_t dwoff, uint32_t dwmax, LPuint32_t pdw, LPuint32_t pln )
//
//   PURPOSE: To display the "found" line.
//
//   FIX20010328 Fix for UNIX file line output. UNIX only has an 0x0a
//
//   =====================================================================
#define  DEF_MAX_LINES     20

void holdlineio(void)
{
   g_ioHold = 1;
}


void Show_bin_Line( WS, uint32_t dwoff, char *lpmf, uint32_t dwmax, uint32_t *pln )
{
   uint32_t dwo = dwoff;
   uint32_t dwtmax = gdwMax1Line;
   char c;
   uint32_t dwk = 0;
   uint32_t dwend;
   int bflg = TRUE;
   char * ptmp;
	if(dwo) {
		dwk = 0;
		dwo--;
		while( dwo ) {
         ptmp = &lpmf[dwo];
			c = lpmf[dwo];
			//if( suc = NotAsciiRng( c ) )
			if( ( c < ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) ) {
				dwo++;	// Move off this
				break;
			}
			// Else, backup some more
			dwo--;
			dwk++;
			if( dwk >= g_dwBackup )	// starts as DEF_BACKUP(10)
				break;
		}
	}
	dwend = g_dwForward;	// DEF_FORWARD
	dwend = dwoff + dwk + g_dwCurFLen + g_dwForward;
	if( dwend > dwmax )
		dwend = dwmax;
	//for( dwo = dwoff; dwo < dwmax; dwo++ )
	dwk = 0;
	for( ; dwo < dwend; dwo++ ) {
		c = lpmf[dwo];
		if( ( c < ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) )
			break;

		lpVerb[dwk++] = c;
      if( dwk > dwtmax ) { // gdwMax1Line
			if( dwk > gdwEnd1Line ) {
				lpVerb[dwk] = 0;
				OutALine( pWS, lpVerb, pln, bflg, dwoff );
				dwk = 0;
				break;
			} else if( c <= ' ' ) {
				// *****************************
				fDoWrp = TRUE;
				if( fDoWrp ) {
					lpVerb[dwk] = 0;
					OutALine( pWS, lpVerb, pln, bflg, dwoff );
					dwk = 0;
					break;
				}
			}
		}
	}
}

#ifdef   EXTRA_DEBUG2
// chkreg2( &lpmf[dwoff], dwmax );
// pcre_compile(spb, options, &error, &errptr, pcretables);
// pl->pre = pcre_compile( pFind, gi_pcre_opions, &error, &errptr, gp_pcre_tables );
#define  MY_PUBLIC_EXEC_OPTIONS   (PCRE_ANCHORED|PCRE_NOTBOL|PCRE_NOTEOL|PCRE_NOTEMPTY|PCRE_NO_UTF8_CHECK| \
   PCRE_PARTIAL|PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|PCRE_NEWLINE_ANY)
pcre * tmp_pre = 0;
void chkreg2( char * pl, uint32_t dwmax )
{
   char * error = 0;
   int errptr = 0;
   int   res;
   char * ptmp = g_pTmpBuf;
   uint32_t dwo;

   while( *pl < ' ' ) {
      pl++;
      if(dwmax) dwmax--;
   }
   for( dwo = 0; dwo < dwmax; dwo++ ) {
      if( pl[dwo] < ' ' )
         break;
      ptmp[dwo] = pl[dwo];
   }
   if( tmp_pre == 0 ) {
      tmp_pre = pcre_compile( "\\w\\.fl(\\s|$)", gi_pcre_opions, &error, &errptr, gp_pcre_tables );
      if( !tmp_pre )
         return;
   }
   ptmp[dwo] = 0;
   if(dwo) {
      if( InStr( ptmp, "about_panel.fl" ) ) {
         res = 0;
      }
      res = pcre_exec( tmp_pre, 0, ptmp, dwo, 0,
         (gi_pcre_opions & MY_PUBLIC_EXEC_OPTIONS),
         &gi_pcre_offsets[0], PCRE_MAX_OFFSETS );
      if( res >= 0 ) {
         sprtf( "Found in line [%s] ..."MEOR, ptmp );
      } else {
         sprtf( "NOT Found in line [%s] ..."MEOR, ptmp );
      }
   }
}
#endif   // EXTRA_DEBUG2

void	ShowLine( WS,
				 uint32_t dwoff,
				 uint32_t dwmax,
				 uint32_t *pdw,
				 uint32_t *pln )
{
    char	c;
    uint32_t	dwo = 0;
    uint32_t   dwk;
    char *	lpmf;
	uint32_t	dwtmax;
//	int	   fDoWrp;
    int    bflg;       // TRUE if binary file
    uint32_t   _dwi2, _dwi3, _dwi4;
    char *  lpv;
    char *   ptmp;
    uint32_t    dwDnCnt = 0;
#ifdef   ADDLOPTION
    uint32_t    dwOutCnt;
    uint32_t    dwMaxOut = 0;
#endif   // #ifdef   ADDLOPTION
	char *	lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]
    int    bDnSome = FALSE;

	if( !VERB ) // if NO VERBALITY, then NO SHOW of line
		return;

    lpv = lpVerb;     // get pointer to out buffer
    *lpLine = 0;
#ifdef   ADDLOPTION
    dwOutCnt = 0;
    if( g_bGotLOpt ) {
        if ( g_bGotLLimit ) {
            dwMaxOut = g_dwLLLeft + g_dwLLRight;
        } else {
          if( g_bGotLFunc )
             dwOutCnt = (uint32_t)-1;
          else
             dwOutCnt = g_dwLOptLen;
       }
   }
#endif   // #ifdef   ADDLOPTION
#ifdef USE_ALLOCATED_LINE_BUFFER    // glpLineBuf & gdwLBSize
    gdwLineLen = GettheLine( pWS, dwoff, dwmax, pdw, pln, gdwLBSize ); 
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
	gdwLineLen = GettheLine( pWS, dwoff, dwmax, pdw, pln, MXLINEBUF ); 
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER y/n
	lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]
#ifdef	ADDINHIB
	lpmf = glpInhib;
	if( ( fInhibit ) &&		// case 'I':
		( giInhibCnt ) &&
		( lpmf ) )
	{
//#ifdef USE_ALLOCATED_LINE_BUFFER    // glpLineBuf & gdwLBSize
//        gdwLineLen = GettheLine( pWS, dwoff, dwmax, pdw, pln, gdwLBSize ); 
//#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
//	    gdwLineLen = GettheLine( pWS, dwoff, dwmax, pdw, pln, MXLINEBUF ); 
//#endif // #ifdef USE_ALLOCATED_LINE_BUFFER y/n
		if(gdwLineLen) {
			while( ( dwo = strlen(lpmf) ) > 0 ) {
				if( HasStg( pWS, glpLineBuf, lpmf ) ) {
					if( VERB6 ) {
						sprintf( lpVerb,
							"Note: -i%s inhibits line output.\r\n",
							lpmf );
						prt( lpVerb );
					}
					gdwFindsOut++;
					return;
				}
				lpmf += (dwo + 1);
			}
		}
	}
#endif	// ADDINHIB

//#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
//   if(( g_bDoAllin1 || g_bAllInOne ) && // -F:1 all finds in one file
//      ( g_dwFCnt > 1 ) ) { // = count on list
//      holdlineio();
//   }
//#else // !#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   if(( g_bDoAllin1  ) && // -F:1 all finds in one file
      ( g_dwFCnt > 1 ) ) { // = count on list
      holdlineio();
   }
//#endif // #ifdef ADD_F2_ALL1LINE y/n // FIX20060621 - Add -F:2 - all finds in 1 line
   

   if( !gfDoneFile ) { // have we already DONE this file name
	   DoOutFileName(pWS);
   }

	if( !VERB2 )
		return;

	if(( g_FileType & FT_BIN ) ||
		( g_fBinary ) ||
      ( g_bIsBinary2 ) ) {
      bflg = TRUE;   // set for BINARY OUTPUT - FIX20070914 - added g_bIsBinary2
   } else {
      bflg = FALSE;
   }

	dwk = 0;
	lpmf = (char *)glpMapView;
	dwtmax = gdwMax1Line;
   fDoWrp = FALSE;
   dwDnCnt = 0;      // lines OUTPUT
   dwo = *pdw;  // extract the BEGIN position from the pointer given
#ifdef   EXTRA_DEBUG2
   chkreg2( &lpmf[dwoff], dwmax );
#endif   // EXTRA_DEBUG2

	//if(( g_FileType & FT_BIN ) ||
	//	  ( g_fBinary ) )
	lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]
    if( bflg ) {
//      Show_bin_Line( pWS, dwoff, lpmf, dwmax, pln );
// #if 0 // code moved to function
// ****************************
        uint32_t    dwend;
		dwo = dwoff; 
        c = 0;
		if(dwo) {
			dwk = 0;
			dwo--;
			while( dwo ) {
				c = lpmf[dwo];
				//if( suc = NotAsciiRng( c ) )
				if( ( c < ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) ) {
					dwo++;	// Move off this
					break;
				}
				// Else, backup some more
				dwo--;
				dwk++;
				if( dwk >= g_dwBackup )	// starts as DEF_BACKUP(10)
					break;
			}
		}
		dwend = g_dwForward;	// DEF_FORWARD
		dwend = dwoff + dwk + g_dwCurFLen + g_dwForward;
		if( dwend > dwmax )
			dwend = dwmax;
		//for( dwo = dwoff; dwo < dwmax; dwo++ )
		dwk = 0;
		for( ; dwo < dwend; dwo++ ) {
			c = lpmf[dwo];
			if( ( c < ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) )
				break;

			lpVerb[dwk++] = c;
         if( dwk > dwtmax ) { // gdwMax1Line
				if( dwk > gdwEnd1Line ) {
					lpVerb[dwk] = 0;
					OutALine( pWS, lpVerb, pln, bflg, dwoff );
					dwk = 0;
					break;
				} else if( c <= ' ' ) {
					// *****************************
					fDoWrp = TRUE;
					if( fDoWrp ) {
						lpVerb[dwk] = 0;
						OutALine( pWS, lpVerb, pln, bflg, dwoff );
						dwk = 0;
						break;
					}
				}
			}
		}
      // FIX20070501 - try to get to the END OF THE ASCII, or space
      if( !( ( c < ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) ) &&
           ( dwk < dwtmax ) &&
           ( dwend < dwmax ) &&
           !fDoWrp ) {
			for( ; dwo < dwmax; dwo++ )
   		{
	   		c = lpmf[dwo];
		   	if( ( c <= ' ' ) || ( c == 0x7f ) || ( c & 0x80 ) )
			   	break;
   			lpVerb[dwk++] = c;
			   // check max. one line
            if( dwk > dwtmax ) {	// gdwMax1Line
				   break;
			   }
         }
      }

//#endif // #if 0 // moved to function
// ****************************
   } else {	// !BINARY = TEXT
      //char *   ptmp;
		// TEXT FILE
		// =========
      // Here we are pointing at the BEGINNING, well actually the END of the
      // last line, and here we search forward to get the END of the line
      // to output. If the line is TOO long, then it is WRAPPED by an
      // EARLY call to OutALine(), and then it continues accumulating LENGTH
      // into the lpVerb[] buffer. dwk is the LENGTH accumulated
      //
      // This seems to have been done in GettheLine(); above, but that is ONLY if
      // we have an INHIBIT active that might NOT output the line. So -
      // FIX20010328 Fix for UNIX file with only an 0x0a
		dwo = *pdw;  // extract the BEGIN position from the pointer given

      // ADDCVSDATE
#ifdef   ADDCVSDATE
      if( g_bCVSDate ) {
         char *   lpout = &g_cWorkBuf[0];
         ptmp = &lpmf[dwo];
			c = *ptmp;
         _dwi3 = 0;
         for( _dwi2 = dwo; _dwi2 < dwmax; _dwi2++ ) {
            ptmp = &lpmf[_dwi2];
	   		c = *ptmp;
   			if( c >= ' ' )
               break;
         }
         for( ; _dwi2 < dwmax; _dwi2++ ) {
            ptmp = &lpmf[_dwi2];
	   		c = *ptmp;
   			if( ( c == 0x0d ) || ( c == 0x0a ) )
               break;
            lpout[_dwi3++] = (char)c;
            if( _dwi3 > MXBIGBUF )
               break;
         }
         if( _dwi3 ) {
            lpout[_dwi3] = 0;
            lpout = IsValidEntries( lpout );
            if(lpout)
               OutALine( pWS, lpout, pln, bflg, dwoff );   // out it
         }
         return;
      }
#endif   // #ifdef   ADDCVSDATE
    dwk = 0;
    if ( g_bGotLOpt && g_bGotLLimit ) {
        if (dwMaxOut == 0) {
            // no limit == NO WRAP
            OutALine( pWS, lpLine, pln, bflg, dwoff );
            return;
        }
        if (gdwLineLen > dwMaxOut) {
            ptmp = &lpLine[g_dwLBOffBgn];
            _dwi2 = (g_dwCurFLen > 1) ? (g_dwCurFLen / 2) : 1;  // get HALF find width
            if (g_dwCurFLen > dwMaxOut) {
                // hmmm, ONLY output the find itself
                _dwi2 = g_dwLBOffBgn;
                while( _dwi2 < g_dwLBOffEnd ) {
    				lpv[dwk++] = lpLine[_dwi2++];
                }
                lpv[dwk] = 0;
            } else {
                // only want part of the line
                // If bgn offset > 0, can back up per 
                // g_dwLLLeft  //    = -LLnn - limit to left and right = nn / 2
                // g_dwLLRight //   = -LLnn:nn - limit left and limit right
                _dwi3 = g_dwLBOffBgn;
                if (g_dwLBOffBgn) {
                    // potential to go LEFT
                    if (_dwi2 < g_dwLLLeft) {
                        _dwi3 = g_dwLLLeft - _dwi2; // get LEFT back up amount
                        if (_dwi3 > g_dwLBOffBgn)
                            _dwi3 = 0;  // start at beginning
                        else
                            _dwi3 = g_dwLBOffBgn - _dwi3;
                        while( _dwi3 < g_dwLBOffEnd )
            				lpv[dwk++] = lpLine[_dwi3++];
                        lpv[dwk] = 0;
                    }
                }
                // get the actual find, if no before done
                while( _dwi3 < g_dwLBOffEnd )
                    lpv[dwk++] = lpLine[_dwi3++];
                lpv[dwk] = 0;
                if (g_dwLBOffBgn < gdwLineLen) {
                    // we can take some more to the right
                    if (_dwi2 < g_dwLLRight) {
                        _dwi4 = g_dwLLRight - _dwi2; // get RIGHT forward amount
                        while( ( _dwi3 < gdwLineLen ) && _dwi4-- )
            				lpv[dwk++] = lpLine[_dwi3++];
                        lpv[dwk] = 0;
                    }
                }
            }
            if(dwk) {
                sprintf(EndBuf(lpv)," (%d of %d)", dwk, gdwLineLen);
                OutALine( pWS, lpv, pln, bflg, dwoff );   // out it
                return;
            }
        }
    }

Rpt_Outs:
   	dwk = 0;
      // extract the LINE from the buffer
      // ********************************
		for( ; dwo < dwmax; dwo++ ) {
         //c = lpmf[dwo];
         ptmp = &lpmf[dwo];
			c = *ptmp;
			//if( c == 0x0d )
			if( ( c == 0x0d ) || ( c == 0x0a ) ) {
				// normally just end this line output
				if( dwk == 0 ) {
					if( (dwo+1) < dwmax ) {
						if( lpmf[dwo+1] == 0x0a )
							dwo++;
					}
					continue;   // we have NOTHING yet to output so CONTINUE
				} else {
					break;   // we have our LINE
               // NOTE: At either an 0x0d ***OR*** 0x0a character
            }
			}

			//if( c > 0x7f )
         if( c & 0x80 ) { // is HIGH bit SET
				//lpVerb[dwk++] = '@';
				lpv[dwk++] = '@';
				c &= 0x7f;  // remove HIGH bit
            // FIX20010824 - I put it BACK - allow TABS thru unharmed
            if( c == '\t' ) { // BUT this is an ARTIFICIAL TAB, so
					lpv[dwk++] = '^';
					c += '@';
            }
			}

			if( c < ' ' ) {
            // First I removed this TAB check, but
            // FIX20010824 - I put it BACK - allow TABS thru unharmed
				if( c != '\t' ) {
					//lpVerb[dwk++] = '^';
					lpv[dwk++] = '^';
					c += '@';
				}
			}
#ifdef   ADDDBL2  // FIX20001022 - fix find of "'@'" - and REMOVE this "doubling"
			else if( c == '^' ) {
				//lpVerb[dwk++] = '^';
				lpv[dwk++] = '^';
			} else if( c == '@' ) {
				lpv[dwk++] = '@';
				//lpVerb[dwk++] = '@';
			}
#endif   // ADDDBL2  // FIX20001022 - fix find of "'@'" - and REMOVE this "doubling"

			//lpVerb[dwk++] = c;		// add this char to OUTPUT
            // FIX20120407 - add -S - output ONLY alpha-numeric character
            if (gfStrip) {
                if (ISALPHANUMERIC(c)) {
                    lpv[dwk++] = c;		// add this char to OUTPUT
                } else {
                    if (dwk || bDnSome)    // if already have output
                        lpv[dwk++] = ' ';	// add SPACE to OUTPUT
                }
            } else {
                lpv[dwk++] = c;		// add this char to OUTPUT
            }
            lpv[dwk]   = 0;
			// check on LINE WRAP
			// ==================
         if( dwk > dwtmax ) {	// gdwMax1Line
				if( dwk > gdwEnd1Line ) {
					fDoWrp = TRUE;
				} else if( c == ' ' ) {
					// Yes, but really if JUST A FEW left
					// go more towards the gdwEnd1Line before
					// doing this LINE WRAP
					fDoWrp = TRUE;
					//	( (gdwEnd1Line - dwtmax) < (dwmax - dwo) ) )
					if( dwtmax < gdwEnd1Line ) {
						//uint32_t	_dwi2, _dwi3, _dwi4;
						uint32_t	_dwdif1 = (gdwEnd1Line - dwtmax);
						uint32_t	_dwdif2 = (dwmax - dwo);	// remaining
						// still less than MAX WRAP Point
						if( _dwdif1 > _dwdif2 )
							_dwdif1 = _dwdif2;
						_dwi3 = _dwi4 = 0;
						for( _dwi2 = (dwo+1); _dwi2 < dwmax; _dwi2++ ) {
							c = lpmf[_dwi2];
							if( c == 0x0d )
								break;
							_dwi3++;
							if( _dwi3 >= _dwdif1 )
								break;
							if( c > ' ' )
								_dwi4++;
						}

						if( ( _dwi3 < _dwdif1 ) &&
							( _dwi4 < 5 ) ) {
							dwtmax += _dwi3 + 1;
							fDoWrp = FALSE;
						}
					}
				}
				if( fDoWrp ) {
					lpVerb[dwk] = 0;  // close this OUTPUT
               if( g_bDoMake2 )
                  dwk = ChkMake2( pWS, lpVerb, dwk, *pdw );
               if( dwk )
                  OutALine( pWS, lpVerb, pln, bflg, dwoff );   // out it
					dwk = 0; // and restart accumulation
					fDoWrp = FALSE;
                    bDnSome = TRUE;
				}
			}
		}	// for( dwo = pdw[0]; dwo < dwmax; dwo++ )
      // that is EXTRACT the LINE from the buffer
	}

   if( dwk ) { // if we HAVE ouput LENGTH
		fDoWrp = FALSE;
      lpVerb[dwk] = 0;  // close this OUTPUT
      if( g_bDoMake ) {  // a makefile. obey continuation character
         while(dwk--) {
            if( lpVerb[dwk] > ' ' ) {
               if( lpVerb[dwk] == '\\' ) {
                  // we could, and perhaps should get fancy here and check
                  // that the following line is NOT a BLANK AND
                  // NOT a comment - that is begins with # char
                  // I have NOTED at least ONE unix make file like this!!!
                  // But try it and see - maybe outs an extra line or 2
                  fDoWrp = TRUE;
               }
               break;
            }
         }
         if( g_bDoMake2 ) // and we want to substitute any MACRO, and maybe KILL any # comment lines
            dwk = ChkMake2( pWS, lpVerb, dwk, *pdw );
      }

      if( dwk ) {
         OutALine( pWS, lpVerb, pln, bflg, dwoff );   // out it
         dwDnCnt++;     // count OUTPUT of lines
         bDnSome = TRUE;
      }

		dwk = 0; // and restart accumulation
      if( fDoWrp ) {
         fDoWrp = FALSE;
         goto Rpt_Outs;
      }
#ifdef   ADDLOPTION
      if(( dwOutCnt != 0 ) &&
         ( dwo < dwmax   ) ) {
         if( dwOutCnt == (uint32_t)-1 ) {
            if( dwDnCnt >= DEF_MAX_LINES ) {
               dwOutCnt = 0;  // arbitrary MAXIMUM reached!!! Kill more flag
               goto Rpt_Outs; // and do just ONE more
            }

            // go until double Cr/Lf ie paragraph,
            // of until function closed with ')' character
            for( _dwi2 = dwo; _dwi2 < dwmax; _dwi2++ ) {
               ptmp = &lpmf[_dwi2];
			      c = *ptmp;
               if( c == ')' ) {
                  dwOutCnt = 0;  // kill OUT counter
                  //goto Rpt_Outs;
                  break;
               } else if( ( c == 0x0d ) || ( c == 0x0a ) ) {
                  // found END OF NEXT LINE
                  if( c == 0x0d ) {
                     if( _dwi2 < dwmax ) {
                        if( lpmf[_dwi2+1] == 0x0a )
                           _dwi2++;
                     }
                  }
                  dwk = 0;
                  if( _dwi2 < dwmax ) {
                     for( ; _dwi2 < dwmax; _dwi2++ ) {
                        ptmp = &lpmf[_dwi2];
			               c = *ptmp;
                        if( c == ')' ) {
                           //dwOutCnt = 0;  // kill OUT counter
                           //goto Rpt_Outs;
                           break;
                        } else if( ( c == 0x0d ) || ( c == 0x0a ) ) {
                           // found END OF SECOND LINE
                           if( dwk == 0 )
                              dwOutCnt = 0;  // END OUTPUT
                        } else {
                           dwk++;   // count ONLY sig chars
                        }
                     }  // until END OF BUFFER
                  }
               } else if( c > ' ' ) {
                  // count SIG chars in line
                  dwk++;   // count NOT Cr/Lf chars
               }
            }  // for LENGTH OF BUFFER
            goto Rpt_Outs;
         } else {
            dwOutCnt--;    // decrement the OUT counter
            goto Rpt_Outs;
         }
      }
#endif   // #ifdef   ADDLOPTION
	}
}


// eof - Fa4Show.c
