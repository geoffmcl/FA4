/*\
 * Fa4.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
\*/
// ==============================================================
// FA4.c
// Windows 95/98 NT/2000 and ... 32-bit implementation of FA3.ASM
//
// Main function is 	Process_Finds( pWS ); This processes the list and
// calls DoThisFile( pWS, lpf );, which in turn calls Find_In_File( pWS );
// Here if it is NOT in ( !InExclude( pWS, glpActive ) ) )
// the TYPE will be set by SetFileType( lpInFil, &W.ws_FileType );  
// setting W.FINDINTYPE = &Find_In_Type2; or &Find_In_Type1;, then
// after if( GetFileMap( pWS ) ) calls (*FindInType) ( pWS, lpInFil );
// but both types call 	Find_In_Gen( pWS, lpInF );  ***THE MAIN PROCESS***
//
// NOTE: Line count was set by if( d == 0x0d ) and end of line by
// if( pd == 0x0a ), but this is BAD for UNIX files!!! Perhaps add -U switch,
// or ALWAYS assume a new line on an 0x0a char!!!
//
// In Find_In_Gen() it ONLY looks for the FIRST characters. If found then
// if( FullComp( pWS, lpc, dwi, dwfs, lpd, flen, &dwl ) ) check for the
// whole FIND string. IF it is SUCCESSFUL then
//
// ShowLine( pWS, dwi, dwfs, &lpoff[0], &dwl ); is called to OUPUT the
// find.
//
// Fa4Help.c for int ProcessArgs( LPWORKSTR pWS, int argc, char **argp, uint32_t level )
//
// History: Note it grows UPWARDS
//
// Mar, 2001 - Fix finding in UNIX files. That is with ONLY a LF (0x0a) [^J]
// Mar, 2001 - Add -V4 will add a list of FINDS in DAT ORDER as last output
// Oct. 2000 - Some error with finding "'@'" !!!
// Sep. 2000 - What about an @inputfile that contains an @inpufile2 ???
// 1999 January - Add a -D Directory Listing Search
// 1998 October - Moved it back into a SINGLE FA4 source
// 1997 August 3 - Commenced using DC4.c source, just adding
//					a FA4 switch.
// 1997 July 18 - Commenced 32-bit version.
// 1987 October, 87 - Commenced 16-bit ASM version on which
//		this is based.
//
// Brief Description
//
// Simple Use: FA4 "text" *.c
//
//	WAS - the "text" will be first placed in gszFindStgs[] buffer,
//		and later moved to a memory buffer glpFindMem,
//		as 0 terminated string(s), ending with double 00.
// NOW - the "text" will be added to g_sFind (double linked list)
// and g_dwFCnt bumped - July 2001 - FIX20010703
//
//  The "*.c" will be first placed in gszFileStgs[] buffer,
//		and later moved to a memory buffer glpFileMem,
//		as 0 terminated string(s).
//
//  Then the glpFileMem will be processed 1 by 1, expanding
//		the name list if, like this sample is a WILD char string,
//		and EACH item of the Find String(s) will be located
//		in a memory mapped file image.
//
// Geoff R. McLane - MailTo: reports _AT_ geoffair _DOT_ info
//
// ==============================================================
#include	"Fa4.h"		// All incusive include
extern void	ShowLine( WS, uint32_t dwoff, uint32_t dwmax, uint32_t *pdw, uint32_t *pln );
extern int gbDirList;
extern int gbDirNorm;
// #ifdef ADD_DIRLIST_SORT2
extern int gbOrdList;
// #endif // #ifdef ADD_DIRLIST_SORT2

#undef   ADDDBL2  // FIX20001022 - fix find of "'@'" - and REMOVE this "doubling"
//#define  MINFNSP           40
#define  MINFNSP   48   // FIX20050212 -r switch fix

// #define	USERDBUF
extern	int	iRetVal;
int g_bNoNewLine = 0;
// WIN32_FIND_DATA   g_sFD;

typedef enum {   // date styles
   ds_english,
   ds_american
}DateStyle;

// *************************************************
#ifdef	ADDERRLST
#ifdef WIN32
extern	void	OutWin32Err( long lFE );
#endif // WIN32
extern void		outvals( void );

#endif	// ADDERRLST
#ifdef   ADDCVSDATE
extern   int  Add2Finds( WS, char * lps, int bFSwitch );
extern   void	Add2Files( WS, char * lps );
#endif   //#ifdef   ADDCVSDATE

// *************************************************
// HISTORY - VERSION - DATE - see Fa4Vers.h
// *************************************************

#ifdef	ADDDIAGT
#ifdef WIN32
#pragma message( "NOTE: Writing diagnostic file." )
#endif // WIN32
extern	void	CloseDiagFile( void );
extern	void	WriteDiagFile( char * lps );
// Does the SAME as prt() - Ensure Cr/Lf pairs on output,
// except unlike prt() does NOT ensure Cr/Lf at END OF STRING!
extern	void	dout( char * lps );

#endif	/* ADDDIAGT */

extern	void	DisableDiagFile( void );
extern	void	EnableDiagFile( void );

typedef struct {
	uint32_t	er_val;
	char *	er_ptr;
	int	er_flag;
}ERRLIST, * LPERRLIST;

//#define		PRTTERM		"\n"

// locals

// LoadFile2 and KillFile2 structure
// =================================
//typedef	struct {
//	char	lf_szNm[MAX_PATH];
//	HANDLE	lf_hHnd;
//	uint32_t	lf_dwSz;
//	uint32_t	lf_dwRd;
//	char *	lf_lpBuf;
//}LFSTR;
//typedef LFSTR FAR * LPLFSTR;
//
//typedef struct {
//	char	fs_szNm[MAX_PATH];
//	HANDLE	fs_hHnd;
//	uint32_t	fs_dwLow;
//	uint32_t	fs_dwHigh;
//	LPVOID	fs_lpV;
//	uint32_t	fs_dwRd;
//}GFS;
//typedef GFS FAR * LPGFS;

int	OpenReadFile( char * lpf, HANDLE * ph );

// jmp_buf mark;		// Address for long jump to jump to
int		iCritErr;	// Program EXIT value
int		iMainRet;
LPWORKSTR	lpWS = 0;   // single/sole allocated work structure

ERRLIST	ErrList[] = {
	{ ERR_N0001, "ERROR: No command line found!"PRTTERM, TRUE },
	{ ERR_N0002, "ERROR: Unknown switch character!"PRTTERM, TRUE },
	{ ERR_N0003, "ERROR: Too many directories given!"PRTTERM, TRUE },
	{ ERR_N0004, "ERROR: Invalid Verbose (-v) value!"PRTTERM, TRUE },
	{ ERR_N0005, "ERROR: Failed to COMPILE regex!"PRTTERM, TRUE },
	{ 0,         0, 0 }
};

char	szHdr[]  = CNAME " *** " PNAME " *** " PVERS " - " PDATE ""PRTTERM ;
char	szComm[] = "Command=[ ";
char	szCom2[] = "]"PRTTERM;
// FIX20071006 - add -x::: to excluded CVS and SVN directories

char  sz_Tail[] = "Compiled using MSVC v.%u on " __DATE__ " at " __TIME__;

// ENGLISH
char	szEVLabel[] = "Volume in drive ";	// C is IPNS-EAST
char	szESerial[] = "Volume Serial Number is "; // 359A-0A25
char	szEDirect[] = "Directory of "; // C:\GEOFF
// FRENCH
char	szFVLabel[] = "Le volume dans le lecteur ";	// L est HDD4
char	szFDirect[] = "Répertoire de "; // L:\DISNEY

// Forward References
void	OutHeader( WS );
void	Cleanup1( WS );
void	Pgm_Exit( WS );	// ALL should come here to exit
void	Error_Exit( WS, int val );
void	OutCmds( WS );
void	PrtErr( WS, uint32_t Err );
void	Find_In_Type1( WS, char *lpInFil ); // FINDINTYPE
void	Find_In_Type2( WS, char *lpInFil ); // FINDINTYPE
int	    GetFileMap( WS );
void	ShutFileMap( WS );
#ifdef WIN32
void	ShowFail( WS, int i, PWIN32_FIND_DATA pfd );
#endif // WIN32
void	ShowZero( WS );
void	ShowNoMap( WS );
// void	DoThisFile( WS, char * lpf, int bFlg ); //FIX20140830: now abandonned
int	IsEVLabel( char * lpv );
int	IsFVLabel( char * lpv );
int	IsEDirect( char * lpv );
int	IsFDirect( char * lpv );
char	getupchar( char * lpc );
void	DoLineChk( WS, char * lpb, uint32_t dwLnBgn, uint32_t dwi );
int		HasStg( WS, char * pStg, char * pCmp );

char g_szFailMsg[] = "WARNING: Failed to %s [%s]";
// #define CHKMEM(a) if( !a ) { prt("C:ERROR: MEMORY FAILED!"MEOR ); exit(-1); }

uint32_t g_dwDoneCnt = 0;
uint32_t g_dwExclCnt = 0;
uint32_t g_dwMapFailedCnt = 0;
uint32_t g_dwTotFinds = 0; // += dwFinds;
uint32_t g_dwItems; // items being found
uint32_t	g_dwFinds; // per file find COUNT
uint32_t dwFind1;
uint32_t g_dwFoundInFiles = 0;   // FIX20081003 - show found in files count at end

uint32_t	   dwLastLn, dwLnBgn;

void DO_OUT( char * pb )
{
   // add2outlist(pb); // add to an ouput list, aynway

#ifdef   ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   if( g_dwMinCnt > 1 ) {
         addprt( pb );
   } else
#endif   // ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   {
       if( g_ioHold ) {
           add2outlist( pb ); // only ADD it at this stage
       } else { // out it NOW
           prt( pb );
       }
   }
}

// FIX20120809 - Add output of formatted system message when open file FAILED
void OutErrorMsg( uint32_t err, char * msg, char * lpf )
{
#ifdef WIN32
    void * pMsgBuf = 0;
    uint32_t len = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err, 	// Results of GetLastError()
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (char *) &pMsgBuf,
        0,
        NULL );
    if( pMsgBuf ) {
        // show the string.
        char * ps = (char *)pMsgBuf;
        while (len--) {
            if (ps[len] > ' ')
                break;
            ps[len] = 0;
        }
        sprtf( "%s: File [%s]"MEOR"Ind: %s"MEOR, msg, lpf, pMsgBuf );
        LocalFree( pMsgBuf );
    } else {
        sprtf( "%s: File [%s] Error %d"MEOR, msg, lpf, err );
    }
#else
    sprtf( "%s: File [%s] Error %d"MEOR, msg, lpf, err );
#endif
}

int iLen_ok( int iLen )
{
   if(( iLen > ( sizeof(szEDirect)-1 ) ) ||
      ( iLen > ( sizeof(szFDirect)-1 ) ) ) {
      return 1;
   }
   return 0;
}

void Show_Found_Nums( void )
{
   LPWORKSTR pWS = lpWS;   // single/sole allocated work structure
   char * nb;
   // FIX20070115 - minor adjustments of the output
   if( g_dwFoundDirsCnt ) {
      sprintf(lpVerb, "Processed %d files, %d directories",
         g_dwFoundFileCnt,
         g_dwFoundDirsCnt );
   } else {
      if( g_dwFoundFileCnt == 1 ) {
         strcpy(lpVerb, "Processed 1 file");
      } else {
         sprintf(lpVerb, "Processed %d files", g_dwFoundFileCnt );
      }
   }
   if( g_dwExclCnt || g_dwDirsExcl) { // FIX20051127
      sprintf(EndBuf(lpVerb),", eXcl=%d:%d", g_dwExclCnt, g_dwDirsExcl );
   }

   if( g_dwFoundRejCnt ) { // FIX20050720
      // processed less than ALL
      sprintf(EndBuf(lpVerb),", rejected %d", g_dwFoundRejCnt );
      sprintf(EndBuf(lpVerb),", done %u", // g_dwFoundMatchCnt );
                  (g_dwFoundMatchCnt - g_dwExclCnt) );
   }

   // FIX20050720 - add byte count, also added if VERB4!
   //sprintf(EndBuf(lpVerb),", %I64u bytes", g_ulTotalBytes );
   // defined g_dwFoundMatchCnt (found - rejected)
   // uint64_t g_ulTotalBytes = {0};
   // uint64_t g_ulTotalBRej  = {0};
   // FIX20070115 - minor adjustments of the output
   nb = GetNxtBuf();
   sprintf(nb,"%llu", (unsigned long long)g_ulTotalBytes );
   nb = My_NiceNumber(nb);
   sprintf(EndBuf(lpVerb),", %s bytes", nb );

   if( g_dwTotFinds ) {
      if(g_dwTotFinds == 1)
         strcat(lpVerb,", for 1 find");
      else
         sprintf(EndBuf(lpVerb),", for %s finds", My_NiceNumberStg(g_dwTotFinds) );
   } else {
      strcat(lpVerb, ", for NO finds");
   }
   if(g_dwFoundInFiles) {   // FIX20081003 - show found in files count at end
      sprintf(EndBuf(lpVerb),", in %d file%s",
         g_dwFoundInFiles,
         ((g_dwFoundInFiles > 1) ? "s" : "") );
   }

   strcat(lpVerb," ..."MEOR);
   prt(lpVerb);
}

void	chkchk( void )
{
	int	i;
	i = 0;
}


void	prt5( char * lps )
{
	LPWORKSTR	pWS;
	if( ( ( pWS = lpWS ) != 0 ) &&
		( VERB5 ) )
	{
		prt(lps);
	}
	else
	{
#ifdef	ADDDIAGT
		dout(lps);
#endif	// ADDDIAGT
	}
}


uint32_t	WriteAFile( HANDLE hf, char * lpb, uint32_t len )
{
	uint32_t	dwi;

#ifdef	WIN32
	WriteFile( hf,	// handle to file to write to
		lpb,	// pointer to data to write to file
		len,	// number of bytes to write
		&dwi,	// pointer to number of bytes written
		NULL );	// pointer to structure needed for overlapped I/O
#else	// !WIN32
	dwi = fwrite( lpb, 1, len, hf );
#endif	// WIN32 y/n
	return dwi;
}

int  MatchXHibit( char * lpf1, char * lpf2 )
{
   int  flg = FALSE;   // NOT match
   uint32_t dw1 = strlen(lpf1);
   uint32_t dw2 = strlen(lpf2);
   if( dw1 == dw2 )
   {
      unsigned char *pb1, *pb2;
      uint32_t dwc1, dwc2;
      // first condition - they are of EQUAL length
      dwc1 = 0;
      dwc2 = 1;
      for( dw1 = 0; dw1 < dw2; dw1++ )
      {
         pb1 = (unsigned char *)&lpf1[dw1];
         pb2 = (unsigned char *)&lpf2[dw1];
         dwc1 = (uint32_t) *pb1;
         dwc2 = (uint32_t) *pb2;
         if( dwc1 != dwc2 )
         {
            // we may have a difference
            if( !( ( dwc1 > 0x7f ) && ( dwc2 > 0x7f ) ) )
            {
               // one or both are NOT HI-BIT chars
               if( ( dwc1 < 0x80 ) && ( dwc2 < 0x80 ) )
               {
                  // if BOTH are below 0x80, then make sure
                  // the upper case is not equal
                  if( toupper(lpf1[dw1]) != toupper(lpf2[dw1]) )
                     break;
               }
               else
                  break;   // one hi-bit - one NOT = NOT EQUAL
            }
         }
      }
      if( dwc1 == dwc2 )
         flg = TRUE;
   }
   return flg;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : OpenReadFile
// Return type: int 
// Arguments  : char * lpf
//            : HANDLE * ph
// Description: Open a file for READING
//              
///////////////////////////////////////////////////////////////////////////////
int	OpenReadFile( char * lpf, HANDLE * ph )
{
	int		flg;

	flg = FALSE;		// SET FAILED!!!
	if( lpf && *lpf && (is_file_or_directory64(lpf) == VALID_FILE))
	{
#ifdef	WIN32
		HANDLE	hdl;
		hdl = CreateFile( lpf,	// pointer to name of the file
			GENERIC_READ,		// access (read-write) mode
			FILE_SHARE_READ,	// share mode
			NULL,				// pointer to security descriptor
			OPEN_EXISTING,		// how to create
			FILE_ATTRIBUTE_NORMAL,	// file attributes
			NULL );				// handle to file with attributes to copy
		if( VH(hdl) )
		{
			*ph = (HANDLE)hdl;
			flg = TRUE;
		}
		else
		{
             // FIX20120809 - Add output of formatted system message when open file FAILED
             OutErrorMsg(GetLastError(), "OPEN FAILED!", lpf );
             // flg = TryAllOpens( lpf, ph ); // depreciated - removed
             // if( !flg )
             // {
   			 *ph = INVALID_HANDLE_VALUE;
             // }
		}
 #else	// !WIN32
        FILE *fp = fopen(lpf,"r");
        if (fp) {
			flg = TRUE;
            *ph = fp;
		} else {
            *ph = INVALID_HANDLE_VALUE;
        }
#endif	// WIN32 y/n
	}

	return flg;
}

HANDLE	CreateAFile( char * lpf )
{
	HANDLE		hf;

	hf = INVALID_HANDLE_VALUE;
	if( ( lpf ) &&
		( *lpf ) )
	{
#ifdef	WIN32
		HANDLE	hdl;
		hdl = CreateFile( lpf,	// pointer to name of the file
			(GENERIC_READ | GENERIC_WRITE),		// access (read-write) mode
			FILE_SHARE_READ,	// share mode
			NULL,				// pointer to security descriptor
			CREATE_ALWAYS,		// how to create
			FILE_ATTRIBUTE_ARCHIVE,	// file attributes
			NULL );				// handle to file with attributes to copy
		if( VH(hdl) )
		{
			hf = (HANDLE)hdl;
		}
#else	// !WIN32
        FILE *fp = fopen(lpf, "w");
        if (fp) {
            hf = (HANDLE)fp;
        }
#endif	// WIN32 y/n
	}
	return hf;
}

void	CloseAFile( HANDLE hf )
{
	if( hf && ( hf != INVALID_HANDLE_VALUE ) )
	{
#ifdef	WIN32
		CloseHandle( (HANDLE) hf );
#else	// !WIN32
		fclose( hf );
#endif	// WIN32 y/n
	}
}

uint32_t	GetFileLen( HANDLE hf )
{
	uint32_t	dwc;
	uint32_t	dw = 0;
	if( hf && (hf != INVALID_HANDLE_VALUE) )
	{
#ifdef	WIN32
		dwc = SetFilePointer( (HANDLE) hf,	// handle of file
			0,	// number of bytes to move file pointer
			NULL,	// address of high-order word of distance to move
			FILE_CURRENT );	// how to move
		dw = SetFilePointer( (HANDLE) hf,	// handle of file
			0,	// number of bytes to move file pointer
			NULL,	// address of high-order word of distance to move
			FILE_END );	// how to move
		dwc = SetFilePointer( (HANDLE) hf,	// handle of file
			dwc,	// number of bytes to move file pointer
			NULL,	// address of high-order word of distance to move
			FILE_BEGIN );	// how to move
#else	// !WIN32
		dwc = fseek( hf, 0, SEEK_CUR );	// Get CURRENT
		dw = fseek( hf, 0, SEEK_END );	// Goto END OF FILE
		fseek( hf, dwc, SEEK_SET );	// Back to Current
#endif	// WIN32 y/n
	}
	return dw;
}

#define	MXIOB		256
void boi( char * lps )
{
	static char iobuf[MXIOB+8];
	static	int	iocnt = 0;
	char *	lpb = &iobuf[iocnt];
	int		i;

	if( ( lps              ) &&
		( ( i = strlen(lps) ) > 0 ) )
	{
		if( i < MXIOB )
		{
			if( ( iocnt + i ) < MXIOB )
			{
				if( iocnt )
					strcat( lpb, lps );
				else
					strcpy( lpb, lps );
				iocnt += i;
			}
			else
			{
				lpb = &iobuf[0];
				if( iocnt )
					prt(lpb);
				strcpy( lpb, lps );
				iocnt = i;
			}
		}
		else
		{
			if( iocnt )
				prt( &iobuf[0] );
			prt(lps);
			iocnt    = 0;
			iobuf[0] = 0;
		}
	}
	else if( iocnt )
	{
		prt( &iobuf[0] );
		iocnt    = 0;
		iobuf[0] = 0;
	}
}

#ifdef ADD_LOAD_FILE

int	LoadFile( WS, char * pDir, HANDLE * phFile,
				 uint32_t *lpDW, HGLOBAL * phGlob,
				 char * * lpFile,
				 int fExit )
{
	int	flg;

	flg = FALSE;
	if( ( pDir ) &&
		( *pDir ) &&
		( phFile ) &&
		( lpDW ) &&
		( phGlob ) &&
		( lpFile ) )
	{
		if( VERB6 )
		{
			boi(0);
			sprintf( lpVerb, 
				"Openning [%s] ... ",
				pDir );
			boi( lpVerb );
		}
		if( OpenReadFile( pDir, phFile ) )
		{
			if( VERB6 )
			{
				sprintf( lpVerb, "FH=%x ... ", *phFile );
				boi( lpVerb );
			}
			*lpDW = GetFileLen( *phFile ); // ONLY #ifdef ADD_LOAD_FILE
			if(*lpDW)
			{
				if( VERB6 )
				{
					sprintf( lpVerb,
						"Len.=%u ... ",
						*lpDW );
					boi( lpVerb );
				}
            *lpFile = 0;
            *phGlob = GlobalAlloc( GHND, (*lpDW + 16) );
            if(*phGlob)
               *lpFile = GlobalLock( *phGlob );
				if( *lpFile )
				{
					if( VERB6 )
						boi( "Reading..." );
					if( _lread( (HFILE)*phFile, *lpFile, *lpDW ) == *lpDW )
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
							pDir );
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
					pDir );
			}
		}
		else
		{
			sprintf( EndBuf(glpError),
				"\r\nERROR: Unable to OPEN file [%s]!"PRTTERM,
				pDir );
		}
	}
	else
	{
		strcat( glpError,
			"\r\nERROR: Bad internal parameters!"PRTTERM );
	}

	if( ( fExit ) &&
		( *glpError ) )
	{
		Err_Exit( -1 );
	}

	return( flg );
}
#endif // #ifdef ADD_LOAD_FILE


