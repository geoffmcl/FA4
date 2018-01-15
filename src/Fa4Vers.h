/*\
 * Fa4Vers.h
 *
 * Copyright (c) 1987 - 2017 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * HISTORY - grwoing upwards
 * NOTE: Also remember to change VERSION information in rc file!
 *
\*/
#ifndef	_Fa4Vers_H
#define	_Fa4Vers_H
// current version

// FIX20180115 - restore -v3 to show date and size of file output
#define  PVERS    "V4.1.8"
#define  PDATE    "2018 Jan 15"

// FIX20171224 - Some 64-bit build fixes, including using 'FILE *' in place of HFILE (int)
// PVERS    "V4.1.7"
// PDATE    "2017 Dec 24"

// FIX20160702 - Add a -Q - Quiet - only output is the find lines
// PVERS    "V4.1.6"
// PDATE    "2016 Jul 02"

// FIX20150627 - Case "*.bak" and "file.txt.bak" FAILED to compare - now FIXED
// PVERS    "V4.1.5"
// PDATE    "2015 Jun 27"

// FIX20150315 - add total lines processed
//  PVERS    "V4.1.4"
//  PDATE    "2015 Mar 15"

// FIX20141214 - Only show full 'failed' list is -V3 or higher
// PVERS    "V4.1.3"
// PDATE    "2014 Dec 14"

// FIX20140926: Add new switch -IP to inhibit finds in Perl comments (begin with #)
// PVERS    "V4.1.2"
// PDATE    "2014 Sep 26"

// FIX20140830: Fixed BUG - Directories were repeated! Abandonned Process_Recursive() and DoThisFile() services
// PVERS    "V4.1.1"
// PDATE    "2014 Aug 30"

// FIX20140727 - Convert to a cmake project, in preparartion of a linux compile
// PVERS    "V4.1.0"
// PDATE    "2014 Jul 27"

// FIX20120809 - Add output of formatted system message when open file FAILED
// PVERS    "V4.0.70"
// PDATE    "2012 Sep  9"

// FIX20120516 - Accept file name MATCHED if REGEX says YES - NEW BEHAVIOUR!!!
// FIX20120407 - Move to WIN7-PC, using MSVC10 - put all source together including pcre and utils
// PVERS    "V4.0.69"
// PDATE    "2012 May 16"

// FIX20111105 - An input of a 'directory', make it directory\*.*
// PVERS    "V4.0.68"
// PDATE    "2011 November  7"

// FIX20110416 - if -f="a" -f="b" -f:2 = find "a" and "b" in the SAME line in perl\*.pl,
// the global FIND counter remains at ZERO!!! That is in Fa4.c:Show_Found_Nums()
// g_dwTotFinds is ZERO!!! It does correctly show 'in 20 files'! ie g_dwFoundInFiles is incd
// With these switches it uses
// #ifdef ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line
//   if( g_bAllInOne && ( g_dwFCnt > 1 )) {
//      Find_In_Lines( pWS, lpInF );
// and later uses, in Fa4Find.c
//             if( Process_Line_For_Finds( pWS, bgnoff, dwi ) ) {
               // we appear to have FOUND all FINDS in this line
//		         ShowLine( pWS, bgnoff, dwi, &lpoff[0], &dwl );
// so it seems here should bump g_dwTotFinds++ = ok, seems to have FIXED that
// but also -v4 and gsFileList contains NO FILES??? In OutFindList() in Fa4.c
// Normally added by void Add2gsFileList( WS ), in fa4Find.c
// AH, the bump should be g_dwFinds, NOT directly g_dwTotFinds
//  PVERS    "V4.0.67"
//  PDATE    "2011 April 16"

// FIX20101026 - make file == file*.*
// PVERS    "V4.0.66"
// PDATE    "2010 October 26"

// FIX20101007 - exclude directories in excluded list when doing directory file finds -d
// PVERS    "V4.0.65"
// PDATE    "2010 October 07"

// FIX20091229 - Add -LL[nn[:nn]] option to LIMIT LINE OUTPUT
//  PVERS    "V4.0.64"
//  PDATE    "2009 December 29"
#define USE_ALLOCATED_LINE_BUFFER

// FIX20091221 if( ( c == '\\' ) && (( nc == '-' )||( nc == '/')) ) { 
// allow an ESCAPED switch value, of a find or file value
//  PVERS    "V4.0.63"
//  PDATE    "2009 December 21"

