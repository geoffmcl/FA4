/*\
 * Fa4Work.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * an allocate 'work' structure to hold variables
 * and macros to address those variables...
 *
\*/
#ifndef	_Fa4Work_H
#define	_Fa4Work_H
#include <stdint.h>

#define		MXINPFS		10
#define     MXOVR       32
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define  MXLINES     32
#define  MXLINEB     256
#define  MXLINEB2    (MXLINEB + 16)

// FIX20120407 - Bump this default for new wider screens
#define DEF_LLIMIT  96  // 80  // = 40 left - 40 right

/* character encodings
*/
#define RAW         0
#define ASCII       1
#define LATIN0      2
#define LATIN1      3
#define UTF8        4
#define ISO2022     5
#define MACROMAN    6
#define WIN1252     7
#define IBM858      8
// #if SUPPORT_UTF16_ENCODINGS
#define UTF16LE     9
#define UTF16BE     10
#define UTF16       11
// #endif
#define  CHENC_UNK   -1

#define  MAX_BIN_SEARCH    256
#define UNICODE_BOM_BE   0xFEFF   /* big-endian (default) UNICODE BOM */
#define UNICODE_BOM      UNICODE_BOM_BE
#define UNICODE_BOM_LE   0xFFFE   /* little-endian UNICODE BOM */
#define UNICODE_BOM_UTF8 0xEFBBBF /* UTF-8 UNICODE BOM */

#define MY_MAX_FILESIZE 2000000000
typedef struct tagGFS {
	char	        fs_szNm[(MAX_PATH+MXOVR)];
	HANDLE	        fs_hHnd;
	uint64_t	    fs_uint64;
	void *  	    fs_lpV;
	uint32_t	    fs_dwRd;
}GFS, * LPGFS;

//	PVERS		"V4.0.16"	// FIX20010703 - multiple FIND strings MUST be in ORDER
// With this change switch from a set of string in gszFindStgs to a simple DOUBLE LINKED
// LIST of FIND strings, one or as many as necessary, limited only by MEMORY
typedef struct tagFLINE {
   LIST_ENTRY  sList; // double list structure - MUST BE FIRST
   uint32_t       dwNum;   // (logical) Line Number - ie 0 based
   uint32_t       uint32_t;   // File Number - 1 based
   uint32_t       dwTyp;   // TYPE (0=none, 1=same file, 2=same line)
   uint32_t       dwTN;    // Type Number to match with
   void *         pFind;   // to avoid the same FIND
#ifdef   ADD_REGEX
   int         isregex;
   pcre *      pre;
#endif   // ADD_REGEX
   char       cLine[1];   // actual line string
}FLINE, * PFLINE;

// NOT COMPLETED - 4 JULY, 2001
typedef struct tagMFILE {
   LIST_ENTRY  sList; // double list structure - MUST BE FIRST
   uint32_t       dwNum;   // (logical) Line Number - ie 0 based
   char       cFile[1];   // actual line string
}MFILE, * PMFILE;

// LoadFile2 and KillFile2 structure
// =================================
typedef	struct tagLFSTR {
	char	    lf_szNm[MAX_PATH+MXOVR];
	HANDLE	    lf_hHnd;
	uint32_t	lf_dwSz;
	uint32_t	lf_dwRd;
	char *      lf_lpBuf;
}LFSTR, * LPLFSTR;

// ADDCVSDATE - Get the latest DATE
// ctime( &ltime ) = Fri Apr 29 12:25:12 1994
typedef struct tagCVSLN {
   LIST_ENTRY  sList;   // first member of a LINKED LIST
   PLE         psName;  // pointer to the FILE name
   uint32_t       dwVers;        // packed version
   int        bDTValid;      // date/time string is valid
   //SYSTEMTIME  sSysTm;        // ascii back to system time
   //FILETIME    sFT;           // file time
   time_t       uFT;
   char         szFile[264];   // file name
}CVSLN, * PCVSLN;


typedef void (*FINDINTYPE) ( void *, char *);

