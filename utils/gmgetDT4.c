

// GMGetDT4.c
#include <stdio.h> // for sprintf(), ...
#ifdef WIN32
#include <windows.h>	// common include
#endif // WIN32
#include <stdint.h>

#ifdef USE_DT4_MODULE

#define		DT3		// Some Get Date and Time functions

#ifndef	NoCrLf

#ifdef	Dv16_App
// Fill in for the WIN32 SYSTEMTIME structure
// ==========================================
#include	<time.h>	// Get the localtime( &tm ) funtion
// =================

typedef struct  _SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    }	SYSTEMTIME;

typedef struct _SYSTEMTIME FAR * LPSYSTEMTIME;

#endif	// Dv16_App

#define	COPYTIME( d, s ) \
{\
	d.wYear = s.wYear;\
	d.wMonth = s.wMonth;\
	d.wDayOfWeek = s.wDayOfWeek;\
	d.wDay = s.wDay;\
	d.wHour = s.wHour;\
	d.wMinute = s.wMinute;\
	d.wSecond = s.wSecond;\
	d.wMilliseconds = s.wMilliseconds;\
}


// TYPES of Date Time string ADDITIONS
// ===================================
#define	NoCrLf			0	// = No Cr/Lf
#define	TrailingCrLf	1	// = Trailing Cr/Lf
#define	TwoCrLf			2	// = Leading AND Trailing Cr/Lf
#define	LeadingCrLf		3	// = Leading Cr/Lf
#endif	// !NoCrLf
#ifdef	WIN32
#ifndef _TM_DEFINED
struct tm {
	int tm_sec;	// seconds after the minute - [0,59]
	int tm_min;	// minutes after the hour - [0,59]
	int tm_hour;	// hours since midnight - [0,23]
	int tm_mday;	// day of the month - [1,31]
	int tm_mon;	// months since January - [0,11]
	int tm_year;	// years since 1900
	int tm_wday;	// days since Sunday - [0,6]
	int tm_yday;	// days since January 1 - [0,365]
	int tm_isdst;	// daylight savings time flag
	};
#define _TM_DEFINED
#endif
#endif	// WIN32

// Done first RUN
int	gmfDnOneTime = FALSE;

SYSTEMTIME gmSysTime; 	// system time structure  
//typedef struct _SYSTEMTIME {  // st  
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
////} SYSTEMTIME; 
// FROM EdTime.c

char	gmszDateTime[48];
char	gmszTime[16];	// Time buffer. HH:MM:ss
char	gmszHrMin[16];	// Just HOUR:MIN
char	gmszDate[32];	// Date buffer
char	gmszCrLf[] = "\r\n";	// = "\r\n";
char	gmszSp1[] = " ";
// Current DAY ascii
char	gmszADay[24];
// Current MONTH ascii
char	gmszAMonth[24];
int	gmfAddSecs = FALSE;
SYSTEMTIME	gmLastTime, gmBeginTime;
//typedef struct _SYSTEMTIME {  // st  
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
//} SYSTEMTIME; 
//Members
//wYear
//Specifies the current year. 
//wMonth
//Specifies the current month; January = 1, February = 2, and so
char gmszCurMth[] = "UnkJanFebMarAprMayJunJulAugSepOctNovDec";
//on. 
//wDayOfWeek
//Specifies the current day of the week; Sunday = 0, Monday = 1,
//and so on. 
char gmszCurDay[] = "SunMonTueWedThuFriSatUnk";
//wDay
//Specifies the current day of the month. 
//wHour
//Specifies the current hour. 
//wMinute
//Specifies the current minute. 
//wSecond
//Specifies the current second. 
//wMilliseconds
//Specifies the current millisecond. 
//Remarks
//It is not recommended that you add and subtract values from the
//SYSTEMTIME structure to obtain relative times. Instead, you
//should 
//·	Convert the SYSTEMTIME structure to a FILETIME structure.
//·	Copy the resulting FILETIME structure to a LARGE_INTEGER
//structure.
//·	Use normal 64-bit arithmetic on the LARGE_INTEGER value.
//See Also
//FILETIME, GetSystemTime, LARGE_INTEGER, SetSystemTime
#ifndef	WIN32
void	GetSystemTime( LPSYSTEMTIME lpst )
{
	time_t	ttime;
	struct tm *	ptm;

	if( ptm = localtime( &ttime ) )
	{
		lpst->wYear = (WORD) ptm->tm_year;
		//int tm_year;	// years since 1900
		lpst->wMonth = (WORD) ptm->tm_mon; 
		//int tm_mon;	// months since January - [0,11]
		lpst->wDayOfWeek = (WORD) ptm->tm_wday; 
		//int tm_wday;	// days since Sunday - [0,6]
		lpst->wDay = (WORD) ptm->tm_mday; 
		//int tm_mday;	// day of the month - [1,31]
		lpst->wHour = (WORD) ptm->tm_hour; 
		//int tm_hour;	// hours since midnight - [0,23]
		lpst->wMinute = (WORD) ptm->tm_min; 
		//int tm_min;	// minutes after the hour - [0,59]
		lpst->wSecond = (WORD) ptm->tm_sec; 
		//int tm_sec;	// seconds after the minute - [0,59]
		lpst->wMilliseconds = 0;
		
		//int tm_yday;	// days since January 1 - [0,365]
		//int tm_isdst;	// daylight savings time flag

	}
}
#endif	// !WIN32

