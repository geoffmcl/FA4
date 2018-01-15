/*\
 * Fa4Wild.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * handle wild card file name input ...
 * Scan a directory, and do finds in files matching the input mask
 *
\*/
#include "Fa4.h"

static const char *module = "Fa4Wild";

uint32_t g_dwFoundFileCnt = 0;
uint32_t g_dwFoundDirsCnt = 0;
uint64_t g_ulTotalBytes = 0;
uint64_t g_ulTotalBRej  = 0;
uint32_t g_dwFoundRejCnt = 0;
#define g_dwFoundMatchCnt ( g_dwFoundFileCnt - g_dwFoundRejCnt ) // difference

#ifdef   ADD_REGEX
static char wildpath[264];
static char wildname[264];
static char wildregex[264];
static int got_wild_fn = 0;
static char temppath[264];
int   char_is_special( int c )
{
   switch(c)
   {
   case '+':
   case '?':
   case '.':
   case '*':
   case '^':
   case '$':
   case '(':
   case ')':
   case '[':
   case ']':
   case '{':
   case '}':
   case '|':
   case '\\':
      return 1;
   }
   return 0;
}

// FIX20070120 - special ENDING '.*' case - quite frequent
void wildtoregex( char * filename )
{
   size_t len = strlen(filename);
   size_t in = 0;
   size_t i;
   int   c, d = 0;
   for( i = 0; i < len; i++ )
   {
      c = filename[i];
      if( c == '*' ) {
         wildregex[in++] = '.';  // find any character
         wildregex[in++] = '*';  // zero, or more times
      } else if( c == '?' ) {
         wildregex[in++] = '.';  // find any character
         wildregex[in++] = '{';
         wildregex[in++] = '1';  // exactly ONE time
         wildregex[in++] = '}';
      } else if( char_is_special(c) ) {
         if(( c == '.' ) && ((len - i) == 2) && (filename[i+1] == '*')) {
            // FIX20070120 - have a dot - '.*'
            wildregex[in] = 0;
            strcat(wildregex, "((\\..*)|$)");
            in = strlen(wildregex);
            i++;
         } else {
            wildregex[in++] = '\\'; // escape
            wildregex[in++] = (char)c;  // the dot, or ...
         }
      } else {
         if(in == 0) {  // if FIRST char is NORMAL char
            wildregex[in++] = '^'; // then FILENAME regex MUST start with this char
         }
         wildregex[in++] = filename[i];
         if((i + 1) >= len) { // if LAST char is NORMAL
            wildregex[in++] = '$';  // then FILENAME regex MUST end with this char
         }
      }
      d = c;
   }
   wildregex[in] = 0;
   if(in)
      got_wild_fn = 1; // set we have a WILD filename
}

void setdospath( char * pathname )
{
   size_t len = strlen(pathname);
   size_t i;
   for( i = 0; i < len; i++ )
   {
      if(pathname[i] == '/') {
         pathname[i] = '\\'; // in setdospath(), so for sure use DOS path sep
      }
   }
}

int regex_gotwildchar( char * filename )
{
   int   iswild = 0;
   size_t len = strlen(filename);
   size_t i;
   for( i = 0; i < len; i++ ) {
      int   c = filename[i];
      if(( c == '?' ) || ( c == '*' )) {
         iswild = 1;
         break;
      }
   }
   return iswild;
}

int regex_iswild( char * filename )
{
   int   iswild = regex_gotwildchar(filename);
   got_wild_fn = 0;
   if(iswild) {
      char * r;
      strcpy(temppath,filename);
      setdospath(temppath);
      r = strrchr( temppath, '\\' );    // done after sedospath() so only need search for '\' here
      if(r) {
         *r = 0;
         strcpy(wildpath, temppath);
         r++;
         strcpy(wildname, r);
      } else {
         strcpy(wildpath,".");
         strcpy(wildname, filename);
      }
      iswild = regex_gotwildchar(wildname);
      if(iswild) {
         wildtoregex( wildname );   // CONVERT TO REGEX
      }
   }
   return iswild;
}
#endif   // #ifdef   ADD_REGEX