// ALLOCATED work structure
typedef	struct tagWORKSTR {

	uint32_t	ws_Size;	// Just SIZE of this structure

	uint32_t	ws_dwFinds;	// Main purpose
	uint32_t	ws_dwTotBytes;	// bytes processed
	uint32_t	ws_dwTotLines;
	uint32_t ws_dwFindsOut;	// found text in line, and not shown via -I:"TextOut" flag

   int     ws_bRedirON;   // g_bRedirON - standard output is being redirected

	char *	ws_lpInFil1;
	char *	ws_lpInDir1;

	// Find string(s)
	// FILE HANDLE, if @InputFile,
	// ITEM Count, MEMORY Size, Handle and (locked) Pointer
	HANDLE	ws_hFindFile;	// File HANDLE (if @FileName)
	uint32_t	   ws_dwFindCnt;	// Count of finds items
	uint32_t	   ws_dwFindMem;	// Total of "stg+0"+"stg+0"+...

	HANDLE	ws_hFileFile;
	uint32_t	   ws_dwFileMem;

	HANDLE	ws_hActFile;
#ifdef WIN32
    //BY_HANDLE_FILE_INFORMATION ws_sFileInf;   // #define  gsFileInf
    //WIN32_FIND_DATA   ws_sFindData;  // gsFindData
#endif // WIN32

	int		ws_iTmpCnt1;
	HANDLE	ws_hTmpFil2;
	int		ws_iTmpCnt2;
	HANDLE	ws_hActMap;
	HANDLE	ws_hMap2;
	void  * ws_lpMapView;
	void  * ws_lpMap2;
	int	    ws_BeTidy;		// Delete the files at end
	int	    ws_fHeader;

   // if ADDCVSDATE
   int         ws_bCVSDate;   // g_bCVSDate = search for latest DATE in CVS entries file
   LIST_ENTRY  ws_sCVSLines;  // g_sCVSLines = lines in entries
   LIST_ENTRY  ws_sEntries;  // g_sEntries = latest lines in entries
   CVSLN       ws_sCvsLn;     // g_sCvsLn
   LIST_ENTRY  ws_sFFList;    // g_sFFList = Found in FILE list
   PLE         ws_pFFList;    // g_pFFList = Latest FOUND FILE pointer

	char *	ws_pDir1;
	char *	ws_pDir2;

	int		ws_fVerbose;
	int	   ws_fReverse;
	int	   ws_fFirst;
	char *	ws_lpVerb;
	char *	ws_glpActive;

	char *	ws_lpForm;
	int	   ws_fDoneFile;

	FINDINTYPE ws_findintype;
	char (*GETCHR) (char *);    // GetChr function - get next CHARACTER from file

   uint32_t    ws_dwCharEncoding;  // g_dwCharEncoding
   int     ws_bIsBinary;      // g_bIsBinary - set ONLY if g_fIgnoreBin set, else
   int     ws_bIsBinary2;      // g_bIsBinary2 is set
   
   int     ws_fIgnoreBin; // g_fIgnoreBin
	int	   ws_fBinary;
	int	   ws_fCase;
	int	   ws_fWhole;
	int	   ws_fWhole2;    // g_fWhole2
	int	   ws_fNumber;
	int	   ws_fParity;
	int	   ws_fSpacey;	// g(W.ws_)fSpacey -I Exclude spaces / tabs in text compare - June 2000

	int	   ws_fRecursive;
    int       ws_fStrip;   // FIX20120407 - Add -S - gfStrip

   int     ws_fBeginLine; // FIX20061106 - "\ntext" to find text at begin of line

   int     ws_bIsMake; // g_bIsMake - obey line continuation character

	int		ws_nFileLen;
	int		ws_nFileCnt;   // g_nFileCnt
	int		ws_nFindLen;
	int		ws_nFindCnt;
	int		ws_nExclLen;
	int		ws_nExclCnt;

	int		ws_fDirLst;
	int		ws_fIsDir;
	int		ws_iDirLen;

	uint32_t	   ws_dwSrchCnt;
	uint32_t	   ws_dwMaskCnt;
	uint32_t	   ws_dwMax1Line;
	uint32_t	   ws_dwEnd1Line;
	int		ws_iFirst;

	uint32_t	   ws_dwTotal;
	uint32_t	   ws_dwLnInFile;

//				case 'I':
	int	   ws_fInhibit;
	int	   ws_bCaseInhib;
   int     ws_bIgComm;   // g_bIgComm -I+ ignore C/C++ comments
   int     ws_bIgPComm;  // g_bIgPComm ignore Perl comments

#ifdef	ADDINHIB
	int		ws_iInhibCnt;
	int		ws_iInhibSiz;
	char *	ws_lpInhib;
#endif	// ADDINHIB

	int	   ws_fInComment;
	uint32_t	   ws_dwActLen;
	uint32_t	   ws_FileType;   // g_FileType
	uint32_t	   ws_dwBackup;	// Starts as DEF_BACKUP
	uint32_t	   ws_dwForward;
	char *	ws_lpCurrFind;	// "raw" current find string
	uint32_t	   ws_dwCurrFLen;	// length of "raw" find sting
	char *	ws_lpCurFind;	// "modified" current find string
	uint32_t	   ws_dwCurFLen;	// g_dwCurFLen = length of "modified" find sting
	uint32_t	   ws_dwMxFilNm;
	uint32_t	   ws_dwBgn;
	uint32_t	   ws_dwEnd;

	uint32_t	   ws_dwGFCnt;
	GFS		ws_sGFS[MXINPFS];

#ifdef		ADD2ZLST
	SSSTR	ws_sFinds;
	SSSTR	ws_sFiles;
	SSSTR	ws_sExcludes;
#endif		// ADD2ZLST

//#ifdef		ADDERRLST
	long	   ws_lFindErr;
	int		ws_bFindErr;
//#endif	// ADDERRLST

	LFSTR	   ws_slfExclude;    // gslfExclude

	HANDLE	ws_hOutFile;

	uint32_t	   ws_dwLineLen;

	int	   ws_bDoneCmds;
	uint32_t	   ws_dwCmdLen;         // gdwCmdLen
   uint32_t    ws_dwCmdLn;          // gdwCmdLn

//#define	giLineCnt		W.ws_iLineCnt
	int		ws_iLineCnt;
//#define	giFileCnt		W.ws_iFileCnt
	int		ws_iFileCnt;

   int     ws_bDoMake; // g_bDoMake - on find output obey makefile continuation
   // character. That is if the line ends with \ goto next line for output
   int     ws_bDoMake2; // g_bDoMake2 - expand $(macro) entries
   int ws_bDoAllin1; // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file
   int ws_bAllInOne; // g_bAllInOne  - FIX20060621 - Add an -F:2, ALL FINDS in 1 LINE
   uint32_t ws_dwMinLen; // g_dwMinLen - Minimum length of FINDS

   // if ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT
   uint32_t    ws_dwMinCnt;      // g_dwMinCnt    // -cnn - minimum find count
   LIST_ENTRY  ws_sOutList;   // g_sOutList

   // FIX20010319 -v4 adds date order of finds at end - VERB4
   LIST_ENTRY  ws_FileList;      // gsFileList

#ifdef   ADDLOPTION
   int     ws_bGotLOpt;      // g_bGotLOpt
   int     ws_bGotLFunc;     // g_bGotLFunc
   uint32_t    ws_dwLOptLen;     // g_dwLOptLen
   int     ws_bGotLLimit;  // g_bGotLLimit = -LL limit line length output
   uint32_t    ws_dwLLLeft;    // g_dwLLLeft   = -LLnn - limit to left and right = nn / 2
   uint32_t    ws_dwLLRight;   // g_dwLLRight  = -LLnn:nn - limit left and limit right
   uint32_t    ws_dwLineLimit;   // g_dwLineLimit  = -LLnn:nn - max. limit
#endif   // ADDLOPTION
//	PVERS		"V4.0.16"	// FIX20010703 - multiple FIND strings MUST be in ORDER
// With this change switch from a set of string in gszFindStgs to a simple DOUBLE LINKED
// LIST of FIND strings, one or as many as necessary, limited only by MEMORY
   LIST_ENTRY  ws_sFind;    // g_sFind = LIST of find strings
   uint32_t       ws_dwFCnt;   // g_dwFCnt = Count of user input of FIND strings
   uint32_t       ws_dwFMax;  // g_dwFMax = Count of BITS-per-FIND
   LIST_ENTRY  ws_sLines;   // g_sLines = lines for output
   uint32_t       ws_dwFileNum;  // g_dwFileNum  // Numerical gfDoneFile order
   PFLINE      ws_psActFind;  // g_psActFind  // Current active FIND structure
   LIST_ENTRY  ws_sFileList;  // g_sFileList  // List of FILES to process
   LIST_ENTRY  ws_sFailList;  // g_sFailList  // List of FAILED FILES

#ifdef USE_EXCLUDE_LIST
   LIST_ENTRY  ws_sExcList; // g_sExclList - list of EXCLUDED files
   LIST_ENTRY  ws_sExcListD; // g_sExclListD - list of EXCLUDED directories
   uint32_t       ws_dwExclDCnt; // g_dwExclDCnt - count of excluded directories
   uint32_t       ws_dwDirsExcl; // g_dwDirsExcl - count excluded matches
#else // !#ifdef USE_EXCLUDE_LIST
	// EXCLUDE File string(s) buffer
	char	   ws_szExclude[MXFILSTGS+MXOVR];
#endif // #ifdef USE_EXCLUDE_LIST y/n
   int      ws_iFPHidden; //  giFPHidden -I exclude FrontPage (HIDDEN) directories
#ifdef   ADD_REGEX
   int      ws_pcre_options;
   const unsigned char * ws_pcretables;
   int      ws_pcre_offsets[PCRE_MAX_OFFSETS];
#endif // #ifdef   ADD_REGEX
   int         ws_iNoOpen; // g_iNoOpen - BUMP failed to OPEN count
   int         ws_ioHold; // g_ioHold - got -f:1 and multiple finds
	char       ws_szCurrFind[MXONEFIND+MXOVR];   // = g_szCurrFind

   int      ws_iDoneList;  // g_iDoneList // Have output the file name
   // for Left, Mid, Right, etc
   int      ws_iLnBuf;      // g_iLnBuf
    uint32_t ws_dwBgnLine; // g_dwBgnLine
    uint32_t ws_dwEndLine; // g_dwEndLine

    uint32_t   ws_dwOffBgn;    // g_dwOffBgn
    uint32_t   ws_dwOffEnd;    // g_dwOffEnd
    uint32_t   ws_dwLBOffBgn;    // g_dwLBOffBgn
    uint32_t   ws_dwLBOffEnd;    // g_dwLBOffEnd
#ifdef WIN32
   // just for the -8 OPTION !!!
   int     ws_bDOSNm;  // gbDOSNm
#endif // WIN32
   char    ws_szDOSNm[MAX_PATH+MXOVR];    // gszDOSNm
   char    ws_szDOSPth[MAX_PATH+MXOVR];   // gszDOSPth
   char    ws_szDOSWork[MAX_PATH+MXOVR];   // gszDOSWork
   char    ws_szFULNm[MAX_PATH+MXOVR];    // gszFULNm
   char    ws_szFULPth[MAX_PATH+MXOVR];   // gszFULPth
   char    ws_szDOSLast[MAX_PATH+MXOVR];  // gszDOSLast

   char    ws_szLnBuf[(MXLINEB2 * MXLINES)]; // g_szLnBuf - small item buffers

	char	   ws_szCmdBuf[MXCMDBUF+MXOVR];

	char	   ws_szActFil[MAX_PATH+MXOVR];

	char	   ws_cVerbBuf[MXVERBBUF+MXOVR];

	char	   ws_DiagBuf[1024+MXOVR];              // DiagBuf
//	char	ws_cWrtBuf[260];

	char	   ws_cDirBuf[264+MXOVR];        // gcDirBuf
    int        ws_bNoStats;                //	g_bNoStats // -NS = no output of stats at end
    int        ws_bNoFile;                //	g_bNoFile //  -NF = no output of file name
    int        ws_fQuiet;          // gfQuiet // // FIX20160702 - Add a -Q - Quiet - only output is the find lines

#ifdef USE_ALLOCATED_LINE_BUFFER
    char * ws_lpLineBuffer;     // glpLineBuf
    uint32_t  ws_dwLBSize;         // gdwLBSize
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
	char	   ws_cLineBuf[MXLINEBUF+MXOVR];
    //#define	gcLineBuf	   W.ws_cLineBuf
    // Size MXLINEBUF
    // #define	glpLineBuf	   &gcLineBuf[0]
#endif  // #ifdef USE_ALLOCATED_LINE_BUFFER y/n

	char	   ws_szDir2[MAX_PATH+MXOVR];
	char	   ws_szFolder[MAX_PATH+MXOVR];   // gszFolder

	// File string(s) buffer
	char	   ws_szFileMask[MXFILSTGS+MXOVR];
	// -R - Recursive find mask
	char	   ws_szOutFile[MAX_PATH+MXOVR];
   char    ws_szDiagFile[264];     // g_szDiagFile - diagnostic output file

	char	   ws_cErrBuf[2048+MXOVR];

	char	   ws_cWorkBuf[MXBIGBUF+MXOVR];
	char	   ws_szTmpBuf[MXBIGBUF+MXOVR];  // passed = glpTmp - global = g_szTmpBuf
	char	   ws_szTmpBuf2[MXBIGBUF+MXOVR];  // passed = glpTmp - global = g_szTmpBuf
	char	   ws_szDirBuf[MXBIGBUF+MXOVR];  // last DIRECTORY output
   
//   char    ws_szSpare[4096];    // just some additonal

}WORKSTR, * LPWORKSTR;