#ifdef WIN32
void	gmDVGetCurrentTime( LPSYSTEMTIME lpt ) // WIN32
{
	GetSystemTime( lpt );
#ifdef	WIN32
	lpt->wHour += 2;
	if( lpt->wHour >= 24 )
	{
		lpt->wHour -= 24;
		lpt->wDay++;
	}
#endif	// WIN32
}

void gtFormatTime( char * lpt, LPSYSTEMTIME lpst ) // WIN32
{
	uint32_t	wCurHr;
	char *	lpampm;
	char *	lptime;

	lptime = lpt;
	if( ((wCurHr = lpst->wHour) == 0 ) ||
		( wCurHr < 12 ) )
	{
		lpampm = "AM";
	}
	else
	{
		if( lpst->wHour > 12 )
			wCurHr = (lpst->wHour - 12);
		else
			wCurHr = lpst->wHour;	// Keep the 12
		lpampm = "PM";
	}

	sprintf( lptime,
		"%2d:%02d:%02d %s",
		(wCurHr & 0xffff),
		lpst->wMinute,
		lpst->wSecond,
		lpampm );

	// And setup 24-Hour time
	lptime = &gmszHrMin[0];
	sprintf( lptime,
			"%2d:%02d",
			lpst->wHour,
			lpst->wMinute );

}

char * gtSetszHrMin( void )
{
	char *	lphm = 0;
	SYSTEMTIME  sysTime;

	//GetLocalTime(&sysTime);
	//GetSystemTime( &sysTime );
	gmDVGetCurrentTime( &sysTime );

	if( gmfAddSecs ||
		( sysTime.wMinute != gmLastTime.wMinute ) ||
		( sysTime.wHour != gmLastTime.wHour ) ||
		( sysTime.wDay != gmLastTime.wDay ) ||
		( sysTime.wMonth != gmLastTime.wMonth ) ||
		( sysTime.wYear != gmLastTime.wYear ) ||
		( sysTime.wDayOfWeek != gmLastTime.wDayOfWeek ) )
	{
		lphm = &gmszHrMin[0];
		sprintf( lphm,
			"%2d:%02d",
			sysTime.wHour,
			sysTime.wMinute );
		gmLastTime = sysTime;
	}
	return( lphm );
}

int gtSetszTime( void )
{
	int	flg = FALSE;
	SYSTEMTIME  sysTime;
	char *	lpt;

	//GetLocalTime(&sysTime);
	// Get SYSTEM TIME
	//GetSystemTime( &sysTime );
	gmDVGetCurrentTime( &sysTime );
	lpt = &gmszTime[0];
	gtFormatTime( lpt, &sysTime );

	if( gmfAddSecs ||
		!gmfDnOneTime ||
		( sysTime.wSecond != gmLastTime.wSecond ) ||
		( sysTime.wMinute != gmLastTime.wMinute ) ||
		( sysTime.wHour != gmLastTime.wHour ) ||
		( sysTime.wDay != gmLastTime.wDay ) ||
		( sysTime.wMonth != gmLastTime.wMonth ) ||
		( sysTime.wYear != gmLastTime.wYear ) ||
		( sysTime.wDayOfWeek != gmLastTime.wDayOfWeek ) )
	{
		gmLastTime = sysTime;
		flg = TRUE;
		gmfDnOneTime = TRUE;
	}
	return( flg );
}

char *	PtrDay( int i )
{
	int	 j, k;
	char *lpDays, *lpRet;

	k = i;
	lpDays = &gmszCurDay[0];
	j = strlen( lpDays ) / 3;
	if( k > 7 )
		k = 7;	// Unknown ends list

	lpDays = &gmszCurDay[k*3];	// Point to string of 3's
	lpRet = &gmszADay[0];
	for( j = 0; j < 3; j++ )
	{
		lpRet[j] = lpDays[j];
	}
	lpRet[j] = 0;
	return lpRet;
}