static int _s_iOnly_Once = 0;
// FIX20120516 - Accept file name MATCHED if REGEX says YES - NEW BEHAVIOUR!!!
int	MatchFiles2( char * lp1, char * lp2 )
{
   int  bRet = MatchFiles( lp1, lp2 );

#if (defined(ADD_REGEX) && defined(ADD_REGEX_FILE_MATCH))
//////////////////////////////////////////////////////////////////////////////////////////////////////
// 20140831: This has served its purpose to test my MatchFiles(f1,f2) service - can now be turned off
   if( regex_iswild( lp1 ) ) {
      char * error = 0;
      int errptr = 0;
      int res;
      pcre * pre;
      if(GVERB9) sprtf( "%s: v9: Compiling regex [%s] to check [%s] ...\n", module, wildregex, lp2 );
      pre = pcre_compile(wildregex, PCRE_CASELESS, &error, &errptr, gp_pcre_tables );
      if(pre) {
         res = pcre_exec( pre, 0, lp2, strlen(lp2), 0, 0, &gi_pcre_offsets[0], PCRE_MAX_OFFSETS );
         if(GVERB9) {
            sprtf( "%s: v9: pcre_exec returned %d (%s)...(MatchFiles=%s)\n", module, res,
               (res >= 0) ? "Ok" : "No",
               (bRet ? "Ok" : "No") );
         }
         if( res >= 0 ) {
            if( !bRet ) {
               // have NOT got the regex right for this ... so
               if(( strcmpi( lp1, "*.c*" ) == 0 )||
                  ( strcmpi( lp1, "*.h*" ) == 0 ) ) {
                  char * p = strrchr( lp2, '.' );
                  if(p) {
                     res = pcre_exec( pre, 0, p, strlen(p), 0, 0, &gi_pcre_offsets[0], PCRE_MAX_OFFSETS );
                     if( res < 0 )
                        goto Free_PRE;
                  }
               }
               if (_s_iOnly_Once == 0) {
                   sprtf( "%s: WARNING: Was NO, but REGEXE is YES [%s] [%s] CHECK [%s] [%s]... accepting REGEX!"MEOR, module,
                      lp1, lp2, wildname, wildregex );
               }
               bRet = TRUE; // FIX20120516 - Accept file name MATCHED if REGEX says YES - NEW BEHAVIOUR!!!
               _s_iOnly_Once++;
            }
         } else {
            if( bRet ) {
               sprtf( "%s: WARNING: previous is YES, REGEXE is NO [%s] [%s] CHECK wild %s [%s]? ..."MEOR, module,
                  lp1, lp2, wildname, wildregex );
            }
         }
Free_PRE:
         pcre_free(pre);
      }
   }
//////////////////////////////////////////////////////////////////////////////////////////////////////
#endif   // #if (defined(ADD_REGEX) && defined(ADD_REGEX_FILE_MATCH))

   return bRet;
}