extern   LPWORKSTR	lpWS;

// when passing the Work stucture pointer
#define	WS		LPWORKSTR	pWS
#define	W		(*pWS)

// referencing it directly = an EXE fixup address item
#define  GW    (*lpWS)  // this is the current PREFERRED method

#define	gdwFinds		   W.ws_dwFinds
#define	gdwTotBytes		W.ws_dwTotBytes
#define	gdwActLen		W.ws_dwActLen
#define	g_dwActLen		GW.ws_dwActLen    // replace gdwActLen
#define	gdwTotLines		W.ws_dwTotLines
#define	gdwFindsOut	   W.ws_dwFindsOut	// found text in line, and not shown via -I:"TextOut" flag

#define	gdwLnInFile		W.ws_dwLnInFile

#ifdef		ADD2ZLST
#define	gsFinds		W.ws_sFinds
#define	gsFiles		W.ws_sFiles
#define	gsExcludes	W.ws_sExcludes

#define	gpsFinds	&gsFinds
#define	gpsFiles	&gsFiles
#define	gpsExcludes	&gsExcludes
#endif	// ADD2ZLST

#define	gslfExclude		W.ws_slfExclude
#define	gfHeader		   W.ws_fHeader

#define	gbDoneCmds	   W.ws_bDoneCmds
#define	gdwCmdLn	      W.ws_dwCmdLn
#define	gdwCmdLen	   W.ws_dwCmdLen
#define	gszCmdBuf	   W.ws_szCmdBuf