// FIX20081003 - show found in files count at end - added g_dwFoundInFiles,
// bumped in DoOutFileName(), AND
// Add escaped hyphen, to search for hyphen - is \- == -, but \\- == \-
// PVERS    "V4.0.62"
// PDATE    "2008 October 03"

// FIX20080905 - minor fix in -v9 output - Recursing into adds filename
// PVERS    "V4.0.61"
// PDATE    "2008 September 05"

// PVERS    "V4.0.60"
// PDATE    "2008 March 20"

// FIX20071010 - add -x:@inputfile to exclude a group of directories listed
// in a file, and a small fix for -f"findme" ...
// PVERS    "V4.0.59"
// PDATE    "2007 Oct 10"

// FIX20071006 - add -x::: to excluded CVS and SVN directories, and increase default
// wrap length to - #define MX1LINE 90 // FIX20071006 - in console about 99 wide
//PVERS    "V4.0.58"
//PDATE    "2007 Oct 6"

// FIX20070914 - set b_gIsBinary2 for ALL files, and do 'binary' line output
// PVERS    "V4.0.57"
// PDATE    "2007 Sept 14"

// FIX20070906 - add -b- to IGNORE binary files
//  PVERS    "V4.0.56"
//  PDATE    "2007 Sept 06"

// FIX20070501 - try to get to the END OF THE ASCII, or space
//  PVERS    "V4.0.55"
//  PDATE    "2007 May 1"

// FIX20070328 - CRUDE HACK when using *.c* or *.h* wild matches
// versus regex match - REAL UGLY HACK TOO ;=((
// PVERS    "V4.0.54"
// PDATE    "2007 March 28"
// FIX20070309 - minor cosmetics on brief HELP
//#define  PVERS    "V4.0.53"
//#define  PDATE    "2007 March 09"

//  PVERS    "V4.0.52"      // 20070306 - fix of copy to FIND list -
//  PDATE    "2007 March 06"   // fix to NOT make first letter upper case
// FIX20070306 - AND reverse SORT order - put LATEST last

// ***********************************************************************
// FIX20070120 - fix regex for filename compare using '.*' -> ((\..*)|$),
// and fixed so 's' additions, not if == 1 ...
// PVERS    "V4.0.51"
//  PDATE    "2007 January 20"
// FIX20070115 - minor adjustments of the output
// PVERS    "V4.0.50"
// PDATE    "2007 January 15"
// FIX20070114 - -dod should put OLDEST first, and -do-d should sort as now
// PVERS    "V4.0.49"
// PDATE    "2007 January 14"
// FIX20070106 - preprocess command line for -vNN, and if -v9 show commands as processed
// PVERS    "V4.0.48"
// PDATE    "2007 January 06"
// FIX20070103 - add 'missed' file warnings at end also ...
// PVERS    "V4.0.47"
// PDATE    "2007 January 03"
// FIX20061227 - add pcre (Perl Compatible Regular Expressions) library
// and -f"text" becomes -f=text or -f="with space", and added
// -fr="regex expression" ...
//  PVERS    "V4.0.46"
//  PDATE    "2006 December 27"
#define  ADD_REGEX   // add regular expressions
#ifndef  PCRE_STATIC
#define  PCRE_STATIC    1
#endif   // PCRE_STATIC
#ifndef  SUPPORT_UTF8
#define  SUPPORT_UTF8    1
#endif   /* SUPPORT_UTF8 */
#define  PCRE_MAX_OFFSETS     128

// FIX20061226 - small fix for help message
// PVERS    "V4.0.45"
// PDATE    "2006 December 26"
// FIX20061211 - small fix for 24 hour time
// PVERS    "V4.0.44"
// PDATE    "2006 December 11"
// FIX20061106 - move to Dell01 machine - compile using MSVC8 - and add
// limited regular expressions. The first being "\ntext" can on find text
// at the beginning of a line ... "\\ntext" to find '\ntext' ...
//  PVERS    "V4.0.43"
//  PDATE    "2006 November 6"

// FIX20060729 - add -dn for 'normal' list, as per DIR command (gbDirNorm)
// -d[nlo[d|s]]
//  PVERS    "V4.0.42"
//  PDATE    "2006 July 29"

