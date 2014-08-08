

// Fa4Util.c
#include	"Fa4.h"
// #include	"Fa4Util.h"

char *   GetNxtBuf( void )
{
   // (MXLINEB * MXLINES)
   int   i = g_iLnBuf;      //     GW.ws_iLnBuf
   i++;
   if( i >= MXLINES )
      i = 0;
   g_iLnBuf = i;
   return( &g_szLnBuf[ (MXLINEB2 * i) ] );    // GW.ws_szLnBuf
}

uint32_t TrimIB( char * lps )
{
   uint32_t    dwr = strlen(lps);
   char *   p   = lps;
   uint32_t    dwk = 0;
   while(dwr)
   {
      if( *p > ' ' )
         break;
      dwk++;
      p++;
      dwr--;   // update return length
   }
   if(dwk)
      strcpy(lps,p);    // copy remainder up to beginning
   dwk = dwr;
   while(dwk--)
   {
      if( lps[dwk] > ' ' )
         break;
      lps[dwk] = 0;
      dwr--;
   }
   return dwr;
}
   
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : InStr
// Return type: int 
// Arguments  : char * lpb
//            : char * lps
// Description: Return the position of the FIRST instance of the string in lps
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
int   InStr( char * lpb, char * lps )
{
   int   iRet = 0;
   int   i, j, k, l, m;
   char    c;
   i = strlen(lpb);
   j = strlen(lps);
   if( i && j && ( i >= j ) )
   {
      c = *lps;   // get the first we are looking for
      l = i - ( j - 1 );   // get the maximum length to search
      for( k = 0; k < l; k++ )
      {
         if( lpb[k] == c )
         {
            // found the FIRST char so check until end of compare string
            for( m = 1; m < j; m++ )
            {
               if( lpb[k+m] != lps[m] )   // on first NOT equal
                  break;   // out of here
            }
            if( m == j )   // if we reached the end of the search string
            {
               iRet = k + 1;  // return NUMERIC position (that is LOGICAL + 1)
               break;   // and out of the outer search loop
            }
         }
      }  // for the search length
   }
   return iRet;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Mid
// Return type: char * 
// Arguments  : char * lpl - Pointer to line
//            : uint32_t dwb  - Begin here
//            : uint32_t dwl  - for this length
// Description: Returns a buffer containing the MIDDLE portion of a string.
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
char *   Mid( char * lpl, uint32_t dwb, uint32_t dwl )
{
   char *   lps = GetNxtBuf();
//   char *   pt;
   uint32_t    dwk = strlen(lpl);
   uint32_t    dwi, dwr;
   *lps = 0;
   if( ( dwl ) && 
      ( dwb ) &&
      ( dwl ) &&
      ( dwb <= dwk ) &&
      ( dwl <= (dwk - (dwb - 1)) ) )
   {
      dwr = 0;
      for(dwi = (dwb - 1); (dwi < dwk), (dwr < dwl); dwi++ )
      {
//         pt = &lpl[dwi];
         lps[dwr++] = lpl[dwi];
      }
      lps[dwr] = 0;
   }
   return lps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Left
// Return type: char * 
// Arguments  : char * lpl
//            : uint32_t dwi
// Description: Return the LEFT prortion of a string
//              Emulates the Visual Basic function
///////////////////////////////////////////////////////////////////////////////
char *   Left( char * lpl, uint32_t dwi )
{
   char *   lps = GetNxtBuf();
   uint32_t    dwk;
   for( dwk = 0; dwk < dwi; dwk++ )
      lps[dwk] = lpl[dwk];
   lps[dwk] = 0;
   return lps;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Right
// Return type: char * 
// Arguments  : char * lpl
//            : uint32_t dwl
// Description: Returns a buffer containing the RIGHT postion of a string
//              Emulates the Visual Basic function.
///////////////////////////////////////////////////////////////////////////////
char *   Right( char * lpl, uint32_t dwl )
{
   char *   lps = GetNxtBuf();
   uint32_t    dwk = strlen(lpl);
   uint32_t    dwi;
   *lps = 0;
   if( ( dwl ) &&
      ( dwk ) &&
      ( dwl <= dwk ) )
   {
      if( dwl == dwk )  // is it right ALL
         dwi = 0;
      else
         dwi = dwk - dwl;
      strcpy(lps, &lpl[dwi] );
   }

   return lps;
}


//typedef struct _SYSTEMTIME { 
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
//} SYSTEMTIME, *PSYSTEMTIME; 
//Members
//wYear 
//Specifies the current year. 
//wMonth 
//Specifies the current month; January = 1, February = 2, and so on. 
//wDayOfWeek 
//Specifies the current day of the week; Sunday = 0, Monday = 1, and so on. 
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
// ADDCVSDATE - Get the latest DATE
// ctime( &ltime ) = Fri Apr 29 12:25:12 1994

char *   pDays[] = {
   { "Sun" },
   { "Mon" },
   { "Tue" },
   { "Wed" },
   { "Thu" },
   { "Fri" },
   { "Sat" }
};

char *   pMths[] = {
   { "Jan" },
   { "Feb" },
   { "Mar" },
   { "Apr" },
   { "May" },
   { "Jun" },
   { "Jul" },
   { "Aug" },
   { "Sep" },
   { "Oct" },
   { "Nov" },
   { "Dec" }
};

#define  EATSPACE(p)    while( *p && (*p <= ' '))p++
#define  EATCHARS(p)    while( *p && (*p >  ' '))p++

int  strbgn( char * lps, char * lpd )
{
   int  bRet = FALSE;
   uint32_t i = strlen(lpd);
   if( i <= strlen(lps) )
   {
      uint32_t   j;
      for( j = 0; j < i; j++ )
      {
         if( lps[j] != lpd[j] )
            return FALSE;
      }
      bRet = TRUE;
   }
   return bRet;
}

int  Stg2SysTm( char * lps, SYSTEMTIME * pt )
{
   char *   lpd;
   int      i;

   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;

   for( i = 0; i < 7; i++ )
   {
      lpd = pDays[i];
      if( strbgn( lps, lpd ) )
         break;
   }
   if( i == 7 )
      return FALSE;

   pt->wDayOfWeek = (WORD)i;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;

   for( i = 0; i < 12; i++ )
   {
      lpd = pMths[i];
      if( strbgn( lps, lpd ) )
         break;
   }
   if( i == 12 )
      return FALSE;

   pt->wMonth = (WORD)(i + 1);

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   pt->wDay = (WORD) atoi(lps);

   if( ( pt->wDay < 1 ) || ( pt->wDay > 31 ) )
      return FALSE;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   if(( strlen(lps) < 8 ) ||   // 12:45:78
      ( lps[2] != ':'   ) ||
      ( lps[5] != ':'   ) )
      return FALSE;

   pt->wHour   = (WORD)atoi(lps);
   pt->wMinute = (WORD) atoi( &lps[3] );
   pt->wSecond = (WORD) atoi( &lps[6] );
   pt->wMilliseconds = 0;

   EATCHARS(lps);
   EATSPACE(lps);
   if( *lps == 0 )
      return FALSE;
   if( !ISNUM(*lps) )
      return FALSE;

   pt->wYear = (WORD)atoi(lps);

   return TRUE;
}

// added April, 2001
#if (defined(WIN32) && defined(USE_SYSTEM_TIME))
void  AppendDateTime( char * lpb, LPSYSTEMTIME pst ) // IFF USE_SYSTEM_TIME
{
   sprintf(EndBuf(lpb),
      "%02d/%02d/%02d %02d:%02d",
      (pst->wDay & 0xffff),
      (pst->wMonth & 0xffff),
      (pst->wYear % 100),
      (pst->wHour & 0xffff),
      (pst->wMinute & 0xffff) );
}
#else
void  AppendDateTime( char * lpb, time_t *ptt )
{
    if (ptt && *ptt) {
        struct tm *pt = localtime(ptt);
        if (pt) {
            sprintf(EndBuf(lpb),
                "%02d/%02d/%02d %02d:%02d",
                pt->tm_mday,
                pt->tm_mon,
                pt->tm_year,
                pt->tm_hour,
                pt->tm_sec);
        }
    }
}

#endif

int   MCDECL sprtf( char * lpf, ... )
{
   static char _s_sprtfbuf[1024];
   char *   lpb = &_s_sprtfbuf[0];
   int   i;
   va_list arglist;
   va_start(arglist, lpf);
   i = vsprintf( lpb, lpf, arglist );
   va_end(arglist);
   prt(lpb);
   return i;
}

char *   DSecs2YDHMSStg( double db )
{
   char *   lps = GetNxtBuf();
   double   dsind = (60*60*24);
   double   dsiny = (dsind * 365);

   *lps = 0;
   if( db >= dsiny )
   {
      int yrs = (int)(db / dsiny);
      db = (db - (dsiny * (double)yrs));
      if(yrs > 1)
         sprintf(lps, "%d years ", yrs);
      else if(yrs == 1)
         strcpy(lps, "1 year ");
      else
         sprintf(lps, "%d year(s) ", yrs);
   }
   if( db >= dsind )
   {
      int dys = (int)(db / dsind);
      db = (db - (dsind * (double)dys));
      if(dys > 1)
         sprintf(EndBuf(lps), "%d days ", dys);
      else if(dys == 1)
         strcat(lps, "1 day ");
      else
         sprintf(EndBuf(lps), "%d day(s) ", dys);
   }

   if( db > 0 )
   {
      int   hrs = (int)( db / (60*60));
      db = ( db - (60*60*hrs) );
      sprintf(EndBuf(lps), "%02d:", hrs);
      if( db > 0 )
      {
         int mins = (int)( db / 60 );
         int secs;

         db = ( db - (60 * mins) );
         secs = (int)db;
         sprintf(EndBuf(lps), "%02d:%02d", mins, secs);
      }
      else
      {
         strcat(lps, "00:00");
      }
   }
   else
   {
      strcat(lps, "00:00:00");
   }

   return lps;
}

// return nice number - with comas
char * My_NiceNumber( char * lpn )
{
   size_t i, j, k;
   char * lpr = GetNxtBuf();
   *lpr = 0;
   i = strlen( lpn );
	if( i ) {
      if( i > 3 ) {
			k = 0;
			for( j = 0; j < i; j++ ) {
            // FIX20070923 - avoid adding FIRST comma
				if( k && ( ( (i - j) % 3 ) == 0 ) )
					lpr[k++] = ',';
				lpr[k++] = lpn[j];
			}
			lpr[k] = 0;
		} else {
			strcpy( lpr, lpn );
		}
   }
   return lpr;
}

// given a uint32_t number, get nice number in buffer
char * My_NiceNumberStg( uint32_t num )
{
   char * lpr = GetNxtBuf();
   sprintf(lpr, "%u", num);
   return (My_NiceNumber(lpr));
}

// eof - Fa4Util.c