//	GFS		ws_sGFS[MXINPFS];
#define	gsGFS		      W.ws_sGFS      // for MXINPFS
#define	gdwGFCnt	      W.ws_dwGFCnt

// FIND STRING MEMORY (and MAYBE FILE)
#define	ghFindFile		W.ws_hFindFile
#define	gdwFindCnt		W.ws_dwFindCnt
#define	gdwFindMem		W.ws_dwFindMem

//#define	ghgFindMem		W.ws_hgFindMem
//#define	glpFindMem		W.ws_lpFindMem

#define	ghActFile	   W.ws_hActFile
#define	iTmpCnt1	      W.ws_iTmpCnt1
#define	ghActMap		   W.ws_hActMap
#define	glpMapView		W.ws_lpMapView
#define	g_lpMapView		GW.ws_lpMapView   // replace glMapView
#define	pDir1		      W.ws_pDir1

#define	hTmpFil2	      W.ws_hTmpFil2
#define	iTmpCnt2	      W.ws_iTmpCnt2
#define	hMap2		      W.ws_hMap2
#define	lpMap2		   W.ws_lpMap2
#define	pDir2		      W.ws_pDir2

#define	fFirst		   W.ws_fFirst

#define	gcErrBuf		   W.ws_cErrBuf      // using the PASSED parameter
#define	glpError		   &gcErrBuf[0]
#define  g_cErrBuf      GW.ws_cErrBuf     // global addressing

