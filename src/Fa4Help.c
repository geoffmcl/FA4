/*\
 * Fa4Help.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * process the command line...
 * help text output
 *
\*/
#include <sys/types.h>
#include <sys/stat.h>   // for stat function
#include	"Fa4.h"
extern void  add2list( PLE ph, char * lpo );
extern   void	OutHeader( WS );
extern   void  boi( char * lps );
extern   void  outvals( void );
extern   int	OpenReadFile( char * lpf, HANDLE * ph );
extern	void	EnableDiagFile( void );
extern   uint32_t	GetFileLen( HANDLE hf );
extern void	PrtErr( WS, uint32_t Err );
extern void prt_out_tail(void); // extern char  szTail[];
// #define	Err_Exit( a )	Error_Exit( pWS, a )

#define MAX_INPUT_LEN 10000000   // flag error if more than a 10 megabyte of input file???

/* ===========================================
Informatique Rouge *** FAST FIND ALL UTILITY *** V4.0.58 - 2007 Oct 10
Purpose:  To locate specific text in a file or group of files.
Usage:    FA4 Arg1 Arg2 [-Switches]
Where:    Arg1 = "Find Text" or @FindItems.
          Arg2 = [[D:\]Path\]File(s) or @FileList.
          The @Input files are interpreted as a set of line delimited items.
Switches: Each preceeded by '/' or '-' , space separated, case ignored.
 -8          = Ouput file name in DOS format.
 -B[-][n:n] = [-] Ignore binary, else force binary file find. [Back:Forward] (def=40:40)
 -c          = cASE tOGGLE - Be case SenSiTive.
 -D[NLO[DS]] = Directory List. (N=Normal, L=list output, O=order[D=date(def)|S=size])
 -Ennnnn     = Find WIN32 Error or closest.(Try ALL).
 -F="text"   = Find multiple items. (-F:1 = all in one file -F:2 = all in one line)
 -FR=regex   = Find using a pcre (Perl Compatible Regular Expression).
 -I:"text"   = Inhibit finds with this text. (-I+ for C/C++ comments)
 -L[W][nn|F] = Output Additional [Wnn=wrap at (def=90)][nn=lines|F=Complete function].
 -LL[nn[:nn] = Limit line output - default = 80, nn:nn = left:right, just nn = left=right=nn/2
 -M[2]       = Process as a MAKE file. Same as -F:M. 2 expands $(mac).
 -N          = Line numbering on finds.
 -NS         = No stats shown at end.
 -NF         = No file name shown on finds.
 -OOutFile   = Add all output to a file.
 -P          = Ignore Parity for compare. (8th bit stripped).
 -R          = Recursive into sub directories.
 -S          = Strip output lines of all non-alpanumeric character, replaced with a space
 -UInFile    = Use InFile as list of files to search.
 -Vn         = Verbosity level and type. (Def=V2).
 -W          = Only whole words (Includes like :WHOLE_. -W2 for spaces).
 -Xname      = Exclude file from search, or as @InFile or ';' separated list.
 -X:[name|:] = Exclude directory from search. ::=FrontPage, :::=CVS/.svn directories.
Notes:  Errorlevel gives the find count. -IC inhibit finds in C/C++ comments.
Output: V0=None, V1=Files, V2=Finds & Files(def), up to V9=ALL ON.
Note -V3 adds date and time after find file, and -V4 add a list at end.
"\ntext" to find text ONLY at the beginning of the line ...
Compiled using MSVC v.1400 on Oct  6 2007 at 10:15:26, run by Geoff McLane, on DELL01 machine.
                                                    Happy searching...
   =========================================== */

#define	TAILSTG	"                                                    Happy searching...\n"
//#define  SWLEAD   "      "
#define  SWLEAD   " "

void show_version( WS )
{
	OutHeader( pWS );
    prt_out_tail();
	Pgm_Exit( pWS );
}

void	Usage( WS, uint32_t Err )
{
   if(Err) PrtErr( pWS, Err );
	OutHeader( pWS );
	prt( "Purpose:  To locate specific text in a file or group of files."PRTTERM );
	prt( "Usage:    FA4 Arg1 Arg2 [-Switches]"PRTTERM );
	prt( "Where:    Arg1 = \"Find Text\" or @FindItems.txt file."PRTTERM );
	//prt( "          Note 'C' runtime requires \\\" to include \"in command!"PRTTERM );
	prt( "          Arg2 = [[D:\\]Path\\]File(s) or @FileList."PRTTERM );
	prt( "          The @Input files are interpreted as a set of line delimited items."PRTTERM );
	prt( "Switches: Each preceeded by '/' or '-' , space separated, case ignored."PRTTERM );
#ifdef WIN32
#ifdef   FIX20000902 // FIX input files in input file and add -8
	prt( SWLEAD"-8          = Ouput file name in DOS format."PRTTERM );
#endif   // FIX20000902 // FIX input files in input file and add -8
#endif // WIN32
   prt( SWLEAD"-B[-][n:n] = [-] Ignore binary, else force binary file find. [Back:Forward] (def=40:40)"PRTTERM );
   // FIX20070906 - add -b- to ignore binary files
   // prt( SWLEAD"-B[n:n]     = Binary file search. [Back:Forward] (def=40:40)"PRTTERM );
#ifdef   ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
	prt( SWLEAD"-c[nn]      = cASE tOGGLE - Be case SenSiTive. (Cnn=Find Minimum)"PRTTERM );
#else // !ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
	prt( SWLEAD"-c          = cASE tOGGLE - Be case SenSiTive."PRTTERM );
#endif   // ADDFCOUNT y/n   // FIX20010824 - Minimum FIND count before OUTPUT
#ifdef		ADDDIRLST	// Special for Directory Listing
#ifdef   ADD_DIRLIST_SORT2
	prt( SWLEAD"-D[NLO[DS]] = Directory List. (N=Normal, L=list output, O=order[D=date(def)|S=size])"PRTTERM );
#else // !#ifdef   ADD_DIRLIST_SORT2
	prt( SWLEAD"-D[L]       = Directory Listing Search. (L=list output)"PRTTERM );
#endif // #ifdef   ADD_DIRLIST_SORT2 y/n
#endif	// ADDDIRLST
#ifdef		ADDERRLST	// Special for Directory Listing
	prt( SWLEAD"-Ennnnn     = Find WIN32 Error or closest.(Try ALL)."PRTTERM );
#endif	// ADDERRLST
//	prt( SWLEAD"-F\"text\"  = Find text. Can be multiple items. (-F:M for make files)"PRTTERM );
#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   prt( SWLEAD"-F=\"text\"   = Find multiple items. (-F:1 = all in one file -F:2 = all in one line)"PRTTERM );
#ifdef   ADD_REGEX
   prt( SWLEAD"-FR=regex   = Find using a pcre (Perl Compatible Regular Expression)."PRTTERM );
#endif // #ifdef   ADD_REGEX
#else // !#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
	prt( SWLEAD"-F=\"text\"   = Find multiple items. (-F:1 = all in one file)"PRTTERM );
#endif // #ifdef ADD_F2_ALL1LINE y/n // FIX20060621 - Add -F:2 - all finds in 1 line

#ifdef	ADDINHIB
//	prt( SWLEAD"-I        = Try to inhibit comment processing."PRTTERM );
//	prt( SWLEAD"-I        = Inhibit space / tab in finding text."PRTTERM );	// gfSpacey = TRUE
//	prt( SWLEAD"-I:\"test\" = Inhibit finds with this text in line."PRTTERM );
// "2001 April 13"	// FIX20010413 - add -I+ inhibit find in C/C++ comments
	prt( SWLEAD"-I:\"text\"   = Inhibit finds with this text. (-I+ for C/C++ comments)"PRTTERM );
#endif	// ADDINHIB
#ifdef   ADDLOPTION
   sprtf( SWLEAD"-L[W][nn|F] = Output Additional [Wnn=wrap at (def=%d)][nn=lines|F=Complete function]."PRTTERM, MX1LINE );
   sprtf( SWLEAD"-LL[nn[:nn] = Limit line output - default = %d, nn:nn = left:right, just nn = left=right=nn/2."PRTTERM, DEF_LLIMIT );
#endif   // ADDLOPTION
   prt( SWLEAD"-M[2]       = Process as a MAKE file. Same as -F:M. 2 expands $(mac)."PRTTERM );
	prt( SWLEAD"-N          = Line numbering on finds."PRTTERM );
	prt( SWLEAD"-NS         = No file stats shown at close."PRTTERM );
	prt( SWLEAD"-NF         = No file name shown on a find."PRTTERM );
#ifdef	ADDOUTFIL
	prt( SWLEAD"-OOutFile   = Add all output to a file."PRTTERM );
#endif	// ADDOUTFIL
	prt( SWLEAD"-P          = Ignore Parity for compare. (8th bit stripped)."PRTTERM );
#ifdef	ADDRECUR
	prt( SWLEAD"-R          = Recursive into sub directories."PRTTERM );
#endif	// ADDRECUR
    // FIX20120407 - Add -S
    prt( SWLEAD"-S          = Strip output lines of ALL non-alpanumeric characters, replaced with a space."PRTTERM );
   // FIX20090320 - add this new command, but can also be @InFile ...
   prt( SWLEAD"-UInFile    = Use InFile as list of files to search."PRTTERM );
	prt( SWLEAD"-Vn         = Verbosity level and type. (Def=V2)."PRTTERM );
// FIX20010413    // add a -w2 meaning MUST be space before and after WHOLE
	prt( SWLEAD"-W          = Only whole words (Includes like :WHOLE_. -W2 for spaces)."PRTTERM );
   prt( SWLEAD"-Xname      = Exclude file from search, or as @InFile or ';' separated list."PRTTERM );
   prt( SWLEAD"-X:[name|:] = Exclude directory from search. ::=FrontPage, :::=CVS/.svn directories."PRTTERM );
//	prt( "Notes on Verbosity: -V0 = No output. Errorlevel gives the find count."PRTTERM );
//	prt( " V1 = Show File Name only, and only when Find(s)."PRTTERM );
//	prt( " V2 = Show File Name AND line(s) with find(s). (Default)"PRTTERM );
//	prt( " V3++ Increasing verbality, up to -V9 Maximum Verbality. ie Very Noisy!"PRTTERM );
// FIX20010413    // add a -w2 meaning MUST be space before and after WHOLE
// and maybe -IC to inhibit finds in C/C++ comment lines
	prt( "Notes:  Errorlevel gives the find count. -IC inhibit finds in C/C++ comments."PRTTERM );
	prt( "Output: V0=None, V1=Files, V2=Finds & Files(def), up to V9=ALL ON."PRTTERM );
// FIX20010319 -v4 adds date order of finds at end - See VERB4
   prt( "Note -V3 adds date and time after find file, and -V4 add a list at end."PRTTERM );
// FIX20061106 - "\ntext" to find text ONLY at beginning of line
   prt( "\"\\ntext\" to find text ONLY at the beginning of the line ..."PRTTERM );

   prt_out_tail();
   // === the last line ===   
//#define	TAILSTG	"                                                    Happy searching..."
//#ifdef	FA4DBG
	{
		int bCrLf = GetEnsureCrLf();
		SetEnsureCrLf( FALSE );
		prt( TAILSTG );
		SetEnsureCrLf( bCrLf );
	}
//#else	/* !FA4DBG */
//	prt( TAILSTG );
//#endif	/* DBG or not */

//   UNREFERENCED_PARAMETER(Err);

	Pgm_Exit( pWS );
}