//#define CHKMEM(a) if( !a ) { prt("C:ERROR: MEMORY FAILED!"MEOR ); exit(-1); }
//uint32_t g_dwFoundFileCnt = 0;
//uint32_t g_dwFoundDirsCnt = 0;
//uint32_t g_dwFoundRejCnt = 0;
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	void	Process_Wilds( WS, char * lpwild )
//
//	PURPOSE:
//		Process a WILD CARD item (off the list)
//
//  20140728 - Switched over to using DIR/dirent portable service
//
//  FIX20140830: BUG - Directories are repeated???
//  1: Try really beefing up skipping '.' and '..' - NOPE, still get REPEATS
//  Much as I hate it, maybe keep a list of directories processed, to at least flag a repeat
//  AH HA: Simple! Process_Recursive( WS, char * lpwild ) is doing the SAME as Process_Wilds?????????
//  Maybe just cancel the Process_Recursive() call in DoThisFile( WS, char * lpf, int bFlg )?
//  OK, that fixed the problem... with command
//  "opendir" -r -xChangeLog -xChangeLog.1 -x:doc -x:po -c -w F:\Projects\tar-1.28\* > tempout.txt 
//  But what about the case -r F:\Projects\tar-1.28\Makefile.am - want to again search all directories
//  Ok, that failed... Maybe this Process_Wild() should be used whether wild or NOT
//  Now back to ok with test "quicktest" -r F:\Projects\tar-1.28\Makefile.am > tempout.txt 
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//  FIX20140830: BUG - Directories are repeated???
#ifdef ADD_DIRS_DONE_DBG
static LIST_ENTRY s_dir_list;
static int done_init = 0;

int is_a_repeat_dir( char *dir )
{
    PLE ph = &s_dir_list;
    PLE pn;
    PMWL pwl;
    if (!done_init) {
        InitLList(ph);
        done_init = 1;
    }
    Traverse_List(ph,pn) {
        pwl = (PMWL)pn;
        if (strcmp(dir,pwl->wl_cName) == 0) {
            return 1;
        }
    }
    pwl = (PMWL)MALLOC(LPTR, sizeof(MWL));
    if (pwl) {
        strcpy( &pwl->wl_cName[0], dir );
        InsertTailList( &s_dir_list, (PLE)pwl );
    }
    return 0;
}
#endif // #ifdef ADD_DIRS_DONE_DBG