#define	DiagBuf		   W.ws_DiagBuf
//#define	cWrtBuf		W.ws_cWrtBuf

//#define	gcWorkBuf	   W.ws_cWorkBuf
//#define	glpWorkBuf	   &gcWorkBuf[0]

#ifdef USE_ALLOCATED_LINE_BUFFER
#define	glpLineBuf  GW.ws_lpLineBuffer
#define gdwLBSize   GW.ws_dwLBSize
#else // !#ifdef USE_ALLOCATED_LINE_BUFFER
#define	gcLineBuf	   W.ws_cLineBuf
// Size MXLINEBUF
#define	glpLineBuf	   &gcLineBuf[0]
#endif  // #ifdef USE_ALLOCATED_LINE_BUFFER y/n

#define	gdwLineLen	   W.ws_dwLineLen

#define	lpForm		   W.ws_lpForm
#define	gfDoneFile		W.ws_fDoneFile
#define	g_fDoneFile		GW.ws_fDoneFile   // replace gfDoneFile
#define	szDir2			W.ws_szDir2
#define	gszFolder		W.ws_szFolder

// Some flags
#define	gfVerbose		W.ws_fVerbose
#define	gfReverse		W.ws_fReverse
//#define	gfBinary		   W.ws_fBinary // drop this form, in preference to
//#define g_fBinary  GW.ws_fBinary // which does NOT need pWS carried around.
//#define	gfCase			W.ws_fCase
#define	gfWhole			W.ws_fWhole
#define	gfNumber		   W.ws_fNumber
#define	gfParity		   W.ws_fParity
// =============================================================================