//typedef struct _WIN32_FIND_DATA { // wfd  
//    uint32_t dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    uint32_t    nFileSizeHigh; 
//    uint32_t    nFileSizeLow; 
//    uint32_t    dwReserved0; 
//    uint32_t    dwReserved1; 
//    char    cFileName[ MAX_PATH ]; 
//    char    cAlternateFileName[ 14 ]; 
//} WIN32_FIND_DATA; 
//typedef struct _FILETIME { // ft  
//    uint32_t dwLowDateTime; 
//    uint32_t dwHighDateTime; 
//} FILETIME; 
//The DosDateTimeToFileTime function converts MS-DOS date and time values to a
//64-bit file time. 
//int DosDateTimeToFileTime(
//    WORD wFatDate,	// 16-bit MS-DOS date 
//    WORD wFatTime,	// 16-bit MS-DOS time 
//    LPFILETIME lpFileTime 	// pointer to buffer for 64-bit file time
//   );	
//Parameters
//wFatDate
//Specifies the MS-DOS date. The date is a packed 16-bit value with the following
//format: 
//Bits	Contents
//0-4	Day of the month (1-31)
//5-8	Month (1 = January, 2 = February, and so on)
//9-15	Year offset from 1980 (add 1980 to get actual year)
//wFatTime
//Specifies the MS-DOS time. The time is a packed 16-bit value with the following
//format: 
//Bits	Contents
//0-4	Second divided by 2
//5-10	Minute (0-59)
//11-15	Hour (0-23 on a 24-hour clock)
//lpFileTime
//Points to a FILETIME structure to receive the converted 64-bit file time. 
//Return Values
//If the function succeeds, the return value is TRUE.
//If the function fails, the return value is FALSE. To get extended error
//information, call GetLastError. 
//See Also
////FILETIME, FileTimeToDosDateTime, FileTimeToSystemTime, SystemTimeToFileTime 
#ifdef WIN32
void	AddDateTime( char * lpb,		// Destination buffer
				   LPWIN32_FIND_DATA pfd,	// Information
				   int	iHour24 )	// 24-Hour time
{
	WORD	FatTime, FatDate;
   int   hr, min, sec, yr, mth, dy;
	// Add File Date / Time
	// ====================
	if( FileTimeToDosDateTime( &pfd->ftLastWriteTime, // pointer to 64-bit file time
		&FatDate,	// pointer to variable for MS-DOS date
		&FatTime ) ) // pointer to variable for MS-DOS time
	{
		hr = DFHour( FatTime );
		min = DFMins( FatTime );
		sec = DFSecs( FatTime );
		yr = DFYear( FatDate );
		mth = DFMonth( FatDate );
		dy = DFDay( FatDate );

		// Add DATE
      if( yr >= 100 ) { // FIX20050201 - 2000 bug
         int cents = yr / 100;
         int nyr = yr - (cents * 100);
         nyr += 1000 + (cents * 1000);
         sprintf( (lpb + strlen(lpb)),
            " %02d/%02d/%04d",
            mth, dy, nyr );

      } else {
         sprintf( (lpb + strlen(lpb)),
            " %02d-%02d-%02d",
            mth, dy, yr );
      }
		// Add TIME
		if( iHour24 )
		{
			sprintf( (lpb + strlen(lpb)),
				" %02d:%02d",
				hr, min );
		}
		else
		{
			int   nhr;
			char *	lpampm;

			nhr = hr;
			if( hr >= 12 )
			{
				if( hr > 12 )
					nhr = hr - 12;
				lpampm = "p";
			}
			else
			{
				lpampm = "a";
			}
			sprintf( (lpb + strlen(lpb)),
				" %2d:%02d%s",
				nhr, min, lpampm );
		}
	}
	else
	{
		strcat( lpb, " ??-??-?? ??:??" );
	}
}

void	AddDateTime2( char * lpb,		// Destination buffer
				   LPWIN32_FIND_DATA pfd,	// Information
				   int	iHour24, 	// 24-Hour time
               DateStyle amer) // american DAY/MONTH or english MONTH/DAY
{
   SYSTEMTIME st;
   if( FileTimeToSystemTime(&pfd->ftLastWriteTime, // pointer to 64-bit file time
      &st) )
   {
      if( amer == ds_american ) {
         sprintf( EndBuf(lpb),
            " %02d/%02d/%04d",
            (st.wDay & 0xffff),
            (st.wMonth & 0xffff),
            (st.wYear & 0xffff) );
      } else {
         sprintf( EndBuf(lpb),
            " %02d/%02d/%04d",
            (st.wMonth & 0xffff),
            (st.wDay & 0xffff),
            (st.wYear & 0xffff) );
      }
      if( iHour24 ) {
         sprintf( EndBuf(lpb),
            " %02d:%02d",
            (st.wHour & 0xffff),
            (st.wMinute & 0xffff) );
      } else {
         char * ampm = "AM";
         if(st.wHour >= 12) {
            //if(st.wHour > 12)
            st.wHour -= 12;
            ampm = "PM";
         }
         sprintf( EndBuf(lpb),
            " %02d:%02d %s",
            (st.wHour & 0xffff),
            (st.wMinute & 0xffff),
            ampm );
      }

   } else {
      strcat( lpb, " ??-??-?? ??:??" );
   }
}
#endif // WIN32

#ifdef WIN32
///////////////////////////////////////////////////////////////

int	AddSizeDate( char * lpb,		// Destination buffer
				LPWIN32_FIND_DATA pfd,	// Information
				int	iSizLen )
{

	PutThous( (lpb + strlen( lpb )),
		iSizLen,
		pfd->nFileSizeLow );

	strcat( lpb, " " );	// Space AFTER number

	AddDateTime( lpb, pfd, FALSE );	// Add File Date and Time

	return( strlen( lpb ) );
}

void	CopyFNA( LPWIN32_FIND_DATA lpFD )
{
	char *lps, *lpd;
	int		i, j;
	if( lpFD &&
		(lpFD->cAlternateFileName[0] == 0) &&
		(lpFD->cFileName[0]) &&
		(lpFD->dwFileAttributes != SPLFA) )	// NOTE special case
	{
		lps = &lpFD->cFileName[0];
		lpd = &lpFD->cAlternateFileName[0];
		i = strlen( lps ); 
		if(i)
		{
			if( i > (8+1+3) )
			{
				for( j = (i - 1); j >= 0; j-- )
				{
					if( lps[j] <= ' ' )
					{
						lps[j] = 0;
						i--;
					}
					else
					{
						break;
					}
				}
				if( i <= (8+1+3) )
				{
					if( i < 14 )
					{
						strcpy( lpd, lps );	// This would add 0!!!
					}
					else
					{
						for( j = 0; j < i; j++ )
							lpd[j] = lps[j];
					}
				}
			}
			else
			{
					if( i < 14 )
					{
						strcpy( lpd, lps );	// This would add 0!!!
					}
					else
					{
						for( j = 0; j < i; j++ )
							lpd[j] = lps[j];
					}
			}
		}
	}
}

// NOTE NOTE NOTE
// It SEEMS if the FindFirstFile/FindNextFile
// Finds a FILE that is ALL CAPITAL, the name of
// which it has placed in cFileName[], then
// this cAlternateFileName[] can be BLANK!!!
// so there has been a FIX has been ADDED HERE
// specifically for this BLANK CASE!!!
// See CopyFNA()
void	FixAlternate( WS, LPWIN32_FIND_DATA lpFD )
{
	int		i;
	if( lpFD->cAlternateFileName[0] == 0 )
	{
		i = strlen( &lpFD->cFileName[0] ); 
		if(i)
		{
			CopyFNA( lpFD );
		}
	}
}

int	AddFile( WS,
				LPWIN32_FIND_DATA lpFD,
				char * lptmp,
				HANDLE * phTmp,
				int * piCnt )
{
	int	flg = TRUE;
	int		wtn, iCnt;
	HANDLE	hTmpFil;

	wtn = 0;
	FixAlternate( pWS, lpFD );
	if( *phTmp == 0 )
	{
		*phTmp = CreateAFile( lptmp );
	}
	hTmpFil = *phTmp;
	if( ( hTmpFil ) &&
		( hTmpFil != INVALID_HANDLE_VALUE ) )
	{
		wtn = WriteAFile( hTmpFil, (char *)lpFD, sizeof( WIN32_FIND_DATA ) );
	}
	if( wtn == sizeof( WIN32_FIND_DATA ) )
	{
		// NOTE: THE ONLY PLACE WHERE COUNT IS INCREMENTED
		// ===============================================
		iCnt = *piCnt;
		iCnt++;
		*piCnt = iCnt;
		flg = FALSE;
	}
	return flg;
}

int	Decode( WS,
			   char * lpb, uint32_t inlen,
			   char * lptmp, HANDLE * phTmp,
			   int * pCnt, uint32_t *pdwTot )
{
	uint32_t	dwi, dwc, len, dws;
	char	c;
	WIN32_FIND_DATA	fd;
	int		k;
	int	flg, flg2;
	uint32_t	ncol, scol, dcol, tcol;
	WORD	mth, day, yr, hr, min, sec;
	WORD	wFatDate;	// 16-bit MS-DOS date
	WORD	wFatTime;	// 16-bit MS-DOS time
#ifdef	ADDDIAG2
	WORD	whr, wmin, wsec, wyr, wmth, wdy;
#endif	// ADDDIAG2
	int		iCnt;
	uint32_t	dwTot;

	iCnt = 0;
	dwc = 0;
	ncol = NMCOL;
	scol = SZCOL;
	dcol = DTCOL;
	tcol = TMCOL;
	wFatDate = 0;
	wFatTime = 0;
	//fd.cAlternateFileName[0] = 0;
	//fd.cFileName[0] = 0;
	memset( &fd, 0, sizeof( WIN32_FIND_DATA ) );
	mth = day = yr = hr = min = sec = 0;
	if( lpb &&
		(*lpb > ' ') &&
		( (len = inlen) > 0 ) )
	{
#ifdef	ADDDIAG3
			for( dwi = 0; dwi < len; dwi++ )
			{
				c = lpb[dwi];
				if( c >= ' ' )
				{
					DiagBuf[dwi] = c;
				}
				else
				{
					dwi++;
					break;
				}
			}
			DiagBuf[dwi] = 0;
			sprintf( &cVerbBuf[0],
				"%s (len = %u)",
				&DiagBuf[0],
				len );
			prt( &cVerbBuf[0] );
			cVerbBuf[0] = 0;
			DiagBuf[0] = 0;
#endif	// ADDDIAG3
		dwi = 0;		// Start CASE as line is decoded
		while( len )
		{
			switch( dwc )
			{
			case 0:	// Get NAME
				k = 0;
				flg = flg2 = FALSE;
				for( ; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( dwi >= ncol )
					{
						if( k < 14 )
							fd.cAlternateFileName[k] = 0;
						dwc++;
						break;
					}
					if( c > ' ' )
					{
						if( flg && !flg2 )
						{	// If more AFTER a space
							if( k < 14 )
								fd.cAlternateFileName[k++] = '.';
							flg2 = TRUE;
						}
						if( k < 14 )
							fd.cAlternateFileName[k++] = c;
					}
					else
					{
						flg = TRUE;
					}
					if( len )
						len--;
				}
				break;

			case 1:		// Get SIZE
				k = 0;
				dws = 0;
				flg = flg2 = FALSE;
				for( ; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( dwi >= scol )
					{
						fd.nFileSizeHigh = 0;
						fd.nFileSizeLow = dws;
						if( flg2 )
							fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
						else
							fd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
						dwc++;
						break;
					}
					if( ( c >= '0' ) && ( c <= '9' ) )
					{
						if( !flg && !flg2 )
						{
							flg = TRUE;
							dws = (c - '0');
						}
						else if( !flg2 )
						{
							dws = (dws * 10) + (c - '0');
						}
					}
					else if( c == ',' )
					{
						k += 3;	// Should be 3 more numbers
					}
					else if( c > ' ' )
					{
						if( !flg2 )
						{
							flg2 = TRUE;
							if( c != '<' )
							{
								iCritErr = GEN_ERROR;
								break;
							}
						}
					}
					if( len )
						len--;
				}
				break;

			case 2:		// Get DATE
				k = 0;
				dws = 0;
				flg = flg2 = FALSE;
				for( ; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( dwi >= dcol )
					{
						dwc++;
						break;
					}
					if( ( c >= '0' ) && ( c <= '9' ) )
					{
						if( !flg )
						{
							flg = TRUE;
							dws = (c - '0');
						}
						else if( !flg2 )
						{
							dws = (dws * 10) + (c - '0');
						}
					}
					else if( c == '-' )
					{
						flg = FALSE;
						switch( k )
						{
						case 0:
							mth = (WORD)dws;
							break;
						case 1:
							day = (WORD)dws;
							break;
						case 2:
							yr = (WORD)dws;
							break;
						}
						k++;
						flg2 = FALSE;
					}
					else if( c > ' ' )
					{
						flg2 = TRUE;
					}
					if( len )
						len--;
				}
				if( k == 2 )
				{
					yr = (WORD)dws;
				}
				break;

			case 3:		// Get TIME
				k = 0;
				dws = 0;
				flg = flg2 = FALSE;
				for( ; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( dwi >= tcol )
					{
						dwc++;
						min = (WORD)dws;
						break;
					}
					if( ( c >= '0' ) && ( c <= '9' ) )
					{
						if( !flg )
						{
							flg = TRUE;
							dws = (c - '0');
						}
						else
						{
							dws = (dws * 10) + (c - '0');
						}
					}
					else if( c == ':' )
					{
						flg = FALSE;
						if( !flg2 )
						{
							hr = (WORD)dws;
							flg2 = TRUE;
						}
						else
						{
							iCritErr = GEN_ERROR;
						}
					}
					else if( ( c == 'a' ) || ( c == 'p' ) )
					{
						if( c == 'p' )
						{
							k = 1;
							if( flg2 )
							{
								if( hr < 12 )
									hr += 12;
							}
						}
					}
					else if( c > ' ' )
					{
						iCritErr = GEN_ERROR;
						break;
					}
					else if( c < ' ' )
					{
						dwc++;
						min = (WORD)dws;
						len = 0;
						break;
					}
					if( len )
						len--;
				}	// for dwi < inlen
				if( dwi == inlen )
				{
					len = 0;
					dwc++;
				}
				break;

			case 4:		// Get LONG FILENAME
				k = 0;
				dws = 0;
				flg = flg2 = FALSE;
				for( ; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( c < ' ' )
					{
						len = 0;
						dwc++;
						break;
					}
					else
					{
						if( k )
						{
							// Add spaces, etc ...
							fd.cFileName[k++] = c;
						}
						else
						{
							// Never BEGIN with a SPACE
							if( c > ' ' )
							{
								fd.cFileName[k++] = c;
							}
						}
					}
					if( len )
						len--;
				}
				fd.cFileName[k] = 0;
				if( dwi == inlen )
				{
					len = 0;
					dwc++;
				}
				break;

			default:
				if( len )
					len--;
				c = lpb[dwi];
				if( c < ' ' )
				{
					len = 0;
				}
				break;
			}	// switch case
			if( iCritErr )
			{
				break;
			}
		}	// while len
		if( iCritErr == 0 )
		{
			// OK, we must convert date/time to FILETIME
//wFatDate
//Specifies the MS-DOS date. The date is a packed 16-bit value with the following
//format: 
//Bits	Contents
//0-4	Day of the month (1-31)
//5-8	Month (1 = January, 2 = February, and so on)
//9-15	Year offset from 1980 (add 1980 to get actual year)
//wFatTime
//Specifies the MS-DOS time. The time is a packed 16-bit value with the following
//format: 
//Bits	Contents
//0-4	Second divided by 2
//5-10	Minute (0-59)
//11-15	Hour (0-23 on a 24-hour clock)
				//wFatDate = (mth << 5) + (day & 0x0f) + ((yr+1980) << 9);
				//wFatTime = (hr << 11) + (min << 5) + ((sec / 2) & 0x0f);
			wFatTime = (WORD)DOSFT( hr, min, sec );
			wFatDate = (WORD)DOSFD( yr, mth, day );
			if( ( ( mth >= 1 ) && ( mth <= 12 ) ) &&
				( hr < 24 ) &&
				( min < 60 ) &&
				( ( day >= 1 ) && ( day <= 31 ) ) &&
				( DosDateTimeToFileTime( wFatDate,	// 16-bit MS-DOS date
					wFatTime,	// 16-bit MS-DOS time
					&fd.ftLastWriteTime ) )	) // pointer to buffer for 64-bit file time
			{
				fd.ftCreationTime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime; 
				fd.ftCreationTime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime; 
				fd.ftLastAccessTime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime; 
				fd.ftLastAccessTime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime;
				if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
#ifdef	ADDDIAG4
					if( VERBM )
					{
						prt( "ADVICE: Following Directory EXCLUDED from list!" );
						FixAlternate( pWS, &fd );
					}
#endif	// ADDDIAG4
				}
				else	// NOT a DIRECTORY
				{
					if( AddFile( pWS, &fd, lptmp, phTmp, pCnt ) )
					{
						// Failed to write to file
						iCritErr = GEN_ERROR;
						sprintf( lpVerb,
							"WARNING: Failed to add to file [%s]!",
							lptmp );
						prt( lpVerb );
						*lpVerb = 0;
					}
					else
					{
						//iCnt = *pCnt;
						iCnt++;
						//*pCnt = iCnt;	// Bump the written count
						dwTot = *pdwTot;
						dwTot += fd.nFileSizeLow;
						*pdwTot = dwTot;
					}
				}
			}	// time in range
			else
			{
				// Something is WRONG with TIME/DATE!!!
				iCritErr = GEN_ERROR;
				for( dwi = 0; dwi < inlen; dwi++ )
				{
					c = lpb[dwi];
					if( c >= ' ' )
					{
						DiagBuf[dwi] = c;
					}
					else
					{
						dwi++;
						break;
					}
				}
				DiagBuf[dwi] = 0;
				sprintf( lpVerb,
					"WARNING: Error in decoding DATE/TIME of line\r\n%s",
					&DiagBuf[0] );
				prt( lpVerb );
				DiagBuf[0] = 0;
				*lpVerb = 0;
			}
		}	// NOT iCritErr
	}	// params ok
	else	// BAD INTERNAL PARAMETERS
	{
		iCritErr = GEN_ERROR;
		prt( "WARNING: Bad internal parameters passed!" );
	}
	return iCritErr;
}


int	FixDir( WS,
		   char * lpd, char * lps )
{
	int		len, i, j, k;
	char	   buf[260];
	char *	   lpt;
	char	   c = 0;

	strcpy( lpd, lps );	// Just COPY IT first
	i = strlen( lpd ); 
	if(i)
	{
		lpt = &buf[0];
		k = 0;
		for( j = 0; j < i; j++ )
		{
			c = lpd[j];
			if( ( c == ':' ) || IS_PATH_SEP_CHAR(c) )
			{
				k = 0;	// Start accumulation again
			}
			else
			{
				lpt[k++] = c;	// Accumulate
			}
		}
		if( k )		// We have some form of TAIL
		{
			lpt[k] = 0;
			if( GotWild( lpt ) )
			{
				len = strlen( lpd );
			}
			else if( IsDot( lpt ) )
			{
				i = strlen( lpd ) - 1;
				lpd[i] = 0;	// Kill DOT
				strcat( lpd, "*.*" );
			}
			else if( IsDDot( lpt ) )
			{
				strcat( lpd, PATH_SEP"*.*" );
			}
			else if( HasDot( lpt ) )
			{
				len = strlen( lpd );
			}
			else	// Assume it ENDS WITH A DIRECTORY
			{
				strcat( lpd, PATH_SEP"*.*" );
			}
		}
		else	// Ends in ":" or "/" or "\"
		{
			if( c == ':' )
			{
				strcat( lpd, PATH_SEP"*.*" );
			}
			else
			{
				strcat( lpd, "*.*" );
			}
		}
	}
	else
	{
		strcpy( lpd, "*.*" );
	}
	len = strlen( lpd );
   UNREFERENCED_PARAMETER(pWS);

	return len;
}

