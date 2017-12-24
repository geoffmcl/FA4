/*\
 * fa4Find.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
\*/
#include "Fa4.h"
#undef  EXTRA_DEBUG

extern uint32_t g_dwFinds, dwFind1, g_dwItems, dwLnBgn, dwLastLn, g_dwTotFinds;
extern void	ShowFNS( WS );
extern void	Move2Find( char * lpd, char * pFind );
extern void	ShowFinding( WS, uint32_t dwItems, PFLINE pl );
// ENGLISH
extern char szEVLabel[]; // = "Volume in drive ";	// C is IPNS-EAST
extern char szESerial[]; // = "Volume Serial Number is "; // 359A-0A25
extern char szEDirect[]; // = "Directory of "; // C:\GEOFF
// FRENCH
extern char szFVLabel[]; // = "Le volume dans le lecteur ";	// L est HDD4
extern char szFDirect[]; // = "Répertoire de "; // L:\DISNEY
extern int iLen_ok( int i );
extern int IsEDirect2( char * lpv, uint32_t ins );
extern int IsFDirect2( char * lpv, uint32_t ins );
extern int	FullComp( WS,
				 char * lpf, uint32_t dwoff, uint32_t dwmax,
				 char * lpc, uint32_t flen); // , Puint32_t pdw );
extern void	ShowLine( WS,
				 uint32_t dwoff,
				 uint32_t dwmax,
				 uint32_t *pdw,
				 uint32_t *pln );
extern void	ShowFind1( WS, uint32_t dwFind1, char * lpf, int doregex );
extern void	prt5( char * lps ); // output if VERB5

//extern void OutGFName( WS );
//#ifndef  NDEBUG
#define  DEBUG
#include "pcre_internal.h"
#include "pcre_printint.c"
//#endif   // !NDEBUG

#define  THE_PUBLIC_EXEC_OPTIONS   (PCRE_ANCHORED|PCRE_NOTBOL|PCRE_NOTEOL|PCRE_NOTEMPTY|PCRE_NO_UTF8_CHECK| \
   PCRE_PARTIAL|PCRE_NEWLINE_CR|PCRE_NEWLINE_LF|PCRE_NEWLINE_ANY)

#define	MXLPS	10
uint32_t	   lpoff[MXLPS+2];      // keep a set of PREVIOUS line offsets

#define	IncLine	\
{\
	for( is = 0; is < MXLPS; is++ )\
		lpoff[is+1] = lpoff[is];\
	lpoff[0] = dwi;\
	dwl++;\
	gdwLnInFile++;\
	gdwTotLines++;\
}


PMWL g_pActWorkList = 0;

#if 0
void	ShowALLFinds_NOT_USED( WS, uint32_t dwFinds, uint32_t dwItems, char * lpd )
{
	char *	lpf;
	if( !gfDoneFile ) {
		sprintf( lpVerb, "%s"PRTTERM, glpActive );
		prt( lpVerb );
		gfDoneFile = TRUE;
	}
	if( dwItems == 0 ) {
		sprintf( lpVerb,
            "WARNING: ShowALLFinds(): No items to FIND! (Internal ERROR)"PRTTERM );
	} else {
		if( dwFinds ) {
			if( dwItems == 1 ) {
				if( dwFinds == 1 )
					lpf = "Found \"%s\" in [%s] 1 time!"PRTTERM;
				else
					lpf = "Found \"%s\" in [%s] %u times!"PRTTERM;

				sprintf( lpVerb,
					lpf,	// like "Found \"%s\" in [%s] 1 time!",
					lpd,
					ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ),
					dwFinds );
			} else {
				lpf = "Found %u items in [%s] %u times!"PRTTERM;
				if( dwFinds == 1 )
					lpf = "Found %u items in [%s] 1 time!PRTTERM";

				sprintf( lpVerb,
					lpf,	// like "Found %u items in [%s] %u times!",
					dwItems,
					ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ),
					dwFinds );
			}
		} else {
			if( dwItems == 1 ) {
				sprintf( lpVerb, "NO Finds of \"%s\" in [%s]!"PRTTERM,
					lpd,
					ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
			} else {
				sprintf( lpVerb, "NO Finds of %u items in [%s]!"PRTTERM,
					dwItems,
					ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
			}
		}
	}

	prt( lpVerb );
}
#endif

void	ShowFinds( WS, uint32_t dwFinds, uint32_t dwItems, char * lpd )
{
	char *	lpf;
	if( !gfDoneFile ) {
		sprintf( lpVerb, "%s"PRTTERM, glpActive );
		prt5( lpVerb );
		if( VERB5 )
			gfDoneFile = TRUE;
	}
	if( dwItems == 0 ) {
		sprintf( lpVerb,
            "WARNING: ShowFinds(): dwItems == 0! No items to FIND! (Internal ERROR)"PRTTERM );
	} else {
        char * psn = ShortName( glpActive, glpTmp, W.ws_dwMxFilNm );
        if( dwFinds ) {
			if( dwItems == 1 ) {

				if( dwFinds == 1 )
					lpf = "Found \"%s\" in [%s] 1 time!"PRTTERM;
				else
					lpf = "Found \"%s\" in [%s] %u times!"PRTTERM;

				sprintf( lpVerb,
					lpf,	// like "Found \"%s\" in [%s] 1 time!",
					lpd,
					psn, // ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ),
					dwFinds );
			} else {
				lpf = "Found %u items in [%s] %u times!"PRTTERM;
				if( dwFinds == 1 )
					lpf = "Found %u items in [%s] 1 time!PRTTERM";

				sprintf( lpVerb,
					lpf,	// like "Found %u items in [%s] %u times!",
					dwItems,
					psn, // ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ),
					dwFinds );
			}
		} else {
			if( dwItems == 1 ) {
				sprintf( lpVerb, "NO Finds of \"%s\" in [%s]!"PRTTERM,
					lpd,
					psn ); // ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
			} else {
                if (g_bAllInOne) {
				    sprintf( lpVerb, "NO Finds of %u items in ONE line in [%s]!"PRTTERM,
					    dwItems,
					    psn ); // ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
                } else {
				    sprintf( lpVerb, "NO Finds of %u items in [%s]!"PRTTERM,
					    dwItems,
					    psn ); // ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
                }
			}
		}
	}
	prt5( lpVerb );
}