#ifdef USE_EXCLUDE_LIST
	// EXCLUDE File string(s) LIST
#define  g_sExclList W.ws_sExcList // g_sExclList - list of EXCLUDED files
#define  g_sExclListD GW.ws_sExcListD // g_sExclListD - list of EXCLUDED direcories
#define g_dwExclDCnt GW.ws_dwExclDCnt // g_dwExclDCnt - count of excluded directories
#define g_dwDirsExcl GW.ws_dwDirsExcl // g_dwDirsExcl - count excluded matches
#else // !#ifdef USE_EXCLUDE_LIST
	// EXCLUDE File string(s) buffer
#define	gszExclude		W.ws_szExclude
#endif // #ifdef USE_EXCLUDE_LIST y/n
#define  giFPHidden     W.ws_iFPHidden // -I exclude FrontPage (HIDDEN) directories

#define	gnExclLen		W.ws_nExclLen
#define	gnExclCnt		W.ws_nExclCnt

#define	dwSrchCnt	   W.ws_dwSrchCnt
#define	dwMaskCnt	   W.ws_dwMaskCnt
#define	gdwMax1Line	   W.ws_dwMax1Line
#define	gdwEnd1Line	   W.ws_dwEnd1Line
#define	giFirst		   W.ws_iFirst

#define		glpTmp		&W.ws_szTmpBuf[0]
#define		lpVerb		W.ws_lpVerb
#define		g_lpVerb		GW.ws_lpVerb   // replace lpVerb

#define		fInhibit	   W.ws_fInhibit
#define		gbCaseInhib	W.ws_bCaseInhib

#ifdef	ADDINHIB
//				case 'I':
#define		giInhibCnt	W.ws_iInhibCnt
#define		giInhibSiz	W.ws_iInhibSiz
#define		glpInhib	   W.ws_lpInhib // char *
#endif	// ADDINHIB
// switch -I = Find "#define STL_" would also find "#define     STL_"!!!
#define	   gfSpacey		W.ws_fSpacey      // -I

#define		fInComment	W.ws_fInComment
#define		gdwBgn		W.ws_dwBgn
#define		gdwEnd		W.ws_dwEnd
#define		FindInType	W.ws_findintype // FINDINTYPE
#define		GetChr		W.GETCHR
#define		glpActive	W.ws_glpActive
#define		g_lpActive	GW.ws_glpActive

// -R - Recursive find mask
#define		gfRecursive	W.ws_fRecursive
// FIX20120407 - Add -S
#define		gfStrip	W.ws_fStrip

// -OOutFile
#define		gszOutFile	W.ws_szOutFile
#define		glpOutFile	&gszOutFile[0]
#define		ghOutFile	W.ws_hOutFile

// -D Directory Listing
#define	gfDirLst		   W.ws_fDirLst
#define	gfIsDir			W.ws_fIsDir
#define	giDirLen		   W.ws_iDirLen
#define	gcDirBuf		   W.ws_cDirBuf

// -Ennnnnn
#define	glFindErr		W.ws_lFindErr
#define	gbFindErr		W.ws_bFindErr

#define	giLineCnt		W.ws_iLineCnt
#define	giFileCnt		W.ws_iFileCnt

#ifdef WIN32
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
//#define  gsFileInf      W.ws_sFileInf
//#define  gsFindData     W.ws_sFindData // WIN32_FIND_DATA

// -8 (output DOS 8.3 name only)
// #define		PVERS		"V4.0.10"	// FIX20000902 -8 shows file names in DOS 8.3 form !!!
#define  gbDOSNm        W.ws_bDOSNm    // COMMAND LINE FLAG - Show in 8.3 form
#endif // WIN32
#define  gszDOSNm       W.ws_szDOSNm   // The 8.3 DOS Name
#define  gszDOSPth      W.ws_szDOSPth  // The 8.3 DOS Path