int	StoreDir( WS,
				 char * lpb, uint32_t len, char * lptmp, HANDLE * phTmp, int * pCnt )
{
	int	flg;
	WIN32_FIND_DATA	fd;
	uint32_t	dwi;
	int		iCnt;

	flg = FALSE;
	memset( &fd, 0, sizeof( WIN32_FIND_DATA ) );
	fd.dwFileAttributes = SPLFA;
	fd.ftCreationTime.dwLowDateTime = 0; 
	fd.ftCreationTime.dwHighDateTime = 0;
	fd.ftLastAccessTime.dwLowDateTime = 0;
	fd.ftLastAccessTime.dwHighDateTime = 0;
	fd.ftLastWriteTime.dwLowDateTime = 0; 
	fd.ftLastWriteTime.dwHighDateTime = 0;
	fd.nFileSizeHigh = 0;
	fd.nFileSizeLow = 0;
	fd.dwReserved0 = 1; 
    fd.dwReserved1 = 2;
	if( ( len < MAX_PATH ) &&
		( (iCnt = *pCnt) == 0 ) )
	{
		for( dwi = 0; dwi < len; dwi++ )
		{
			fd.cFileName[dwi] = lpb[dwi];
		}
		fd.cFileName[dwi] = 0;
		fd.cAlternateFileName[0] = 0;
		if( AddFile( pWS, &fd, lptmp, phTmp, pCnt ) )
		{
			// Failed to write to file
			iCritErr = GEN_ERROR;
		}
		else
		{
			iCnt++;
			//*pCnt = iCnt;	// Bump the written count
		}
		flg = TRUE;
	}
	return flg;
}

void	MoveDir( WS,
				char * lpDir, char * lps, uint32_t len )
{
	char	buf[260+4];
	char *	lpd;
	uint32_t	dwi;

	lpd = &buf[0];
	for( dwi = 0; dwi < len; dwi++ )
	{
		lpd[dwi] = lps[dwi];
	}
	lpd[dwi] = 0;
	FixDir( pWS, lpDir, lpd );
}

int	GetDir( WS,
		   char * pDir, HANDLE * phFile,
		   uint32_t *lpDW, HGLOBAL * phGlob,
		   char * * lpFile,
		   char * lptf,
		   HANDLE * pTmp,
		   int * pCnt,
		   uint32_t *pdwTot )
{
	WIN32_FIND_DATA	fd;
	HANDLE	hFind;
	int		fCnt;
	FILETIME lft;
	uint32_t	dwTot;

	if( VERB5 )
	{
		sprintf( lpVerb, "Find using [%s]"PRTTERM,
			pDir );
		prt( lpVerb );
	}
	StoreDir( pWS, pDir, strlen( pDir ), lptf, pTmp, pCnt );
	dwTot = 0;
	fCnt = 0;
	hFind = FindFirstFile( pDir, &fd );
	if( ( hFind ) &&
		( hFind != INVALID_HANDLE_VALUE ) )
	{
		if( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
// This is UTC (Coordinate Universal Time), so we could use
//int FileTimeToSystemTime(
//    CONST FILETIME *lpFileTime,	// pointer to file time to convert 
//    LPSYSTEMTIME lpSystemTime 	// pointer to structure to receive system time
// to convert it to the LOCAL PC time
//   OR we could use
			if( FileTimeToLocalFileTime( &fd.ftLastWriteTime,
				&lft ) )
			{
				fd.ftLastWriteTime.dwLowDateTime  = lft.dwLowDateTime;
				fd.ftLastWriteTime.dwHighDateTime = lft.dwHighDateTime;
			}
			if( AddFile( pWS, &fd, lptf, pTmp, pCnt ) )
			{
				iCritErr = GEN_ERROR;
			}
			else
			{
				fCnt++;
				dwTot += fd.nFileSizeLow;
			}
		}
		while( FindNextFile( hFind, &fd ) )
		{
			if( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				if( FileTimeToLocalFileTime( &fd.ftLastWriteTime,
					&lft ) )
				{
					fd.ftLastWriteTime.dwLowDateTime  = lft.dwLowDateTime;
					fd.ftLastWriteTime.dwHighDateTime = lft.dwHighDateTime;
				}
				if( AddFile( pWS, &fd, lptf, pTmp, pCnt ) )
				{
					iCritErr = GEN_ERROR;
				}
				else
				{
					fCnt++;
					dwTot += fd.nFileSizeLow;
				}
			}
		}
		FindClose( hFind );
		if( fCnt == 0 )
		{
			sprintf( glpError,
				"ERROR: No finds using [%s]!"PRTTERM,
				pDir );
			Err_Exit( -1 );
		}
		else
		{
//			fCnt += *pCnt;
//			*pCnt = fCnt;
			*pdwTot = dwTot;
		}
	}
	else
	{
		sprintf( glpError,
			"ERROR: No finds using [%s]!"PRTTERM,
			pDir );
		Err_Exit( -1 );
	}
   UNREFERENCED_PARAMETER(lpFile);
   UNREFERENCED_PARAMETER(phGlob);
   UNREFERENCED_PARAMETER(lpDW);
   UNREFERENCED_PARAMETER(phFile);

	return iCritErr;
}

///////////////////////////////////////////////////////////////////////////////////
#endif // WIN32

LPWORKSTR	GetWS( void )
{
	return( lpWS );
}

void	NulWS( WS )
{
	memset( (char *)pWS, 0, sizeof( WORKSTR ) );
	W.ws_Size = sizeof( WORKSTR );
}

void  MakeDiagFile( void )
{
   char *   lpf = &g_szDiagFile[0];
   char *   p;
#ifdef WIN32
   GetModuleFileName( NULL, lpf, 256 );
#else // !WIN32
   strcpy(lpf,g_ProgramName);
#endif // WIN32 y/n

   p = strrchr(lpf, PATH_CHAR);
   if(p)
      p++;
   else
      p = lpf;
   *p = 0;
#ifndef  NDEBUG
   if( p != lpf )
   {
      char *   ptmp = &g_szTmpBuf[0];
      strcpy(ptmp,lpf);
      ptmp[ strlen(ptmp) - 1 ] = 0;
      p = strrchr(ptmp,PATH_CHAR);
      if( p && (strcmpi( &p[1], "DEBUG")==0) )
      {
         p++;
         *p = 0;
         strcpy(lpf,ptmp);
      }
   }
#endif   // !NDEBUG
#ifdef WIN32
   strcat(lpf,"tempfa4.txt");
#else
    strcpy(lpf,"/tmp/tempfa4.log");
#endif
    // fprintf(stderr,"Setting diag file to '%s'\n", lpf );
	SetDiagFile(lpf);
}

#ifdef USE_ALLOCATED_LINE_BUFFER
void ReAllocate_LineBuffer( WS )
{
    uint32_t   current = gdwLBSize;
    char * nb;

    gdwLBSize += MXLINEBUF; // bump buufer size
    nb = (char *)MALLOC( LPTR, gdwLBSize ); // allocate new buffer
    if ( !nb ) {
		prt( "ERROR: Memory allocation of LINE BUFFER FAILED!"PRTTERM );
		Err_Exit( -1 );
    }

    if (glpLineBuf) {
        memcpy(nb,glpLineBuf,current);
        MFREE(glpLineBuf);
    }

    glpLineBuf = nb;
}
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER

#ifndef WIN32
uint32_t GetTickCount()
{
    struct timespec ts;
    uint32_t theTick = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    theTick = ts.tv_nsec / 1000000;
    theTick += ts.tv_sec * 1000;
    return theTick;
}
#endif // !WIN32


LPWORKSTR	DoSetup( void )
{
	LPWORKSTR	pWS = (LPWORKSTR)MALLOC( LPTR, sizeof( WORKSTR ) );
	if( !pWS )
	{
		prt( "ERROR: Memory allocation of WORKSTR FAILED!"PRTTERM );
		Err_Exit( -1 );
	}
	NulWS( pWS );
	lpWS = pWS;

#ifdef USE_ALLOCATED_LINE_BUFFER
    ReAllocate_LineBuffer( pWS );
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER

	gdwBgn = GetTickCount();
	// set the diag file name

    MakeDiagFile();

	iMainRet = 0;	// Assume NO FINDS
	iCritErr = 0;	// and NO critical errors
	gfReverse = FALSE;
	gfVerbose = DEF_VERB;
//	fOnce = TRUE;
	gfHeader = FALSE;


	W.ws_findintype = &Find_In_Type1;	// Case insensitive
	W.GETCHR = &getupchar;			// Set GetChr with Compare UPPER with UPPER

	*glpError = 0;
	fFirst = TRUE;
	g_fBinary = FALSE;
	g_fCase = FALSE;
	gfWhole = FALSE;
	gfNumber = FALSE;
	gfParity = FALSE;
	dwSrchCnt = dwMaskCnt = 0;
	gdwMax1Line = MX1LINE;
	gdwEnd1Line = MXELINE;

	// 	char	ws_cVerbBuf[MXVERBBUF+16];
	lpWS->ws_lpVerb = &lpWS->ws_cVerbBuf[0];

	// Set some DEFAULT values
	g_dwBackup = DEF_BACKUP;	// Began as 10 20041210 - changed to 32 = 1/2 line
	g_dwForward = DEF_FORWARD;
	W.ws_dwMxFilNm = DEF_MAXFN;

	glpActive = &W.ws_szActFil[0];	// Set this pointer

#ifdef		ADD2ZLST
	InitZList( gpsFinds );
	InitZList( gpsFiles );
	InitZList( gpsExcludes );

	gsFinds.ss_bChkD = TRUE;
	gsFiles.ss_bChkD = TRUE;
	gsExcludes.ss_bChkD = TRUE;
#endif	// ADD2ZLST

   // FIX20010319 -v4 adds date order of finds at end - VERB4
   InitLList( &gsFileList );
#ifdef USE_EXCLUDE_LIST
	// EXCLUDE File string(s) LIST
   InitLList( &g_sExclList ); // list of EXCLUDED files
   InitLList( &g_sExclListD ); // list of EXCLUDED directories
#endif // #ifdef USE_EXCLUDE_LIST

   InitLList( &g_sFind );
   InitLList( &g_sLines );
   InitLList( &g_sFileList );    // List of FILES to process
   // PLE   ph = &g_sFileList;   // count in g_nFileCnt
   InitLList( &g_sFailList );    // List of FAILED FILES

   InitLList( &g_sCVSLines );    // list of Entries lines
   InitLList( &g_sEntries );    // list of LATEST Entries lines
   InitLList( &g_sFFList  );     // FOUND in FILE list

   // if ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   g_dwMinCnt = 1;   // -cnn - minimum find count
   InitLList( &g_sOutList );

   //ReadINI();   // if we had one
#ifdef   ADD_REGEX
   gi_pcre_opions = PCRE_CASELESS;
   gp_pcre_tables = pcre_maketables();
#endif   // #ifdef ADD_REGEX
	// Only if allocation succeeded!
	return	lpWS;

}


//typedef struct _WIN32_FIND_DATA { // wfd  
//    uint32_t dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    uint32_t    nFileSizeHigh; 
//    uint32_t    nFileSizeLow; 
//    uint32_t    dwReserved0; 
//    uint32_t    dwReserved1; 
//    char    cFileName[ MAX_PATH ]; 
//    char    cAlternateFileName[ 14 ]; 
//} WIN32_FIND_DATA; 

#ifdef   ADDCVSDATE
//   if( g_bCVSDate )
char *   IsValidEntries( char * lpv )
{
   char *   lpr = 0;
   uint32_t    i = strlen(lpv);
   uint32_t    j, k;
   PCVSLN   pcvs = &g_sCvsLn;

   pcvs->bDTValid = FALSE;
   pcvs->psName   = g_pFFList;   // pointer to ACTIVE file name
   k = 0;
   for(j = 0; j < i; j++ )
   {
      if( lpv[j] == '/' )
      {
         if(k == 0)
            lpr = &lpv[j];    // point to FIRST
         k++;
         if(k > 5)
            break;
      }
   }
   if( k < 5 )
   {
      lpr = 0;
   }
   else
   {
      char *lps = lpr;
      char *p, *lpd;
      uint32_t    i, j;
      if(lps)
      {
         lps++;   // bump past lead char
         p = strchr(lps,'/');
         if(p)
         {
            pcvs = &g_sCvsLn;
            lpd = &pcvs->szFile[0];
            i = 0;
            while( lps < p )
            {
               lpd[i++] = *lps++;
            }
            lpd[i] = 0;
            lps++;   // get to next (i think) version
            i = j = 0;
            while( *lps && (*lps != '/'))
            {
               if(*lps == '.')
               {
                  i = (i << 8) | j;
                  j = 0;
               }
               else if( ISNUM(*lps) )
               {
                  j = ((j * 10) + (*lps - '0') );
               }
               else
                  break;
               lps++;
            }
            pcvs->dwVers = i;
            //pcvs->bDTValid = FALSE;
            if( *lps == '/' )
            {
               lps++;
               //pcvs->bDTValid = Stg2SysTm(lps, &pcvs->sSysTm);
               pcvs->bDTValid = pcvs->uFT ? 1 : 0;
               //if(pcvs->bDTValid)
               //{
               //   pcvs->bDTValid = SystemTimeToFileTime(
               //      &pcvs->sSysTm, // system time
               //      &pcvs->sFT );  // file time
               //}
            }
            if( !pcvs->bDTValid )
            {
               lpr = 0;
            }
         }
      }
   }
   return lpr;
}

#endif   // #ifdef   ADDCVSDATE

typedef struct tagLINEPTRS {
   char * pdate; // pointer to DATE
   char * ptime; // pointer to TIME
   // FIX20061211 - time may be 24 hours time = NO AM|PM
   int   time12; // is 12 hour time - ie with AM|PM
   char * psize; // pointer to SIZE
   char * pname; // pointer to NAME
   char fullname[1024]; // and the full PATH name
}LINEPTRS, * PLINEPTRS;

static LINEPTRS sLinePtrs;
PLINEPTRS psLP = &sLinePtrs;

#ifdef WIN32

int gotfiledatetime( char * lpv, WIN32_FIND_DATA * pfd, DateStyle amer ) // WIN32 ONLY
{
   int c;
   char * ps = lpv;
   SYSTEMTIME st;
   char * pend = 0;

   ZeroMemory( pfd, sizeof(WIN32_FIND_DATA) );
   if( ISNUM(*ps) ) {
      // on our way
      int day = atoi(ps);
      while( ISNUM(*ps) ) ps++;
      c = *ps;
      if(c) ps++;
      if( ISNUM(*ps) ) {
         int month = atoi(ps);
         while( ISNUM(*ps) ) ps++;
         ps++;
         if( ISNUM(*ps) ) {
            int year = atoi(ps);
            while( ISNUM(*ps) ) ps++;
            pend = ps;
            while( *ps && (*ps <= ' ') ) ps++;
            if(ISNUM(*ps)) {
               int hours = atoi(ps);
               while( ISNUM(*ps) ) ps++;
               ps++;
               if(ISNUM(*ps)) {
                  int mins = atoi(ps);
                  int Valid;
                  while( ISNUM(*ps) ) ps++;
                  while( *ps && (*ps <= ' ') ) ps++;
                  c = toupper(*ps);
                  if( (c == 'A') || (c == 'P') ) {
                     if( c == 'P' )
                        hours += 12;
                     if( amer == ds_american ) {
                        st.wDay = (WORD)month;
                        st.wMonth = (WORD)day;
                     } else {
                        st.wDay = (WORD)day;
                        st.wMonth = (WORD)month;
                     }
                     st.wYear = (WORD)year;
                     st.wHour = (WORD)hours;
                     st.wMinute = (WORD)mins;

                     Valid = SystemTimeToFileTime(
                        &st, // system time
                        &pfd->ftLastWriteTime );  // file time
                     if( !Valid ) {
                        if( amer == ds_american ) {
                           st.wDay = (WORD)day;
                           st.wMonth = (WORD)month;
                        } else {
                           st.wDay = (WORD)month;
                           st.wMonth = (WORD)day;
                        }
                        Valid = SystemTimeToFileTime(
                           &st, // system time
                           &pfd->ftLastWriteTime );  // file time
                     }
                     return Valid;
                  } else {
                     // FIX20061211 - deal with 24 hour time
                     if( amer == ds_american ) {
                        st.wDay = (WORD)month;
                        st.wMonth = (WORD)day;
                     } else {
                        st.wDay = (WORD)day;
                        st.wMonth = (WORD)month;
                     }
                     st.wYear = (WORD)year;
                     st.wHour = (WORD)hours;
                     st.wMinute = (WORD)mins;

                     Valid = SystemTimeToFileTime(
                        &st, // system time
                        &pfd->ftLastWriteTime );  // file time
                     if( !Valid ) {
                        if( amer == ds_american ) {
                           st.wDay = (WORD)day;
                           st.wMonth = (WORD)month;
                        } else {
                           st.wDay = (WORD)month;
                           st.wMonth = (WORD)day;
                        }
                        Valid = SystemTimeToFileTime(
                           &st, // system time
                           &pfd->ftLastWriteTime );  // file time
                     }
                     return Valid;
                  }
               }
            }
         }
      }
   }

   return 0; // failed
}

FILETIME AddFileDateTime( char * lpv2, char * lpv, DateStyle amer ) // WIN32 ONLY
{
   static char _s_filetimedate[264];
   char * ps1 = _s_filetimedate;
   WIN32_FIND_DATA fd1;
   WIN32_FIND_DATA fd2;
   HANDLE hfind = FindFirstFile(lpv2, &fd1);
   int got = gotfiledatetime( lpv, &fd2, amer );
   char * ps2 = &ps1[128];
   FILETIME ft = {0};
   *ps1 = 0;
   *ps2 = 0;
   if( VFH(hfind) ) {
      AddDateTime2( ps1,		// Destination buffer
				   &fd1,	// Information
				   TRUE,	// 24-Hour time
               amer );
      FindClose(hfind);
   }
   if( got ) {
      AddDateTime2( ps2,		// Destination buffer
				   &fd2,	// Information
				   TRUE,	// 24-Hour time
               amer );
   }
   if( *ps1 )
   {
      // FIX20070115 - minor adjustments of the output
      //strcat(lpv2, " Current");
      strcat(lpv2, " ");
      strcat(lpv2, ps1);
   }
   if( *ps2 )
   {
      // FIX20070115 - minor adjustments of the output
      //strcat(lpv2, " Found");
      strcat(lpv2, " F");
      strcat(lpv2, ps2);
   }

   if( *ps1 )
      ft = fd1.ftLastWriteTime;
   else if( *ps2 )
      ft = fd2.ftLastWriteTime;

   return ft;
}

#endif // !WIN32

#ifdef ADDDIRLST
////////////////////////////////////////////

typedef enum {
   header,
   entry
}LNTYPE;

typedef struct tagSORTEDLINE {
   LIST_ENTRY  le;
   uint32_t noff;
   union
   {
      FILETIME ft;
      LONGLONG fs;
   }f;
   LNTYPE  type;
   char line[1];
}SORTEDLINE, * PSORTEDLINE;

LIST_ENTRY  g_lnlist = { &g_lnlist, &g_lnlist };

#ifdef ADD_DIRLIST_SORT2
// void OutSortedLines( WS )
void OutSortedLines( void )
{
   PLE ph = &g_lnlist;
   PLE pn;
   PSORTEDLINE psl;
   char * pb = &g_cWorkBuf[0];  // switch to a general work buffer
   LONG  cnt = 0;
   uint32_t mxoff = 0;
   uint32_t off;
   // ListCount2(ph,&cnt);
   Traverse_List(ph,pn) {
      cnt++;
      psl = (PSORTEDLINE)pn;
      if(psl->type != header) {
         if( psl->noff > mxoff )
            mxoff = psl->noff;
      }
   }
   if(cnt) {
      sprintf(pb,"Output of %d SORTED lines ... ", cnt);
      switch( gbOrdList )
      {
      case od_dateup:
         strcat(pb, " by DATE, oldest first ...");
         break;
      case od_datedown:
         strcat(pb, " by DATE, newest first ...");
         break;
      case od_sizeup:
         strcat(pb, " by SIZE, smallest first ...");
         break;
      case od_sizedown:
         strcat(pb, " by SIZE, newest first ...");
         break;
      }
      strcat(pb,MEOR);
      prt(pb);
      cnt = 0;
      Traverse_List(ph,pn)
      {
         cnt++;
         psl = (PSORTEDLINE)pn;
         strcpy(pb, &psl->line[0]);
         if(psl->type == header) {
            strcat(pb, " (HEADER)");
         } else {
            off = psl->noff;
            if(off && (off < mxoff)) {
               pb[off] = 0;
               while(off < mxoff) {
                  strcat(pb," ");
                  off++;
               }
               strcat(pb, &psl->line[psl->noff] );
            }
         }
         strcat(pb,MEOR);
         prt(pb);
      }
      sprintf(pb,"END Output of %d SORTED lines ... ", cnt);
      switch( gbOrdList )
      {
      case od_dateup:
         strcat(pb, " by DATE, oldest first ...");
         break;
      case od_datedown:
         strcat(pb, " by DATE, newest first ...");
         break;
      case od_sizeup:
         strcat(pb, " by SIZE, smallest first ...");
         break;
      case od_sizedown:
         strcat(pb, " by SIZE, newest first ...");
         break;
      }
      strcat(pb,MEOR);
      prt(pb);
   }
}