// FIX20060621
// Add an -F:2, indicating ALL FINDS in one LINE of the file
// PVERS    "V4.0.41"
// PDATE    "2006 June 21"
#define ADD_F2_ALL1LINE // FIX20060621 - Add -F:2 - all finds in 1 line

// FIX20060605 
// one or the other is BLANK
// this is a MATCH if one is an ASTERIX or ".*"!!!
// PVERS    "V4.0.40"
// PDATE    "2006 June 5"

// FIX20060426 - Compiled with MSVC7 - some *.c* or *.h* matching BUG
// PVERS    "V4.0.39"
//  PDATE    "2006 April 26"

// FIX20060311 - minor fix to KinfofCompare
//  PVERS    "V4.0.38"
//  PDATE    "2006 March 15"

// FIX20060311 - fix -do to produce SORTED list
//  PVERS    "V4.0.37"  // // FIX20060311 - fix -do[d|s]
//  PDATE    "2006 March 11" // also ordered list if VERB4 

// FIX20060126 - An input of say cmptidy* should be the same as cmptidy*.*,
// but perhaps NOT cmptidy*. != cmptidy*.* - 
// Also a small fix for output file time ranking done in Pgm_Exit()
// PVERS    "V4.0.36"  // // FIX20060126 - An input a* == a*.*
// PDATE    "2006 January 26" // and ranked list if VERB5 

// PVERS    "V4.0.35"  // FIX20060123 - add -do[ds] to order date(def) or size
// PDATE    "2006 January 23" // -do[d|s]
#ifdef WIN32
#define  ADD_DIRLIST_SORT2
#endif // WIN32

//  PVERS    "V4.0.34"  // FIX20051220 - add -x:: to exclude a FRONTPAGE folders
//  PDATE    "2005 December 20" // note: -x:_* also works
//  PVERS    "V4.0.33"  // FIX20051127 - add -x: to exclude a folder -r assumed
//  PDATE    "2005 November 27" // also fix -x@infile - see Fa4.txt file
#define USE_EXCLUDE_LIST   // switch from 'silly' double zero terminated, to list

// PVERS    "V4.0.32"  // FIX20050304 continued -f:1
// PDATE    "2005 November 16" // see Fa4.txt file
// PVERS    "V4.0.31"  // FIX20051107 additional output if -v9
// PDATE    "2005 November 7" // allow multiple files on command
// PVERS    "V4.0.30"  // FIX20050724 using -dl -v4 show file date, size in output
// PDATE    "2005 July 24"
// PVERS    "V4.0.29"  // FIX20050720 = minor output addition
// PDATE    "2005 July 20"
//   int ws_bDoAllin1; // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file
// PVERS    "V4.0.28"  // FIX20050304 = g_bDoAllin1 -F:1 all finds in one file
// PDATE    "2005 March 4"
// PVERS    "V4.0.27"  // FIX20050216 - find crash
// PDATE    "2005 February 16"
// PVERS    "V4.0.26"  // FIX20050212 - fix -r switch
// PDATE    "2005 February 12" // in PRO-1 using MSVC7(sln)
//   PVERS    "V4.0.25"  // FIX20050201 - add -DL switch - filelist
//   PDATE    "2005 February 1" // int PRO-1 using MSVC7(sln)
// PVERS    "V4.0.24"  // FIX20050121 - fix -D switch - speed up actually
// PDATE    "2005 January 20" // int PRO-1 using MSVC7(sln)
// PVERS    "V4.0.23"  // FIX20041210 - fix "file.doc" and -b output
// PDATE    "2004 December 10" // changed DEF_BACKUP
// PVERS    "V4.0.22"  // FIX20041205 - "page" *.htm* -r FAILED
// fixups turned out to be in GrmLib.c, now corrected ...
// PDATE    "2004 December 5"
//  PVERS    "V4.0.21"  // FIX20040928 - first in PRO-1 ...
//  PDATE    "2004 September 28"
// PVERS    "V4.0.20"  // FIX20030529 - Some failed to open msg????
// PDATE    "2003 May 29"  // hope its fixed?
// previous versions
// PVERS    "V4.0.19"  // FIX20011011 - Add -L[nn|F] option.
// PDATE    "2001 October 11"  // FIX20011011 - Add -L[nn|F] option.
//PVERS		"V4.0.2"	// 1998 October
//PVERS		"V4.0.3"	// FIX981029 - 'C' runtime
//PVERS		"V4.0.4"	// FIX981120 - Add -R recursive
//PVERS		"V4.0.5"	// FIX990123 - Folder & Zero fix
//PVERS		"V4.0.6"	// FIX990125 - Add -I[nhibit]
//PVERS		"V4.0.7"	// FIX20000217 - Add stats at end
//PVERS		"V4.0.8"	// FIX20000606 - Add -I ignore spaces in compare
//PVERS		"V4.0.9"	// FIX20000801 - -V3 also shows file date and size
//PVERS		"V4.0.10"	// FIX20000902 -8 shows file names in DOS 8.3 form !!!
//PVERS		"V4.0.11"	// FIX20000919 -8 fix to show full PATH names in DOS 8.3
//PVERS		"V4.0.12"	// FIX20001022 fix find of "'@'"!!!
//PVERS		"V4.0.13"	// FIX20010319 -v4 adds date order of finds at end
//PVERS		"V4.0.14"	// FIX20010328 Fix for UNIX file searching
//PVERS		"V4.0.15"	// FIX20010413 - add -w2 for space whole
// and -I+ inhibit finds in C/C++ comment lines
//PVERS		"V4.0.16"	// FIX20010703 - multiple FIND strings MUST be in ORDER
// With this change switch from a set of string in gszFindStgs to a simple DOUBLE LINKED
// LIST of FIND strings, one or as many as necessary, limited only by MEMORY
//	PVERS		"V4.0.17"   // FIX20010824 - I put it BACK - allow TABS thru unharmed
//	PVERS		"V4.0.18"   // FIX20010911 - Hi-char in input list