#ifdef _MSC_VER
#define M_IS_DIR _S_IFDIR
#else
#define M_IS_DIR S_IFDIR
#endif

int is_file_or_directory(char * ps)
{
    struct stat buf;
    if (stat(ps,&buf) == 0) {
        if (buf.st_mode & M_IS_DIR)
            return 2;
        else
            return 1;
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Add2CmdBuf
// Return type: void 
// Arguments  : WS
//            : char * cp
//            : int j
// Description: Simply ADD a command argument to the command
//              buffer ....
///////////////////////////////////////////////////////////////////////////////
static int iCmdCount = 0;
void  Add2CmdBuf( WS, char * cp, int j )
{
   iCmdCount++;
   if(VERB9) {
      sprtf( "v9:CMD:%u:[%s]"PRTTERM, iCmdCount, cp );
   }
   if( ( j + gdwCmdLen ) < MXCMDBUF )
	{
		if( gdwCmdLen )
      {
         if( ( gdwCmdLn + j ) > MX1LINE )
         {
            gdwCmdLn = 0;
            strcat( &gszCmdBuf[0], MEOR );
         }
         else
            strcat( &gszCmdBuf[0], " " );
      }
		strcat( &gszCmdBuf[0], cp );
		gdwCmdLen = strlen( &gszCmdBuf[0] );
      gdwCmdLn += j;
	}
	else
   {
      if( ( gdwCmdLen + 4 ) < MXCMDBUF )
	   {
		   strcat( &gszCmdBuf[0], "+++" );
		   gdwCmdLen = strlen( &gszCmdBuf[0] );
	   }
      gdwCmdLen = MXCMDBUF;
   }
}

// =======================================
// void	OutCmds( WS )
//
// Show the COMMANDS on the command line
//
// =======================================
void	OutCmds( WS )
{
	if( !gbDoneCmds )
	{
		gbDoneCmds = TRUE;
		boi(0);
		boi( "Commands: " );
		if( gdwCmdLen )
		{
			boi( &gszCmdBuf[0] );
		}
		else
		{
			boi( "<none>" );
		}
		boi( PRTTERM );
		boi(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Check4Help
// Return type: void 
// Arguments  : WS
//            : int argc
//            : char * * argp
// Description: This seaches the command arguments for
//              one of -? -h or -H
///////////////////////////////////////////////////////////////////////////////
void	Check4Help( WS, int argc, char * * argp )
{
	int	i;
	char * cp;
	char	c;
	if( argc > 1 )
	{
		for( i = 1; i < argc; i++ )
		{
         c = 0;
         cp = argp[i];
         if(cp)
            c = *cp;
         if(c)
			{
				if( ( c == '-' ) || ( c == '/' ) )
				{
					cp++;
					c = *cp; 
					if(c)
					{
						if( ( c == '?' ) || ( c == 'h' ) || ( c == 'H' ) )
						{
                     // FIX20010330 - Also OUPUT the commands, at least in this set
                     for( i = 1; i < argc; i++ )
                     {
                        cp = argp[i];
                        Add2CmdBuf( pWS, cp, (int)strlen(cp) );
                     }
                     OutCmds( pWS );
							Usage( pWS, 0 );	// NEVER RETURN!!!
						}	// got a HELP request
					}	// and a char following switch
				}	// if it is a switch
			}	// got pointer AND character
		}	// for( i = 1; i < argc; i++ )
	}	// more than 1 argc
}

void	Check4Verb( WS, int argc, char * * argp )
{
	int	i, c;
	char * cp;
	for( i = 1; i < argc; i++ ) {
      c = 0;
      cp = argp[i];
      if(cp)
         c = *cp;
      if(c) {
			if( ( c == '-' ) || ( c == '/' ) )
			{
				cp++;
				c = toupper(*cp);
            if( c == 'V' ) {
               cp++;
               if( ISNUM(*cp) )
                  gfVerbose = atoi(cp);
				}	// and a char following switch
			}	// if it is a switch
		}	// got pointer AND character
	}	// for( i = 1; i < argc; i++ )
}


#ifdef		ADDERRLST	// Special for Directory Listing
//				case 'E':	// WIN32 Error.
void	GetErrNum( WS, char * lpIn )
{
	int		i, j, k;
	char	c;
	long	lg;
	char *	cp;

   j = 0;
   cp = lpIn;
   if(cp)
      j = strlen(cp);
   if(j)
	{
		lg = 0;
		k = 0;
		c = (char)toupper(*cp);
		if( ( c == 'A' ) &&
			( j >  2   ) &&
			( ( cp[1] == 'l' ) || ( cp[1] == 'L' ) ) &&
			( ( cp[2] == 'l' ) || ( cp[2] == 'L' ) ) )
		{
			outvals();
			return;
		}

		if( ( j > 2 ) &&
			( cp[0] == '0' ) &&
			( ( cp[1] == 'x' ) || ( cp[1] == 'X' ) ) )
		{
			// assume HEX
			j  -= 2;
			cp += 2;
			for( i = 0; i < j; i++ )
			{
				c = (char)toupper(cp[i]);
				if( ( c >= '0' ) && ( c <= '9' ) )
				{
					lg = ( lg * 16 ) + ( c - '0' );
					k++;
				}
				else if( ( c >= 'A' ) && ( c <= 'F' ) )
				{
					lg = ( lg * 16 ) + ( c - ('0'+7) );
					k++;
				}
				else
				{
					if( k )
						break;
				}

			}
		}
		else
		{
			// assume DECIMAL
			for( i = 0; i < j; i++ )
			{
				c = cp[i];
				if( ( c >= '0' ) && ( c <= '9' ) )
				{
					lg = ( lg * 10 ) + ( c - '0' );
					k++;
				}
				else
				{
					if( k )
						break;
				}
			}
		}
		if( k )
		{
			glFindErr = lg;
			gbFindErr = TRUE;
		}
	}
}

#endif	// ADDERRLST

int	IsBgnType( char c )
{
	int	flg = FALSE;
   // FIX20001022    // fix find of "'@'"!!!
   // remove the '\'' from this list
	//	( c == '\'' ) ||
	if( ( c == '"' ) ||
		( c == '+' ) )
	{
		flg = TRUE;
	}
	return flg;
}


/* ============================================================
 * void	Add2Finds( WS, char * lps, int bFSwitch )
 * case 'F':
 * Present begin/end type characters are only " or +
 * So I can use -F:M as a special MAKEFILE switch
 *
 * If bFSwitch is TRUE this is a FIND using -F"abc"
 * We are already pointing to the "abc"
 * If bFSwitch is FALSE,
 * it is an 'unswitched' argument on the command line
 *
 * FIX20050304 - Add -F:1 = all finds before file display
 * #define ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
 */
//#define  ISNUM(a)    ( ( a >= '0' ) && ( a <= '9' ) )
// Do_F_Switch()
// FIX20061227 - add REGEX for finds
// -fr=regex
// removed support for NOT USEFINDLIST
int  Add2Finds( WS, char * lps, int bFSwitch )
{
   int  bRet = TRUE;   // assume ok
	char	c, d, uc;
	char *	cp;
   int   ilen;
// #ifdef   USEFINDLIST    // FIX20010703
   uint32_t    dwn = 0;
   uint32_t    dwo = 0;
// #endif   // #ifdef   USEFINDLIST    // FIX20010703
#ifdef   ADD_REGEX
   int   isregex = 0;
#endif   // #ifdef   ADD_REGEX
   c = 0;
   cp = lps;
   ilen = 0;
   if(cp) {
      c = *cp;
      // 20070306 - fix of copy to FIND list -
      // do NOT make forst letter upper case
      uc = (char)toupper(*cp);
      ilen = strlen(cp);
      if( bFSwitch ) {
#ifdef   ADD_REGEX
         if( uc == 'R' ) {
            cp++;
            c = *cp;
            ilen = strlen(cp);
            isregex = 1;
         }
#endif   // #ifdef   ADD_REGEX
         if( c == '=' ) {
            cp++;
            c = *cp;
            ilen = strlen(cp);
         } else if( c == ':' ) {
            // this is a VALID option
         } else if( ( ilen >= 5 ) && ( ( c == '1' ) || ( c == '2' ) ) && ( cp[1] == ':' ) && ( cp[2] ) ) {
            // this is a VALID option
         } else if ( IsBgnType( c ) ) { // FIX20071010
            // this is also VALID
         } else {
            return FALSE;
         }
      }
   }

   // FIX20050304 - Add -F:1 = all finds before file display
   // 1 - Handle -F:1 = all finds in file before display
   if( ( bFSwitch ) && ( c == ':' ) ) {
      cp++;
      // 20070306 - fix of copy to FIND list -
      // do NOT make forst letter upper case
      c = *cp;
      uc = (char)toupper(*cp);
      if( uc == '1' ) {
         g_bDoAllin1 = 1;
      }
#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
      else if( uc == '2' ) {
         g_bAllInOne = 1;
      }
#endif // #define ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
      else {
         bRet = FALSE;  // flag an error
      }
      return bRet;
   }

// #ifdef   USEFINDLIST    // FIX20010703
   if(( bFSwitch  ) && ( ilen >= 5 ) ) {
      // 2 - Handle -F1|2:nn:"abc"
      if(( ( c == '1' ) || ( c == '2' ) ) &&
         ( cp[1] == ':' ) && ( cp[2] ) )
      {
         cp += 2;
         dwn = (uint32_t)c;
         c = *cp;
         while( c && (c != ':') )
         {
            if( ISNUM(c) ) {
               dwo = ((dwo * 10) + (c - '0'));
            }
            else
               return FALSE;
            cp++;
            c = *cp;
         }
         if( c == ':' ) {
            cp++;
            c = *cp;    // get the FIRST letter of the FIND string
         }
         else
            return FALSE;
      }
   }
// #endif   // #ifdef   USEFINDLIST    // FIX20010703

   if(c) {
// #ifdef   USEFINDLIST    // FIX20010703
      char *   lpb = &g_szTmpBuf[0];   // accumulate into a temp buffer
      uint32_t    dwLen = 0;               // keeping the LENGTH
      char *   lps;
// #endif   // USEFINDLIST

      // check if a BEGIN character
      // WAS '"' | '\'' | '+' !!!
      // BUT that excludes using "'@'" as the FIND part
      // FIX20001022    // fix find of "'@'"!!!
      if( IsBgnType( c ) )  {  // presently only '"' | '+' !!!
			d = c;	// MAYBE Beginning character
		} else {
			d = 0;   // NOTE: WIN32 command line STRIP the double quotes also
// #ifdef   USEFINDLIST    // FIX20010703
         lpb[dwLen++] = c; // insert FIRST character
// #endif   // USEFINDLIST
		}
		cp++;	// Bump past beginning
      // IFF from the command line with double quotes stripped, and there was only
      // ONE character, then we are DONE
		while( ( c = *cp++ ) > 0 )
		{
			if( d ) {
				// It commenced with a double quote or a plus
				// This BEGIN character must be excluded from
				// the end of the argument.
				if( c == d ) {
					// Do NOT add this char
					break;
				}
         } else {	// NOT A BEGIN CHAR
				// FIX981029
				// NOTE: 'C' runtime has already REMOVED
				// any QUOTES around the argument, so
				// we should CONTINUE until the argument ends,
				// and NOT
				//if( c <= ' ' )
				//{
				//	c = 0;
				//	break;
				//}
			}
// #ifdef   USEFINDLIST
         lpb[dwLen++] = c;
         if(dwLen >= MXONEFIND)
         {
				strcat( glpError, "ERROR: Find string appears too long!"PRTTERM );
            //bRet = FALSE;   error line already ADDED so no need to flag this
				break;
         }
// #endif   // USEFINDLIST
		}

// #ifdef   USEFINDLIST    // FIX20010703
      if(dwLen)
      {
         PLE   ph = &g_sFind; // ADD To FIND LIST
         PLE   pn;
         PFLINE pl;
         lpb[dwLen] = 0;    // zero terminate string
         // FIX20061106 - \ntext means ONLY at beginning of line
         if (lpb[0] == '\\') {
            uint32_t i2;
            if ( ( dwLen >= 3 ) && ( lpb[1] == 'n' ) ) {
               for ( i2 = 2; i2 < dwLen; i2++ ) {
                  lpb[i2 - 2] = lpb[i2];
               }
               dwLen -= 2;
               lpb[dwLen] = 0;
               g_fBeginLine = TRUE;
            } else if ( (lpb[1] == '\\') || (lpb[1] == '-') ) {
               // FIX20081003 Add escaped hyphen, to search for hyphen - 
               // ie '\-' == '-', but '\\-' == '\-'
               for ( i2 = 1; i2 < dwLen; i2++ ) {
                  lpb[i2 - 1] = lpb[i2];
               }
               dwLen -= 1;
               lpb[dwLen] = 0;
            }
         }
         //pn = MALLOC( LPTR, (sizeof(LIST_ENTRY) + ilen + 1) );
         pn = (PLE)MALLOC( LPTR, (sizeof(FLINE) + dwLen) );
         if(pn)
         {
            //char *   lps = (char *)pn;
            //lps += sizeof(LIST_ENTRY);
            pl = (PFLINE)pn;
            pl->dwTyp = dwn;     // set TYPE (1=same file, 2=same file and line)
            pl->dwTN  = dwo;     // set NUMBER to match with other find string nums
#ifdef   ADD_REGEX
            pl->isregex = isregex;
            pl->pre     = NULL;
#endif   // #ifdef   ADD_REGEX
            lps = &pl->cLine[0];
            if( VERB5 )
               sprtf( "Adding [%s] to find list ...\n", lpb );
            strcpy( lps, lpb );
            InsertTailList(ph,pn);
            g_dwFCnt++; // stored a FIND on the list
            if( dwLen > g_dwMinLen )
               g_dwMinLen = dwLen; // keep the MAXIMUM find length

   			if( giFirst == 0 )   // ONE DAY THIS ALSO NEEDS TO GO AWAY!!!
	   			giFirst++;
         }
         else
         {
			strcat( glpError, "ERROR: Find string memory FAILED!"PRTTERM );
            bRet = FALSE;  // flag an error
         }
      }
      else
      {
         //sprtf( "WARNING: No find string found!!!"MEOR );
      }
// #endif   // USEFINDLIST
	}
   return bRet;
}

static int	priv_GetFSize( LPGFS lpgfs )
{
	int	flg = FALSE;
	if( ( lpgfs ) &&
		( VH(lpgfs->fs_hHnd) ) )
	{
        lpgfs->fs_uint64 = get_last_file_size64();
        if (lpgfs->fs_uint64 > 0)
		{
			flg = TRUE;
		}
	}
	return flg;
}


int	Open4Read( LPGFS lpgfs )
{
	int	flg = FALSE;
	if( ( lpgfs ) &&
		( OpenReadFile( &lpgfs->fs_szNm[0], &lpgfs->fs_hHnd ) ) &&
		( VH( lpgfs->fs_hHnd ) ) &&
		( priv_GetFSize( lpgfs ) ) )
	{
		flg = TRUE;
	}
	if( ( !flg ) &&
		( lpgfs ) )
	{
		if( VH( lpgfs->fs_hHnd  ) ) {
#ifdef _MSC_VER
			CloseHandle( lpgfs->fs_hHnd );
#else
            fclose( lpgfs->fs_hHnd );
#endif
        }
		lpgfs->fs_hHnd = INVALID_HANDLE_VALUE;
	}
	return flg;
}


// 	int	ws_fSpacey;	// g(W.ws_)fSpacey -I Exclude spaces / tabs in text compare - June 2000
#define	MXSTKBUF		64

//#ifdef	ADDINHIB
LPGFS GetNxtGFS( WS )
{
   LPGFS	lpgfs;
   lpgfs = &gsGFS[gdwGFCnt];
   gdwGFCnt++;
   if( gdwGFCnt >= MXINPFS )
      gdwGFCnt = 0;
   return lpgfs;
}

int	CloseARead( LPGFS lpgfs )
{
	int	flg = FALSE;
	if( lpgfs )
	{
		if( VH( lpgfs->fs_hHnd ) )
		{
#ifdef _MSC_VER
			CloseHandle( lpgfs->fs_hHnd );
#else
            fclose( lpgfs->fs_hHnd );
#endif
			flg = TRUE;
		}
		lpgfs->fs_hHnd = 0;
	}
	return flg;
}

#ifndef _MSC_VER
int ReadFile( HANDLE hFile, void *pBuf, uint32_t len, uint32_t *pread, void *vp )
{
    int iret = 0;
    uint32_t red = fread( pBuf, 1, len, hFile );
    if (red == len) {
        *pread = red;
        iret = 1;
    }
    return iret;
}
#endif
//				case 'I':
//#define		fInhibit	W.ws_fInhibit
//#define		giInhibCnt	W.ws_iInhibCnt
//#define		giInhibSiz	W.ws_iInhibSiz
//#define		glpInhib	W.ws_lpInhib
int	GetIFile( WS, char * psw )
{
	int	flg = FALSE;
	char *	cp;
    uint32_t size;
	cp = psw; 
	if(cp)
	{
		LPGFS	lpgfs = GetNxtGFS(pWS); // get one of a rotating set of MXINPFS ...
		strcpy( &lpgfs->fs_szNm[0], cp );
		if( Open4Read( lpgfs ) )
		{
            size = (uint32_t) lpgfs->fs_uint64;
			if( lpgfs->fs_uint64 > MY_MAX_FILESIZE )
			{
				CloseARead( lpgfs );
				strcat( glpError, "ERROR: File TOO large!"PRTTERM );
			}
			else if( ( lpgfs->fs_lpV = MALLOC( LPTR, ( size + 16 ) ) ) != 0 )
			{
				lpgfs->fs_dwRd = 0;
				if( ( ReadFile( lpgfs->fs_hHnd,
								lpgfs->fs_lpV,
								size,
								&lpgfs->fs_dwRd,
								NULL ) ) &&
					( lpgfs->fs_dwRd == size ) )
				{
					char *	lpb;
					uint32_t	dwi;
					char	c;

					lpb = lpgfs->fs_lpV;
					for( dwi = 0; dwi < size; dwi++ )
					{
						c = lpb[dwi];	// get char
					}

				}
				else
				{
					strcat( glpError, "ERROR: Read file failed!"PRTTERM );
				}
				MFREE( lpgfs->fs_lpV );
				lpgfs->fs_lpV = 0;
				CloseARead( lpgfs );
			}
			else
			{
				CloseARead( lpgfs );
				strcat( glpError, "ERROR: Allocate memory failed!"PRTTERM );
			}
		}
		else
		{
			sprintf( EndBuf(glpError),
				"ERROR: Unable to OPEN input [%s] file!"PRTTERM,
				cp );
		}
	}
	return flg;
}

int	GetI22( WS, char * pchrs, int len )
{
	int	flg = FALSE;
	int		i, j, k, l;
	char *	cp;
	char	c;
	cp = pchrs;
	i = len;
	if( ( pWS ) &&
		( cp ) &&
		( i ) )
	{
		l = 0;
		if( *cp == 0x22 )
		{
			l++;
			cp++;
			i--;
		}
		k = giInhibCnt;
		for( j = 0; j < i; j++ )
		{
			c = cp[j];
			if( c == 0x22 )
			{
				j++;
				break;
			}
			glpInhib[k++] = c;
		}
		if( k )
		{
			glpInhib[k++] = 0;
			glpInhib[k]   = 0;
			giInhibCnt = k;
			fInhibit = TRUE;
	//if !GetInhibs( pWS, cp ) )
	//	SetInvSwitch( pWS, cp );
			flg = TRUE;
			while( (j+3) < i )
			{
				if( ( cp[j] == ';' ) &&
					( cp[j+1] == 0x22 ) &&
					( cp[j+2] != 0x22 ) )
				{
					j += 2;
					for( ; j < i; j++ )
					{
						c = cp[j];
						if( c == 0x22 )
						{
							j++;
							break;
						}
						glpInhib[k++] = c;
					}
					glpInhib[k++] = 0;
					glpInhib[k]   = 0;
					giInhibCnt = k;
				}
				else
				{
					break;
				}
			}
		}
	}
	return flg;
}

int	GetI( WS, char * pchrs, int len )
{
	int	flg = FALSE;
	int		i, j, k;
	char *	cp;
	char	c;

   i  = len;
	cp = pchrs;
	if( ( pWS ) &&
		( cp ) &&
		( i ) )
	{
		// get previous input point
		k = giInhibCnt;
		for( j = 0; j < i; j++ )
		{
			c = cp[j];
			if( c == ';' )
			{
				break;
			}
			glpInhib[k++] = c;
		}
		if( k )
		{
			glpInhib[k++] = 0;
			glpInhib[k]   = 0;
			giInhibCnt = k;
			fInhibit = TRUE;
	//if !GetInhibs( pWS, cp ) )
	//	SetInvSwitch( pWS, cp );
			flg = TRUE;
			while( j < i )
			{
				if( cp[j] == ';' )
				{
					j++;
					for( ; j < i; j++ )
					{
						c = cp[j];
						if( c == ';' )
						{
							break;
						}
						glpInhib[k++] = c;
					}
					glpInhib[k++] = 0;
					glpInhib[k]   = 0;
					giInhibCnt = k;
				}
				else
				{
					break;
				}
			}
		}
	}
	return flg;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Do_I_Switch ( was GetInhibs )
// Return type: int 
// Arguments  : WS
//            : char * psw
// Description: case 'I': Get the INHIBIT string
//              
// "2001 April 13"	// FIX20010413 - add -I+ inhibit find in C/C++ comments
///////////////////////////////////////////////////////////////////////////////
// int	GetInhibs( WS, char * psw )
int	Do_I_Switch( WS, char * psw )
{
	int	flg = FALSE;      // set FAIL for now
	char	buf[MXSTKBUF];
	char *	cp;
	char	c;
	int	i, j;

	cp = psw; 
	if(cp)
	{
		i = strlen(cp);
		// we have a pointer and length
		if( i < (MXSTKBUF - 1) )
		{
			strcpy(buf,cp);	// make COPY of input string
			while( ( i ) &&
				( buf[i-1] <= ' ' ) )
			{
				i--;
				buf[i] = 0;
			}
			if( i )
			{
				j = 0;
				while( j < i )
				{
					// see what we got - or FALL through to bigger function below more likely ...
					j++;
				}
			}
		}

		if( i == 0 )
		{
			{
				// we had the NEW switch -I = just ignore space/tabs in finding compare
				flg = TRUE;
				gfSpacey = TRUE;	// -I Exclude spaces / tabs in text compare - June 2000
				return flg;
			}
		}

		while( ( *cp ) && ( *cp != ':' ) )
		{
			if( toupper(*cp) == 'C' )
				gbCaseInhib = TRUE;
         else if( *cp == '+' )
            g_bIgComm = TRUE;
         else
            return flg;    // whatever this is it is NOT valid
			cp++;
		}
      if( *cp == 0 )
      {
         return TRUE;
      }
		if( *cp == ':' )
		{
			cp++;
			c = *cp; 
			if(c)
			{
				if( c == '@' )
				{
					cp++;
					flg = GetIFile( pWS, cp );
				}
				else
				{
					// not an input file
					int		i;
					i = strlen( cp ); 
					if(i)
					{
						if( glpInhib == 0 )
						{
							giInhibCnt = 0;
							giInhibSiz = i+16;
							glpInhib = (char *)MALLOC( LPTR, (i+16) ); 
							if(glpInhib)
							{
								if( *cp == 0x22 )
									flg = GetI22( pWS, cp, i );
								else
									flg = GetI( pWS, cp, i );
							}
						}
						else
						{
							// this is an ADDITION
							char *lpnew, *lpold, *lpbgn;
							int		j, k;

                     lpnew = 0;
                     lpold = glpInhib;
                     if(lpold)
                        lpnew = (char *)MALLOC( LPTR, (giInhibSiz + i + 16) );
                     if(lpnew)
							{
								giInhibSiz += (i + 16);
								lpbgn = lpnew;
								k = 0;
								while( ( j = strlen(lpold) ) > 0 )
								{
									strcpy(lpnew, lpold);
									lpold += (j + 1);
									lpnew += (j + 1);
									k     += (j + 1);
								}
								*lpnew = 0;
								lpold = glpInhib; 
								if(lpold)
									MFREE(lpold);
								glpInhib   = lpbgn;
								giInhibCnt = k;
								if( *cp == 0x22 )
									flg = GetI22( pWS, cp, i );
								else
									flg = GetI( pWS, cp, i );
							}
						}
					}
				}
			}
		}
	}

	return flg;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : DoReadBuf
// Return type: void 
// Arguments  : LPWORKSTR pWS
//            : uint32_t level
//            : LPGFS lpgfs
// Description: Read the data in the buffer seeking COMMANDS.
//              Skip all after ";" until END OF LINE
// FIX20010319 When adding -v4 adds date order of finds at end (VERB4) also
// fixed the skipping to ignore TAB characters ("\t") 09 hex!!!
// note: MXARGS is the allocated MAXIMUM
///////////////////////////////////////////////////////////////////////////////
void	DoReadBuf( LPWORKSTR pWS, uint32_t level, LPGFS	lpgfs )
{
	char *	   lpb;
   char *    ptmp;
	uint32_t	   dwi, dwl;
	char	   c, cc, dd;
	char * *  lpa;
	int		ic;
    uint32_t size = (uint32_t)lpgfs->fs_uint64;

	lpb = lpgfs->fs_lpV;
	lpa = (char * *)&lpb[size];  // get AFTER file read size
	ic = 0;
	lpa[ic++] = "FA4";   // add first dummy argument = 1 of MXARGS
	for( dwi = 0; dwi < size; dwi++ )
	{
		c = lpb[dwi];	// get char
		if( c > ' ' )
		{
			if( c == ';' ) 
			{
				// skip commented lines
				dwi++;	// to next
				for( ; dwi < size; dwi++ )
				{
               c = lpb[dwi];  // NOTE: Also skip TABS
					if( ( c < ' ' ) && ( c != '\t' ) )
					{
                  // at END OF LINE
						break;
					}
				}
			}
			else
			{
            ptmp = &lpb[dwi];
				lpa[ic++] = &lpb[dwi];  // set argument pointer
            if( c == '"' )
               dd = c;
            else
               dd = 0;	// begin NOT in quotes
				dwi++;      // bump PAST first char
				for( ; dwi < size; dwi++ )
				{
//					if( lpb[dwi] <= ' ' )
//					if( lpb[dwi] < ' ' )
					cc = lpb[dwi];	// get character
					if( ( cc < ' '  ) &&
						( cc != '\t' ) )
					{
						// out of here to CLOSE command line item
                  // but first, clean TAIL
						lpb[dwi] = 0;
						dwl = dwi;
                  while(dwl--)
                  {
                     if( lpb[dwl] > ' ' )
                     {
                        dwl++;   // back up to COUNT
                        break;   // and add argument
                     }
                     lpb[dwl] = 0;
                  }
						break;
					}
					else
					{
						if( cc == ';' ) // got message comment indicator
						{
							// and NOT within QUOTE marks
							if( dd == 0 )
							{
								// then EXIT here
								// but first, clean TAIL
								lpb[dwi] = 0;
								dwl = dwi;
								while( ( dwl ) &&
									( lpb[dwl-1] <= ' ' ) )
								{
									dwl--;
								}
								lpb[dwl] = 0;

								// and throw away the COMMENT section
								dwi++;	// to next
								for( ; dwi < size; dwi++ )
								{
									cc = lpb[dwi];
									if( ( cc < ' '  ) &&
										( cc != '\t' ) )
									{
										break;
									}
								}

								// ok, now out of here
								break;	// have established a COMMAND in the buffer
							}
						}  // if a "comment" character
					}

					if( cc == '"' )
					{
						if( dd )
							dd = 0;
						else
							dd = cc;
					}

				}
				lpb[dwi] = 0;
			}
		}
      if( ic > (MXARGS - 1) )
      {
         strcat( glpError, "ERROR: Exceeded maximum ARGUMENTS!"PRTTERM );
         break;
      }
	}

	if( ic > 1 )
	{
		lpa[ic] = 0;
		ProcessArgs( pWS, ic, lpa, (level+1) );
	}
	else
	{
		// no arguments
		// this could be an ERROR
      // but return quietly, for now ...
	}

}



//#endif	// ADDINHIB
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : DoAnINP
// Return type: void 
// Arguments  : WS
//            : char * cp
//            : int level
// Description: 
//              
///////////////////////////////////////////////////////////////////////////////
void  DoAnINP( WS, char * cp, int level )
{
   LPGFS	lpgfs = GetNxtGFS(pWS);
   // cp++; // already DONE before here
   strcpy( &lpgfs->fs_szNm[0], cp );
   if( Open4Read( lpgfs ) )
   {
       uint32_t size = (uint32_t)lpgfs->fs_uint64;
	   if( lpgfs->fs_uint64 > MY_MAX_FILESIZE)
	   {
		   CloseARead( lpgfs );
		   strcat( glpError, "ERROR: File TOO large!"PRTTERM );
	   }
	   else if( ( lpgfs->fs_lpV = MALLOC( LPTR,
		   ( size + (MXARGS * sizeof(char *)) ) ) ) != 0 )
	   {
		   lpgfs->fs_dwRd = 0;
		   if( ( ReadFile( lpgfs->fs_hHnd,
					   lpgfs->fs_lpV,
					   size,
					   &lpgfs->fs_dwRd,
					   NULL ) ) &&
			   ( lpgfs->fs_dwRd == size ) )
		   {
   //#ifdef	USERDBUF
			   DoReadBuf( pWS, level, lpgfs );
   //#endif	/* USERDBUF */
		   }
		   else
		   {
			   strcat( glpError, "ERROR: Read file failed!"PRTTERM );
		   }
		   MFREE( lpgfs->fs_lpV );
		   lpgfs->fs_lpV = 0;
		   CloseARead( lpgfs );
	   }
	   else
	   {
		   CloseARead( lpgfs );
		   strcat( glpError, "ERROR: Allocate memory failed!"PRTTERM );
	   }
   }
   else
   {
	   sprintf( EndBuf(glpError),
		   "ERROR: Unable to OPEN input [%s] file!"PRTTERM,
		   cp );
   }
}

void	SetInvSwitch( WS, char * psw )
{
	char * cp;

	cp = psw;
	if( ( pWS ) &&
		( cp ) )
	{
		cp--;
		cp--;
		sprintf( EndBuf(glpError),
				"ERROR: Invalid switch! [%s]"PRTTERM,
				cp );
	}
}


int	MoveFileName( WS, char * lpdest, char * lpsrc )
{
	char *	lps;
	int		i, j, k;
	char	c;

	j = k = 0;
	lps = lpsrc;
   if(lps)
      j = strlen(lps);
	if( lpdest && j )
	{
		*lpdest = 0;
		if( *lps == '"' )
		{
			lps++;
			j = strlen(lps);
			for( i = 0; i < j; i++ )
			{
				c = lps[i];
				if( c == '"' )
					break;
				lpdest[i] = c;
			}
			lpdest[i] = 0;
		}
		else
		{
			strcpy( lpdest, lpsrc );
		}
		k = strlen(lpdest);
	}
   UNREFERENCED_PARAMETER(pWS);
	return k;
}

int	LoadFile2( WS, LPLFSTR lpLF, int fExit )
{
	char * lpFN;
	FILE * fp;
	uint32_t *lpDW, *lpRD;
	char * *	lpBuf;
	int	flg;

	flg = 0;
	if( lpLF )
	{
		lpFN = &lpLF->lf_szNm[0];
		if( *lpFN && IsValidFile(lpFN)) 
		{
            uint64_t s64 = get_last_file_size64();
            if (s64 > MAX_INPUT_LEN) {
                sprintf( EndBuf(glpError),
                    "\r\nERROR: File [%s] is too large!"PRTTERM,
                    lpFN );
                goto Exit;
            }
			lpLF->lf_hHnd = 0;
			lpDW   = &lpLF->lf_dwSz;
			lpRD   = &lpLF->lf_dwRd;
			lpBuf  = &lpLF->lf_lpBuf;  // pointer to BUFFER
			if( VERB6 )
			{
				boi(0);
				sprintf( lpVerb, 
					"Openning [%s] ... ",
					lpFN );
				boi( lpVerb );
			}
			if( ( fp = fopen( lpFN, "rb" ) ) != 0 )
			{
				if( VERB6 )
				{
					sprintf( lpVerb, "FH=%p ... ", (void *)fp );
					boi( lpVerb );
				}
				*lpDW = (uint32_t)s64; // LoadFile2() - get LENGTH of an INPUT file
				if(*lpDW)
				{
					if( VERB6 )
					{
						sprintf( lpVerb,
							"Len.=%u ... ",
							*lpDW );
						boi( lpVerb );
					}
					*lpBuf = (char *)MALLOC( LPTR, (*lpDW + 16) ); // allocate for INPUT file
					if(*lpBuf)
					{
						if( VERB6 )
							boi( "Reading..." );
						// Read DATA into buffer
                        *lpRD = fread( *lpBuf, 1, *lpDW, fp );
                        fclose(fp);
						if( *lpRD == *lpDW )
						{
							if( VERB6 )
							{
								//prt( "OK"PRTTERM );
								boi( "OK"PRTTERM );
								boi(0);
							}
							flg = TRUE;
						}
						else
						{
							sprintf( EndBuf(glpError),
								"\r\nERROR: Unable to READ file [%s]!"PRTTERM,
								lpFN );
						}
					}
					else
					{
						sprintf( EndBuf(glpError),
							"\r\nERROR: Allocation of %u bytes FAILED!"PRTTERM,
							*lpDW );
					}
				}
				else
				{
					sprintf( EndBuf(glpError),
						"\r\nERROR: File [%s] is NULL!"PRTTERM,
						lpFN );
				}
			}
			else
			{
				sprintf( EndBuf(glpError),
					"\r\nERROR: Unable to OPEN file [%s]!"PRTTERM,
					lpFN );
			}
		}
		else
		{
			strcat( glpError,
				"\r\nERROR: Bad internal parameters!"PRTTERM );
		}
	}
	else
	{
		strcat( glpError,
			"\r\nERROR: Bad internal parameters!"PRTTERM );
	}
Exit:
	if( ( fExit ) &&
		( *glpError ) )
	{
		Err_Exit( -1 );
	}
	return( flg );
}


//typedef	struct {
//	char	lf_szNm[MAX_PATH];
//	HANDLE	lf_hHnd;
//	uint32_t	lf_dwSz;
//	uint32_t	lf_dwRd;
//	char *	lf_lpBuf;
//}LFSTR;
//typedef LFSTR FAR * LPLFSTR;

int	KillFile2( WS, LPLFSTR lpLF )
{
	int	flg = FALSE;
	char *	lpFN;
	if( lpLF )
	{
		lpFN = &lpLF->lf_szNm[0];
		//if( VH( lpLF->lf_hHnd ) )
		//{
		//	CloseHandle( lpLF->lf_hHnd );
		//	flg++;
		//}
		lpLF->lf_hHnd = 0;
		if( lpLF->lf_lpBuf )
		{
			MFREE( lpLF->lf_lpBuf );
			flg++;
		}
		lpLF->lf_lpBuf = 0;
	}
    UNREFERENCED_PARAMETER(pWS);
	return flg;
}


void	Add2Files( WS, char * lps ) // PLE ph = &g_sFileList, count in g_nFileCnt
{
   static char _s_addfil_buf[264];
	char *	cp = _s_addfil_buf;
#ifndef   USEFINDLIST    // above change to FIND list = FIX20010703
	char	c;
#endif // #ifndef   USEFINDLIST    // above change to FIND list = FIX20010703
   int ilen = 0;
	if(lps) {
      strcpy(cp,lps);
      // FIX20041210 - remove any "..." from the file name
      if( *lps == '"' ) {
         strcpy(cp, &lps[1]);
         ilen = strlen(cp); // get the count
         if((ilen) && (cp[ilen-1] == '"')) {
            ilen--;
            cp[ilen] = 0;
         }
      }
      ilen = strlen(cp);
   }

   if(ilen) {
#ifdef   USEFINDLIST    // above change to FIND list = FIX20010703
       // FIX20111105 - An input of a 'directory', make it directory\*.*
      PLE   ph = &g_sFileList;   // count in g_nFileCnt
      PLE   pn = (PLE)MALLOC( LPTR, (sizeof(MFILE) + strlen(cp)) + 8);
      if(pn)
      {
         PMFILE    pf = (PMFILE)pn;
         char *    ps = &pf->cFile[0];
         pf->dwNum = 0;
         strcpy( ps, cp );
         if (is_file_or_directory(ps) == 2) {
             strcat(ps,"\\*.*"); // is a DIRECTORY
         }
         InsertTailList(ph,pn);
         g_nFileCnt++;           // count of files in LIST
      }
      else
      {
         strcat( glpError,
            "ERROR: File Name MEMORY FAILED!"PRTTERM );
      }

#else   // !#ifdef   USEFINDLIST    // above change to FIND list = FIX20010703

		while( ( c = *cp++ ) > 0 )
		{
			if( gnFileLen < (MXFILSTGS - 2) )
			{
				gszFileMask[gnFileLen++] = c;
			}
			else
			{
				strcat( glpError,
					"ERROR: File Name strings appears too long!"PRTTERM );
				break;
			}
		}
		if( gnFileLen )
		{
			if( gszFileMask[gnFileLen-1] != 0 )
				gszFileMask[gnFileLen++] = 0;
			gszFileMask[gnFileLen]   = 0;
		}
#endif   // #ifdef   USEFINDLIST    // above change to FIND list = FIX20010703
	}
}

typedef unsigned int uint;

// void Do_B_Switch( WS, char * pcmd ) {
void Do_B_Switch( char * pcmd ) {
   int err = 0; // no error yet
   char * cp = pcmd;
   if( *cp == '-' ) { // FIX20070906 - new -b- to IGNORE binary files
      g_fIgnoreBin = TRUE;
      cp++;
      if( *cp ) {
         err = 1;
      }
   } else {
      g_fBinary = TRUE;  // set BINARY search
      if( *cp ) { // may have back/forward numbers
         if( ISNUM(*cp) ) {
            int n1 = atoi(cp); // get first num
            if(n1 > 0) {
               g_dwBackup = n1;
            }
            while( *cp && (ISNUM(*cp) ) ){ cp++; }
            if(*cp) {
               if(*cp == ':') {
                  cp++;
                  if(ISNUM(*cp)) {
                     n1 = atoi(cp); // get second num
                     if(n1 > 0) {
                        g_dwForward = n1;
                     }
                  } else {
                     err = 1;
                  }
               } else {
                  err = 1;
               }
            }
         } else {
            err = 1;
         }
      }
   }
   if(err) {
      sprintf( EndBuf(&g_cErrBuf[0]), "ERROR: Invalid -B switch! [%s]"MEOR,
         (pcmd - 2) );
   }
}

void  Do_C_Switch( WS, char * cp )
{
#ifdef   ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   if( ISNUM( *cp ) )   // if it starts with a NUMBER
   {
      uint32_t    dwi = ( *cp - '0' );
      char *   bcp = cp;
      cp++;
      while( ISNUM(*cp) )
      {
         dwi = ((dwi * 10) + (*cp - '0'));
         cp++;
      }
      if( dwi && ( *cp == 0 ) )
      {
         g_dwMinCnt = dwi; // set the minimum FIND counter
      }
      else
      {
   		sprintf( EndBuf(glpError),
				"ERROR: Invalid -C switch! [%s]! Only -C, Cnn (min=1) or -cvs!!"PRTTERM,
				bcp );
      }
      return;
   }
#endif   // ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT

#ifdef   ADDCVSDATE
   if( *cp )
   {
      if(( toupper(cp[0]) == 'V' ) &&
         ( toupper(cp[1]) == 'S' ) )
      {
         g_bCVSDate = TRUE;
      }
      else
      {
   		sprintf( EndBuf(glpError),
				"ERROR: Invalid -C switch! [%s]! Only -C, Cnn or -cvs!!"PRTTERM,
				cp );
      }
   }
   else
   {
	   g_fCase = TRUE;
   }
#else    // !ADDCVSDATE
	g_fCase = TRUE;
#endif   // ADDCVSDATE y/n
}

void  Do_U_Switch( WS, char * cp )
{
    if( *cp ) { // we seem to have an input file name
        if( IsValidFile( cp ) ) {
            FILE *fp;
            uint64_t len = get_last_file_size64();
            size_t read, sz, i;
            char * pb;
            if (len > MAX_INPUT_LEN) {
                sprintf( EndBuf(glpError),
		    	    "ERROR: Invalid -U switch! Input file exceeds %u bytes!"PRTTERM, MAX_INPUT_LEN );
                return;
            } else if (len == 0) {
                return;   // no error - silently skip a null file
            }
            sz = (size_t)len;
            fp = fopen(cp,"rb");
            if (!fp) {
                sprintf( EndBuf(glpError),
		    	    "ERROR: Invalid -U switch! Failed to 'open' file '%s'!"PRTTERM, cp );
                return;
            }
            pb = (char *)MALLOC( LPTR, sz + 2 ); // allocate for INPUT file
            CHKMEM(pb);
            read = fread(pb,1,sz,fp);
            fclose(fp);
            if (read != sz) {
                MFREE(pb);
                sprintf( EndBuf(glpError),
                    "ERROR: Invalid -U switch! Failed to 'open' file '%s'!"PRTTERM, cp );
                return;
            }
            for(i = 0; i < read; i++) {
                int c = pb[i];
                if( c > ' ' ) {
                    char * pc = &pb[i];
                    sz = 1;
                    i++;
                    for(; i < read; i++) {
                        c = pb[len];
                        if( c < ' ' ) {
                            break;
                        }
                        sz++;
                    }
                    pb[i] = 0;
                    while(sz) {
                        if( pc[sz] > ' ' )
                            break;
                        pc[sz] = 0;
                        sz--;
                    }
                    if(sz) {
                        Add2Files( pWS, pc ); // PLE ph = &g_sFileList, count in g_nFileCnt
                        // ****************************
                    }
                }
            }   // process the buffer
            MFREE(pb);
        } else {
            goto Error_U;
        }
    } else {
Error_U:
      sprintf( EndBuf(glpError),
				"ERROR: Invalid -U switch! Must be followed by a valid input file!"PRTTERM );
    }
}


int   gbDirList = 0;
int   gbDirNorm = 0; // FIX20060729 - add -dn for 'normal' list, as per DIR command

// #ifdef ADD_DIRLIST_SORT2
int   gbOrdList = 0; // 1=order date 2=order size
// #endif // #ifdef ADD_DIRLIST_SORT2

char * pszFrontPage[] = {
   { "_derived" },
   { "_private" },
   { "_vti_cnf" },
   { "_vti_pvt" },
   { 0 }
};

void  Add_FP_Directories( void )
{
   char *  * ps = &pszFrontPage[0];
   while(*ps) {
      add2list( &g_sExclListD, *ps );
      g_dwExclDCnt++;
      ps++;
   }
}

// FIX20071006 - add -x::: to excluded CVS and SVN directories
char * pszCVSSVN[] = {
   { "CVS" },
   { ".svn" },
   { ".git" },  // 20201007 - Added GIT
   { 0 }
};


// FIX20071006 - add -x::: to excluded CVS and SVN directories
void  Add_CVSSVN_Directories( void )
{
   char *  * ps = &pszCVSSVN[0];
   while(*ps) {
      add2list( &g_sExclListD, *ps );
      g_dwExclDCnt++;
      ps++;
   }
}

// case 'X':	// Exclude list, either ; separate list, or @input file
void  Do_X_Switch( WS, char * cp )
{
   char * lpb;
   uint32_t dwi, dwj, dwk;
   char  c;
#ifdef USE_EXCLUDE_LIST
   if( *cp ==':' ) { // we have -X:directory
		cp++; // move up to first directory
      lpb = g_szTmpBuf2;
      c = *cp;
      dwk = 0;
      if( c == ':' ) { // we have -X::
   		cp++; // move past second
         if( *cp ) {
            if( *cp == ':' ) {
               // FIX20071006 - add -x::: to excluded CVS and SVN directories
   		      cp++; // move past second
               if( *cp ) {
                  strcat( glpError, "ERROR: -X::: must not be followed by anything!"PRTTERM );
               } else {
                  Add_CVSSVN_Directories();
               }
               return;
            }
            strcat( glpError, "ERROR: -X:: can only be followed by another :!"PRTTERM );
         } else {
            Add_FP_Directories();
         }
         return;
      } else if( c == '@' ) { // FIX20071010
           // we have an EXCLUDE DIRECTORY INPUT file to read
		   cp++;
		   strcpy( &gslfExclude.lf_szNm[0], cp );
		   gslfExclude.lf_dwSz = 0;
         lpb = 0;
		   if( LoadFile2( pWS, &gslfExclude, FALSE ) ) // have an EXCLUDE DIRECTORY INPUT file to read
            lpb = gslfExclude.lf_lpBuf;
         if(lpb)
		   {
			   dwj = gslfExclude.lf_dwSz; 
			   if(dwj)
			   {
				   dwk = 0;    // remove all extranious data
				   for( dwi = 0; dwi < dwj; dwi++ )
				   {
					   c = lpb[dwi];  // get file character
					   if( c > ' ' )
					   {
						   lpb[dwk++] = c;   // save to new location
						   dwi++;   // bump to next and process until end
						   for( ; dwi < dwj; dwi++ )
						   {
							   c = lpb[dwi];
							   if( c < ' ' )
							   {
								   lpb[dwk++] = 0;   // zero terminate
   #ifdef USE_EXCLUDE_LIST
                           add2list( &g_sExclListD, lpb );
                           g_dwExclDCnt++; // bump the EXCLUDE count
                           dwk = 0;
   #endif // #ifdef USE_EXCLUDE_LIST
								   break;
                        } else {
      						   lpb[dwk++] = c;   // save to potentially new location
                        }
						   }
						   lpb[dwk] = 0;  // double zero terminate
					   }
				   }
				   lpb[dwk] = 0;  // ensure last is double zero terminated
   #ifdef USE_EXCLUDE_LIST
               if(dwk) {
                  add2list( &g_sExclListD, lpb );
                  g_dwExclDCnt++; // bump the EXCLUDE count
                  dwk = 0;
               }
   			   KillFile2( pWS, &gslfExclude );
   #endif // #ifdef USE_EXCLUDE_LIST
			   }
		   }
		   else
		   {
			   // FAILED
			   KillFile2( pWS, &gslfExclude );
			   sprintf( EndBuf(glpError),
				   "ERROR: Unable to OPEN input [%s] file!"PRTTERM,
				   cp );
		   }
         return;

      }
      while(c) {
         if(c == ';') {
            lpb[dwk] = 0; // zero terminate
            if(dwk) {
               add2list( &g_sExclListD, lpb );
               g_dwExclDCnt++;
            }
            dwk = 0;
         } else {
            lpb[dwk++] = c;
         }
         cp++;
         c = *cp;
      }
      // catch the last
      lpb[dwk] = 0; // zero terminate
      if(dwk) {
         add2list( &g_sExclListD, lpb );
         g_dwExclDCnt++;
      }
      dwk = 0;
   } else 
#endif // #ifdef USE_EXCLUDE_LIST
	if( *cp == '@' )
	{
		// we have an INPUT file list of EXCLUDES
		cp++;
		strcpy( &gslfExclude.lf_szNm[0], cp );
		gslfExclude.lf_dwSz = 0;
      lpb = 0;
		if( LoadFile2( pWS, &gslfExclude, FALSE ) ) //  have an INPUT file list of EXCLUDES
         lpb = gslfExclude.lf_lpBuf;
      if(lpb)
		{
			dwj = gslfExclude.lf_dwSz; 
			if(dwj)
			{
				dwk = 0;    // remove all extranious data
				for( dwi = 0; dwi < dwj; dwi++ )
				{
					c = lpb[dwi];  // get file character
					if( c > ' ' )
					{
						lpb[dwk++] = c;   // save to new location
						dwi++;   // bump to next and process until end
						for( ; dwi < dwj; dwi++ )
						{
							c = lpb[dwi];
							if( c < ' ' )
							{
								lpb[dwk++] = 0;   // zero terminate
#ifdef USE_EXCLUDE_LIST
                        add2list( &g_sExclList, lpb );
                        gnExclLen++; // bump the EXCLUDE count
                        dwk = 0;
#endif // #ifdef USE_EXCLUDE_LIST
								break;
                     } else {
      						lpb[dwk++] = c;   // save to potentially new location
                     }
						}
						lpb[dwk] = 0;  // double zero terminate
					}
				}
				lpb[dwk] = 0;  // ensure last is double zero terminated
#ifdef USE_EXCLUDE_LIST
            if(dwk) {
               add2list( &g_sExclList, lpb );
               gnExclLen++; // bump the EXCLUDE count
               dwk = 0;
            }
   			KillFile2( pWS, &gslfExclude );
#endif // #ifdef USE_EXCLUDE_LIST
			}
		}
		else
		{
			// FAILED
			KillFile2( pWS, &gslfExclude );
			sprintf( EndBuf(glpError),
				"ERROR: Unable to OPEN input [%s] file!"PRTTERM,
				cp );
		}
	}
	else
	{
		// assume a ';' separated list
#ifdef USE_EXCLUDE_LIST
      char * pt = g_szTmpBuf2;
      uint32_t dwn = 0;
      PLE   ph = &g_sExclList;
      // EXCLUDE File string(s) LIST g_sExclList // list of EXCLUDED files
		dwj = strlen( cp );
		for( dwi = 0; dwi < dwj; dwi++ ) {
			c = cp[dwi];
         if( c == ';' ) { // reached end of this item
            pt[dwn] = 0; // zero terminate
            if(dwn) { // if we have LENGTH
               add2list( ph, pt );
               gnExclLen++; // bump the EXCLUDE count
            }
            dwn = 0;
         } else {
            pt[dwn++] = c;
         }
      }
      // catch last
      if(dwn) {
         pt[dwn] = 0;
         add2list( ph, pt );
         gnExclLen++; // bump the EXCLUDE count
      }
#else // !#ifdef USE_EXCLUDE_LIST
		dwj = strlen( cp );
		for( dwi = 0; dwi < dwj; dwi++ )
		{
			c = cp[dwi];
			if( ( c > ' ' ) &&
				( c != ';' ) )
			{
				if( gnExclLen < MXFILSTGS )
				{
					gszExclude[gnExclLen++] = c;
					dwi++;
					for( ; dwi < dwj; dwi++ )
					{
						c = cp[dwi];
						if( c == ';' )
						{
							gszExclude[gnExclLen++] = 0;
							break;
						}
						else if( gnExclLen < MXFILSTGS )
						{
							gszExclude[gnExclLen++] = c;
						}
						else
						{
							fOK = FALSE;
							break;
						}
					}
					if( c != ';' )
						gszExclude[gnExclLen++] = 0;
				}
				else
				{
					fOK = FALSE;
				}
				if( !fOK )
				{
					strcat( glpError, "ERROR: Exclude Name strings appears too long!"PRTTERM
						"Suggest the list be placed in a file, say FileList,"PRTTERM
						"and then use -x@FileList."PRTTERM );
					break;
				}
			}
			gszExclude[gnExclLen] = 0;
			gszExclude[gnExclLen+1] = 0;
		}
#endif // #ifdef USE_EXCLUDE_LIST
	}
}

// handles finding files in a DIRECTORY list file
// -d[nlo[d|s]]
// FIX20060729 - add -dn for 'normal' list, as per DIR command (gbDirNorm)

void  Do_D_Switch( WS, char * cp )
{
   int c = toupper(*cp);
   int bad = 0;
   int neg = 0;
   char * bgncp = cp;
   if(c) { // check the command
      do {
         switch(c)
         {
         case 'L':
            gbDirList = 1;
            bad = 0;
            break;
         case 'N':
            gbDirNorm = 1;
            break;

#ifdef ADD_DIRLIST_SORT2
         case 'O':
            gbOrdList = od_dateup; // set DATE ORDER (default)
            bad = 0;
            c = toupper(cp[1]);
            if(c) {
               cp++; // bump to next
               if( c == '-' ) { // FIX20070114 - add like -do-d
                  neg = 1;
                  cp++; // bump to next
                  c = toupper(cp[1]);
               }
               if( c == 'D' ){
                  if(neg)
                     gbOrdList = od_datedown;
                  else
                     gbOrdList = od_dateup;
               } else if ( c == 'S') { // if SIZE
                  if(neg)
                     gbOrdList = od_sizedown;
                  else
                     gbOrdList = od_sizeup;
               } else {
                  bad = 1;
               }
            }
            break;
#endif // #ifdef ADD_DIRLIST_SORT2
         default:
            bad = 1;
            break;
         }

         if(bad) {   // FIX20070114 - add like -do-d
   		   sprintf( EndBuf(glpError),
				   "ERROR: Invalid -D switch! [%s]! Only -D[N][LO[[-]DS]], for listing, order!"PRTTERM,
				   bgncp );
            return;
         }
         cp++; // bump to next letter
         c = toupper(*cp); // get next
      } while ( c );
   }

   gfDirLst = TRUE;
}

int IsAllNum( char * str )
{
   uint32_t len = strlen(str);
   if(len) {
      uint32_t dwi;
      for(dwi = 0; dwi < len; dwi++)
      {
         if( !ISNUM(str[dwi]) )
            return 0;
      }
      return 1;
   }
   return 0;
}

#ifdef   ADDLOPTION
//#define  g_bGotLOpt     GW.ws_bGotLOpt    // FIX20011011 - Add -L[nn|F] option
//#define  g_bGotLFunc    GW.ws_bGotLFunc
//#define  g_dwLOptLen    GW.ws_dwLOptLen
//            case 'L':
//               g_bGotLOpt = TRUE;
//   				cp++;		// already bumped past the L
//               if(*cp)
//void  GetLOpt( WS, char * cp )
// FIX20120408 - An LL0 should DISABLE any line wrap
void  Do_L_Switch( WS, char * cp )
{
   uint32_t    dwi = 0;
   uint32_t    dwj;
   char *   bgn = cp - 1;
   if(strlen(cp)) {
      if( toupper(*cp) == 'F' )
      {
         g_bGotLOpt = TRUE;
         g_bGotLFunc = TRUE;
      }
      else if( toupper(*cp) == 'W' )
      {
         cp++;
         if( IsAllNum(cp) && (sscanf(cp, "%u", &dwi) == 1 ))
         {
            //	gdwMax1Line = MX1LINE;
	         // gdwEnd1Line = MXELINE;
             // FIX20061106 - allow -lw0 to be MAX
             if ( dwi == 0 )
                 dwi = 1024;    // 0 = MAX
            gdwMax1Line = dwi;
            gdwEnd1Line = dwi + 40;
            if( (dwi < 32  ) ||
                (dwi > 1024) )
            {
               sprintf( EndBuf(&g_cErrBuf[0]), "ERROR: Invalid -LWnnn switch!"MEOR
                  "Min is 32 and Max is 1024 - got [%s]"MEOR,
                  (cp - 3) );
            }
         }
         else
         {
            sprintf( EndBuf(&g_cErrBuf[0]), "ERROR: Invalid -LWnnn switch! [%s]"MEOR,
               (cp - 3) );
         }
      }
      else if( toupper(*cp) == 'L' )
      {
         // FIX20091229 - Add -LL[nn[:nn]] option to LIMIT LINE OUTPUT
         cp++;
         g_bGotLOpt = TRUE;
         g_bGotLLimit = TRUE;
         if (*cp) {
             if( IsAllNum(cp) && (sscanf(cp, "%u", &dwi) == 1 )) {
                 g_dwLineLimit = dwi;
                 g_dwLLLeft = dwi / 2;
                 g_dwLLRight = dwi / 2;
             } else if ( sscanf(cp,"%u:%u", &dwi, &dwj) == 2 ) {
                 g_dwLineLimit = dwi + dwj;
                 g_dwLLLeft = dwi;
                 g_dwLLRight = dwj;
             } else {
                 sprintf( EndBuf(&g_cErrBuf[0]), "ERROR: Invalid -LLnn[:nn] switch! [%s]"MEOR,
                   bgn );
             }
         } else {
             g_dwLineLimit = DEF_LLIMIT;
            g_dwLLLeft  = DEF_LLIMIT / 2;
            g_dwLLRight = DEF_LLIMIT / 2;
         }
      }
      else
      {
         g_bGotLOpt = TRUE;
         if( IsAllNum(cp) && (sscanf(cp, "%u", &dwi) == 1 ))
         {
            g_dwLOptLen = dwi;
         }
         else
         {
            sprintf( EndBuf(&g_cErrBuf[0]), "ERROR: Invalid -L switch! [%s]"MEOR,
               (cp - 2) );
         }
      }
   } else {
      // just -L with nothing ...
      g_bGotLOpt = TRUE;
   }
}

#endif   // ADDLOPTION

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ProcessArgs
// Return type: int 
// Arguments  : LPWORKSTR pWS
//            : int argc
//            : char **argp
//            : uint32_t level
// Description: COMMAND LINE PROCESSING
//              
///////////////////////////////////////////////////////////////////////////////
int	ProcessArgs( LPWORKSTR pWS, int argc, char **argp, uint32_t level )
{
	static int	   bHadErr = FALSE;
	int		i, j, k;
	char *	cp;
	char		c, nc;
//	uint32_t	   dwi, dwj, dwk;
#ifndef	USERDBUF
	char		cc, dd;
	uint32_t	   dwl;
#endif	/* !USERDBUF */
//	char *	lpb;
	int	   fOK;

    if( argc > 1 ) {
		Check4Verb( pWS, argc, argp );
		Check4Help( pWS, argc, argp );
		fOK = TRUE;
		for( i = 1; i < argc; i++ )
		{
            cp = argp[i];
			if( cp == 0 )
				continue;
			c = *cp;
			if( c == 0 )
				continue;
			j = strlen(cp);
            nc = (j ? cp[1] : 0);
			// Keep a COPY of the command
            Add2CmdBuf( pWS, cp, j );
            if (strcmp(cp,"--version") == 0) {
                show_version(pWS);
            }
            if( ( c == '\\' ) && (( nc == '-' )||( nc == '/')) ) { // FIX20091221
                // we have an ESCAPED switch value, of a find or file value
                cp++;   // Bump past this ESCAPE char
                c = 0;  // clear the current to get to file or folder
            }
			if( ( c == '-' ) || ( c == '/' ) )
			{
				// Got a SWITCH
				// ============
				cp++;		// Bump past switch
				c = (char)toupper( *cp++ );	// Get next and bump
				switch( c )
				{

#ifdef WIN32
#ifdef   FIX20000902 // FIX input files in input file and add -8
            case '8':
               gbDOSNm = TRUE;
               break;
#endif // FIX20000902 // FIX input files in input file and add -8
#endif // WIN32
				case '?':
					Usage( pWS, 0 );
					break;

				case 'B':
               Do_B_Switch( cp );
					break;

				case 'C':
               Do_C_Switch( pWS, cp );
					break;

#ifdef		ADDDIRLST	// Special for Directory Listing
				case 'D':	// Directory Listing Search.
					//gfDirLst = TRUE;
               Do_D_Switch( pWS, cp );
					break;
#endif	// ADDDIRLST

#ifdef		ADDERRLST	// Special for Directory Listing
				case 'E':	// WIN32 Error.
					GetErrNum( pWS, cp );
					break;
#endif	// ADDERRLST

				case 'F':
					if( !Add2Finds( pWS, cp, TRUE ) ) {
						cp -= 2; // back up to beginning of switch
						sprintf( EndBuf(glpError),
							"ERROR: Invalid FIND switch! [%s]"PRTTERM,
							cp );
                    }
					break;

				case 'H':
					Usage( pWS, 0 );
					break;

#ifdef	ADDINHIB
				case 'I':
					//fInhibit = TRUE;
					//if( !GetInhibs( pWS, cp ) )
					if( !Do_I_Switch( pWS, cp ) )
						SetInvSwitch( pWS, cp );
					break;
#endif	// ADDINHIB
#ifdef   ADDLOPTION
//#define  g_bGotLOpt     GW.ws_bGotLOpt    // FIX20011011 - Add -L[nn|F] option
//#define  g_bGotLFunc    GW.ws_bGotLFunc
//#define  g_dwLOptLen    GW.ws_dwLOptLen
            case 'L':
//             g_bGotLOpt = TRUE;
//   				cp++;		// already bumped past the L
               //if(*cp)
               Do_L_Switch( pWS, cp);
               break;
#endif   // ADDLOPTION
            case 'M':   // just like -F:M switch
               g_bDoMake = TRUE;
//   				cp++;		// already bumped past the M
               if( *cp )
               {
                  if( *cp == '2' )
                     g_bDoMake2 = TRUE;
                  else
                  {
                     cp -= 2;
   						sprintf( EndBuf(glpError),
	   						"ERROR: Invalid -M2 switch! [%s]"PRTTERM,
		   					cp );
                  }
               }
               break;

				case 'N':
                    if (*cp) {
                        c = toupper(*cp++);
                        if (*cp) {
                            goto Bad_N_Switch;
                        } else if (c == 'S') {
                            g_bNoStats = TRUE;
                        } else if (c == 'F') {
                            g_bNoFile = TRUE;
                        } else {
 Bad_N_Switch:
                            cp -= 2;
       						sprintf( EndBuf(glpError),
	       						"ERROR: Invalid -N or -NS (no stats) or -NF (no file) switch! [%s]"PRTTERM, cp );
                        }
                    } else
                        gfNumber = TRUE;
					break;

#ifdef	ADDOUTFIL
				case 'O':
					if( !MoveFileName( pWS, glpOutFile, cp ) )
					{
						cp--;
						cp--;
						sprintf( EndBuf(glpError),
							"ERROR: Invalid switch! [%s]"PRTTERM,
							cp );
					}
					break;
#endif	// ADDOUTFIL

				case 'P':
					gfParity = TRUE;
					break;

#ifdef	ADDRECUR
				case 'R':
					gfRecursive = TRUE;
					break;
#endif	// ADDRECUR
                case 'S': // FIX20120407 - Add -S
                    gfStrip = TRUE;
                    break;
            case 'U':   // FIX20080320 - added this switch
               Do_U_Switch( pWS, cp );  // list of files to search
               break;

				case 'V':
// FIX20010319 -v4 adds date order of finds at end
					k = 1;
					if( *cp )
					{
						k = 0;
						while( ( c = *cp++ ) != 0 )
						{
							if( ( c >= '0' ) && ( c <= '9' ) )
							{
								k = ( k * 10 ) + ( c - '0' );
							}
							else
							{
								strcat( glpError, "ERROR: Invalid -V[n] switch!"PRTTERM );
								break;
							}
						}
					}
					gfVerbose = k;
					if( VERB6 )
						EnableDiagFile();

					break;

				case 'W':
					gfWhole = TRUE;
               // FIX20010413 - add -w2 for space whole
               if( *cp ) {
                  if( *cp == '2' ) {
                     g_fWhole2 = TRUE;
                  } else {
							strcat( glpError, "ERROR: Invalid -W[2] switch!"PRTTERM );
							break;
                  }
               }
					break;

				case 'X':	// Exclude list, either ; separate list, or @input file
               Do_X_Switch( pWS, cp );
					break;

				default:
					strcat( glpError, "ERROR: Unknown switch!"PRTTERM );
					break;

				}
			}
			else if( c == '@' )
			{
         	// *** FIRSTS *** INPUT file command ...
				if( giFirst == 0 )
				{
#ifdef   FIX20000902 // FIX input files in input file and add -8
                    cp++;
                    DoAnINP( pWS, cp, level );
#endif // FIX20000902 y/n // FIX input files in input file and add -8

				// } else if( giFirst == 1 ) {
                } else { // FIX20051107 allow multiple file arguments
                    // we already have our FIRST "find" argument
					giFirst++;  // bump again
//#ifdef   FIX20000902 // FIX input files in input file and add -8
               // FIX20080320 - restore this action, NOT a findfile, but filelist
               // also added -uListFile to do similar list action
                    cp++;
                    DoAnINP( pWS, cp, level );
//#else // !FIX20000902 // FIX input files in input file and add -8
//					Add2Files( pWS, cp ); // PLE ph = &g_sFileList, count in g_nFileCnt
//#endif   // FIX20000902
				}
                // END dealing with else if( c == '@' )
            } else {	// NO SWITCH and NO '@'
				if( giFirst == 0 )
				{
					// First NON-SWITCHED
					// is ASSUMED to be the FIND
					giFirst++;
					Add2Finds( pWS, cp, FALSE );
				// } else if( giFirst == 1 ) {
				} else { // FIX20051107 allow multiple files on command line
					giFirst++;
					// Second NON-SWITCHED
					// is ASSUMED to be the FILE(S)
                    // ****************************
					Add2Files( pWS, cp ); // PLE ph = &g_sFileList, count in g_nFileCnt
                    // ****************************
				}
			}
            if( *glpError )
            {
                bHadErr = TRUE;
                break;
            }
		}	// for argc count
	}   // 	end if( argc > 1 )
	return bHadErr;
}

// Fa4Help.c