#endif // #ifdef ADD_DIRLIST_SORT2

char * get_clean_number(PSTR ps)
{
   char * pb = GetNxtBuf();
   int len = strlen(ps);
   int   i, c, out;
   out = 0;
   for(i = 0; i < len; i++) {
      c = ps[i];
      if(ISNUM(c)) {
         pb[out++] = (char)c;
      } else if( c == ',' ) {
         // just skip this
      } else {
         break;
      }
   }
   pb[out] = 0;
   return pb;
}

#ifdef ADD_DIRLIST_SORT2
//     if( gbOrdList || VERB4 ) { Add2SortedList( lpv2 );
void Add2SortedList( WS, char * lpv2, char * lpv, int Type )
{
   PSORTEDLINE psl; // = MALLOC( LPTR, sizeof(SORTEDLINE) + strlen(lpv2));
   PSORTEDLINE psl2;
   FILETIME ft;
   LONGLONG fs;
   PLE ph = &g_lnlist;
   PLE pn;
   long lg;
   uint32_t off;
   if( g_iDoneList == FALSE ) {
      psl2 = (PSORTEDLINE)MALLOC( LPTR, sizeof(SORTEDLINE) + strlen(glpActive));
      CHKMEM(psl2);
      strcpy(&psl2->line[0], glpActive);
      psl2->type = header;
      InsertTailList(ph,(PLE)psl2);
      g_iDoneList = TRUE;
   }
   if( (gbOrdList == od_dateup) || (gbOrdList == od_datedown) || VERB4 ) {
      // ORDER BY DATE
      if( Type == 0 ) {
         off = strlen( lpv2 );
         ft = AddFileDateTime( lpv2, lpv, ds_american );
      } else {
         off = strlen(psLP->fullname);
         ft = AddFileDateTime( psLP->fullname, lpv, ds_american );
      }
      psl = (PSORTEDLINE)MALLOC( LPTR, sizeof(SORTEDLINE) + strlen(lpv2));
      CHKMEM(psl);
      psl->f.ft = ft;
      psl->noff = off;
      strcpy(&psl->line[0], lpv2);
      psl->type = entry;
      Traverse_List(ph,pn)
      {
         psl2 = (PSORTEDLINE)pn;
         if(psl2->type != header) {
            lg = CompareFileTime( &psl->f.ft, &psl2->f.ft );
            // Value Meaning 
            // -1 First file time is less than second file time.
            // 0  First file time is equal to second file time. 
            // 1  First file time is greater than second file time.
            if( gbOrdList == od_datedown ) {
               if( lg > 0 ) {
                  InsertBefore(pn,(PLE)psl);
                  pn = 0;
                  break;
               }
            } else { // if ( gbOrdList == od_dateup )
               if( lg < 0 ) {
                  InsertBefore(pn,(PLE)psl);
                  pn = 0;
                  break;
               }
            }
         }
      }
      if(pn) {
         InsertTailList(ph,(PLE)psl);
      }
   } else {
      // ORDER BY SIZE
      off = strlen(lpv2);
      fs = atol(get_clean_number(psLP->psize));
      sprintf(EndBuf(lpv2), " %d", fs);
      psl = (PSORTEDLINE)MALLOC( LPTR, sizeof(SORTEDLINE) + strlen(lpv2));
      CHKMEM(psl);
      psl->f.fs = fs;
      psl->noff = off;
      strcpy(&psl->line[0], lpv2);
      psl->type = entry;
      Traverse_List(ph,pn)
      {
         psl2 = (PSORTEDLINE)pn;
         if(psl2->type != header) {
            // the ORDER is by SIZE
            if( gbOrdList == od_sizedown ) {
               if( psl->f.fs < psl2->f.fs ) {
                  InsertBefore(pn,(PLE)psl);
                  pn = 0;
                  break;
               }
            } else { // if( gbOrdList == od_sizeup )
               if( psl->f.fs > psl2->f.fs ) {
                  InsertBefore(pn,(PLE)psl);
                  pn = 0;
                  break;
               }
            }
         }
      }
      if(pn) {
         InsertTailList(ph,(PLE)psl);
      }
   }
}

#endif // #ifdef ADD_DIRLIST_SORT2

#endif // #ifdef ADDDIRLST

void  AddTrailingSlash( char * lpv2 )
{
   size_t ilen = strlen( lpv2 );
   if( ilen && ( lpv2[ilen-1] != PATH_CHAR ) )
      strcat( lpv2, PATH_SEP );
}

// #ifdef ADDDIRLST
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : OutALine
// Return type: void 
// Arguments  : WS
//            : char * lpv
//            : LPuint32_t pln
//            : int bflg
//            : uint32_t dwoff
// Description: Actually output the line containing the FIND
//              
// FIX20010703 - multiple FIND strings MUST be in ORDER
// so if there are multiple find strings, then the output is
// added to a LIST of finds (g_sFinds), and only output when
// all the finds in this file are added in order of line numbers
//
///////////////////////////////////////////////////////////////////////////////
void	OutALine( WS, char * lpv, uint32_t *pln, int bflg, uint32_t dwoff )
{
	uint32_t    dwl;
	char *	lpv2 = lpv;

#ifdef   ADDCVSDATE
   if( g_bCVSDate )
   {
      // have already decode the LINE into the GLOBAL structure
      PLE   ph = &g_sCVSLines;
      PLE   pn;
      PCVSLN   pcvsg = &g_sCvsLn;
      PCVSLN   pcvs;
      if(pcvsg->bDTValid)  // so if it is VALID
      {
         PLE      pn2;
         PCVSLN   pcvs2;
         //LONG     lg;
         pn = (PLE)MALLOC( LPTR, sizeof(CVSLN) );
         if( !pn )
         {
			   sprintf( glpError,
               "CRITICAL ERROR: Get memory FAILED on %d bytes!"MEOR, sizeof(CVSLN) );
            prt(glpError);
            exit(0);
         }
         pcvs = (PCVSLN)pn;
         memcpy(pcvs, pcvsg, sizeof(CVSLN));
         Traverse_List(ph, pn2)
         {
            pcvs2 = (PCVSLN)pn2;
            //lg = CompareFileTime(
            //         &pcvs->sFT, // first file time
            //         &pcvs2->sFT ); // second file time
                  // Value Meaning 
                  // -1 First file time is less than second file time.
                  // 0  First file time is equal to second file time. 
                  // 1  First file time is greater than second file time.
            //if( lg > 0 )
            if (pcvs->uFT < pcvs2->uFT)
            {
               InsertBefore(pn2,pn);
               pn = 0;
               break;
            }
         }
         if(pn)
         {
            InsertTailList(ph,pn);
         }
      }
      if( !VERB5 )   // if NOT very verbal
         return;     // do NOT output the File

   }
#endif   // ADDCVSDATE

   if( bflg )
      dwl = dwoff;
   else
      dwl = *pln;

	if( gfNumber )
	{
		//lpv2 = &gcWorkBuf[0];
		//lpv2 = glpWorkBuf;
      lpv2 = &g_cWorkBuf[0];  // switch to a general work buffer
		//if( ( g_FileType & FT_BIN ) ||
		//	( g_fBinary ) )
      if( bflg )
		{
			dwl = dwoff;
			sprintf( lpv2, "%08X", dwl );
		}
		else
		{
			//dwl = *pln;
			//dwl++;
			PutThous( lpv2,
				7,
				(dwl + 1) );
			//if( (dwl+1) == 13447 )
			//	chkchk();
		}
		strcat( lpv2, " <" );
		strcat( lpv2, lpv );       // copy in the prepared line
		strcat( lpv2, ">" );
		if( gfDirLst && giDirLen ) // add gcDirBuf?  // WITH LINE NUMBERS
		{
			strcat( lpv2, " " );
			strcat( lpv2, gcDirBuf );
		}
		strcat( lpv2, PRTTERM );   // add Cr/Lf termination
	}
	else  // NO number
	{
      int ilen; // = strlen(ps);
      // FIX20050201 - added -DL to out a simple list of files found, with path
		if( gfDirLst && giDirLen ) // with NO LINE NUMBERS
		{
         char * pb = 0;
         int getoffs = gbDirList;
#ifdef ADD_DIRLIST_SORT2
         getoffs |= (gbOrdList << 1);
#endif // #ifdef ADD_DIRLIST_SORT2
         // if( gbDirList ) { // we have -DL - wants file list only
         if( getoffs ) { // we have -DL (file list) or -DO (order list)
            char * ps = lpv; // get the line
            ilen = strlen(ps);
            pb = ps; // check, is it a date
            psLP->pdate = pb;
            while( *ps && (*ps > ' ') ) ps++; // get past the DATE
            while( *ps && (*ps <= ' ') ) ps++; // get past the DATE space
// like 18/01/2005  10:38 AM               117 "fg_mkdir.hxx C:\FGCVS\Atlas\src\"
            pb = ps; // check a time
            psLP->ptime = pb;
            while( *ps && (*ps > ' ') ) ps++; // get past the TIME
            while( *ps && (*ps <= ' ') ) ps++; // get past the TIME space
            pb = ps;
            // FIX20061211 - time may be 24 hours time = NO AM|PM
            psLP->time12 = 0; // assume NOT
            if( ( toupper(*ps) == 'A' ) || ( toupper(*ps) == 'P' ) ) {
               while( *ps && (*ps > ' ') ) ps++; // get past the TIME AM|PM
               while( *ps && (*ps <= ' ') ) ps++; // get past the AM|PM space
               psLP->time12 = 1; // yes it IS
            }
            pb = ps;
            psLP->psize = pb;
            while( *ps && (*ps > ' ') ) ps++; // get past the SIZE
            while( *ps && (*ps <= ' ') ) ps++; // get past the SIZE space
            pb = ps; // got start of FILE NAME
            psLP->pname = pb;
            strcpy( psLP->fullname, gcDirBuf );
            AddTrailingSlash( psLP->fullname );
            strcat( psLP->fullname, pb );    // plus the file name
         }
         if( gbDirList ) { // we have -DL - wants file list only
            if(*pb) {
              lpv2 = &g_cWorkBuf[0];  // switch to a work buffer
              strcpy(lpv2, psLP->fullname);
#ifdef ADD_DIRLIST_SORT2
              if( gbOrdList || VERB4 ) {
                  Add2SortedList( pWS, lpv2, lpv, 0 );
              }
 #endif // #ifdef ADD_DIRLIST_SORT2
           } else {
               pb = 0; // no file name, no list listing, but output normally
            }
         } else if( gbDirNorm ) { // FIX20060729 - add -dn for 'normal' list, as per DIR command (gbDirNorm)
            lpv2 = &g_cWorkBuf[0];  // switch to a work buffer
            *lpv2 = 0;
            if ( strcmpi( g_szDirBuf, gcDirBuf ) ) {
               strcpy(g_szDirBuf, gcDirBuf);
               strcpy(lpv2," Directory of ");
               strcat(lpv2, gcDirBuf);
               strcat(lpv2, PRTTERM);
            }
			   strcat( lpv2, lpv );    // copy the line from the file
            pb = lpv2;  // make non zero, to avoid standard output
         } else { // NOT Directory LIST listing - just the file line
#ifdef ADD_DIRLIST_SORT2
            if( gbOrdList || VERB4 ) {
               lpv2 = &g_cWorkBuf[0];  // switch to a work buffer
               strcpy(lpv2,lpv); // make a COPY of the LINE
               Add2SortedList( pWS, lpv2, lpv, 1 );
            }
 #endif // #ifdef ADD_DIRLIST_SORT2
         }
         
         if( pb == 0 ) {
            // output, from file line, like -
            // "24/07/2005  04:36 PM            42,510 php.ini"
			   //lpv2 = glpWorkBuf;
            lpv2 = &g_cWorkBuf[0];  // switch to a work buffer
			   strcpy( lpv2, lpv );    // copy the line
			   strcat( lpv2, " " );    // plus a space
            // add folder to end, like                        "C:\WINDOWS\"
			   strcat( lpv2, gcDirBuf );  // PLUS the DirBuf string
            // "24/07/2005  04:36 PM            42,510 php.ini C:\WINDOWS\"
         }
		}
		strcat( lpv2, PRTTERM );   // add Cr/Lf termination
	}

   // add2outlist(lpv2); // add line to OUTPUT list

   // note we have added our own Cr/Lf pair
// #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
   if( g_dwFCnt > 1 )
   {
      PLE   ph = &g_sLines;
      PFLINE pl = (PFLINE)MALLOC( LPTR, (sizeof(FLINE) + strlen(lpv2)) );
      PLE   pn;
      PFLINE pl2;
      if(pl)
      {
         pl->dwNum = dwl;  // can be LINE or OFFSET
         pl->uint32_t = g_dwFileNum;    // keep which file this is - 1 based numbering
         strcpy( &pl->cLine[0], lpv2 );
         pl2 = g_psActFind;   // get the active FIND structure
         pl->pFind = pl2;        // keep the FIND
         // since the pFind is kept, these are really redundant
         // but perhaps for speed of compare, make copies
         pl->dwTyp = pl2->dwTyp; // copy over the TYPE
         pl->dwTN  = pl2->dwTN;  // and associated numeric
         // order now, or later? - let's try now!
         Traverse_List( ph, pn )
         {
            pl2 = (PFLINE)pn;
            if( pl2->dwNum > dwl )
            {
               InsertBefore(pn, (PLE)pl);
               return;  // all done
            }
         }
         InsertTailList(ph,(PLE)pl);   // add it to the end
      }
      else
      {
         prt( "ERROR: MEMORY FAILURE - SHOULD EXIT!"MEOR );
      }
   } else {
      if ( VERB4 || (gbOrdList == 0) )
         prt( lpv2 );
   }
// #endif   // #ifdef USEFINDLIST y/n // = PVERS "V4.0.16" // FIX20010703 - order FIND strings

   // =====================================
   // UNREFERENCED_PARAMETER(bflg);
}

// #endif // #ifdef ADDDIRLST


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GettheLine
// Return type: uint32_t 
// Arguments  : WS
//            : uint32_t dwoff
//            : uint32_t dwmax
//            : LPuint32_t pdw
//            : LPuint32_t pln
//            : uint32_t dwmxvb
// Description: Extract the line we want to SHOW in ShowLine()
//              FIX20010328 Fix for UNIX file searching
///////////////////////////////////////////////////////////////////////////////
uint32_t	GettheLine( WS,
				 uint32_t dwoff,
				 uint32_t dwmax,
				 uint32_t *pdw,
				 uint32_t *pln_NOT_USED,
				 uint32_t dwmxvb_NOT_USED )
{
	char	c;
	uint32_t	dwo, dwk, dwend;
	char *	lpmf;
	uint32_t	dwCurrMax;
	// Size MXLINEBUF
	char *	lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]

	dwk = 0;
	lpmf = (char *)glpMapView;