// final OUTPUT buffer to contain the COMBINED PATH\FILENAME (in 8.3 form)
#define  gszDOSWork     W.ws_szDOSWork

// if different, use this to COPY each component, as in
// for( i = 0; i < kkk; i++ )
//      gszDOSLast[i] = glpActive[i];
// and when a "\" is found
// we appear to HAVE a path
//               gszDOSLast[i] = 0;   // this is our NEXT FIND
//               hFind = FindFirstFile(gszDOSLast,&fd);
//               if( VH(hFind) )
//               {
//                  FixAlternate(pWS,&fd);
//                  strcat(gszDOSPth,&fd.cAlternateFileName[0]);
//                  strcat(gszDOSPth,PATH_SEP); // add in the PATH
// so the SIMPLE fix seems to be do not KILL gszDOSPth
// UNLESS THERE IS A DIFFERENCE
#define  gszDOSLast     W.ws_szDOSLast

// used as temporary buffers with - SplitFN( gszFULPth, gszFULNm, glpActive );
#define  gszFULNm       W.ws_szFULNm   // [MAX_PATH+8];    // gszDOSNm
#define  gszFULPth      W.ws_szFULPth

// FIX20010319 -v4 adds date order of finds at end - VERB4
#define  gsFileList     W.ws_FileList  // list of FINDS

// FIX20010413 - add -w2
#define  g_fWhole2      GW.ws_fWhole2
#define  g_bIgComm      GW.ws_bIgComm  // ignore C/C++ comments
// FIX20140926: Add new switch -IP to inhibit finds in Perl comments
#define  g_bIgPComm     GW.ws_bIgPComm  // ignore Perl comments

// switch -F:M or -M[2] switch
#define  g_bDoMake      GW.ws_bDoMake  // obey makefile continuation char on output
#define  g_bDoMake2     GW.ws_bDoMake2 // expand $(mac) entries
#define  g_bDoAllin1    GW.ws_bDoAllin1 // -F:1 all finds in one file
#define  g_bAllInOne    GW.ws_bAllInOne // FIX20060621
// Add an -F:2, indicating ALL FINDS in one LINE of the file
#define  g_dwMinLen     GW.ws_dwMinLen // Minimum length of FINDS

#define  g_iLnBuf       GW.ws_iLnBuf
#define  g_szLnBuf      GW.ws_szLnBuf

//	PVERS		"V4.0.16"	// FIX20010703 - multiple FIND strings MUST be in ORDER
// With this change switch from a set of string in gszFindStgs to a simple DOUBLE LINKED
// LIST of FIND strings, one or as many as necessary, limited only by MEMORY
// see typedef struct tagLINE {
#define  g_sFind      GW.ws_sFind    // = LIST of find strings
#define  g_dwFCnt     GW.ws_dwFCnt   // = Count of user input of FIND strings
#define  g_dwFMax     GW.ws_dwFMax   // = Count of BITS-per-FIND
#define  g_szCurrFind GW.ws_szCurrFind  // = next item taken off list
#define  g_sLines     GW.ws_sLines   // lines for output
#define  g_dwFileNum  GW.ws_dwFileNum  // Numerical gfDoneFile order
#define  g_psActFind  GW.ws_psActFind  // Current active FIND structure
#define  g_sFileList  GW.ws_sFileList  // List of FILES to process
#define  g_sFailList  GW.ws_sFailList  // List of FAILED FILES

#define  g_szTmpBuf   GW.ws_szTmpBuf // = [MXBIGBUF+MXOVR];  global temp buffer
#define  g_szTmpBuf2  GW.ws_szTmpBuf2 // = [MXBIGBUF+MXOVR];  global temp buffer
#define	g_cWorkBuf   GW.ws_cWorkBuf   // general work buffer for LINE output
#define  g_szDirBuf  GW.ws_szDirBuf // [MXBIGBUF+MXOVR];  // last DIRECTORY output

#define  g_bRedirON   GW.ws_bRedirON   // standard output is being redirected
#define  g_szDiagFile GW.ws_szDiagFile // diagnostic output file