void Process_Wilds( WS, char *lpwild ) // FIX20140830: Slight miss naming - now does ALL files/dirs
{
    char *pdn, *pfn, *lpfil;
    uint64_t ul1;
    DIR *dp;
    // LOCAL STATS - just for this directory
    uint32_t dircount = 0;
    uint32_t filcount = 0;
    if( !gfRecursive && IsValidFile(lpwild) ) {
        // NOT recursive, and found just the single file
        ul1 = get_last_file_size64();
        strcpy(glpActive,lpwild);   // set as active
        // FIX20180115 - restore -v3 to show date and size of file output
        gActSize = ul1;             // set act SIZE
        gActDate = get_last_file_mtime64(); // set act DATE

        g_dwFoundFileCnt++;
        if( VERB9 ) {
            sprintf( lpVerb, "%s: v9: Checking %s ..."PRTTERM, module, lpwild );
            prt( lpVerb );
        }
        g_ulTotalBytes += ul1;
        gfDoneFile = FALSE;		// reset DONE FILE name
        // Process a FILE, after MAPPING, for the FIND STRING(s)
	    Find_In_File( pWS );
        return; // found and done a SINGLE file
    }

 	pfn = (char *)MALLOC( LPTR, (2*(MAX_PATH+32)) ); // FIX20050212 - fix -r switch
    CHKMEM(pfn);
    pdn = &pfn[(MAX_PATH+32)];
    *pfn = 0;
    *pdn = 0;
	SplitFN( pdn, pfn, lpwild );
    if (*pdn == 0) {
        strcpy(pdn,"." PATH_SEP);
    }

#ifdef ADD_DIRS_DONE_DBG
    if (is_a_repeat_dir(pdn)) {
       sprintf( lpVerb, "%s: A repeated directory '%s', while seeking matching [%s]"MEOR, module, pdn, pfn );
        prt(lpVerb);
        return;
    }
#endif // #ifdef ADD_DIRS_DONE_DBG

	lpfil = glpActive;	// Get the BUFFER for the ACTIVE file/dir name
    if( VERB9 ) {
       sprintf( lpVerb, "\n%s: v9: Opening dir %s, seek matching [%s]"MEOR, module, pdn, pfn );
        prt(lpVerb);
    }
    dp = opendir(pdn);
    if(dp) { 
        struct dirent *d = readdir(dp);
        while (d) {  // got a FIND FIRST
            //  FIX20140830: Make really SURE '.' and '..' never get through!
            char * lpn = d->d_name;
            if (lpn[0] == '.') {
                if (lpn[1] == 0) {
                    d = readdir(dp);
                    continue;
                } else if (lpn[1] == '.') {
                    if (lpn[2] == 0) {
                        d = readdir(dp);
                        continue;
                    }
                }
            }
            strcpy( lpfil, pdn );
            strcat( lpfil, d->d_name );
            if (IsValidFile(lpfil)) {
                ul1 = get_last_file_size64();
                filcount++;
                g_dwFoundFileCnt++;
                if( VERB9 ) {
                    sprintf( lpVerb, "%s: v9: Checking %s ..."PRTTERM, module, d->d_name );
                    prt( lpVerb );
                }
                if( MatchFiles2( pfn, d->d_name ) ) {
                    // defined g_dwFoundMatchCnt = (total - unmatched)
                    g_ulTotalBytes += ul1;
                    // FIX20180115 - restore -v3 to show date and size of file output
                    gActSize = ul1;             // set act SIZE
                    gActDate = get_last_file_mtime64(); // set act DATE

                    gfDoneFile = FALSE;		// reset DONE FILE name
                    // Process a FILE, after MAPPING, for the FIND STRING(s)
				    Find_In_File( pWS );
                } else {
                    g_dwFoundRejCnt++;
                    g_ulTotalBRej += ul1;
                    if( VERB5 ) {
                        sprintf( lpVerb, 
                           "%s: v5: %d REJECT '%s' by MatchFiles to '%s'"PRTTERM, module,
                            g_dwFoundRejCnt,
                            d->d_name,
                            pfn );
                        prt( lpVerb );
                    }
                }
            } else {
                // is a DIRECTORY - DOT and DOUBLE DOT skipped
                // if( strcmp(lpn,".") && strcmp(lpn,"..") ) {
                if (IsValidDir(lpfil)) {
                    strcat(lpfil,PATH_SEP);
                    strcat(lpfil,pfn);
                    dircount++;
#ifdef	ADDRECUR
                	if( gfRecursive )
	                {
                        g_dwFoundDirsCnt++;
#ifdef USE_EXCLUDE_LIST
                       if( InExcludeD( d->d_name ) ) {
                           g_dwDirsExcl++; // count another match to EXCLUDED
                            if( VERB9 ) {
                                sprintf( lpVerb, "%s: v9: Excluded '%s' ..."PRTTERM, module, d->d_name );
                                prt( lpVerb );
                            }
                       } else {
                           Process_Wilds( pWS, lpfil ); // Process_Wilds calling itself
                       }
#else // !#ifdef USE_EXCLUDE_LIST
                       Process_Wilds( pWS, lprm );  // Process_Wilds calling itself - NO USE_EXCLUDE_LIST
#endif // #ifdef USE_EXCLUDE_LIST y/n
	                }
#endif	// ADDRECUR
                } else {
                    // NOT valid FILE or DIRECTORY - forget it??? - probably a link, or ???
                }
            }
            d = readdir(dp);
        }
        closedir(dp);
        if( VERB9 ) {
           sprintf( lpVerb, "%s: v9: Done dir %s. Found %u file%s, and %u folder%s"MEOR, module,
               pdn,
               filcount,
               ((filcount == 1) ? "" : "s"),
               dircount,
               ((dircount == 1) ? "" : "s") );
            prt(lpVerb);
        }
    } else {
        if( VERB5 ) {
           sprintf( lpVerb, "%s: v5: opendir(%s) FAILED!"PRTTERM, module, pdn );
            prt( lpVerb );
        }
    }
    MFREE(pfn); // toss the MEMORY
}

// eof - Fa4Wild.c