#ifdef USE_ALLOCATED_LINE_BUFFER    // glpLineBuf & gdwLBSize
    dwCurrMax = gdwLBSize - 4; 
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
    dwCurrMax = MXLINEBUF; 
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER y/n

	if ( lpmf )
	{
      if(( g_FileType & FT_BIN ) ||
			( g_fBinary           ) ) {
			dwo = dwoff;
            c = 0;
			if(dwo) {   // have an offset - so back up in buffer
				dwk = 0;
				dwo--;
				while( dwo ) {
					c = lpmf[dwo];
					//if( suc = NotAsciiRng( c ) )
					if( ( c < ' ' ) || ( c >= 0x7f ) ) {
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
            g_dwBgnLine = dwo;  // keep offset of beginning of 'line'
			dwend = dwoff + dwk + g_dwCurFLen + g_dwForward;
			if( dwend > dwmax )
				dwend = dwmax;
			//for( dwo = dwoff; dwo < dwmax; dwo++ )
			dwk = 0;
			for( ; dwo < dwend; dwo++ )
			{
				c = lpmf[dwo];
				if( ( c < ' ' ) || ( c >= 0x7f ) )
					break;
                if (dwo == g_dwOffBgn)
                    g_dwLBOffBgn = dwk; // save BEGIN of find in line buffer
                if (dwo == g_dwOffEnd)
                    g_dwLBOffEnd = dwk; // save BEGIN of find in line buffer
				lpLine[dwk++] = c;
				// check max.
				// ==========
#ifdef USE_ALLOCATED_LINE_BUFFER    // glpLineBuf & gdwLBSize
                if (dwk >= dwCurrMax) {
                    // TIME TO RE-ALLOCATE MORE LINE BUFFER
                    ReAllocate_LineBuffer(pWS);
                    dwCurrMax = gdwLBSize - 4; // set NEW max size 
                    lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]
                }
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
				if( dwk > dwCurrMax )	// gdwMax1Line = FIXED SIZE
				{
					break;
				}
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER y/n
			}
            g_dwEndLine = dwo;  // keep offset of end of 'line'
		}
		else	// !BINARY = TEXT
		{
			// TEXT FILE
			// =========
         // FIX20010328 Fix for UNIX file with ONLY an 0x0a
         // extract the line from the buffer
            g_dwBgnLine = pdw[0];  // keep offset of beginning of 'line'
			for( dwo = pdw[0]; dwo < dwmax; dwo++ )
			{
				c = lpmf[dwo];
				//if( c == 0x0d )
				if( ( c == 0x0d ) || ( c == 0x0a ) )
				{
					// normally just end this line output
					if( dwk == 0 )    // BUT if we have NOTHING
					{
						if( (dwo+1) < dwmax )
						{
							if( lpmf[dwo+1] == 0x0a )
								dwo++;
						}
						continue;   // and continue for more
					}
					else
               {
						break;      // else OUT of here
               }
				}
				if( c > 0x7f )
				{
					lpLine[dwk++] = '@';
					c &= 0x7f;
				}
				if( c < ' ' )
				{
					if( c != '\t' )
					{
						lpLine[dwk++] = '^';
						c += '@';
					}
				}
#ifdef   ADDDBL2  // FIX20001022 - fix find of "'@'" - and REMOVE this "doubling"
				else if( c == '^' )
				{
					lpl[dwk++] = '^';
				}
				else if( c == '@' )
				{
					lpl[dwk++] = '@';
				}
#endif   // ADDDBL2
                if (dwo == g_dwOffBgn)
                    g_dwLBOffBgn = dwk; // save BEGIN of find in line buffer
                if (dwo == g_dwOffEnd)
                    g_dwLBOffEnd = dwk; // save BEGIN of find in line buffer
				lpLine[dwk++] = c;		// add this char to OUTPUT
				// check max.
				// ==========
#ifdef USE_ALLOCATED_LINE_BUFFER    // glpLineBuf & gdwLBSize
                if (dwk >= dwCurrMax) {
                    // TIME TO RE-ALLOCATE MORE LINE BUFFER
                    ReAllocate_LineBuffer(pWS);
                    dwCurrMax = gdwLBSize - 4; // set NEW max size 
                    lpLine = glpLineBuf;	// ALLOCATED or FIXED &gcLineBuf[0]
                }
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
				if( dwk > dwCurrMax )	// gdwMax1Line
				{
					break;
				}
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER y/n

			}	// for( dwo = pdw[0]; dwo < dwmax; dwo++ )
            g_dwEndLine = dwo;  // keep offset of end of 'line'
         // FIX20010328 Fix for UNIX file searching
         // we exited on an 0x0d or now also an 0x0a character
		}  // binary or not

		lpLine[dwk] = 0;    // ZERO TERMINATE LINE BUFFER
	}
   UNREFERENCED_PARAMETER(pln_NOT_USED);
   UNREFERENCED_PARAMETER(dwmxvb_NOT_USED);
	return dwk;    // return the LENGTH of line collected
}


#ifdef   ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT - NOT COMPLETED 20050304
// Add it to the LIST of OUTPUTS
// =============================
void  addprt( char * lpo ) { add2outlist( lpo ); }
#endif   // ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT


/* -------------------------------------------------------
   void  DoOutDOSName( WS )

  DESCRIPTION: To construct an 8.3 DOS form of the full path name.
  
  ACTIONS:
  1. Split the glpActive into PATH and NAME using
      the gszFULPth and gszFULNm buffers.
  2. If we have a PATH, and it is NOT the same as the last, try to
      find and construct an 8.3 form of the PATH into gszDOSPth buffer.
      This uses the gszDOSLast buffer to chop into pieces, and
      put the resulting 8.3 'finds' into gszDOSPth.
  3. Try to get an 8.3 DOS form of the file NAME into gszDOSNm buffer.
  4. Combine gszDOSPth and gszDOSNm into gszDOSWork buffer, and
      output this 8.3 form (if -8 on command line).

  -------------------------------------------------------- */
#ifdef WIN32
void  DoOutDOSName( WS )
{
   int   i, j, k;
   WIN32_FIND_DATA   fd;
   HANDLE   hFind;
   SplitFN( gszFULPth, gszFULNm, glpActive );
//#ifdef	FIX20000919    // -8 fix to show full path if the same as previous
// do NOT kill this here !!!
//   gszDOSPth[0] = 0; // restart the buffer
   k = strlen(gszFULPth);
   if( ( k ) &&
      ( strcmpi( gszDOSLast, gszFULPth ) ) )
   {
      // we have a path, and NOT the SAME as the LAST
      // work hard to get the DOS 8.3 type PATH
      // ********************************************
//#ifdef	FIX20000919    // -8 fix to show full path if the same as previous
// Now we know it is NOT valid for this entry, kill the old !!!
      gszDOSPth[0] = 0; // restart the 8.3 form of path buffer
      j = 0;
      for( i = 0; i < k; i++ )
      {
         if( glpActive[i] == PATH_CHAR )
         {
            if( glpActive[j] == ':' )
            {
               gszDOSLast[i] = 0;
               strcpy(gszDOSPth,gszDOSLast); // copy in the DRIVE
               strcat(gszDOSPth,PATH_SEP);
            }
            else
            {
               // we appear to HAVE a path
               gszDOSLast[i] = 0;   // this is our NEXT FIND
               hFind = FindFirstFile(gszDOSLast,&fd);
               if( VH(hFind) )
               {
                  FixAlternate(pWS,&fd);
                  strcat(gszDOSPth,&fd.cAlternateFileName[0]);
                  strcat(gszDOSPth,PATH_SEP); // add in the PATH
                  FindClose(hFind);
               }
               //else
               // what happens if this in NOT found
               // *** SHOULD NEVER HAPPEN ***
            }
         }
         gszDOSLast[i] = glpActive[i];
         j = i;
      }  // for the PATH name
   }  // we have a path, and NOT the SAME as the LAST = get NEW

   // now the FILE NAME itself
   hFind = FindFirstFile(glpActive,&fd);
   if( VH(hFind) )
   {
      FixAlternate(pWS,&fd);
      strcpy(gszDOSNm,&fd.cAlternateFileName[0]);
      FindClose(hFind); // NOTE: we may have a VERB3 active
      // but it only uses the fd structure
   }
   else
   {
      // can ONLY copy what we have!!!
      sprintf( lpVerb, "%s (FF!)", glpActive );
   }

   // combine -
   // gszDOSPth - the 8.3 form of the path, with
   // gszDOSNm  - the 8.3 form of the file name,
   // INTO gszDOSWork buffer
   strcpy(gszDOSWork,gszDOSPth);
   strcat(gszDOSWork,gszDOSNm);

   if( VH(hFind) )
   {
      if( VERB3 )
      {
            strcpy( lpVerb, gszDOSWork );
            strcat( lpVerb, " " );
//#define  MINFNSP           40
            while( strlen(lpVerb) < MINFNSP )
               strcat( lpVerb, " " );
            AddDateTime( lpVerb,		// Destination buffer
				   &fd,	// Information
				   TRUE );	// 24-Hour time
            // Add2gsFileList(pWS);
      }
      else
      {
         sprintf( lpVerb, "%s", gszDOSWork );
      }
   }
   else
   {
      sprintf( lpVerb, "%s (FAIL!)", glpActive );  // = W.ws_glpActive
   }

   strcat(lpVerb,PRTTERM);

   DO_OUT(lpVerb);

   gfDoneFile = TRUE;

   // update LAST full folder name (with "\" tail if got path.
   strcpy( gszDOSLast, gszFULPth );

}
#endif // WIN32

void  Add2FFList( WS )
{
   PLE      ph = &g_sFFList;
   PLE      pn;
   char *   lpf;
   uint32_t    dwl;

   dwl = strlen( glpActive );
   pn  = (PLE)MALLOC( LPTR, (sizeof(LIST_ENTRY) + dwl + 1) );
   if(!pn)
   {
      prt( "ERROR: Memory allocation FAILED!"MEOR );
      exit(0);
   }
   lpf = (char *)pn;
   lpf += sizeof(LIST_ENTRY);
   strcpy(lpf, glpActive);
   InsertTailList(ph,pn);
   g_pFFList = pn;
}

void OutGFName_with_Info( WS )
{
    int b = 0;
#ifdef USE_WIN32_FIND_DATA // WIN32
         // Get File Info
//typedef struct _BY_HANDLE_FILE_INFORMATION {
//  uint32_t    dwFileAttributes; 
//  FILETIME ftCreationTime; 
//  FILETIME ftLastAccessTime; 
//  FILETIME ftLastWriteTime; 
//  uint32_t    dwVolumeSerialNumber; 
//  uint32_t    nFileSizeHigh; 
//  uint32_t    nFileSizeLow; 
//  uint32_t    nNumberOfLinks; 
//  uint32_t    nFileIndexHigh; 
//  uint32_t    nFileIndexLow; 
//} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION;
// FindFirstFile
//typedef struct _WIN32_FIND_DATA {
//  uint32_t    dwFileAttributes; 
//  FILETIME ftCreationTime; 
//  FILETIME ftLastAccessTime; 
//  FILETIME ftLastWriteTime; 
//  uint32_t    nFileSizeHigh; 
//  uint32_t    nFileSizeLow; 
//  uint32_t    dwReserved0; 
//  uint32_t    dwReserved1; 
//  char    cFileName[ MAX_PATH ]; 
//  char    cAlternateFileName[ 14 ]; 
//} WIN32_FIND_DATA, *PWIN32_FIND_DATA; 
   PBY_HANDLE_FILE_INFORMATION phi = &gsFileInf;
   PWIN32_FIND_DATA pfd = &gsFindData;
   b = GetFileInformationByHandle(
      ghActFile,  // handle to file
      &gsFileInf );  // LPBY_HANDLE_FILE_INFORMATION lpFileInformation // buffer
   if(b)
   {
      gsFindData.dwFileAttributes = gsFileInf.dwFileAttributes;
      pfd->ftLastWriteTime = phi->ftLastWriteTime;
      pfd->nFileSizeHigh = phi->nFileSizeHigh;
      pfd->nFileSizeLow = phi->nFileSizeLow;
      strcpy( lpVerb, glpActive );  // = W.ws_glpActive
      strcat( lpVerb, " " );
   //#define  MINFNSP           40
      while( strlen(lpVerb) < MINFNSP ) {
         strcat( lpVerb, " " );
      }
      AddDateTime( lpVerb,		// Destination buffer
		   pfd,	// Information
		   TRUE );	// 24-Hour time
      // Add2gsFileList(pWS);
   }
   else
#endif  // WIN32
   {
      sprintf( lpVerb, "%s (f)"PRTTERM, glpActive );  // = W.ws_glpActive
   }
}

//	if( !gfDoneFile )    // have we already DONE this file name
// void  DoOutFileName( WS )
void  OutGFName( WS )
{
#ifdef WIN32
   if(gbDOSNm) {
      DoOutDOSName( pWS );
   } else 
#endif // WIN32
   {
      if( VERB3 )
      {
         OutGFName_with_Info( pWS );
      }
      else
      {
         if( g_bNoNewLine ) {
            sprintf( lpVerb, "%s"PRTTERM, glpActive );
         } else {
            sprintf( lpVerb, PRTTERM"%s"PRTTERM, glpActive );
         }
      }

      DO_OUT(lpVerb);

		gfDoneFile = TRUE;   // set as TRUE - we HAVE output the FILE NAME

   }
}

// Show File Name - Out File Name
//	if( !gfDoneFile )    // have we already DONE this file name
void  DoOutFileName( WS )
{
   g_dwFoundInFiles++;   // FIX20081003 - show found in files count at end

   Add2FFList( pWS );   // add it to the FIND list anyway

   if( g_bCVSDate )
      return;        // no output yet

   if (!g_bNoFile)  // FIX20120407 - if do NOT have -NF switch
       OutGFName( pWS );

}

char *   GetMakeLine( char * lpm, uint32_t dwm )
{
   uint32_t   dwi, dwii, dwk;
   char *     lpr = 0;
   int        bcont = FALSE;
   int        c;

   dwk = 0;
   c = 0;
   for( dwi = 0; dwi < dwm; dwi++ )
   {
      c = lpm[dwi];
      if( c > ' ' )
         break;
   }
   if( ( dwi < dwm ) &&
       ( c   > ' ' ) )
   {
      dwk++;   // we have at least one letter
      dwii = dwi + 1;
      for( ; dwii < dwm; dwii++ )
      {
         c = lpm[dwii];
         if( ( c < ' ' ) && ( c != '\t' ) )
         {
            if( !bcont )
               break;
            // we are continuing on along the line, but get over the Cr/Lf
            dwk++;   // count a space type
            dwii++;  // and bump to next
            for( ; dwii < dwm; dwii++ )
            {
               c = lpm[dwii];
               if( c > ' ' )
               {
                  dwii--;  // backup to this char
                  break;   // and continue next line
               }
            }
            bcont = FALSE;
         }
         else
         {
            dwk++;
            if( c == PATH_CHAR )
               bcont = TRUE;
         }
      }

      lpr = (char *)MALLOC( LPTR, (dwk+1) );  // allocate sufficient memory
      if(lpr)
      {
         dwk = 0;
         bcont = FALSE;
         dwii = 1;
         for( ; dwi < dwm; dwi++ )
         {
            c = lpm[dwi];
            if( c > ' ' )
            {
               if( c == PATH_CHAR )
               {
                  bcont = TRUE;     // got a LINE continuation character
               }
               else
               {
                  lpr[dwk++] = (char)c;
                  dwii = 0;
               }
            }
            else
            {
               // is a space or less
               if( ( c < ' ' ) && ( c != '\t' ) )
               {
                  if( !bcont )
                     break;
                  dwi++;  // and bump to next
                  for( ; dwi < dwm; dwi++ )
                  {
                     c = lpm[dwi];
                     if( c > ' ' )
                     {
                        dwi--;
                        break;
                     }
                  }
                  bcont = FALSE;
               }

               if( dwii == 0 )
               {
                  lpr[dwk++] = ' ';    // add a SPACE
                  dwii++;              // but ONLY one
               }
            }
         }
      }
   }

   return lpr;
}
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : ChkMake2
// Return type: uint32_t 
// Arguments  : WS
//            : char * lpv
//            : uint32_t dwk
// Description: Further check of this OUTPUT line for
//              (a) $(macro) expansion, and (b) inhibit if comment line
///////////////////////////////////////////////////////////////////////////////
uint32_t ChkMake2( WS, char * lpv, uint32_t dwk, uint32_t dwo )
{
   uint32_t    dwr = dwk;        // return this if nothing found
   uint32_t    dwm = gdwActLen;  // this is the FULL length of buffer
   char *   p = lpv;
   char *   lpb = (char *)glpMapView;  // and the buffer itself
   int      c, d, c2, d2;
   uint32_t    dwl;
   char *   ptmp;
   char *   lpml = 0;
   uint32_t    dwi, dwil;
   char *   lpm;
   int     bGotM = FALSE;

   c = *p;
   while( c )
   {
      if( c == '#' )
         return 0;   // forget this line
      else if( c > ' ' )
         break;
      p++;           // bump to next
      c = *p;        // and get character
   }

   if( c == 0 )
      return 0;   // was an ALL blank line - forget it

   d = 0;            // initialise previous
   dwl = strlen(p);  // get the length remaining in verbal buffer
   while( dwl >= 4 ) // minimum is $(I)
   {
      if( ( c == '$' ) &&
          ( d <= ' ' ) )
      {
         int   iPos1, iPos2;
         // we know we have at least 4
         iPos1 = InStr( &p[1], "(" );
         iPos2 = InStr( &p[1], ")" );
         if( iPos1 && iPos2 && (iPos1 < iPos2) )
         {
            // we have a MACRO
            lpm = Mid( &p[1], (iPos1+1), (iPos2 - iPos1 - 1) );
            dwl = TrimIB(lpm);   // Trim in buffer
            if(dwl)
            {
               if( dwo > dwm )
                  dwo = dwm;
               dwo -= (dwl - 1);    // reduce by lenght of search strin
               // actually could reduce more since MUST find "=1" after it
               c = toupper(*lpm);   // get first char
               d2 = 0;
               for( dwi = 0; dwi < dwo; dwi++ )
               {
                  // search the buffer for this MACRO
                  c2 = toupper(lpb[dwi]);
                  if( ( c2 == c   ) &&
                      ( d2 <= ' ' ) )
                  {
                     // we have a first char match - is this it? the macro word
                     ptmp = &lpb[dwi];
                     for( dwil = 1; dwil < dwl; dwil++ )
                     {
                        //if( toupper(lpb[dwi+dwil]) != toupper(lpm[dwil]) )
                        if( toupper(ptmp[dwil]) != toupper(lpm[dwil]) )
                           break;
                     }
                     c2 = 0;
                     if( dwil == dwl )
                     {
                        // matched for LENGTH
                        dwil++;        // bump to next
                        //dwil += dwi;   // and add in current offset
                        c2 = ptmp[dwil];
                        if( c2 <= ' ' )
                        {
                           dwil++;
                           for( ; dwil < dwo; dwil++ )
                           {
                              c2 = ptmp[dwil];
                              if(c2 > ' ' )
                                 break;
                           }
                        }
                     }
                     if( c2 == '=' )
                     {
                        // This appears to be IT
                        dwil++;
                        ptmp = &ptmp[dwil];  // update to the MAKEFILE line begin
                        lpml = GetMakeLine( ptmp, (dwo - dwil) );
                        if(lpml)
                           bGotM = TRUE;
                        break;
                     }
                  }
                  d2 = c2;    // keep previous
               }  // while collecting the MACRO line
            }
         }
      }
      if( bGotM )
         break;   // all done
      d = c;   // keep previous
      p++;     // bump to next
      c = *p;  // get next char
      dwl--;   // reduce remaining length
   }
   if( lpml )
   {
      dwl = strlen(lpml);  // get the length of the ADDITION
      dwi = strlen(lpv);
      if( lpv[dwi-1] >= ' ' )
         strcat(lpv, "=="MEOR);
      lpm = lpml;
      while(dwl > 70)
      {
         for(dwi = 70; dwi >= 0; dwi--)
         {
            if( lpm[dwi] <= ' ' )
               break;
         }
         if( lpm[dwi] > ' ' )
         {
            for(dwi = 70; dwi < dwl; dwi++ )
            {
               if( lpm[dwi] <= ' ' )
                  break;
            }
         }
         ptmp = Left(lpm, dwi);
         strcat(lpv,ptmp);
         strcat(lpv,MEOR);
         dwl -= dwi;
         lpm = &lpm[dwi];
      }

      strcat(lpv,lpm);
      dwr = strlen(lpv);   // update the OUTPUT length
      MFREE(lpml);
   }

   return dwr;
}


///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Move2Find
// Return type: void 
// Arguments  : WS
//            : char * lpd
//            : char * pFind
// Description: COPY the string to find to a common buffer,
//              taking care of CASE switch.
///////////////////////////////////////////////////////////////////////////////
// void	Move2Find( WS, char * lpd, char * pFind )
void	Move2Find( char * lpd, char * pFind )
{
	uint32_t i, j;
	if( g_fCase )
	{
		// Retain INPUT case
		strcpy( lpd, pFind );	// Just a COPY
	}
	else	// Convert to UPPER CASE
	{
		i = strlen( pFind );

		for( j = 0; j < i; j++ )
			lpd[j] = (char)toupper( pFind[j] );

		lpd[j] = 0;
	}

}

int KindofCompare( char * lpex, char * lpfn )
{
   int	flg = FALSE;
   if( strcmpi(lpex,lpfn) == 0 ) {
      flg = TRUE;
   } else {
	   int		ii, jj;
	   int      c, d;
	   int      i = strlen( lpex );
      int      j = strlen( lpfn );
	   if( i && j ) {
		   ii = jj = 0;
		   while( i && j ) {
			   c = toupper(lpex[ii]);
			   d = toupper(lpfn[jj]);
			   if( ( c == d ) ||
				   ( c == '?' ) ) {
				   // JUST GO TO NEXT char IN EACH
				   ii++;
				   jj++;
				   i--;
				   j--;
				   if(( j == 0 ) && ( i == 0 ) ) {
					   // both ended
					   flg = TRUE;
					   break;	// not really required
					   // since would exit the loop anyway.
				   } else if( i == 0 ) {
                  if(( j == 1 ) && ( lpfn[jj] == '*')) {
					      flg = TRUE; // assume * is NONE or many
					      break;	// not really required
                  } else if( (i == 3) && (strcmp(&lpfn[jj],"*.*") == 0)) {
                      // FIX20101026 - make file == file*.*
                      flg = TRUE;
                  }
               } else if( j == 0 ) {
                  if(( i == 1 ) && ( lpex[ii] == '*')) {
					      flg = TRUE; // assume * is NONE or many
					      break;	// not really required
                  } else if( (i == 3) && (strcmp(&lpex[ii],"*.*") == 0)) {
                      // FIX20101026 - make file == file*.*
                      flg = TRUE;
                  }
               }

			   } else if( c == '*' ) {
				   if( i == 1 ) {
					   // last char of EXCLUDE is WILD
					   // that means it matches everything
					   flg = TRUE;
					   break;
               } else {	// There are move chars in EXCLUDE
					   while(( c == '*' ) &&
							   ( i > 0    ) ) {
						   ii++;
						   i--;
						   c = toupper(lpex[ii]);
					   }
					   if( c == '*' ) {
						   // all WILD chars
						   flg = TRUE;
						   break;
					   }
					   while( j ) {
						   if( c == d ) {
							   // found this char in filename
							   break;
						   }
						   jj++;
						   j--;
						   d = toupper(lpfn[jj]);
					   }
					   if( j == 0 ) {
						   // ran OUT of chars in FILENAME
						   // while looking for c
						   // Special case of *.*
						   if(( c == '.' ) &&
							   ( i == 2   ) &&
							   ( lpex[ii+1] == '*' ) ) {
							   // This is a match between say
							   // lpex = "temp*.*" and
							   // lpf  = "temps
							   flg = TRUE;
							   break;
						   }
					   }
				   }
			   } else {
				   // THEY ARE DIFFERENT
				   break;
			   }
		   }
	   }
   }
	return flg;
}

int	CompKind( WS, char * lpact, char * lpex, char * lpdir, char * lpfn )
{
	int	flg = FALSE;
	// first a simple compare
	if( strcmpi( lpact, lpex ) == 0 ) {
		flg = TRUE;
	} else {
      flg = KindofCompare( lpex, lpfn );
	}
   UNREFERENCED_PARAMETER(lpdir);
   UNREFERENCED_PARAMETER(pWS);

	return flg;
}

int	InExclude( WS, char * lpact )
{
	int	flg = FALSE; // assume NOT
	char *lpd, *lpf, *lps;
	if(( lpact     ) &&
		( *lpact    ) &&
		( gnExclLen ) ) {
#ifdef USE_EXCLUDE_LIST
      PLE ph = &g_sExclList;
      PLE pn;
		lpd = gszFolder;
		lpf = szDir2;
		SplitFN( lpd, lpf, lpact );
      Traverse_List( ph, pn ) {
         lps = (char *)pn;
         lps += sizeof(LIST_ENTRY);
			if( CompKind( pWS, lpact, lps, lpd, lpf ) ) {
				flg = TRUE;
				break;
			}
      }
#else // !#ifdef USE_EXCLUDE_LIST
   	int	i;
		lpd = &gszFolder[0];
		lpf = &szDir2[0];
		SplitFN( lpd, lpf, lpact );
		lps = &gszExclude[0];   // get the EXCLUDE buffer
		while( ( i = strlen(lps) ) > 0 )   // while there are 0 terminated entries
		{
			if( CompKind( pWS, lpact, lps, lpd, lpf ) )
			{
				flg = TRUE;
				break;
			}
			lps = lps + i + 1;   // bmp to next exclude
		}
#endif // #ifdef USE_EXCLUDE_LIST y/n
	}
	return flg;
}

#ifdef USE_EXCLUDE_LIST

int	InExcludeD( char * lpact )
{
	int	flg = FALSE; // assume NOT
	char *	lps;
	if(( lpact     ) &&
		( *lpact    ) &&
		( g_dwExclDCnt ) ) {
      PLE ph = &g_sExclListD;
      PLE pn;
      Traverse_List( ph, pn ) {
         lps = (char *)pn;
         lps += sizeof(LIST_ENTRY);
         if( KindofCompare( lps, lpact ) ) {
			//if( strcmpi( lpact, lps ) == 0 ) {
				flg = TRUE;
				break;
			}
      }
	}
	return flg;
}

#endif // #ifdef USE_EXCLUDE_LIST

uint32_t Check_If_Uncode( void )
{
   uint32_t 	dwfs = g_dwActLen;
   unsigned char *lpc = (unsigned char *)g_lpMapView;
   unsigned char c1, c2, c3;
   uint32_t    bom;
   if( dwfs > 3 ) {
      c1 = lpc[0];
      c2 = lpc[1];
      c3 = lpc[2];
      bom = (c1 << 8) + c2;
      if ( bom == UNICODE_BOM_BE )
         return UTF16BE;
      if ( bom == UNICODE_BOM_LE )
         return UTF16LE;
      if (((c1 << 16) + (c2 << 8) + c3) == UNICODE_BOM_UTF8)
         return UTF8;
   }
   return (uint32_t)CHENC_UNK;
}

int  Is_Binary_File( void )
{
   // check if it is binary
   uint32_t 	dwfs = g_dwActLen;
   uint32_t    dwi;
   unsigned char *lpc = (unsigned char *)g_lpMapView;
   unsigned char c;
   if(( g_dwCharEncloding == UTF16BE ) ||
      ( g_dwCharEncloding == UTF16LE ) ||
      ( g_dwCharEncloding == UTF8    ))
      return FALSE;

   for( dwi = 0; dwi < dwfs; dwi++ )
   {
      c = lpc[dwi];
      if( c < ' ' ) {
         if ( !(( c == '\r') ||
            ( c == '\n' ) ||
            ( c == '\t' ) ||
            ( c == '\015' )) ) {   // allow FF also
            // ASSUME NOT BINARY
         	char *	lpInFil = g_lpActive;
            if( lpInFil ) {
               g_dwExclCnt++;
		         if( GVERB5 )
		         {
			         if( !g_fDoneFile )
			         {
				         sprintf( g_lpVerb, "%s"PRTTERM, g_lpActive );
				         prt( g_lpVerb );
				         g_fDoneFile = TRUE;
			         }
			         prt( "File EXCLUDED from find list due to is BINARY (-b-)."PRTTERM );
		         }
            }
            return TRUE;
         }
      }
      if( dwi > MAX_BIN_SEARCH )
         break;
   }

   return FALSE;
}

// ===============================================================
// void	Find_In_File( WS )
//
// Process a FILE, before MAPPING, for the FIND STRING(s)
//
// ===============================================================
void	Find_In_File( WS )
{
	int	fInh;
	char *	lpInFil;

	lpInFil = glpActive;
	if( ( lpInFil) &&
		( !InExclude( pWS, glpActive ) ) )
	{

		// set the TYPE
		SetFileType( lpInFil, &g_FileType );  
		if( g_FileType & (FT_ISC | FT_ISA) )
			fInh = fInhibit;
		else
			fInh = FALSE;

//		if( g_fCase || gfParity )
		if( g_fCase )
		{
			// Use TYPE 2
			FindInType = &Find_In_Type2;
		}
		else
		{
			// Use TYPE 1
			FindInType = &Find_In_Type1;
		}

		if( GetFileMap( pWS ) )
		{
         if ( g_fIgnoreBin && g_bIsBinary ) {
			   ShutFileMap( pWS );
         } else {
			   (*FindInType) ( pWS, lpInFil );
			   ShutFileMap( pWS );
            g_dwDoneCnt++;
         }
      } else {
         g_dwMapFailedCnt++;
      }
	}
	else if( lpInFil )
	{
		// This active file is EXCLUDED
      g_dwExclCnt++;
		if( VERB5 )
		{
			if( !gfDoneFile )
			{
				sprintf( lpVerb, "%s"PRTTERM, glpActive );
				prt( lpVerb );
				gfDoneFile = TRUE;
			}
			prt( "File EXCLUDED from find list."PRTTERM );
		}
		else
		{
#ifndef	NDEBUG
#ifdef	ADDDIAGT
			if( !gfDoneFile )
			{
				sprintf( lpVerb, "%s"PRTTERM, glpActive );
				dout( lpVerb );
			}
			dout( "File EXCLUDED from find list."PRTTERM );
#endif // #ifdef	ADDDIAGT
#endif	// !NDEBUG
		}
	}

}	// end - void	Find_In_File( WS )

#ifdef WIN32
/////////////////////////////////////////////////////////////////
void  AddFInfo( WS, int bAddWarn, int bLocal )
{
   HANDLE   hFind;
   WIN32_FIND_DATA fd;
   SYSTEMTIME  st;
   FILETIME    ft;
   int        flg1, flg2;
   
   flg1 = flg2 = FALSE;
   hFind = FindFirstFile(glpActive, &fd);
   if( ( hFind ) &&
       ( hFind != INVALID_HANDLE_VALUE ) )
   {
      if( bLocal )
      {
         if( FileTimeToLocalFileTime(
            &fd.ftCreationTime,  // UTC file time to convert
            &ft ) )  // converted file time to LOCAL time
         {
            flg2 = TRUE;   // flag NOT UTC
         }
         else
         {
            // we have NO conversion
            if( bAddWarn )
            {
               strcat( lpVerb, "WARNING: Local FAILED!!!" );
               flg1 = TRUE;
            }
            // just move the UTC
            ft.dwHighDateTime = fd.ftCreationTime.dwHighDateTime;
            ft.dwLowDateTime  = fd.ftCreationTime.dwLowDateTime;
            flg2 = FALSE;
         }
      }
      else
      {
         // just move the UTC
         ft.dwHighDateTime = fd.ftCreationTime.dwHighDateTime;
         ft.dwLowDateTime  = fd.ftCreationTime.dwLowDateTime;
         flg2 = FALSE;
      }
      if( FileTimeToSystemTime(
         &ft,  // &fd.ftCreationTime,  // file time to convert
         &st ) )  // receives system time
      {
         sprintf( EndBuf(lpVerb),
            " %02d/%02d/%02d %02d:%02d",
            (st.wDay & 0xffff),
            (st.wMonth & 0xffff),
            (st.wYear % 100),
            (st.wHour & 0xffff),
            (st.wMinute & 0xffff) );
         // ========================
         if( !flg2 )
            strcat(lpVerb, " UTC");
      }
      else if( bAddWarn && !flg1 )
      {
            strcat( lpVerb, " WARNING: No TIME conversion!!!" );
      }

      FindClose(hFind);
   }
   else if( bAddWarn && !flg1 )
   {
      strcat( lpVerb, " WARNING: Unable to FIND!!!" );
   }
}
#else // !WIN32
void  AddFInfo( WS, int bAddWarn, int bLocal )
{
   if (IsValidFile(glpActive) ) {
       uint64_t ft = get_last_file_mtime64();
       struct tm *pt = 0;
       if (bLocal)
           pt = localtime((time_t *)&ft);
       else
           pt = gmtime((time_t *)&ft);
       if (pt) {
           sprintf(EndBuf(lpVerb),
               "%4d/%02d/%02d  %02d:%02d",
               pt->tm_year + 1900,
               pt->tm_mon,
               pt->tm_mday,
               pt->tm_hour,
               pt->tm_sec );
           if (!bLocal)
               strcat(lpVerb," UTC");
       } else if (bAddWarn) {
           if (bLocal)
               strcat(lpVerb,"WANRING: locatime() FAILED");
           else
               strcat(lpVerb,"WANRING: gmtime() FAILED");
       }
   } else if (bAddWarn) {
       strcat( lpVerb, " WARNING: Unable to FIND!!!" );
   }
}
#endif // WIN32

/* ================================================================
 * void	ShowFNS( WS )
 *
 */
void	ShowFNS( WS )
{
	sprintf( lpVerb,
		"Processing [%s] of ",
		ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
	PutThous( EndBuf(lpVerb),
		12,
		gdwActLen );
	strcat( lpVerb, " bytes." );
   // add more info???
   if( VERBM )
      AddFInfo( pWS, TRUE, TRUE );
   strcat( lpVerb, PRTTERM );
	prt( lpVerb );
}

// ======================================================
// void	ShowFind1( WS, uint32_t dwFind1, char * lpf, int doregex )
//
// Presently ONLY used if VERBM
//
// ======================================================
void	ShowFind1( WS, uint32_t dwFind1, char * lpf, int doregex )
{
	if( dwFind1 ) {
      sprintf( lpVerb, "Found: %s[%s] %u time%s..."PRTTERM,
         (doregex ? "regex" : ""),
         lpf,
			dwFind1,
         ((dwFind1 == 1) ? "" : "s") );
	} else {
		sprintf( lpVerb, "NO finds of %s[%s] ..."PRTTERM,
         (doregex ? "regex" : ""),
			lpf );
	}
	prt( lpVerb );
}

void  prterr( char * lpm )
{
   prt(lpm);
#ifdef WIN323
   if( bRedirON )
   {
      uint32_t dww;
      WriteFile( hErrOut, lpm, strlen(lpm), &dww, NULL );
   }
#endif // WIN32
}

#ifdef WIN32
//char g_szFailMsg[] = "WARNING: Failed to %s [%s]";
// ===========================================================
// void	ShowFail( WS, int i, PWIN32_FIND_DATA pfd )
//
// ===========================================================
void	ShowFail( WS , int i, PWIN32_FIND_DATA pfd )
{
   PLE   ph = &g_sFailList;
   PMFILE pmf;
   sprintf( lpVerb,
		   g_szFailMsg,   // "WARNING: Failed to %s [%s]"PRTTERM,
         ((i == 1) ? "open directory" : (i == 2) ? "open file" : "find file"),
		   glpActive );
   sprintf(EndBuf(lpVerb), "(%d)", i );
   if(pfd) {
      strcat(lpVerb, " ");
      AddDateTime2( EndBuf(lpVerb), pfd, TRUE, ds_english );
   }
   strcat(lpVerb,PRTTERM);
   pmf = (PMFILE)MALLOC( LPTR, sizeof(MFILE) + strlen(lpVerb) );
   CHKMEM(pmf);
   strcpy( pmf->cFile, lpVerb );
   InsertTailList(ph,(PLE)pmf);
	if( VERB ) {
		prterr( lpVerb );
	} else {
#ifdef	ADDDIAGT
		dout( lpVerb );
#endif	// ADDDIAGT
	}
}
#else // !WIN32
void	ShowFail( WS , int i )
{
   PLE   ph = &g_sFailList;
   PMFILE pmf;
   sprintf( lpVerb,
		   g_szFailMsg,   // "WARNING: Failed to %s [%s]"PRTTERM,
         ((i == 1) ? "open directory" : (i == 2) ? "open file" : "find file"),
		   glpActive );
   sprintf(EndBuf(lpVerb), "(%d)", i );
   strcat(lpVerb,PRTTERM);
   pmf = (PMFILE)MALLOC( LPTR, sizeof(MFILE) + strlen(lpVerb) );
   CHKMEM(pmf);
   strcpy( pmf->cFile, lpVerb );
   InsertTailList(ph,(PLE)pmf);
	if( VERB ) {
		prterr( lpVerb );
	} else {
#ifdef	ADDDIAGT
		dout( lpVerb );
#endif	// ADDDIAGT
	}
}

#endif // WIN32

// ===========================================================
// void	ShowZero( WS )
//
// ===========================================================
void	ShowZero( WS )
{
	sprintf( lpVerb,
		"WARNING: File [%s] is ZERO length."PRTTERM,
		ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
	if( VERB )
	{
		prterr( lpVerb );
	}
	else
	{
#ifdef	ADDDIAGT
		dout( lpVerb );
#endif	// ADDDIAGT
	}
}

// ===========================================================
// void	ShowNoMap( WS )
//
// ===========================================================
void	ShowNoMap( WS )
{
	sprintf( lpVerb,
		"WARNING: Failed to MAP file [%s]"PRTTERM,
		ShortName( glpActive, glpTmp, W.ws_dwMxFilNm ) );
	if( VERB )
	{
		prterr( lpVerb );
	}
	else
	{
#ifdef	ADDDIAGT
		dout( lpVerb );
#endif	// ADDDIAGT
	}
}

#define prt9(a) if (VERB9) prt5(a)
void	ShowFinding( WS, uint32_t dwItems, PFLINE pl )
{
	if( !gfDoneFile )
	{
      if(VERB5) {
         OutGFName( pWS );
         //sprintf( lpVerb, "%s"PRTTERM, glpActive );
		   //prt5( lpVerb );
		   //if( VERB5 )
			//   gfDoneFile = TRUE;
      }
	}
	if( gdwFindCnt > 1 )
	{
		sprintf( lpVerb,
			"Find #%u of %u %s[%s] ... "PRTTERM,
			dwItems,
			gdwFindCnt,
         pl->isregex ? "by regex" : "raw",
			g_lpCurrFind );
	}
	else
	{
		sprintf( lpVerb,
			"Find of 1 item %s[%s] ... "PRTTERM,
         pl->isregex ? "by regex" : "raw",
			g_lpCurrFind );
	}
	prt9( lpVerb );
}

void	ShutFileMap( WS )
{
#ifdef WIN32
	if( glpMapView )
		UnmapViewOfFile( glpMapView );	// address where mapped view begins

	if( ghActMap )
		CloseHandle( ghActMap );

	if( VH(ghActFile) )
		CloseAFile( ghActFile );
#else 
	if( glpMapView )
        munmap( glpMapView, gdwActLen );
	if( VH(ghActFile) )
		close( (int)ghActFile );

#endif
	glpMapView = 0;
	ghActMap = 0;
	ghActFile = 0;
    gdwActLen = 0;
}

// ===============================================
// int	GetFileMap( WS )
//
// ===============================================
#ifdef WIN32
////////////////////////////////////////////////////////////////////
int	GetFileMap( WS )
{
	int	flg = FALSE;
	int	opn;

	ghActFile  = 0;
	gdwActLen  = 0;
	ghActMap   = 0;
	glpMapView = 0;
	opn = OpenReadFile( glpActive, &ghActFile );
   if(opn)
      gdwActLen = GetFileLen( ghActFile ); // part of GetFileMap(WS)
   if(gdwActLen)
      ghActMap = CreateFileMapping( (HANDLE)ghActFile,	// handle to file to map 
			NULL,	// optional security attributes
			g_flProtext,	// protection for mapping object
			0,				// high-order 32 bits of object size
			0,				// low-order 32 bits of object size
			NULL );			// name of file-mapping object
   if( ghActMap )
      glpMapView = MapViewOfFile( ghActMap,	// file-mapping object to map into address space
					g_ReqAccess,	// access mode
					0,		// high-order 32 bits of file offset
					0,		// low-order 32 bits of file offset
					0 );	// number of bytes to map
   if( glpMapView )
	{
      // set ENCODING, if file starts with a BOM, else -1 = unknown
      g_dwCharEncloding = Check_If_Uncode();
      g_bIsBinary2 = Is_Binary_File();
      g_bIsBinary = FALSE;
      if( g_fIgnoreBin )
         g_bIsBinary = g_bIsBinary2;

		gdwTotBytes += gdwActLen;
		flg = TRUE;
	}
	else
	{
// FIX990123 - show or not show problem in open/size/map
// =====================================================
		if( !opn )
		{
			WIN32_FIND_DATA fd;
			HANDLE find = FindFirstFile( glpActive, &fd );
			if( VFH( find ) )
            {
				if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
               if( VERB6 ) {
                  ShowFail( pWS, 1, &fd );  // "WARNING: Failed to open directory [%s]"PRTTERM,
					}
				} else {
					ShowFail( pWS, 2, &fd );  // "WARNING: Failed to open file [%s]"PRTTERM,
				}
				FindClose(find);
			} else {
				ShowFail( pWS, 3, NULL );  // "WARNING: Failed to find file [%s]"PRTTERM,
			}
		}
		else if( !gdwActLen )
		{
			if( VERB6 ) {
				ShowZero( pWS );
			}
		} else {
			ShowNoMap( pWS );
		}
		ShutFileMap( pWS );
// =====================================================
	}
	return flg;
}

#else // !WIN32
int	GetFileMap( WS )
{
    uint64_t len;
    size_t length;
    int fd;
    char *addr;
	ghActFile  = 0;
	gdwActLen  = 0;
	ghActMap   = 0;
	glpMapView = 0;
    if (!IsValidFile(glpActive)) {
		ShowFail( pWS, 2 );  // "WARNING: Failed to open file [%s]"PRTTERM,
        return 0;
    }
    len = get_last_file_size64();
    fd = open(glpActive, O_RDONLY);
    if ( !fd || (fd == -1) ) {
		ShowFail( pWS, 2 );  // "WARNING: Failed to open file [%s]"PRTTERM,
        return 0;
    }
    length = (size_t)len;
    addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
		ShowFail( pWS, 2 );  // "WARNING: Failed to open file [%s]"PRTTERM,
        return 0;
    }
    gdwActLen = length;
    glpMapView = addr;
	gdwTotBytes += length;
    ghActFile = (HANDLE)fd;
    return 1;
}

#endif // WIN32 y/n


// Set of W.GETCHR = GetChr functions
// ==========================================
char	getachar( char * lpc )
{
	return( *lpc );
}
char	getacharpar( char * lpc )
{
	return( (char)( *lpc & 0x7f ) );
}
char	getupchar( char * lpc )
{
	return( (char)toupper( *lpc ) );
}
char	getupcharpar( char * lpc )
{
	return( (char)( toupper( *lpc ) & 0x7f ) );
}
// ============================================
	
// #ifdef   USEFINDLIST // = PVERS "V4.0.16" // FIX20010703 - order FIND strings
extern void Find_In_Gen( WS, char * lpInF );
extern void Find_In_Lines( WS, char * lpInF );
// #else // NOT #ifdef   USEFINDLIST    // = FIX20010703
// #endif   // #ifdef   USEFINDLIST y/n   // = FIX20010703

void	Find_In_Type1( WS, char * lpInF )
{
	if( gfParity )
		W.GETCHR = &getupcharpar;  // set GetChr to upper without parity
	else
		W.GETCHR = &getupchar;     // set GetChr to just get UPPER
#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   if( g_bAllInOne && ( g_dwFCnt > 1 )) {
      Find_In_Lines( pWS, lpInF );
   } else
#endif // #ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   {
      Find_In_Gen( pWS, lpInF );
   }
}