char *PtrMonth( int i )
{
	int		j, k;
	char *lpMths, *lpRet;

	k = i;
	lpMths = &gmszCurMth[0];
	j = strlen( lpMths ) / 3;
	if( k > 12 )
		k = 0;		// Unknown begins list

	lpMths = &gmszCurMth[k*3];	// Point to string of 3's
	lpRet = &gmszAMonth[0];
	for( j = 0; j < 3; j++ )
	{
		lpRet[j] = lpMths[j];
	}
	lpRet[j] = 0;
	return lpRet;
}

int gtSetszDate( void )
{
	int	flg = FALSE;
	SYSTEMTIME  sysTime;
	char *	lpd;
	char	buf[64+4];

	lpd = &gmszDate[0];
	//GetSystemTime( &sysTime );
	gmDVGetCurrentTime( &sysTime );
	gtSetszTime();

	// Copy the TIME
	COPYTIME( sysTime, gmLastTime );
	//sysTime = gmLastTime;
	//GetLocalTime(&sysTime);

	//GetSystemTime( &sysTime );
	gmDVGetCurrentTime( &sysTime );

	if( sysTime.wDayOfWeek > 7 )
		sysTime.wDayOfWeek = 7;
	if( sysTime.wMonth > 12 )
		sysTime.wMonth = 0;

	sprintf( &buf[0],
		"%s., %s %2d, %4d",
		PtrDay(sysTime.wDayOfWeek),	//	Cur.Day. MonTue...etc
		PtrMonth(sysTime.wMonth),	//  Cur.Mth. JanFebMar...etc
		sysTime.wDay,
		sysTime.wYear );

	if( strcmp( &buf[0], lpd ) )
	{
		flg = TRUE;
		strcpy( lpd, &buf[0] );
	}
	return( flg );
}

// Types
// NoCrLf		=	0 = No Cr/Lf
// TrailingCrLf	=	1 = Trailing Cr/Lf
// TwoCrLf		=	2 = Leading AND Trailing Cr/Lf
// LeadingCrLf	=	3 = Leading Cr/Lf
char * GetTimeStr( int Typ )
{
	char *	lpt;
	lpt = &gmszDateTime[0];
	gtSetszDate();
	lpt[0] = 0;

	// Maybe START with Cr/Lf???
	if( ( Typ == TwoCrLf ) || ( Typ == LeadingCrLf ) )
		strcpy( lpt, &gmszCrLf[0] );

	// Put in SHORT Day, Date
	strcat( lpt, &gmszDate[0] );
	// Add a space
	strcat( lpt, &gmszSp1[0] );
	// Decided ADD seconds
	if( gmfAddSecs )
		strcat( lpt, &gmszTime[0] );
	else
		strcat( lpt, &gmszHrMin[0] );

	// Maybe ADD Cr/Lf after
	if( ( Typ == TrailingCrLf ) || ( Typ == TwoCrLf ) )
		strcat( lpt, &gmszCrLf[0] );

	// Adn return POINTER to this STRING
	return( lpt );

}

// This is the EVOLUTION of DT3, I used in DOS for a long time
// ============================
//#define	NoCrLf			0	// = No Cr/Lf
//#define	TrailingCrLf	1	// = Trailing Cr/Lf
//#define	TwoCrLf			2	// = Leading AND Trailing Cr/Lf
//#define	LeadingCrLf		3	// = Leading Cr/Lf
LPSYSTEMTIME GMSetSystemTime( void )
{
	LPSYSTEMTIME lpt;
	lpt = &gmSysTime;
	GetSystemTime( lpt );
	return lpt;
}

char *	GetDT4( int Typ )
{
	GMSetSystemTime();
	// For FILE Use - Usually ADD (terminator or 2)
	// ============================================
	return( GetTimeStr( Typ ) );
}

char *	GetDT4s( int Typ )
{
	char *	lpt;
	int	fSecs;

	fSecs = gmfAddSecs;
	gmfAddSecs = TRUE;
	GetSystemTime( &gmSysTime );
	// For FILE Use - Usually ADD (terminator or 2)
	// ============================================
	lpt = GetTimeStr( Typ );
	gmfAddSecs = fSecs;

	return lpt;
}

#endif // WIN32


#endif // #ifdef USE_DT4_MODULE

// eof - GNGetDT4.c