//PDATE		"1998 October"
//PDATE		"1998 November"
//PDATE		"1999 January"
//PDATE		"2000 February"
//PDATE		"2000 June"
//PDATE		"2000 August"
//PDATE		"2000 September 19"
//PDATE		"2000 October 22" // try to FIX a find of "'@'" command!!!
//PDATE		"2001 March 19"	// FIX20010319 -v4 adds date order of finds at end
//PDATE		"2001 March 28"	// FIX20010328 Fix for UNIX (Only 0x0a)
//PDATE		"2001 March 29"	// FIX20010329 Fix for overrun of error buffer
//PDATE		"2001 April 13"	// FIX20010413 - add -w2 for space whole and
// -I+ inhibit finds in C/C++ comment lines
//PDATE		"2001 July 3"	// FIX20010703 - multiple FIND strings MUST be
// if ADDCVSDATE
//PDATE    "2001 July 15" // FIX20010715 = search for latest DATE in CVS entries file
//PDATE    "2001 August 24"  // FIX20010824 - I put it BACK - allow TABS thru unharmed
// and ADDFCOUNT = Minimum FIND count before OUTPUT
//PDATE    "2001 September 11"  // FIX20010911 - Hi-char in input list
#define     ADDLOPTION

#undef     ADDFCOUNT   // FIX20010824 - Minimum FIND count before OUTPUT

#ifdef WIN32
// FIX20010715 = search for latest DATE in CVS entries file
#define  ADDCVSDATE
#endif

// output in ORDER
#define  USEFINDLIST    // above change to FIND list = FIX20010703
#define  MXONEFIND      1024     // max for a single find string

// where are these used???
//#define		MAJ_VERS		5
//#define		MIN_VERS		0
//#define		BLD_VERS		1

#define     MAXPBUF     1000     // getting too BIG for sprintf()

#define		MXCMDBUF	   1024
#define		MXVERBBUF	1024

#define		MXLINEBUF	2048	// a LINE buffer

#define		MXFNDSTGS	2048

#define		MXFILSTGS	2048

#define     MXBIGBUF    (1024*16)   // 16K buffer

#define		DEF_VERB		2

#undef		ADD2ZLST    // REMOVE FOREVER - NOw use doubel LINKED lists

#define		ADDRECUR	// recursive into sub-directories
#define		ADDOUTFIL	// establish an OUTPUT file

#ifdef WIN32
// a WIN32 directory list - quite special functions for windows
#define		ADDDIRLST	// Special for Directory Listing
#define		ADDERRLST	/* -Ennnn Find error */
#endif // WIN32

#define		USERDBUF	// when adding -I, improved INPUT of a "switch" file - June, 2000