void	Find_In_Type2( WS, char * lpInF )
{
	if( gfParity )
		W.GETCHR = &getacharpar;   // set GetChr to get without parity
	else
		W.GETCHR = &getachar;      // set GetChr to just get a char
#ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   if( g_bAllInOne && ( g_dwFCnt > 1 )) {
      Find_In_Lines( pWS, lpInF );
   } else
#endif // #ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
   {
      Find_In_Gen( pWS, lpInF );
   }
}


#ifdef	ADDRECUR

typedef struct tagRECURSIVE {
	//WIN32_FIND_DATA	r_fd;
   char             r_szFolder[MAX_PATH];
   char             r_szDir[MAX_PATH];
   char             r_szMask[MAX_PATH];
   char             r_szWork[(MAX_PATH*2)];
} RECURSIVE, * PRECURSIVE;

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Process_Recursive - FIX20140830: now abandonned
// Return type: void 
// Arguments  : WS
//            : char * lpwild
// Description: -R switch to be RECURSIVE into subdirectories
//              
///////////////////////////////////////////////////////////////////////////////
#if (defined(WIN32) && defined(USE_WIN32_DATA))
void	Process_Recursive( WS, char * lpwild ) // FIX20140830: now abandonned
{
    char *lpd, *lpf, *lpmask, *lpfil, *lprm, *lpn;
	HANDLE	        hFind;
	WIN32_FIND_DATA	fd;
	PWIN32_FIND_DATA	pfd;
	int		        mlen;
    PRECURSIVE prc = (PRECURSIVE)MALLOC( LPTR, sizeof(RECURSIVE) );
    CHKMEM(prc);
    pfd = &fd;
	// lpd = &gszFolder[0];
    lpd = prc->r_szFolder;
	//lpf = &szDir2[0]; // CARE: Only ONE USER!!!
    lpf = prc->r_szDir;
	//lpmask = &gszFileMask[0];
	lpmask = prc->r_szMask;
	SplitFN( lpd, lpf, lpwild );
    if( VERB9 ) {
        sprintf( lpVerb, "v9: Recursing into [%s], with mask [%s]"PRTTERM, lpd, lpf );
        prt( lpVerb );
    }
    strcpy( lpmask, lpd );
    lprm = prc->r_szWork;
    strcpy( lprm, lpmask );
	mlen = strlen(lprm);
    strcat( lpmask, "*.*" );
	lpfil = glpActive;	// Get the BUFFER for the file name
    //sprtf( "NOTE: Find using [%s]"MEOR, lpmask );
    //lpn = &fd.cFileName[0];
	//hFind = FindFirstFile( lpmask, &fd );
    lpn = &pfd->cFileName[0];
	hFind = FindFirstFile( lpmask, pfd );
    if( VFH( hFind ) ) {
      do {
         //	( fd.cFileName[0] != '.' ) )
			//if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if( pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            // forget DOT and DOUBLE DOT
            if( strcmp(lpn,".") && strcmp(lpn,"..") ) {
               g_dwFoundDirsCnt++;
               // it is a REAL directory
               // ==============================================================
					lprm[mlen] = 0;      // clear back to the previous FOLDER
					strcat( lprm, lpn ); // &fd.cFileName[0] );  // add this FOLDER
					strcat( lprm, PATH_SEP );  // and separator
					strcat( lprm, lpf );   // and our FILE name/mask
#ifdef USE_EXCLUDE_LIST
               if( InExcludeD( lpn ) ) {
                  g_dwDirsExcl++; // count another match to EXCLUDED
               } else {
   					DoThisFile( pWS, lprm, TRUE );   // if defined(USE_WIN32_DATA)
               }
#else // !#ifdef USE_EXCLUDE_LIST
					DoThisFile( pWS, lprm, TRUE );   // if defined(USE_WIN32_DATA)
#endif // #ifdef USE_EXCLUDE_LIST y/n
               // but the call to here check that,
               // BUT more importantly handles the INTERATION into lower folders
               // =============================================================
            } // no dot or double dot
         } // is a DIRECTORY
      //} while( FindNextFile( hFind, &fd ) );
      } while( FindNextFile( hFind, pfd ) );
		FindClose( hFind );
	}
	MFREE(prc);
}

