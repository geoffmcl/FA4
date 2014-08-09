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
      if(pathname[i] == '/')
         pathname[i] = '\\';
   }
}

int gotwildchar( char * filename )
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

int iswild( char * filename )
{
   int   iswild = gotwildchar(filename);
   got_wild_fn = 0;
   if(iswild) {
      char * r;
      strcpy(temppath,filename);
      setdospath(temppath);
      r = strrchr( temppath, '\\' );
      if(r) {
         *r = 0;
         strcpy(wildpath, temppath);
         r++;
         strcpy(wildname, r);
      } else {
         strcpy(wildpath,".");
         strcpy(wildname, filename);
      }
      iswild = gotwildchar(wildname);
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
#ifdef   ADD_REGEX
   if( iswild( lp1 ) ) {
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
#endif   // ADD_REGEX
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
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Process_Wilds( WS, char *lpwild )
{
    char *lpd, *lpf, *lpmask, *lpfil;
    uint64_t ul1;
    DIR *dp;
    uint32_t dircount = 0;
    uint32_t filcount = 0;
	//lpd = &gszFolder[0];
	// lpf = &szDir2[0];
 	lpf = (char *)MALLOC( LPTR, (3*(MAX_PATH+32)) ); // FIX20050212 - fix -r switch
    CHKMEM(lpf);
    lpd = &lpf[(MAX_PATH+32)];
	//lpmask = &gszFileMask[0];
    lpmask = &lpd[(MAX_PATH+32)];

	SplitFN( lpd, lpf, lpwild );
    if (*lpd == 0) {
        strcpy(lpd,"." PATH_SEP);
    }
	strcpy( lpmask, lpd );
	strcat( lpmask, "*.*" );

	lpfil = glpActive;	// Get the BUFFER for the file name
    if( VERB9 ) {
       sprintf( lpVerb, "%s: v9: NOTE: Find using [%s]"MEOR, module, lpmask );
        prt(lpVerb);
    }
    dp = opendir(lpd);
    if(dp) { 
        struct dirent *d = readdir(dp);
        while (d) {  // got a FIND FIRST
            strcpy( lpfil, lpd );
            strcat( lpfil, d->d_name );
            if (IsValidFile(lpfil)) {
                ul1 = get_last_file_size64();
                filcount++;
                g_dwFoundFileCnt++;
                if( VERB9 ) {
                    sprintf( lpVerb, "%s: v9: Checking %s ..."PRTTERM, module, d->d_name );
                    prt( lpVerb );
                }
                if( MatchFiles2( lpf, d->d_name ) ) {
                    // defined g_dwFoundMatchCnt = (total - unmatched)
                    g_ulTotalBytes += ul1;
                    gfDoneFile = FALSE;		// reset DONE FILE name
                    // Process a FILE, after MAPPING, for the FIND STRING(s)
				    Find_In_File( pWS );
                } else {
                    g_dwFoundRejCnt++;
                    g_ulTotalBRej += ul1;
                    if( VERB9 ) {
                        sprintf( lpVerb, 
                           "%s: v9: REJECT %u by MatchFiles %s ..."PRTTERM, module,
                            g_dwFoundRejCnt,
                            d->d_name );
                        prt( lpVerb );
                    }
                }
            } else {
                // is a DIRECTORY - forget DOT and DOUBLE DOT
                char * lpn = d->d_name;
                if( strcmp(lpn,".") && strcmp(lpn,"..") ) {
                    dircount++;
                }
            }
            d = readdir(dp);
        }
        closedir(dp);
        if( VERB9 ) {
           sprintf( lpVerb, "%s: v9: Found %u file%s, and %u folder%s"MEOR, module,
                filcount,
                ((filcount == 1) ? "" : "s"),
                dircount,
                ((dircount == 1) ? "" : "s") );
            prt(lpVerb);
        }
    } else {
        if( VERB9 ) {
           sprintf( lpVerb, "%s: v9: None found %s ..."PRTTERM, module, lpwild );
            prt( lpVerb );
        }
    }
    MFREE(lpf); // toss the MEMORY
}

// eof - Fa4Wild.c