#ifndef	ADDDIAGT
#ifdef	_DEBUG
#define		ADDDIAGT
#else	// !_DEBUG
#undef		ADDDIAGT
#endif	// _DEBUG y/n
#endif	// ADDDIAGT

#undef		ADDDIAG2	// Some BIG TIME/DATE DIAGS
#undef		ADDDIAG3	// Output the FILE LINE in Decode...
#undef		ADDDIAG4	// Show EXCLUDED Directories
#define		ADDINHIB

//#define		MXINPFS		10
#define		MXARGS		512   // FIX20080320 - was a MEAN 64
//#define		MXCMDBUF	2048
//#define		MXVERBBUF	1024
//#define		MXLINEBUF	2048	// a LINE buffer

#define		GEN_ERROR		(int)-1

// Buffers for FIND and FILE strings
//#define		MXFNDSTGS		2048
//#define		MXFILSTGS		MXFNDSTGS

//#define		VH(a)		( ( a ) && ( a != INVALID_HANDLE_VALUE ) )

#define		CNAME		      "Informatique Rouge"
#define		PNAME		      "FAST FIND ALL UTILITY"
#define		MXIO		      256
#define		MXDNAME		   13
#define		SZLEN		      13
#define		MXITEM1		   38
#define		MXITEM2		   14

//#define		MX1LINE		   74
#define		MX1LINE		   90 // FIX20071006 - in console about 99 wide
// could use say GetConsoleScreenBufferInfo and dynamically adjust this!!!
#define		MXELINE		   120

//#define		DEF_BACKUP		16
//#define		DEF_FORWARD		16
#define		DEF_BACKUP		40
#define		DEF_FORWARD		40
#define		DEF_MAXFN		35

// Columns in Decode (DC4)
#define		NMCOL		13
#define		SZCOL		27
#define		DTCOL		37
#define		TMCOL		43

// Simple READ ONLY mapping attributes
#define		g_flProtext		PAGE_READONLY
#define		g_ReqAccess		FILE_MAP_READ

// List of Errors
#define		ERR_N0001		1
#define		ERR_N0002		2
#define		ERR_N0003		3
#define		ERR_N0004		4
#define		ERR_N0005		5     // failed in regex compile

#define		VERB		( gfVerbose >  0 )
#define     VERB1    VERB
#define		VERB2		( gfVerbose >  1 )
#define		VERB3		( gfVerbose >  2 )
#define		VERB4		( gfVerbose >  3 )
#define		VERB5		( gfVerbose >  4 )
#define		VERB6		( gfVerbose >  5 )
#define		VERBM		( gfVerbose >  8 )
#define     VERB9    VERBM

#define		GVERB		( g_fVerbose >  0 )
#define		GVERB2	( g_fVerbose >  1 )
#define		GVERB3	( g_fVerbose >  2 )
#define		GVERB4	( g_fVerbose >  3 )
#define		GVERB5	( g_fVerbose >  4 )
#define		GVERB6	( g_fVerbose >  5 )
#define		GVERB9	( g_fVerbose >  8 )


#define		SPLFA	(FILE_ATTRIBUTE_DIRECTORY |\
					FILE_ATTRIBUTE_HIDDEN |\
					FILE_ATTRIBUTE_OFFLINE |\
					FILE_ATTRIBUTE_READONLY |\
					FILE_ATTRIBUTE_SYSTEM)

#define		EndBuf(a)		( a + strlen(a) )

#define		PRTTERM		"\n"
#ifdef WIN32
#define     MEOR        "\r\n"
#else
#define     MEOR        "\n"
#endif

#define		FIX20000902    // -8 shows file names in DOS 8.3 form !!!
#define		FIX20000919    // -8 fix to show full path if the same as previous
#define		FIX20001022    // fix find of "'@'"!!!
#define     FIX20010413    // add a -w2 meaning MUST be space before and after WHOLE
// and maybe -IC to inhibit finds in C/C++ comment lines

// FIX20010319 -v4 adds date order of finds at end - VERB4
#ifdef WIN32
#define  MALLOC(a,b)    LocalAlloc(a,b)
#define  MFREE          LocalFree
#else
#define  MALLOC(a,b)    malloc(b)
#define  MFREE          free
#endif

#endif	/* _Fa4Vers_H */
// eof - Fa4Vers.h