#else // !WIN32

#if 0 // 000000000000000000000000000000000000000000000000
void Process_Recursive( WS, char * lpwild )  // FIX20140830: now abandonned
{
    char *lpd, *lpf, *lpmask, *lprm;
	DIR *	        hDir;
    PRECURSIVE prc = (PRECURSIVE)MALLOC( LPTR, sizeof(RECURSIVE) );
    CHKMEM(prc);
    lpd = prc->r_szFolder;
    lpf = prc->r_szDir;
	lpmask = prc->r_szMask;
	SplitFN( lpd, lpf, lpwild );
    if (lpd[0] == 0)
        strcpy(lpd,"." PATH_SEP);
    if( VERB9 ) {
        sprintf( lpVerb, "v9: Recursing into [%s], with mask [%s]"PRTTERM, lpd, lpf );
        prt( lpVerb );
    }
    lprm = prc->r_szWork;
	hDir = opendir(lpd);
    if( hDir ) {
        struct dirent *d = readdir(hDir);
        while (d) {  // got a FIND FIRST
            strcpy( lprm, lpd );
            strcat( lprm, d->d_name );
            if (IsValidDir(lprm)) {
                // forget DOT and DOUBLE DOT
                if( strcmp(d->d_name,".") && strcmp(d->d_name,"..") ) {
                    g_dwFoundDirsCnt++;
#ifdef USE_EXCLUDE_LIST
                   if( InExcludeD( d->d_name ) ) {
                       g_dwDirsExcl++; // count another match to EXCLUDED
                   } else {
                       strcat(lprm, PATH_SEP "*");
                       Process_Wilds( pWS, lprm ); // in Process_Recursive() - now abandonned
                   }
#else // !#ifdef USE_EXCLUDE_LIST
                   Process_Wilds( pWS, lprm ); //  // in Process_Recursive() - now abandonned
#endif // #ifdef USE_EXCLUDE_LIST y/n
                    // but the call to here check that,
                    // BUT more importantly handles the INTERATION into lower folders
                } // not dot or double dot
            } // is a valid dir
            d = readdir(hDir);
        } // got an entryY
		closedir(hDir);
	}
	MFREE(prc);
}
#endif // 000000000000000000000000000000000000000000000000000

#endif // WIN32 y/n

#endif	// ADDRECUR

#ifdef WIN32
void  SetDOSNm( WS, char * lpf )
{
   WIN32_FIND_DATA   fd;
   HANDLE   hFind = FindFirstFile(lpf,&fd);

   if( ( hFind ) &&
      ( hFind != INVALID_HANDLE_VALUE ) )
   {
      FixAlternate( pWS, &fd );
      strcpy( gszDOSNm, &fd.cAlternateFileName[0] );
      FindClose( hFind );
   }
}
#endif // WIN32

#if 0 // 0000000000000000000000000000000000000000000000000000000000000000000
// ====================================================================
//	void	DoThisFile( WS, char * lpf )
//	PURPOSE:
//		Process a file or file mask (off the glpFileMem)
// ====================================================================
void	DoThisFile( WS, char * lpf, int bFlg ) // FIX20140830: abandonned service
{
    uint64_t ul1;
	gfDoneFile = FALSE;		// reset DONE FILE name
	if( GotWild( lpf ) )
	{
        if( VERB9 ) {
            sprintf( lpVerb, "v9: Processing WILD: %s"PRTTERM, lpf );
            prt( lpVerb );
        }
		Process_Wilds( pWS, lpf ); // in DoThisFile() - now abandonned
	}
	else
	{
		// make this name / mask ACTIVE
		strcpy( glpActive, lpf );
        if( bFlg || gfRecursive )    // if just a DIRECTORY find, with our file added
        {
            if( IsValidFile(lpf) ) {
                ul1 = get_last_file_size64();
                g_ulTotalBytes += ul1;
                g_dwFoundFileCnt++;
                Find_In_File( pWS );
            } else if (IsValidDir(lpf)) {
                // FIX20070914 - If given a DIRECTORY
                strcat( glpActive, PATH_SEP"*.*" );   // make it WILD
                if( VERB9 ) {
                    sprintf( lpVerb, "v9: Processing WILD: %s adding *.*"PRTTERM, lpf );
                    prt( lpVerb );
                }
		        Process_Wilds( pWS, glpActive ); // in DoThisFile() - now abandonned
            }
        }
        else
        {
		    // go FIND EM ...
            // Process a FILE, before MAPPING, for the FIND STRING(s)
            if( IsValidFile(lpf) )  // ADDED // FIX20030529 - Some failed to open msg????
            {
                ul1 = get_last_file_size64();
                g_ulTotalBytes += ul1;
                g_dwFoundFileCnt++;
                Find_In_File( pWS );
            }
            else
            {
                if( VERB1 ) {  // FIX20080320
                    sprintf( lpVerb, "WARNING: Unable to open [%s]!"PRTTERM, lpf );
                    prt( lpVerb );
                }
                g_iNoOpen++;   // BUMP failed to OPEN
            }
        }
	}

//#ifdef	ADDRECUR
//	if( gfRecursive )
//	{
//		Process_Recursive( pWS, lpf ); // FIX20140830: In DoThisFile, now abandonned
//	}
//#endif	// ADDRECUR
}
#endif // 000000000000000000000000000000000000000000000000000000000000000000000

void  Show_Openning( WS )
{
   if( VERB && !VERB4 )
   {
      if( gfRecursive )
         prt( "Moment. Searching for LATEST update ..."MEOR );
      else
         prt( "Searching for LATEST update locally ..."MEOR );
   }
}

void  Show_Entries( WS )
{
   if( VERB && !VERB4 )
   {
      PLE      ph = &g_sEntries;
      char *   lpt = &g_szTmpBuf[0];
      uint32_t    dwc;

      if( IsListEmpty(ph) )
      {
         sprintf(lpt, "No valid entries found!"MEOR );
         prt(lpt);
      }
      else
      {
         PLE pn = ph->Flink;
         PCVSLN pcvs = (PCVSLN)pn;
         PLE      pf = pcvs->psName;
         //SYSTEMTIME  st;
         ListCount2(ph, &dwc);   // get COUNT in this LIST
         if(pf)
         {
            char *   _lpf = (char *)pf;
            _lpf += sizeof(LIST_ENTRY);
            strcpy(lpt, _lpf);
            strcat(lpt,MEOR);
            prt(lpt);
         }
         strcpy(lpt, &pcvs->szFile[0]);
         strcat(lpt, " ");
         //AppendDateTime( lpt, &pcvs->sSysTm );
         AppendDateTime( lpt, &pcvs->uFT );
         sprintf(EndBuf(lpt), " (LATEST of %d entries)", dwc );
         strcat(lpt, MEOR);
         prt(lpt);

#if 0 // 00000000000000000000000000000000000000000000000000
         GetLocalTime( &st );
         if(( pcvs->sSysTm.wYear == st.wYear ) &&
            ( pcvs->sSysTm.wMonth == st.wMonth ) &&
            ( pcvs->sSysTm.wDay == st.wDay ) )
         {
            prt( "NOTE: This is today's DATE!"MEOR );
         }
         else
         {
            // time()
            FILETIME ft1, ft2;
            strcpy(lpt, "Today is ");
            AppendDateTime( lpt, &st );
            // The FILETIME structure is a 64-bit value representing the number of
            // 100-nanosecond (billionths of a second) intervals since January 1, 1601 (UTC). 
            // ie 0.0000001 seconds since 01/01/1601 (UTC)
            if(( SystemTimeToFileTime( &st, &ft2 ) ) && // system time, file time
               ( SystemTimeToFileTime( &pcvs->sSysTm, &ft1 ) ) )
            {
               LARGE_INTEGER li1, li2, li3;
               double   db;
               li1.HighPart = ft1.dwHighDateTime;
               li1.LowPart  = ft1.dwLowDateTime;
               li2.HighPart = ft2.dwHighDateTime;
               li2.LowPart  = ft2.dwLowDateTime;
               if( li1.QuadPart > li2.QuadPart )
                  li3.QuadPart = li1.QuadPart - li2.QuadPart;
               else
                  li3.QuadPart = li2.QuadPart - li1.QuadPart;
               db = (double) (li3.QuadPart / 10000000);
               //sprintf(EndBuf(lpt), " (Diff=%I64u)", li3 );
               sprintf(EndBuf(lpt), " (D=%s)", DSecs2YDHMSStg( db ) );
            }
            strcat(lpt,MEOR);
            prt(lpt);
         }
#endif // 00000000000000000000000000000000000000000000000000
      }
   }
}

// this is the 4th level main( ... ) CONSOLE OS entry.
void	Process_Finds( WS )
{
	char *	lpf;
	int		len;
	time_t	bt, et;
// #ifdef   USEFINDLIST    // changed to MULTIPLE FILE FIND list = FIX20010703
   PLE   ph = &g_sFileList;
   PLE   pn;
	time(&bt);
	giLineCnt = 0;
	giFileCnt = 0;
   if( g_bCVSDate )
      Show_Openning( pWS );
   Traverse_List( ph, pn )
   {
      PMFILE    pf = (PMFILE)pn;
      lpf = pf->cFile;
      len = strlen(lpf);
      gfDoneFile = FALSE;		// reset DONE FILE name
      Process_Wilds( pWS, lpf ); // FIX20140830: was to DoThisFile( pWS, lpf, FALSE ); now abandonned
   }
   if( g_bCVSDate )
      Show_Entries( pWS );

   // SORTEDLINE
#ifdef ADD_DIRLIST_SORT2
   if( gfDirLst && giDirLen ) { // with NO LINE NUMBERS
      // if( gbDirList ) { // we have -DL - wants file list only
      if( gbDirList || gbOrdList || VERB4 ) {
         // we have -DL or -DO or VERB4 - wants sorted list
         // OutSortedLines( pWS );
         OutSortedLines();
      }
   }
#endif // #ifdef ADD_DIRLIST_SORT2
	time(&et);
// #endif // #ifdef   USEFINDLIST y/n   // changed to MULTIPLE FILE FIND list = FIX20010703

}

void	AddCmds( WS )
{
	if( ( !gbDoneCmds ) &&
		( gszCmdBuf[0] ) )
	{
		gbDoneCmds = TRUE;
		strcat( glpError, "Commands: " );
		strcat( glpError, &gszCmdBuf[0] );
		strcat( glpError, MEOR );
	}
}

#ifdef	ADDINHIB
// ============================================
// void	AddISw( WS )
//
// ============================================
void	AddISw( WS )
{
	char *	lpmf;		// case 'I':
	int		i, j;
	if(( fInhibit   ) &&		// case 'I':
		( giInhibCnt ) &&
		( glpInhib   ) )
	{
		lpmf = glpInhib;
		j = 0;
		while( ( i = strlen(lpmf) ) > 0 )
		{
			j++;
			lpmf += ( i + 1 );
		}
		lpmf = glpInhib;
		sprintf( EndBuf(lpVerb), " Inhib=ON %d", j );
		i = strlen(lpVerb);
      j = strlen(lpmf);
		if(( i            ) &&
			( j            ) &&
			( (i+j+2) < 80 ) )
		{
			strcat( lpVerb, lpmf );
		}
	} else {
		strcat( lpVerb, " Inhib=OFF" );
	}
}	// end - void	AddISw( WS )

#endif	// ADDINHIB

// #ifdef   USEFINDLIST    // = FIX20010703
void  OutFinds( WS )
{
   // show the FIND string(s)
   static int bDnFinds = FALSE;
   //char *   lpf = &gszFindStgs[0];  // = W.ws_szFindStgs
   char *   lpf;
   uint32_t    dwi, dwj, dwk;
   PLE      ph = &g_sFind;
   PLE      pn;
   PFLINE   pl;

   if( !bDnFinds )
   {
      bDnFinds = TRUE;
      // FIX20001022    // fix find of "'@'"!!!
      // FIX20010703 - change to double linked list
      ListCount2(ph, &dwk);
      sprintf( lpVerb, "Finding: %d ", dwk );
      dwj = strlen(lpVerb);
      Traverse_List( ph, pn )
      {
         //lpf = (char *)pn;
         //lpf += sizeof(LIST_ENTRY);
         pl = (PFLINE)pn;
         lpf = &pl->cLine[0];

         dwi = strlen(lpf); 
         if(dwi)
         {
#ifdef   ADD_REGEX
            sprintf( EndBuf(lpVerb),
               "%s[%s] ",
               ( pl->isregex ? "regex" : "" ),
               lpf );
#else // !#ifdef ADD_REGEX
            sprintf( EndBuf(lpVerb),
               "[%s] ",
               lpf );
#endif   // ADD_REGEX y/n

         }
         else
         {
            strcat( lpVerb, "[<Null>]! " );
         }
         dwj = strlen(lpVerb);
         if( dwj > 65 )
         {
            prt( lpVerb );
            dwj = 0;
            *lpVerb = 0;
         }
      }
      if(dwj)
        prt( lpVerb );
   }
}
// #endif   // #ifdef   USEFINDLIST y/n   // = FIX20010703

// FIX20070906 - added -b-, and re-compiled in Pro-1
void prt_out_tail( void )
{
   char buf[256];
   char * bp = buf;
#ifdef _MSC_VER
   char * env = getenv("USERNAME");
   sprintf(bp, sz_Tail, _MSC_VER);

   sprintf(EndBuf(bp),", run by %s",
      (env ? env : "<unknown>") );

   env = getenv("COMPUTERNAME");
   if(env) {
      sprintf(EndBuf(bp), ", on %s machine.", env);
   }
#else
   char * env = getenv("USER");
   if (!env)
	env = getenv("LOGNAME");
   sprintf(bp,"Compiled by %s", (env ? env : "<unknown>"));
   env = getenv("SESSION");
   if (!env)
	env = getenv("DESKTOP_SESSION");
   if (env)
	sprintf(EndBuf(bp),", in %s", env );
#endif
   strcat(bp,PRTTERM);
   prt(bp);
}

void	ShowSwitches( WS )
{
//	int		i;
	OutHeader( pWS );
	OutCmds( pWS );

   OutFinds( pWS );  // FIX20001022    // fix find of "'@'"!!!

	// start a line
	sprintf( lpVerb, "Switches: Verb=%u", gfVerbose );
	if( g_fCase )
		strcat( lpVerb, " Case=ON" );
	else
		strcat( lpVerb, " Case=OFF" );
	if( gfWhole )
		strcat( lpVerb, " Whole=ON" );
	else
		strcat( lpVerb, " Whole=OFF" );
	if( gfNumber )
		strcat( lpVerb, " Number=ON" );
	else
		strcat( lpVerb, " Number=OFF" );
	if( gfParity )
		strcat( lpVerb, " Parity=ON" );
	else
		strcat( lpVerb, " Parity=OFF" );
   if( g_fBinary )
		strcat( lpVerb, " Binary=ON" );
   else
		strcat( lpVerb, " Binary=OFF" );

#ifdef	ADDINHIB
	AddISw( pWS );
#endif	// ADDINHIB

	prt( lpVerb ); // slam it out ...
}

void Set_Max_Find( uint32_t max )
{
   uint32_t dwi;
   g_dwFMax = 0;
   for( dwi = 0; dwi < max; dwi++ ) { //  g_dwFndFlg = 0; // up to 32 bits
      g_dwFMax |= (1 << dwi);
   }
}

// this is the 3rd main( ... ) CONSOLE OS entry.
int Fa4_main( int argc, char **argp )
{
	LPWORKSTR	pWS;

	pWS = DoSetup();	// Allocate the MEMORY

	giFirst = 0;

	if( argc <= 1 )
		Usage( pWS, ERR_N0001 );

	ProcessArgs( pWS, argc, argp, 0 );

#ifdef   ADD_REGEX
   // fix pcre OPTIONS per command line
   if( g_fCase )   // if CASE is IMPORTANT
      gi_pcre_opions &= ~(PCRE_CASELESS); // remove caseless flag
#endif   // ADD_REGEX

#ifdef	ADDERRLST

	if(	gbFindErr )
	{
#ifdef WIN32
		// we have an error
		OutWin32Err( glFindErr );
		// ****************
#endif // WIN32
	}

#endif	// ADDERRLST

// #ifdef   USEFINDLIST    // = FIX20010703
#ifdef   ADDCVSDATE
//extern   int  Add2Finds( WS, char * lps, int bFSwitch );
//extern   void	Add2Files( WS, char * lps );
   if( g_bCVSDate )
   {
      if( g_dwFCnt == 0 )
      {
			giFirst++;
         Add2Finds( pWS, "/", FALSE ); // non-switch addition
      }
      if( g_nFileCnt == 0 )
      {
			giFirst++;
         Add2Files( pWS, "Entries" );  // PLE ph = &g_sFileList, count in g_nFileCnt

			gfRecursive = TRUE;
      }
   }
#endif   // ADDCVSDATE

	if( g_dwFCnt == 0 )
	{
      // NO FIND STRING(S) FOUNT IN COMMAND
		//OutCmds( pWS );
		if( gbFindErr )
			strcat( glpError, "Shown WIN32 error value!"PRTTERM );
		else
		{
			AddCmds( pWS );
			strcat( glpError, "ERROR: Unable to locate FIND string in command!"PRTTERM );
		}
	}

   Set_Max_Find(g_dwFCnt);

   if( g_nFileCnt == 0 )
   {
      // FIX20071010
		//AddCmds( pWS );
		//strcat( glpError, "ERROR: Unable to locate file, or file mask, in command!"PRTTERM );
		sprtf( "WARNING: Unable to locate file, or file mask, in command! Using *.* ..."PRTTERM );
		giFirst++;
      Add2Files( pWS, "*.*" );  // PLE ph = &g_sFileList, count in g_nFileCnt

   }
// #endif   // #ifdef   USEFINDLIST y/n    // = FIX20010703


	if( *glpOutFile )
	{
		ghOutFile = CreateUserFile( glpOutFile );
		if( VH(ghOutFile) )
			SetUserHandle( ghOutFile );
		else
			strcat( glpError, "ERROR: Unable to create OUT file!"PRTTERM );

	}

   if( *glpError == 0 )
   {
      if( VERBM )
      {
         ShowSwitches( pWS );
      }
      else if( VERB5 )
      {
         OutCmds( pWS );
      }
	}

	if( *glpError )
	{
		AddCmds( pWS );
		Err_Exit( -1 );
	}

	Process_Finds( pWS );

#ifdef	ADDRECUR
//	if( gfRecursive ) {
      if( VERB && !g_bNoStats) {
         Show_Found_Nums();
      }
//	}
#endif	// ADDRECUR

	return iCritErr;	// Any errors

}

// this is the 2nd main( ... ) CONSOLE OS entry.
//int JMP_main( int argc, char **argp )
//{
//	int		iret;
//	int		jmpret;
//
//	jmpret = 0;
//	jmpret = setjmp( mark );
//	if( jmpret == 0 )
//	{
//		Fa4_main( argc, argp );
//		Pgm_Exit( lpWS );	// Do the CLEANUP
//	}
//	if( iCritErr )
//		iret = iCritErr;
//	else
//		iret = iMainRet;
//
//	return iret;	// the ONLY return to the OS
//
//}

extern  void    pfile( void );
extern  void    pfile2( void );

// ONLY FOR CONSOLE APPLICATION
//int main( int argc, char **argp )
//{
//	int	iret = 0;
//    pfile2();
//    outvals();
//	DisableDiagFile();
//	iret = JMP_main( argc, argp );
//	return iret;
//}

void	AddHeader( WS, char * lps )
{
	if( lps )
	{
		if( pWS )
		{
			if( !gfHeader )
			{
				gfHeader = TRUE;
				strcpy( lps, &szHdr[0] );
			}
		}
		else
		{
			strcpy( lps, &szHdr[0] );
		}
	}
}

