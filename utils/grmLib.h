
// grmLib.h - MAIN - August, 1997
// Nov 1999 - Added ICO file type
#ifndef	_GRMLIB_H
#define	_GRMLIB_H
#include <stdint.h>
#include <stdio.h> // for fopen(), sprintf(), ...

#if !defined(WIN32)
#include <string.h> // for strlen(), ...
#include <strings.h> // for strcasecmp(), ...
#include <ctype.h> // for toupper(), ...
#include <stdarg.h> // for va_start, ...

#if !defined(HANDLE_DECLARED)
#define HANDLE_DECLARED
typedef void * HANDLE;
typedef void * HFILE;
#endif // !HANDLE_DECLARED

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define UNREFERENCED_PARAMETER(a) (void)a
#define strcmpi strcasecmp

#endif

extern	int	SplitFN( char * lpdir, char * lpfil, char * lpext );
extern	int	MatchFiles( char * lp1, char * lp2 );
extern	int	GotWild( char * lps );
extern	int	HasDot( char * lps );
extern	int	IsDot( char * lps );
extern	int	IsDDot( char * lps );
extern	int	SetFileType( char *, uint32_t *pu );
extern	int	GMInStr( char * lpsrc, char * lpfind );

extern	void Buffer2Stg( char * lps, char * lpb, int decimal,
				 int sign, int precision );
extern	void RTrimDecimal( char * lpr );
extern	void Double2Stg( char * lps, double factor );
extern	void Dbl2Stg( char * lps, double factor, int prec );
extern	void Double2TStg( char * lps, double factor );
extern	uint32_t Dbl2TStg( char * lps, double ds );
extern	uint32_t DW2TStg( char * lps, uint32_t dw );

extern	int	SetDiagFile( char * lpf );
extern	HANDLE CreateUserFile( char * lpf );
extern	void SetUserHandle( HANDLE hf );
extern	void CloseUserFile( HANDLE hf );

#define NOT_VALID 0
#define VALID_DIR 1
#define VALID_FILE 2

extern int is_file_or_directory64( char *path );
extern uint64_t get_last_file_size64();
extern uint64_t get_last_file_mtime64();
extern uint64_t get_last_file_atime64();
extern uint64_t get_last_file_ctime64();
extern int IsValidDir( char * lpf );
extern int IsValidFile( char * lpf );

#ifdef WIN32
// ADDED FROM GMUtils.c - December, 1999
// =====================================
extern	int	CenterDialog( HWND hChild, HWND hParent );
//	(NOTE: was called CenterWindow in GMUtils)
#endif // WIN32

#define		MAX_EXT				256

#define		FT_UNKNOWN			0
#define		FT_UNK				0
#define		FT_BIN				0x80000000
#define		FT_ISC				0x40000000
#define		FT_ISA				0x20000000

#define		FT_LIBRARY			FT_BIN | 0x00000001
#define		FT_DYNALIB			FT_BIN | 0x00000002
#define		FT_EXE				FT_BIN | 0x00000003
#define		FT_BAT				FT_UNK | 0x00000004
#define		FT_COM				FT_BIN | 0x00000005
#define		FT_BMP				FT_BIN | 0x00000006
#define		FT_C				FT_ISC | 0x00000007
#define		FT_CPP				FT_ISC | 0x00000008
#define		FT_CXX				FT_ISC | 0x00000009
#define		FT_H				FT_ISC | 0x0000000a
#define		FT_ASM				FT_ISA | 0x0000000b
#define		FT_TXT				FT_UNK | 0x0000000c
#define		FT_GIF				FT_BIN | 0x0000000d
#define		FT_JPG				FT_BIN | 0x0000000e
#define		FT_OCX				FT_BIN | 0x0000000f
#define		FT_OBJ				FT_BIN | 0x00000010
#define		FT_DSW				FT_UNK | 0x00000011
#define		FT_DSP				FT_UNK | 0x00000012
#define		FT_MAK				FT_UNK | 0x00000013
#define		FT_PLG				FT_UNK | 0x00000014
#define		FT_OPT				FT_BIN | 0x00000015
#define		FT_OLD				FT_UNK | 0x00000016
#define		FT_BAK				FT_UNK | 0x00000017
#define		FT_IDB				FT_BIN | 0x00000018
#define		FT_PDB				FT_BIN | 0x00000019
#define		FT_PCH				FT_BIN | 0x0000001a
#define		FT_ILK				FT_BIN | 0x0000001b
#define		FT_ICO				FT_BIN | 0x0000001c
// NOTE: This MUST be the LAST
#define		FT_NONE				FT_UNK | 0x0000001d

#ifndef ECVT
#ifdef WIN32
#define ECVT _ecvt
#else
#define ECVT ecvt
#endif
#endif // ECVT

#endif	// _GRMLIB_H
// eof - grmLib.h - MAIN
