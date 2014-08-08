

// Fa4Util.h
#ifndef	_Fa4Util_H
#define	_Fa4Util_H

#define  ISNUM(a)    ( ( a >= '0' ) && ( a <= '9' ) )

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : InStr
// Return type: int32_t 
// Arguments  : char * lpb
//            : char * lps
// Description: Return the position of the FIRST instance of the string in lps
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
extern	int32_t   InStr( char * lpb, char * lps );

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Mid
// Return type: char * 
// Arguments  : char * lpl - Point32_ter to line
//            : uint32_t dwb  - Begin here
//            : uint32_t dwl  - for this length
// Description: Returns a buffer containing the MIDDLE portion of a string.
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
extern   char *   Mid( char * lpl, uint32_t dwb, uint32_t dwl );

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Left
// Return type: char * 
// Arguments  : char * lpl
//            : uint32_t dwi
// Description: Return the LEFT prortion of a string
//              Emulates the Visual Basic function
///////////////////////////////////////////////////////////////////////////////
extern   char *   Left( char * lpl, uint32_t dwi );

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Right
// Return type: char * 
// Arguments  : char * lpl
//            : uint32_t dwl
// Description: Returns a buffer containing the RIGHT postion of a string
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
extern   char *   Right( char * lpl, uint32_t dwl );


extern   uint32_t    TrimIB( char * lps );  // trim leading and trailing spacey stuff
#if (defined(WIN32) && defined(USE_SYSTEM_TIME))
extern   int     Stg2SysTm( char * lps, SYSTEMTIME * pt );  // for WIN32
extern   void     AppendDateTime( char * lpb, LPSYSTEMTIME pst ); // for WIN32
#else
extern   void     AppendDateTime( char * lpb, time_t *pst ); // for ALL
#endif // WIN32
extern   int32_t MCDECL sprtf( char * lpf, ... );
extern   char *   DSecs2YDHMSStg( double db );
extern char * GetNxtBuf( void );
extern char * My_NiceNumber( char * lpn );
extern char * My_NiceNumberStg( uint32_t num );

#endif	// _Fa4Util_H
// eof - Fa4Util.h