void	OutHeader( WS )
{
	if( pWS )
	{
		if( !gfHeader )
		{
			gfHeader = TRUE;
			prt( &szHdr[0] );
		}
	}
	else
	{
		prt( &szHdr[0] );
	}
}

void	Cleanup1( WS )
{
#ifdef		ADD2ZLST
	KillZList( gpsFinds );
	KillZList( gpsFiles );
	KillZList( gpsExcludes );
#endif		// ADD2ZLST

   // FIX20010319 -v4 adds date order of finds at end - VERB4
   KillLList( &gsFileList );
#ifdef USE_EXCLUDE_LIST
	// EXCLUDE File string(s) LIST
   KillLList( &g_sExclList ); // list of EXCLUDED files
   KillLList( &g_sExclListD ); // list of EXCLUDED directories
#endif // #ifdef USE_EXCLUDE_LIST

#ifdef ADD_REGEX
   {
      PLE ph = &g_sFind;
      PLE pn;
      Traverse_List(ph,pn) {
         PFLINE pl = (PFLINE)pn;
         if( pl->pre )
            pcre_free(pl->pre);
      }
      if(gp_pcre_tables) // = pcre_maketables();
         pcre_free((void *)gp_pcre_tables);
   }
#endif   // ADD_REGEX
   KillLList( &g_sFind );
   KillLList( &g_sLines );
   KillLList( &g_sFileList );    // List of FILES to process
   KillLList( &g_sFailList );    // List of FAILED FILES

   KillLList( &g_sCVSLines );    // list of Entries lines
   KillLList( &g_sEntries );    // list of LATEST Entries lines

   KillLList( &g_sFFList  );     // FOUND in FILE list

   // if ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   KillLList( &g_sOutList );     // g_dwMinCnt (default = 1)

   //WriteINI();  // if we had one

}

void	PrtErr( WS, uint32_t Err )
{
	LPERRLIST	lpE;
	char *		lperr;

	lpE = &ErrList[0];

	while( ( lperr = lpE->er_ptr ) != 0 )
	{
		if( lpE->er_val == Err )
			break;
		lpE++;
	}
	if( lperr )
	{
		OutCmds( pWS );
		prt( lperr );
	}
}

// An ERROR EXIT
void	Error_Exit( WS, int val )
{
	int		i = 0;
   PrtErr( pWS, val );
   if( pWS )
      i = strlen( glpError );
   if(i)
	{
		prt( glpError );
		*glpError = 0;
	}
	if( ( val ) &&
		( iCritErr == 0 ) )
	{
		iCritErr = GEN_ERROR;
	}
	if( VH(ghOutFile) )
	{
		CloseUserFile( ghOutFile );
		SetUserHandle( 0 );
	}
	SetUserHandle( 0 );
	ghOutFile = 0;

	// but still go to Pgm_Exit ...
	Pgm_Exit( pWS );
}

void	UnmapFile1( WS )
{
	ShutFileMap( pWS );
}

void	UnmapFile2( WS )
{
#ifdef _MSC_VER
	if( lpMap2 )
		UnmapViewOfFile( lpMap2 );	// address where mapped view begins
	lpMap2 = 0;
	if( hMap2 )
		CloseHandle( hMap2 );
	hMap2 = 0;
	if( VH(hTmpFil2) )
		CloseAFile( hTmpFil2 );
	hTmpFil2 = 0;
#else
	// TODO: functions for mmpa close
#endif
}
void	SetIretVal( WS )
{
	iRetVal =
			( iCritErr ? iCritErr :
			( gdwFinds - gdwFindsOut ) );
}

void	PutActiveStg( WS, uint32_t dwMs )
{
    uint32_t	dwDif, dwSecs, dwMins;
	char *	lpV = 0;

	if( pWS )
      lpV = lpVerb;
    if(lpV) {
		if( (dwDif = dwMs) > 1000 ) {
			dwSecs = dwDif / 1000;
			dwDif  = dwDif % 1000;
			if( dwSecs > 60 )
			{
				dwMins = dwSecs / 60;
				dwSecs = dwSecs % 60;
				sprintf( lpV, "Active for %u:%02u.%03u mins ... ",
					dwMins,
					dwSecs,
					dwDif );
			}
			else
			{
				dwMins = 0;
				sprintf( lpV, "Active for %u.%03u secs ... ",
					dwSecs,
					dwDif );
			}
		}
		else
		{
			sprintf( lpV, "Active for %u ms ... ",
				dwDif );
		}

		if( gdwTotBytes ) {
			double	dms, dsecs, dbyts, dres;
			char *	lpt;

			//PutThous( EndBuf(lpV),
			//	7,
			//	gdwTotBytes );
			//strcat( lpV, " bytes at " );
			DW2TStg( EndBuf(lpV), gdwTotBytes );
			strcat( lpV, " at " );

			dms = (double)dwDif;
			dsecs = dms / (double)1000;
			dbyts = (double)(gdwTotBytes * 8);
            if( dsecs > 0.0 ) {
			    dres = dbyts / dsecs;
			    if( dres > (double)1000000 ) {
				    //dres = dres / (double)1000000;
				    dres = floor( ( dres / (double)10000 ) ) / (double)100;
				    lpt = " Mbps. ";
			    } else if( dres > (double)1000 ) {
				    dres = floor( ( dres / (double)10 ) ) / (double)100;
				    lpt = " Kbps. ";
			    } else {
				    lpt = " bps. ";
			    }
			    Double2TStg( EndBuf(lpV), dres );
			    strcat( lpV, lpt );
            } else {
                strcat( lpV, " very fast! " );
            }
		}

//#ifdef	FIX20000606
		SetIretVal( pWS );
		sprintf( EndBuf(lpV),
			"ErrorLevel=%d",
			iRetVal );

//#else
//		sprintf( EndBuf(lpV),
//			"ErrorLevel=%d",
//			( iCritErr ? iCritErr : gdwFinds ) );
//#endif
        sprintf(EndBuf(lpV),", log '%s'", GetDiagFile());

		strcat( lpV, ""PRTTERM );

		// show activity string
		if( VERB3 )
		{
			prt( lpV );
		}
		else
		{
#ifdef	ADDDIAGT
			dout( lpV );
#endif	// ADDDIAGT
		}

	}

}


// "%4d/%02d/%02d  %02d:%02d",
static const char *dummy_datetime = "                ";
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : OutFindList
// Return type: void 
// Argument   : WS
// Description: If there is more than ONE find in the gsFileList,
//              then RANK, and output a simple list.
// FIX20010319 -v4 adds date order of finds at end - if( VERB4 )
///////////////////////////////////////////////////////////////////////////////
void  OutFindList( WS ) // *** DONE AT EXIT ***
{
   // put out a SORTED list of file
   PLE         pHead, pNext;
   uint64_t    uli, ul1;
   uint32_t       dwr, dwc, dwcnt, dwocnt, dwncnt;
   PMWL        pmwl, pmwl1;
   char *      lps = lpVerb;
   //SYSTEMTIME  st;

   pHead = &gsFileList;
   dwr = 0;
   dwc = 0;
   Traverse_List( pHead, pNext ) {
      dwc++;
   }

   dwocnt = dwcnt = dwc;
   dwncnt = 0;
   //if(dwc == 1)  // if just ONE find, then
   //{
      //return;     // ALL DONE 
   //}
   if(VERB9) {
      sprintf(lps, "v9: Ranking list of %d files."MEOR, dwcnt);
      prt(lps);
   }
   // ===========================================================
   while(dwc) {   // while a change count
      dwc = 0;    // CLEAR change count
      uli = 0; // restart TIME
      pmwl1 = 0;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwRank == 0 )
         {
            ul1 = pmwl->wl_DateTime64;
            // if( ul1.QuadPart > uli.QuadPart ) FIX20060126 make >=
            if( ul1 >= uli ) {
               uli = ul1;
               pmwl1 = pmwl;
               dwc++;   // mark a change
            }
         }
      }
      if(dwc && pmwl1 ) {
         dwr++;   // bump RANK
         pmwl1->wl_dwRank = dwr; // set RANK
         if(VERB9) {
            sprintf(lps, "v9: Rank %d set on %s"MEOR, dwr, &pmwl1->wl_cName[0] );
            prt(lps);
         }
      }
   }

   // all SORTED, now OUT THE LIST
   // FIX20070306 - reverse SORT order - put LATEST last
   // dwr = 1;
   if( dwcnt && lps ) {
      sprintf( lps, "List of %d finds in date order, ascending -"PRTTERM, dwcnt );
      prt( lps );
      while( dwcnt ) {
         dwc = 0;
         Traverse_List( pHead, pNext ) {
            pmwl = (PMWL)pNext;
            if( pmwl->wl_dwRank == dwr ) {
               dwncnt++;
               strcpy( lps, &pmwl->wl_cName[0] );
               strcat( lps, " " );
               while( strlen(lps) < MINFNSP )
                    strcat( lps, " " );
               if (pmwl->wl_DateTime64) {
                   struct tm *pt = gmtime((time_t *)&pmwl->wl_DateTime64);
                   if (pt) {
                        sprintf(EndBuf(lps),
                            "%4d/%02d/%02d  %02d:%02d",
                            pt->tm_year + 1900,
                            pt->tm_mon,
                            pt->tm_mday,
                            pt->tm_hour,
                            pt->tm_sec );
                   } else {
                       strcat(lps,dummy_datetime);
                   }
               } else {
                   strcat(lps,dummy_datetime);
               }

               if( pmwl->wl_dwFound ) {
                  sprintf(EndBuf(lps)," f=%d", pmwl->wl_dwFound );
               }
               strcat( lps, PRTTERM );
               prt(lps);
               dwc++;
               break;
            }
         }  // for the LIST

         if( dwc == 0 ) {
            sprintf( lps, "Oops, appear to have MISSED rank %d"PRTTERM, dwr );
            prt(lps);
         }

         dwcnt--; // reduce done count
         // FIX20070306 - reverse SORT order - put LATEST last
         //dwr++;   // bump RANK to next
         dwr--;   // reduce RANK to next
      }
   }

   if( dwocnt > 2 ) { // was 5 )
      sprintf( lps, "Done list of %d in date order, earliest first ..."PRTTERM, dwncnt );
      prt(lps);
   }
}

void  OutFailList( void )
{
   PLE ph = &g_sFailList;
   PLE pn;
   PMFILE pmf;
   Traverse_List(ph, pn)
   {
      pmf = (PMFILE)pn;
      prt( pmf->cFile );
   }
}

// ALL should come here to exit
void	Pgm_Exit( WS )
{
	// Cleanup!!!
	if( pWS )
	{
      // FIX20010319 -v4 adds date order of finds at end
      if( VERB ) {
         OutFailList();
      }
      if( VERB4 ) {
         OutFindList( pWS );
      }
      if( g_iNoOpen && lpVerb )
      {
         sprintf(lpVerb,
            "Warning: Note, some %d input file name(s) could not be VERIFIED!"MEOR,
            g_iNoOpen );
         prt(lpVerb);
      }
		Cleanup1( pWS );
		UnmapFile1( pWS );
		UnmapFile2( pWS );
//#define		fInhibit	W.ws_fInhibit
#ifdef	ADDINHIB
//				case 'I':
//#define		giInhibCnt	W.ws_iInhibCnt
//#define		giInhibSiz	W.ws_iInhibSiz
//#define		glpInhib	W.ws_lpInhib
		if( glpInhib )
			MFREE( glpInhib );
		glpInhib = 0;
#endif	// ADDINHIB

	}

	gdwEnd= GetTickCount();
	if( pWS )
	{
		PutActiveStg( pWS, (gdwEnd - gdwBgn) );
	}

	if( iCritErr == 0 )
	{
		// Transfer FIND results to iMainRet;
//		iMainRet = (int)gdwFinds;
		iMainRet = (int)( gdwFinds - gdwFindsOut );
	}

#ifdef	ADDDIAGT
	CloseDiagFile();
#endif	// ADDDIAGT

	// FREE THE MEMORY
	// ===============
    if( lpWS ) {
#ifdef USE_ALLOCATED_LINE_BUFFER
        if ( glpLineBuf )
            MFREE( glpLineBuf );
        glpLineBuf = NULL;
#endif // #ifdef USE_ALLOCATED_LINE_BUFFER
		MFREE( lpWS );
    }
	lpWS = 0;

	// NOTICE THE EXIT!!!
	// ==================
	longjmp( mark, iCritErr );

}	// end - void	Pgm_Exit( WS )


int	IsString( char * lpv, int ins, char * lpc, int len )
{
	int	flg = FALSE;
	int		i, j;
	char *	lps;

   j = len;
	if( ( ( lps = lpv ) != 0 ) &&
		( ( i = ins ) > 0 ) &&
		( i > j ) )
	{
		flg = TRUE;
		if( *lps <= ' ' )
		{
			flg = FALSE;
			while( ( *lps <= ' ' ) && ( *lps != 0 ) )
				lps++;

			i = strlen(lps); 
			if( ( i ) &&
				( i > j ) )
				flg = TRUE;
		}
		if( flg )
		{
			flg = FALSE;
			for( i = 0; i < j; i++ )
			{
				if( lps[i] != lpc[i] )
				{
					if( ( (lps[i] & 0xff) == 0x82 ) &&
						( (lpc[i] & 0xff) == 0xe9 ) )
					{
						// French accented e
					}
					else
					{
						break;
					}
				}
			}
			if( i == j )
				flg = TRUE;
		}
	}
	return flg;
}


int	IsEVLabel( char * lpv )
{
	int	flg = FALSE;
	int		j = (sizeof(szEVLabel)-1);
	char *	lpc = &szEVLabel[0];
	int		i = 0;
	char *	lps;

	lps = lpv;
   if(lps)
      i = strlen(lps);
	if( ( i ) &&
		( i > j ) )
	{
		flg = TRUE;
		if( *lps <= ' ' )
		{
			flg = FALSE;
			while( ( *lps <= ' ' ) && ( *lps != 0 ) )
				lps++;

			i = strlen(lps);
			if( ( i ) &&
				( i > j ) )
				flg = TRUE;
		}
		if( flg )
		{
			flg = FALSE;
			for( i = 0; i < j; i++ )
			{
				if( lps[i] != lpc[i] )
					break;
			}
			if( i == j )
				flg = TRUE;
		}
	}

	return flg;

}

int	IsFVLabel( char * lpv )
{
	int	flg = FALSE;
	int		j = (sizeof(szFVLabel)-1);
	char *	lpc = &szFVLabel[0];
	int		i = 0;
	char *	lps;

   lps = lpv;
   if(lps)
      i = strlen(lps);

	if( ( i ) &&
		( i > j ) )
	{
		flg = TRUE;
		if( *lps <= ' ' )
		{
			flg = FALSE;
			while( ( *lps <= ' ' ) && ( *lps != 0 ) )
				lps++;

			i = strlen(lps);
			if( ( i ) &&
				( i > j ) )
				flg = TRUE;
		}
		if( flg )
		{
			flg = FALSE;
			for( i = 0; i < j; i++ )
			{
				if( lps[i] != lpc[i] )
					break;
			}
			if( i == j )
				flg = TRUE;
		}
	}

	return flg;

}

int	IsEDirect( char * lpv )
{
	int	flg = FALSE;
	int		j = (sizeof(szEDirect)-1);
	char *	lpc = &szEDirect[0];
	int		i = 0;
	char *	lps;

	lps = lpv;
   if(lps)
      i = strlen(lps);
	if( ( i ) &&
		( i > j ) )
	{
		flg = TRUE;
		if( *lps <= ' ' )
		{
			flg = FALSE;
			while( ( *lps <= ' ' ) && ( *lps != 0 ) )
				lps++;

			i = strlen(lps);
			if( ( i ) &&
				( i > j ) )
				flg = TRUE;
		}
		if( flg )
		{
			flg = FALSE;
			for( i = 0; i < j; i++ )
			{
				if( lps[i] != lpc[i] )
					break;
			}
			if( i == j )
				flg = TRUE;
		}
	}
	return flg;
}

int	IsFDirect( char * lpv )
{
	int	flg = FALSE;
	int		j = (sizeof(szFDirect)-1);
	char *	lpc = &szFDirect[0];
	int		i = 0;
	char *	lps = lpv;

   if(lps)
      i = strlen(lps);
   if( ( i ) &&
		( i > j ) )
	{
		flg = TRUE;
		if( *lps <= ' ' )
		{
			flg = FALSE;
			while( ( *lps <= ' ' ) && ( *lps != 0 ) )
				lps++;

			i = strlen(lps);
			if( ( i ) &&
				( i > j ) )
				flg = TRUE;
		}
		if( flg )
		{
			flg = FALSE;
			for( i = 0; i < j; i++ )
			{
				if( lps[i] != lpc[i] )
					break;
			}
			if( i == j )
				flg = TRUE;
		}
	}
	return flg;
}


int	IsEDirect2( char * lpv, uint32_t ins )
{
	int		flg = 0;
	int		j = (sizeof(szEDirect)-1);

	if( ( lpv ) &&
		( ins > (uint32_t)j ) )
	{
		char *	lpc = &szEDirect[0]; // = "Directory of "; // C:\GEOFF
		if( IsString( lpv, ins, lpc, j ) )
			flg = j;
	}
	return flg;
}

int	IsFDirect2( char * lpv, uint32_t ins )
{
	int		flg = 0;
	int		j = (sizeof(szFDirect)-1);

	if( ( lpv ) &&
		( ins > (uint32_t)j ) )
	{
		char *	lpc = &szFDirect[0];
		if( IsString( lpv, ins, lpc, j ) )
			flg = j;
	}
	return flg;
}


int	mstrncmp( char * s1, char * s2, int i )
{
	int	k = 1;
	if( ( i ) &&
		( s1 ) &&
		( s2 ) )
	{
		while( i )
		{
			if( *s1 != *s2 )
				break;
			s1++;
			s2++;
			i--;
		}
		if( i == 0 )
			k = 0;
	}
	return k;
}

int	mstrncmpi( char * s1, char * s2, int i )
{
	int	k = 1;

	if( ( i ) &&
		( s1 ) &&
		( s2 ) )
	{
		while( i )
		{
			if( toupper(*s1) != toupper(*s2) )
				break;
			s1++;
			s2++;
			i--;
		}
		if( i == 0 )
			k = 0;
	}
	return k;
}

int	mstrncmp2( WS, char * s1, char * s2, int i )
{
	int	k = 1;

	if( ( i ) &&
		( s1 ) &&
		( s2 ) )
	{
		while( i )
		{
			if( *s1 != *s2 )
			{
				if( gfSpacey )
				{
					if( ( *s1 <= ' ' ) &&
						( *s2 <= ' '  ) )
					{
						goto EquSpace;
					}
					while( ( *s1 ) &&
						( *s1 <= ' ' ) )
					{
						s1++;
					}
					while( ( *s2 ) &&
						( *s2 <= ' ' ) )
					{
						s2++;
					}
					if( *s1 != *s2 )
					{
						break;
					}
				}
				else
				{
					break;
				}
			}

EquSpace:
			s1++;
			s2++;
			i--;
		}
		if( i == 0 )
			k = 0;
	}
	return k;
}

int	mstrncmpi2( WS, char * s1, char * s2, int i )
{
	int	k = 1;
	if( ( i ) &&
		( s1 ) &&
		( s2 ) )
	{
		while( i )
		{
			if( toupper(*s1) != toupper(*s2) )
			{
				if( gfSpacey )
				{
					if( ( *s1 <= ' ' ) &&
						( *s2 <= ' '  ) )
					{
						goto EquSpacei;
					}
					while( ( *s1 ) &&
						( *s1 <= ' ' ) )
					{
						s1++;
					}
					while( ( *s2 ) &&
						( *s2 <= ' ' ) )
					{
						s2++;
					}
					if( toupper(*s1) != toupper(*s2) )
					{
						break;
					}
				}
				else
				{
					break;
				}
			}

EquSpacei:

			// continue scanning and comparing in this line
			s1++;
			s2++;
			i--;
			// === loop, until i runs out, or FOUND NOT EQUAL

		}

		if( i == 0 )
			k = 0;
	}
	return k;
}


int	HasStg( WS, char * pn, char * ps )
{
	int i = 0;
	size_t	j, k, l, m;

	j = k = l = 0;
	if( ( pn ) &&
		( ps ) )
   {
      j = strlen(pn);
      k = strlen(ps);
   }
   if( ( j && k ) &&
		( k <= j ) )
	{

		while( strlen(pn) >= k )
		{
			if( gbCaseInhib )
			{
//				m = mstrncmp( pn, ps, k );
				m = mstrncmp2( pWS, pn, ps, k );
			}
			else
			{
//				m = mstrncmpi( pn, ps, k );
				m = mstrncmpi2( pWS, pn, ps, k );
			}
			if( m == 0 )
			{
				i = l + 1;
				break;
			}
			pn++;
			l++;
		}
	}

	return i;
}


// eof - Fa4.c