// if ADDCVSDATE
#define  g_bCVSDate     GW.ws_bCVSDate // search for latest DATE in CVS entries file
#define	g_nFileCnt		GW.ws_nFileCnt // count of FILE to FIND in
#define  g_sCVSLines    GW.ws_sCVSLines   // = lines in entries file
#define  g_sEntries     GW.ws_sEntries    // = latest lines in entries
#define  g_sCvsLn       GW.ws_sCvsLn      // global pad
#define  g_sFFList      GW.ws_sFFList     // = Found in FILE list
#define  g_pFFList      GW.ws_pFFList     // = Latest FOUND FILE pointer

// if ADDFCOUNT
#define  g_dwMinCnt     GW.ws_dwMinCnt    // -cnn - minimum find count
#define  g_sOutList     GW.ws_sOutList    // store the OUT before showing

#ifdef   ADDLOPTION
#define  g_bGotLOpt     GW.ws_bGotLOpt    // FIX20011011 - Add -L[nn|F] option
#define  g_bGotLFunc    GW.ws_bGotLFunc
#define  g_dwLOptLen    GW.ws_dwLOptLen
// FIX20091229 - Add -LL[nn[:nn]] option to LIMIT LINE OUTPUT
#define g_bGotLLimit    GW.ws_bGotLLimit  // = -LL limit line length output
#define g_dwLLLeft      GW.ws_dwLLLeft    //    = -LLnn - limit to left and right = nn / 2
#define g_dwLLRight     GW.ws_dwLLRight   //   = -LLnn:nn - limit left and limit right
#define g_dwLineLimit   GW.ws_dwLineLimit //   = -LLnn:nn - max. limit length
#endif   // ADDLOPTION

#define  g_iNoOpen      GW.ws_iNoOpen  // BUMP failed to OPEN

#define g_ioHold        GW.ws_ioHold // g_ioHold - got -f:1 and multiple finds

#define  g_iDoneList  GW.ws_iDoneList  // Have output the file name

#define g_fBeginLine    GW.ws_fBeginLine // FIX20061106 - "\ntext" to find text at begin of line

#ifdef   ADD_REGEX
#define  gi_pcre_opions GW.ws_pcre_options
#define  gp_pcre_tables GW.ws_pcretables
#define  gi_pcre_offsets GW.ws_pcre_offsets  // [PCRE_MAX_OFFSETS];
#endif // #ifdef   ADD_REGEX

#define  g_pTmpBuf   GW.ws_szTmpBuf // [MXBIGBUF+MXOVR];  
#define	g_fVerbose	GW.ws_fVerbose
#define	g_fBinary	GW.ws_fBinary
#define	g_fIgnoreBin	GW.ws_fIgnoreBin
#define  g_FileType  GW.ws_FileType

#define  g_dwBackup  GW.ws_dwBackup	// Starts as DEF_BACKUP
#define  g_dwForward GW.ws_dwForward
#define	g_fCase     GW.ws_fCase

#define  g_dwCharEncloding GW.ws_dwCharEncoding  // character encloding
#define  g_bIsBinary GW.ws_bIsBinary   // set if g_fIgnoreBin is ON
#define  g_bIsBinary2 GW.ws_bIsBinary2   // set for EACH file - FIX20070914

#define g_dwBgnLine GW.ws_dwBgnLine
#define g_dwEndLine GW.ws_dwEndLine

#define g_dwOffBgn  GW.ws_dwOffBgn    // Begin offset of find
#define g_dwOffEnd  GW.ws_dwOffEnd    // End offset of find
#define g_dwLBOffBgn  GW.ws_dwLBOffBgn    // Begin offset of find in line buffer
#define g_dwLBOffEnd  GW.ws_dwLBOffEnd    // End offset of find in line buffer

#define g_lpCurrFind GW.ws_lpCurrFind	// ptr "raw" current find string
#define g_dwCurrFLen GW.ws_dwCurrFLen	// length of "raw" find sting
#define g_lpCurFind  GW.ws_lpCurFind	// ptr "modified" current find string
#define g_dwCurFLen  GW.ws_dwCurFLen	// length of "modified" find sting
#define	g_cDirBuf GW.ws_cDirBuf // same as gcDirBuf, without the lpW needed

#define	g_bNoStats GW.ws_bNoStats // -NS = no output of stats at end
#define g_bNoFile  GW.ws_bNoFile  // -NF = no output of file name

#define	gfQuiet		   W.ws_fQuiet


#endif	/* _Fa4Work_H */
// eof - Fa4Work.h