void Add2gsFileList( WS )
{
   //PBY_HANDLE_FILE_INFORMATION phi = &gsFileInf;
   if( VERB4 )
   {
      // FIX20010319 -v4 adds date order of finds at end
      PMWL  pmwl = (PMWL)MALLOC(LPTR, sizeof(MWL));
      if(pmwl)
      {
         g_pActWorkList = pmwl;
         pmwl->wl_dwRank = 0;
         strcpy( &pmwl->wl_cName[0], glpActive );
         //pmwl->wl_DateTime = phi->ftLastWriteTime;
         pmwl->wl_DateTime64 = 0;
         pmwl->wl_FileSize64 = 0;
         if (IsValidFile( glpActive )) {
             pmwl->wl_DateTime64 = get_last_file_mtime64();
             pmwl->wl_FileSize64 = get_last_file_size64();
         }
         pmwl->wl_dwFound = g_dwFinds; // get the finds in this file
         pmwl->wl_dwItems = g_dwItems; // count of items being found
         InsertTailList( &gsFileList, (PLE)pmwl );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : DoLineChk
// Return type: void 
// Arguments  : WS
//            : char * lpb
//            : uint32_t dwLnBgn
//            : uint32_t dwi
// Description: I can see I am looking for "Directory of "; like C:\GEOFF
//              or the french equivalent, but WHY beats me now.
// FIX20050121 - If finding a file name in a directory list, it would be good
// if, when found, the DIRECTORY NAME is also output ...
// char	szEDirect[] = "Directory of "; // C:\GEOFF
// FRENCH
// char	szFVLabel[] = "Le volume dans le lecteur ";	// L est HDD4
// char	szFDirect[] = "Répertoire de "; // L:\DISNEY

///////////////////////////////////////////////////////////////////////////////
void	DoLineChk( WS, char * lpb, uint32_t dwLnBgn, uint32_t dwi )
{
	int i, j, k, c, d;
	if(( lpb ) &&
		( dwi > dwLnBgn ) )
	{
		uint32_t	iLen = (dwi - dwLnBgn); // get length of line
      char *	lpd  = &lpb[dwLnBgn]; // pointer to beginning
      //extern int iLen_ok( int i );
      if( iLen_ok( iLen ) ) {
		//if(( iLen > (sizeof(szEDirect)-1) ) ||
		//	( iLen > (sizeof(szFDirect)-1) ) ) 	{
         k = 0;
         while( iLen && (*lpd <= ' ') ) {
            lpd++;
            iLen--;
            k++;
         }
         // at least it has the length
         c = *lpd;
         i = j = d = 0;
         if( c == szEDirect[0] ) {
            i = IsEDirect2( lpd, iLen );
            j = i;
            d = szEDirect[0];
         } else if( c == szFDirect[0] ) {
            j = IsFDirect2( lpd, iLen );
            i = j;
            d = szFDirect[0];
         }
		   //if( ( ( iLen > (sizeof(szEDirect)-1) ) && ( i ) ) ||
			//   ( ( iLen > (sizeof(szFDirect)-1) ) && ( j ) ) )
		   if( i  || j ) {
            // got a match ...
			   char *	lpdir;
			   if( (uint32_t)i < iLen ) {
				   lpdir = &lpb[dwLnBgn + i + k];
				   k = (iLen + k) - i;
				   for( j = 0; j < k; j++ )
				   {
					   if( lpdir[j] < ' ' ) {
						   break;
					   }
					   gcDirBuf[j] = lpdir[j];
					   if( j > 260 )
						   break;
				   }
				   gcDirBuf[j] = 0;
				   i = (int)strlen( &gcDirBuf[0] ); 
				   if(i)
				   {
					   if( gcDirBuf[i-1] != PATH_CHAR )
					   {
						   strcat( gcDirBuf, PATH_SEP );
						   i++;
					   }
				   }
				   giDirLen = i; // set the LENGTH of the string copied
			   }
		   }
      }
	}
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ShowFLines
// Return type: void 
// Argument   : PLE ph == = &g_sLines;
// Description: Output the actual FINDS, but take into account any
//              TYPE and numbering during the output
// Called if( g_dwFCnt > 1 ) { ShowFLines( &g_sLines ); }
///////////////////////////////////////////////////////////////////////////////
void  ShowFLines( PLE ph )  // = &g_sLines;
{
   PLE   pn, pn1;
   PFLINE pl, pl1;
   uint32_t dwt;
   uint32_t dwc = 0;
   Traverse_List(ph,pn)
   {
      pl = (PFLINE)pn;
      dwt = pl->dwTyp; // if we have a TYPE (1=same file, 2=same file and line)
      if(dwt)   
      {
         switch(dwt)
         {
         case '1':   // can ONLY output if other types found in same FILE
            Traverse_List(ph, pn1)
            {
               if( pn != pn1 )
               {
                  pl1 = (PFLINE)pn1;
                  if(( pl1->dwTyp ) && // also has to have a TYPE
                     ( pl->pFind != pl1->pFind ) ) // and is a DIFFERENT find object
                  {
                     if( pl->uint32_t == pl1->uint32_t )    // if in the SAME file
                     {
                        pl->dwTyp = 0;
                        dwc++;
                        if( pl->dwNum != pl1->dwNum ) // if NOT the SAME line
                        {
                           pl1->dwTyp = 0;
                           dwc++;
                        }
                     }
                  }
               }
            }
            break;
         case '2':   // can ONLY output if other types found in same file AND line
            Traverse_List(ph, pn1)
            {
               if( pn != pn1 )
               {
                  pl1 = (PFLINE)pn1;
                  if(( pl1->dwTyp == '2' ) && // also has to have a TYPE
                     ( pl->pFind != pl1->pFind ) ) // and is a DIFFERENT find object
                  {
                     if(( pl->uint32_t == pl1->uint32_t ) &&   // if in the SAME file
                        ( pl->dwNum == pl1->dwNum ) )    // and on the SAME line
                     {
                        pl->dwTyp = 0;
                        //pl1->dwTyp = 0;
                        dwc++;
                     }
                  }
               }
            }
            break;
         default:    // unknown TYPE
            pl->dwTyp = 0;
            dwc++;
            prt( "WARNING: Appear to have an UNKNOWN type!"MEOR );
            break;
         }
      }
      else
         dwc++;
   }

   if(dwc) {
      Traverse_List(ph,pn) {
         pl = (PFLINE)pn;
         if( pl->dwTyp == 0 ) {
             if( g_ioHold ) {
                 add2outlist( &pl->cLine[0] ); // add to out list only
             } else {
                 prt( &pl->cLine[0] );
             }
         }
      }
      dwc = 0;
   }
   FreeLList(ph,pn);
}

// #endif   // #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
// #ifdef   USEFINDLIST    // = FIX20010703
void Find_Skip( WS, uint32_t dwfs, char * lpc, char c, uint32_t flen, char * lpd, int fBin );
void Find_Skip_Perl( WS, uint32_t dwfs, char * lpc, char c, uint32_t flen, char * lpd, int fBin );

uint32_t g_dwMaxFnds = 0;
uint32_t g_dwFndFlg;
uint32_t g_dwThisFnd;
// ===============================================================
// uint32_t Process_Line_For_Finds( WS, uint32_t bgn, uint32_t end )
// process this line, from offset 'bgn' to offset 'end'
// for the multiple finds = ie find all in one line
// ===============================================================
uint32_t Process_Line_For_Finds( WS, uint32_t bgn, uint32_t end )
{
    char     *lpc = (char *)glpMapView;
    PLE      phf = &g_sFind;        // get list of FIND strings
    PLE      pn;
    PFLINE   pl;
    char     *pFind, *lpd;
    uint32_t    dwi, flen, c1, c, fnd, fndcnt;

	lpd = &g_szCurrFind[0];  // = GW.ws_szCurrFind
    fndcnt = fnd = 0;
    Traverse_List( phf, pn ) // for EACH of the FIND strings, search this FILE
	{
        pl = (PFLINE)pn;
        g_psActFind = pl; // set current active FIND structure
        pFind = &pl->cLine[0];
        flen = (uint32_t)strlen(pFind);
		Move2Find( lpd, pFind );
		g_lpCurrFind = pFind;   // original string
		g_dwCurrFLen = flen;    // length of FIND string
		g_lpCurFind  = lpd;     // copy of FIND string (with CASE fixed)
		g_dwCurFLen  = (uint32_t)strlen( lpd );
#ifdef   ADD_REGEX
        if( pl->isregex ) {
            if( !pl->pre ) {
                char * error = 0;
                int errptr = 0;
                // pcre_compile(spb, options, &error, &errptr, pcretables);
                pl->pre = pcre_compile( pFind, gi_pcre_opions, &error, &errptr, gp_pcre_tables );
                if( !pl->pre ) {
                    sprintf( glpError, "ERROR: Failed COMPILE of regex [%s]!"PRTTERM,
                        pFind );
                    Err_Exit( -1 );
                }
            }
        }
#endif   // ADD_REGEX
		c1 = *lpd;	// Get first character from COPY of find string
		ShowFinding( pWS, g_dwItems, pl );
        fnd = 0;
        if( pl->isregex && pl->pre ) {
            int res = -1;
            if( bgn < end ) {
                gi_pcre_offsets[0] = 0;
                gi_pcre_offsets[1] = 0;
                res = pcre_exec( pl->pre, 0, &lpc[bgn], end - bgn, 0,
                    (gi_pcre_opions & THE_PUBLIC_EXEC_OPTIONS),
                    &gi_pcre_offsets[0], PCRE_MAX_OFFSETS );
                if( res >= 0 ) {
                    // success with this line
                    if(VERB9) {
                        if(( gi_pcre_offsets[0] >= 0 ) &&
                            ( gi_pcre_offsets[1] > gi_pcre_offsets[0] )) {
                            int fndlen = gi_pcre_offsets[1] - gi_pcre_offsets[0];
                            char * pfnd = &lpc[dwLnBgn] + gi_pcre_offsets[0];
                            if(fndlen >= 0) {
                                char * ptmp = GetNxtBuf();
                                strncpy(ptmp, pfnd, fndlen);
                                ptmp[fndlen] = 0;
                                sprtf( "v9: Found [%s]..."MEOR, ptmp );
                            }
                     }
                }
                fnd++;
                fndcnt++;
            } else {
               // failed with this line
            }
         }
      } else {
         for( dwi = bgn; dwi < end; dwi++ ) {
  			   c = (*GetChr) ( &lpc[dwi] );  // call indirectly to GetChr (W.GETCHR) function
            // c = lpc[dwi];
            if( c1 == c ) {
                //if( FullComp( pWS, lpc, dwi, end, lpd, flen, &dwl ) ) {
                    if( FullComp( pWS, lpc, dwi, end, lpd, flen ) ) {
                        fnd++;
                        fndcnt++;
                        break; // only need to find is once, in this line
                    }
                }
            }
        }
        if( !fnd ) {
            break; // any NOT found all done
        }
    } // end Traverse_List( phf, pn ) // for EACH FIND strings, search this LINE

   if( fndcnt < g_dwFCnt )
      fnd = 0;

   if(fnd) {
      lpd = &lpc[bgn];
   }
   return fnd;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Find_In_Lines
// Return type: void 
// Arguments  : WS
//            : char * lpInF
// Description: This is where it ALL happens if g_bAllInOne. The file has
//              been mapped and we process the buffer char by char looking
//              for the first find char.
// Process the file, line by line, if NOT binary, finding each find
// within that line
///////////////////////////////////////////////////////////////////////////////
void Find_In_Lines( WS, char * lpInF )
{
   uint32_t 	dwfs = gdwActLen;
   uint32_t    dwi;
   char *    lpc = (char *)glpMapView;
   uint32_t    c, bgnoff, is, dwl;
   char * 	lpd = &g_szCurrFind[0];  // = GW.ws_szCurrFind
   PLE      phf = &g_sFind;        // get list of FIND strings
   PLE      pn;

   // simple scan through the buffer
   bgnoff = 0;
   dwl = 0;
	for( is = 0; is < MXLPS; is++ )
			lpoff[is] = 0; // start with NO previous line offsets
   g_dwFinds = 0; // FIX20110416 - start with NO FINDS for this file
   g_dwItems = 0; // FIX20110616 - and number of items to find
   Traverse_List(phf,pn) {
       g_dwItems++; // FIX20110616 - and number of items to find
   }
   for( dwi = 0; dwi < dwfs; dwi++ ) {
      c = lpc[dwi];
      if(( c == '\r' ) || ( c == '\n' )) {
         // got end of line - we have the LINE
         if( (dwi - bgnoff) >= g_dwMinLen ) {
            if( Process_Line_For_Finds( pWS, bgnoff, dwi ) ) {
               // we appear to have FOUND all FINDS in this line
		         ShowLine( pWS, bgnoff, dwi, &lpoff[0], &dwl );
#ifdef ADD_F2_ALL1LINE // FIX20110416 - When -F:2 - all finds in 1 line, bump FIND counter
                if( g_bAllInOne && ( g_dwFCnt > 1 )) {
                    g_dwFinds++; // FIX20110416 - NOT directly g_dwTotFinds++;
                }
#endif // ADD_F2_ALL1LINE
            }
         }
         bgnoff = dwi;
         if( c == 0x0a ) {
  				IncLine;	// dwl++; yes, but it also backs-up
            // the offsets of previous lines. Sort of HISTORY in lpoff[]
         }
      }
   }
// #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
   if( g_dwFCnt > 1 ) {
      ShowFLines( &g_sLines );
   }

// #endif   // #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
   // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file g_dwFCnt = count on list
   dwi = 0; // no post find output
   if( g_dwFinds ) {
      dwi = 1; // SET post find output
      //if( g_bDoAllin1 && ( g_dwFCnt > 1 ) ) {
      if( g_ioHold ) {
         if( g_bDoAllin1 ) {
            // decide if output allowed
            if( g_dwFndFlg == g_dwFMax ) { // up to 32 bits // = (1 << g_dwItems)
               g_dwMaxFnds++; // bump 'maximum' find ie all finds found in this file
               prtoutlist();
            } else { // *NOT* a max find
               dwi = 0;
            }
         }
      }
   }

   killoutlist();

   g_dwTotFinds += g_dwFinds;
   //if(dwi)
   //   ShowALLFinds( pWS, g_dwFinds, g_dwItems, lpd );
   //else
   ShowFinds( pWS, g_dwFinds, g_dwItems, lpd ); // only if VERB5

   if( dwi ) {
      // OutGFName( pWS );
      Add2gsFileList( pWS ); // keep files with desired finds ...
   }

   UNREFERENCED_PARAMETER(lpInF);
}

/* This function applies a compiled re to a subject string and picks out
portions of the string if it matches. Two elements in the vector are set for
each substring: the offsets to the start and end of the substring.

Arguments:
  argument_re     points to the compiled expression
  extra_data      points to extra data or is NULL
  subject         points to the subject string
  length          length of subject string (may contain binary zeros)
  start_offset    where to start in the subject string
  options         option bits
  offsets         points to a vector of ints to be filled in with offsets
  offsetcount     the number of elements in the vector

Returns:          > 0 => success; value is the number of elements filled in
                  = 0 => success, but offsets is not big enough
                   -1 => failed to match
                 < -1 => some kind of unexpected problem
PCRE_DATA_SCOPE int
pcre_exec(const pcre *argument_re, const pcre_extra *extra_data,
  PCRE_SPTR subject, int length, int start_offset, int options, int *offsets,
  int offsetcount)
*/

#ifdef   EXTRA_DEBUG
// Show_Line( &lpc[dwLnBgn], dwi - dwLnBgn );
void Show_Line( char * ps, uint32_t len )
{
   char * pout = g_pTmpBuf;
   uint32_t dwi, out, cnt, bz, inz;
   char   c, d;
   out = 0;
   cnt = 0;
   bz = 0;
   inz = 0;
   d = ~(ps[0]);
   for ( dwi = 0; dwi < len; dwi++ )
   {
      c = ps[dwi];
      if ( c == d )
      {
         cnt++;
         if ( !inz )
         {
            bz = out; // start of zeros
            inz = 1;
         }
      } else {
         if ( cnt > 2 )
         {
            // bz += 2;
            out = bz;
            pout[out] = 0;
            sprintf(EndBuf(pout), "{%x-%u}", (d & 0xff), cnt);
            out = (uint32_t)strlen(pout);
         }
         cnt = 0;
         inz = 0;
      }
      d = c;
      if ( c & 0x80 )
      {
         pout[out++] = '@';
         c &= ~(0x80);
      }
      if ( c < ' ' )
      {
         pout[out++] = '^';
         c += '@';
      }
      pout[out++] = c;
      if( out > 1000 )
      {
         if ( inz && (cnt > 2) )
         {
            out = bz;
            pout[out] = 0;
            sprintf(EndBuf(pout), "{%x-%u}", (d & 0xff), cnt);
            out = (uint32_t)strlen(pout);
         }
         pout[out] = 0;
         sprtf( "%s"MEOR, pout );
         out = 0;
         d = ~(c);
         cnt = 0;
         inz = 0;
      }
   }
   if (out) {
      if ( inz && (cnt > 2) )
      {
         out = bz;
         pout[out] = 0;
         sprintf(EndBuf(pout), "{%x-%u}", (d & 0xff), cnt);
         out = (uint32_t)strlen(pout);
      }
      pout[out] = 0;
      sprtf( "%s"MEOR, pout );
   }

}
#endif   // #ifdef   EXTRA_DEBUG


// FIX20010328 Fix for UNIX file searching
// if( d == 0x0d )
#define GO_TO_END_LINE dwi++;       \
    for( ; dwi < dwfs; dwi++ ) {    \
        d = lpc[dwi];               \
        if( ( d == 0x0a ) || ( d == 0x0d ) ) break; \
    }

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Find_In_Gen
// Return type: void 
// Arguments  : WS
//            : char * lpInF
// Description: This is where it ALL happens. The file has been mapped
//              and we process the buffer char by char looking for the
// first find char.
///////////////////////////////////////////////////////////////////////////////
void	Find_In_Gen( WS, char * lpInF )
{
	uint32_t	   dwi, dwfs, dwl;
	char *pFind;            // pointer to string to FIND
   char *lpc, *lpd;
	uint32_t    flen, is;
	char	   firstchar, d, prevchar;
//	uint32_t	   dwFinds, dwFind1, dwItems;
//	uint32_t	   lpoff[MXLPS+2];      // keep a set of PREVIOUS line offsets
//	int	fPar;
	int	   fBin;
//	uint32_t	   dwLastLn, dwLnBgn;
   int     bIgC = g_bIgComm;    // extract the FLAG
   PLE      phf = &g_sFind;        // get list of FIND strings
   PLE      pn;
   PFLINE   pl;
   int      doregex = 0;

	g_dwFinds = 0;
	dwFind1 = 0;
	g_dwItems = 0;
	// Got file MEMORY MAPPED
	// ======================
	if( ( g_FileType & FT_BIN ) || ( g_fBinary ) )
		fBin = TRUE;
	else
		fBin = FALSE;
//	fPar = gfParity;
	dwfs = gdwActLen;
	W.ws_dwTotal += dwfs;

	if( VERB6 )
      ShowFNS( pWS );

	gfDoneFile = FALSE;		// reset DONE FILE
   g_iDoneList = FALSE;     // reset done list
   g_dwFileNum++;          // bump the file number

// ADDCVSDATE
   if( g_bCVSDate ) {
      PLE ph1 = &g_sCVSLines;
      KillLList(ph1);
   }

	//pFind = glpFindMem;	// get the FIND strings memory buffer
	//lpd = &gszFindStgs[0];  // = W.ws_szFindStgs
	lpd = &g_szCurrFind[0];  // = GW.ws_szCurrFind

   // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file g_dwFCnt = count on list
   g_dwFndFlg = 0; // up to 32 bits

	//while( ( flen = strlen( pFind ) ) > 0 )  // while there are FIND string(s)
   Traverse_List( phf, pn ) // for EACH of the FIND strings, search this FILE
   {
      g_dwThisFnd = (1 << g_dwItems);
		// count of items to search for
      g_dwItems++;
      dwFind1 = 0;
      doregex = 0;
      //pFind  = (char *)pn;
      //pFind += sizeof(LIST_ENTRY);
      pl = (PFLINE)pn;
      g_psActFind = pl; // set current active FIND structure
      pFind = &pl->cLine[0];
      flen = (uint32_t)strlen(pFind);
		Move2Find( lpd, pFind );
		W.ws_lpCurrFind = pFind;   // original string
		W.ws_dwCurrFLen = flen;    // length of FIND string
		W.ws_lpCurFind  = lpd;     // copy of FIND string (with CASE fixed)
		g_dwCurFLen  = (uint32_t)strlen( lpd );
#ifdef   ADD_REGEX
      if( pl->isregex ) {
         if( !pl->pre ) {
            char * error = 0;
            int errptr = 0;
            // pcre_compile(spb, options, &error, &errptr, pcretables);
            if(VERB5) {
               sprintf( lpVerb, "NOTE: Compiling regex[%s]"MEOR, pFind);
               prt(lpVerb);
            }
            pl->pre = pcre_compile( pFind, gi_pcre_opions, &error, &errptr, gp_pcre_tables );
            if( pl->pre ) {
//#ifndef  NDEBUG
               if(VERB9) {
                  pcre_printint(pl->pre, stdout);
               }
//#endif   // !NDEBUG
            } else {
               sprintf( glpError, "ERROR: Failed COMPILE of regex [%s]!"PRTTERM,
                  pFind );
               Err_Exit( -1 );
            }
            doregex = 1;
         }
      }
#endif   // ADD_REGEX
		firstchar = *lpd;	// Get first character from COPY of find string
		ShowFinding( pWS, g_dwItems, pl );
		lpc = (char *)glpMapView;
		dwl = 0;
		gdwLnInFile = 0;
		prevchar = 0;

		for( is = 0; is < MXLPS; is++ )
			lpoff[is] = 0; // start with NO previous line offsets

		// Process the file
      // ******************************
		dwLnBgn = 0;
      // FIX20010413 -I+ = Skip C/C++ comments in g_bIgComm
      if( bIgC ) {
         Find_Skip( pWS, dwfs, lpc, firstchar, flen, lpd, fBin );
      } else if (g_bIgPComm) {
          // FIX20140926: Add new switch -IP to inhibit finds in Perl comments
          Find_Skip_Perl( pWS, dwfs, lpc, firstchar, flen, lpd, fBin );
      } else if( doregex ) {
         // process the buffer, line by line ...
         int   res;
         dwi = 0;
         while( dwi < dwfs ) 
         {
  				dwLnBgn  = dwi;
      		for( ; dwi < dwfs; dwi++ )
      		{
               d = lpc[dwi];
               if(( d == 0x0d )||( d == 0x0a )) {
                  if( d == 0x0a ) {
         				if(( gfDirLst ) && (( dwi - dwLnBgn) > 2 ) ) {
   	      				DoLineChk( pWS, lpc, dwLnBgn, dwi );
   			      	}
         				IncLine;	// dwl++; yes, but it also backs-up
                  }
                  if( dwi > dwLnBgn ) {
                     gi_pcre_offsets[0] = 0;
                     gi_pcre_offsets[1] = 0;
#ifdef   EXTRA_DEBUG
                     if(VERB9) Show_Line( &lpc[dwLnBgn], dwi - dwLnBgn );
#endif   // #ifdef   EXTRA_DEBUG
                     res = pcre_exec( pl->pre, 0, &lpc[dwLnBgn], dwi - dwLnBgn, 0,
                        (gi_pcre_opions & THE_PUBLIC_EXEC_OPTIONS),
                        &gi_pcre_offsets[0], PCRE_MAX_OFFSETS );
                     if( res >= 0 ) {
                        // success with this line
                        // output a line, passing offsets to previous lines as well
                        if(VERB5) {
                           if(( gi_pcre_offsets[0] >= 0 ) &&
                              ( gi_pcre_offsets[1] > gi_pcre_offsets[0] )) {
                              char * ptmp = GetNxtBuf();
                              int fndlen = gi_pcre_offsets[1] - gi_pcre_offsets[0];
                              char * pfnd = &lpc[dwLnBgn] + gi_pcre_offsets[0];
                              int   fndcnt = 1;
                              int   k;
                              sprintf(ptmp, "Offsets %d", gi_pcre_offsets[0]);
                              while(fndcnt < PCRE_MAX_OFFSETS) {
                                 if(gi_pcre_offsets[fndcnt] == 0)
                                    break;
                                 sprintf(EndBuf(ptmp), " %d", gi_pcre_offsets[fndcnt]);
                                 fndcnt++;
                              }
                              sprintf(EndBuf(ptmp), " (%d)", fndcnt);
                              if(VERB9) sprtf("v9: %s"MEOR, ptmp);
                              for(k = 0; k < PCRE_MAX_OFFSETS; k++) {
                                 ptmp = GetNxtBuf();
                                 if(gi_pcre_offsets[k+1] == 0 && gi_pcre_offsets[k] == 0)
                                    break;
                                 fndlen = gi_pcre_offsets[k+1] - gi_pcre_offsets[k];
                                 if( fndlen > 0 ) {
                                    pfnd = &lpc[dwLnBgn] + gi_pcre_offsets[k];
                                    strncpy(ptmp, pfnd, fndlen);
                                    ptmp[fndlen] = 0;
                                    if( VERB9 || (k > 1) )
                                       sprtf( "%d Found [%s]..."MEOR, (k+1), ptmp );
                                 }
                              }
                           }
                        }
                        g_dwFndFlg |= g_dwThisFnd; // up to 32 bits // = (1 << g_dwItems);
   					      ShowLine( pWS, dwLnBgn, dwfs, &lpoff[0], &dwl );
   					      g_dwFinds++;
   					      dwFind1++;
                     } else {
                        // failed with this line
                     }
                  }
                  dwi++;
            		for( ; dwi < dwfs; dwi++ )
            		{
                     d = lpc[dwi];
                     if( !(( d == 0x0d )||( d == 0x0a )) ) {
                        break; // exit inner loop
                     }
                     if( d == 0x0a ) {
         				   if(( gfDirLst ) && (( dwi - dwLnBgn) > 2 ) ) {
   	      				   DoLineChk( pWS, lpc, dwLnBgn, dwi );
   			      	   }
            				IncLine;	// dwl++; yes, but it also backs-up
                     }
                  }
                  break; // exit outter loop
               }
            }
         } // while dwi <= dwf
      } else { // !bIgC
         // simple scan through the buffer
   		for( dwi = 0; dwi < dwfs; dwi++ )
   		{
   			d = (*GetChr) ( &lpc[dwi] );  // call indirectly to GetChr (W.GETCHR) function
   			if( prevchar == 0x0a )
   			{
   				dwLastLn = dwLnBgn;
   				if(( gfDirLst ) && (( dwi - dwLnBgn) > 2 ) ) {
   					DoLineChk( pWS, lpc, dwLnBgn, dwi );
   				}
   				dwLnBgn  = dwi;
   			}
   
            // if we match with character one
   			if( firstchar == d )
   			{
   				// if GOT the first char, do a FULL COMPARE
   				// if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen, &dwl ) )
   				if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen ) )
   				{
                  // output a line, passing offsets to previous lines as well
                  g_dwFndFlg |= g_dwThisFnd; // up to 32 bits // = (1 << g_dwItems);
   					ShowLine( pWS, dwi, dwfs, &lpoff[0], &dwl );
   					if( fBin ) { // if BINARY, just bump past the FOUND
   						dwi += g_dwCurFLen;
   					} else { // if TEXT, move to END OF LINE
                        GO_TO_END_LINE;
   					}
   					g_dwFinds++;
   					dwFind1++;
   				} else if ( g_fBeginLine && !fBin && ( !( (d == 0x0a) || (d == 0x0d) ) ) ) {
                    GO_TO_END_LINE;
                }
            } else if ( g_fBeginLine && !fBin && ( !( (d == 0x0a) || (d == 0x0d) ) ) ) {
                GO_TO_END_LINE;
            }

            // FIX20010328 Fix for UNIX file searching
   			//if( d == 0x0d )
   			if( d == 0x0a )
   			{
               // =========================================================
   				IncLine;	// dwl++; yes, but it also backs-up
               // the offsets of previous lines. Sort of HISTORY in lpoff[]
   			}
   			prevchar = d;
   		} // for( dwi = 0; dwi < dwfs; dwi++ ) loop through the buffer
      }  // ignore comment or not
      // ******************************

		if( VERBM ) ShowFind1( pWS, dwFind1, pFind, doregex );
		//pFind += (flen + 1); // up to NEXT find string, if more than one

		// Processing MULTIPLE finds with
   } // Traverse_List( phf, pn ) // for EACH of the FIND strings, search this FILE
   // was while( flen = strlen( pFind ) )

// ADDCVSDATE
   if( g_bCVSDate )  // ( !VERB4     ) )
   {
      PLE      ph = &g_sCVSLines;
      char *   lpt = &g_szTmpBuf[0];
      uint32_t    dwc;
      if( IsListEmpty(ph) ) {
         if( VERB5 ) {
            sprintf(lpt, "No valid entries in %s!", lpInF );
            prt(lpt);
         }
      } else {
         PLE pn = ph->Flink;
         PCVSLN pcvs = (PCVSLN)pn;
         PLE   pn2;
         PCVSLN pcvs2;
         //LONG     lg;
         ListCount2(ph, &dwc);   // get COUNT in this LIST
         RemoveEntryList(pn);    // extract this entry from the list

         ph = &g_sEntries;       // get the ENTRIES list
         Traverse_List(ph, pn2)  // traverse it
         {
            pcvs2 = (PCVSLN)pn2; // re-cast pointer
            //lg = CompareFileTime(
            //         &pcvs->sFT, // first file time
            //         &pcvs2->sFT ); // second file time
                  // Value Meaning 
                  // -1 First file time is less than second file time.
                  // 0  First file time is equal to second file time. 
                  // 1  First file time is greater than second file time.
            //if( lg > 0 ) {
            if (pcvs->uFT < pcvs2->uFT) {
               InsertBefore(pn2,pn);
               pn = 0;
               break;
            }
         }
         if(pn) {
            InsertTailList(ph,pn);
         }

         if( VERB4 ) {
            if( !gfDoneFile ) {
               PLE   pf = pcvs->psName;
               if(pf) {
                  char *   _lpf = (char *)pf;
                  _lpf += sizeof(LIST_ENTRY);
                  strcpy(lpt, _lpf);
                  strcat(lpt,MEOR);
                  prt(lpt);
               }
      			gfDoneFile = TRUE;
            }
            strcpy(lpt, &pcvs->szFile[0]);
            strcat(lpt, " ");
            //AppendDateTime( lpt, &pcvs->sSysTm );
            AppendDateTime( lpt, &pcvs->uFT );
            sprintf(EndBuf(lpt), " (LATEST of %d entries)", dwc );
            strcat(lpt, MEOR);
            prt(lpt);
         }
      }
   }

// #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
   if( g_dwFCnt > 1 ) {
      ShowFLines( &g_sLines );
   }

// #endif   // #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
   // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file g_dwFCnt = count on list
   dwi = 0; // no post find output
   if( g_dwFinds ) {
      dwi = 1; // SET post find output
      //if( g_bDoAllin1 && ( g_dwFCnt > 1 ) ) {
      if( g_ioHold ) {
         if( g_bDoAllin1 ) {
            // decide if output allowed
            if( g_dwFndFlg == g_dwFMax ) { // up to 32 bits // = (1 << g_dwItems)
               g_dwMaxFnds++; // bump 'maximum' find ie all finds found in this file
               prtoutlist();
            } else { // *NOT* a max find
               dwi = 0;
            }
         }
      }
   }

   killoutlist();

   g_dwTotFinds += g_dwFinds;
   //if(dwi)
   //   ShowALLFinds( pWS, g_dwFinds, g_dwItems, lpd );
   //else
   ShowFinds( pWS, g_dwFinds, g_dwItems, lpd ); // only if VERB5

   if( dwi ) {
      // OutGFName( pWS );
      Add2gsFileList( pWS ); // keep files with desired finds ...
   }

   UNREFERENCED_PARAMETER(lpInF);

}

// *******************************************
void Find_Skip( WS, uint32_t dwfs, char * lpc, char c, uint32_t flen, char * lpd, int fBin )
{
   uint32_t dwi, is, dwl;
   char  d, pd;
   pd = 0;
   dwl = 0;
   		for( dwi = 0; dwi < dwfs; dwi++ )
   		{
   			d = (*GetChr) ( &lpc[dwi] );  // call indirectly to GetChr (W.GETCHR) function
            if( (  d == '*' ) &&
                ( pd == '/' ) )
            {
               // entered a C/C++ comment field - go until the END
               // began with /* and end with */
               dwi++;   // bump to NEXT
               for( ; dwi < dwfs; dwi++ )
               {
                  d = lpc[dwi];  // extract a char
                  if( ( d == '/' ) &&
                     ( pd == '*' ) )
                  {
                     d = 0;   // make sure NO first match below
                     break;   // few, end of comment
                  }
         			if( d == 0x0a )
         			{
         				IncLine;	// dwl++;
         			}
                  pd = d;
               }
            }
            else if( ( d == '/' ) &&
               ( pd == '/' ) )
            {
               // entered a single line comment field - go to end of line
					dwi++;
					for( ; dwi < dwfs; dwi++ )
					{
						d = lpc[dwi];
                  // FIX20010328 Fix for UNIX file searching
						if( ( d == 0x0a ) || ( d == 0x0d ) )
                     break;
					}
            }
            else if( pd == 0x0a )
   			{
   				dwLastLn = dwLnBgn;
   				if(( gfDirLst ) && (( dwi - dwLnBgn ) > 2) ) {
                  // we have a -D - search a DIRECTORY listing
   					DoLineChk( pWS, lpc, dwLnBgn, dwi );
   				}
   				dwLnBgn  = dwi;
   			}
   
            // if we match with character one
            // ******************************
   			if( c == d )
   			{
   				// if GOT the first char, do a FULL COMPARE
   				// if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen, &dwl ) )
   				if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen ) )
   				{
                  // output a line, passing offsets to previous lines as well
   					ShowLine( pWS, dwi, dwfs, &lpoff[0], &dwl );
   					if( fBin ) {
   						// if BINARY, just bump past the FOUND
   						dwi += g_dwCurFLen;
   					} else {
   						// if TEXT, move to END OF LINE
   						dwi++;
   						for( ; dwi < dwfs; dwi++ )
   						{
   							d = lpc[dwi];
                        // FIX20010328 Fix for UNIX file searching
   							//if( d == 0x0d )
   							if( ( d == 0x0a ) || ( d == 0x0d ) )
                           break;
   						}
   					}
   					g_dwFinds++;
   					dwFind1++;
   				}
   			}
            // FIX20010328 Fix for UNIX file searching
   			//if( d == 0x0d )
   			if( d == 0x0a )
   			{
   				IncLine;	// dwl++;
   			}
   			pd = d;
   		}


}

// -IP = Skip Perl comments
// FIX20140926: Add new switch -IP to inhibit finds in Perl comments
void Find_Skip_Perl( WS, uint32_t dwfs, char * lpc, char c, uint32_t flen, char * lpd, int fBin )
{
    uint32_t dwi, is, dwl;
    int inQuot = 0;
    char  d, pd, qc;
    pd = 0;
    dwl = 0;
    qc = 0;
    for( dwi = 0; dwi < dwfs; dwi++ ) {
        d = (*GetChr) ( &lpc[dwi] );  // call indirectly to GetChr (W.GETCHR) function
        if (( d == '#' ) && !(pd == '\\') && !inQuot) {
            // entered a single line comment field - go to end of line
            dwi++;
            for( ; dwi < dwfs; dwi++ ) {
                d = lpc[dwi];
                // FIX20010328 Fix for UNIX file searching
                if( ( d == 0x0a ) || ( d == 0x0d ) )
                    break;
            }
        } else if ((d == '"')||(d == '\'')) {
            if (inQuot) {
                if (d == qc) {
                    inQuot = 0; // end of quoted text
                }
            } else {
                qc = d;
                inQuot = 1;
            }
        } else if( pd == 0x0a ) {
            dwLastLn = dwLnBgn;
            dwLnBgn  = dwi;
        }

        // if we match with character one
        // ******************************
        if( c == d ) {
            // if GOT the first char, do a FULL COMPARE
   			// if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen, &dwl ) )
            if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen ) ) {
                // output a line, passing offsets to previous lines as well
                ShowLine( pWS, dwi, dwfs, &lpoff[0], &dwl );
                if( fBin ) {
                    // if BINARY, just bump past the FOUND
                    dwi += g_dwCurFLen;
                } else {
                    // if TEXT, move to END OF LINE
                    dwi++;
                    for( ; dwi < dwfs; dwi++ ) {
                        d = lpc[dwi];
                        // FIX20010328 Fix for UNIX file searching
                        if( ( d == 0x0a ) || ( d == 0x0d ) )
                            break;
                    }
                }
                g_dwFinds++;
                dwFind1++;
            }
        }
        // FIX20010328 Fix for UNIX file searching
        if( d == 0x0a ) {
            IncLine;	// dwl++;
            inQuot = 0; // assume not in quotes any more
        }
        pd = d;
    }
}

// *******************************************
// eof - Fa4Find.c
