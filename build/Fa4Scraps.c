
// code extracted from Fa4
// =======================

#if 0 // 000000000000000000000000000000

void UnixTimeToFileTime(time_t t, LPFILETIME pft) // depreciated - not used
{
     // Note that LONGLONG is a 64-bit value
     long long ll;
     ll = Int32x32To64(t, 10000000) + 116444736000000000;
     pft->dwLowDateTime = (uint32_t)ll;
     pft->dwHighDateTime = (uint32_t)(ll >> 32);
}

void FileTimeToUnixTime(time_t * pt, LPFILETIME pft) // depreciated - not used
{
   // doc->filetimes.actime =
   // (time_t)( ( actime - 116444736000000000LL) / 10000000LL );
#if (defined(_MSC_VER) && (_MSC_VER < 1300))
   LONGLONG ll = (pft->dwHighDateTime << 32) + pft->dwLowDateTime;
   *pt = (time_t)((ll - 116444736000000000) / 10000000);
#else // MSVC8 or later
    ULARGE_INTEGER ul;
    ul.HighPart = pft->dwHighDateTime;
    ul.LowPart  = pft->dwLowDateTime;
   *pt = (time_t)((ul.QuadPart - 116444736000000000) / 10000000);
#endif

}

void UnixTimeToSystemTime(time_t t, LPSYSTEMTIME pst)  // depreciated  - not used
{
   FILETIME ft;
   UnixTimeToFileTime(t, &ft);
   FileTimeToSystemTime(&ft, pst);
} 

#endif // 0000000000000000000000000000000

#if 0 // 00000000000000000000000000000000000000000000000000000000000000000000
// perhaps we should TRY HARDER to FIND the file
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : TryAllOpens
// Return type: int 
// Arguments  : char * lpf
//            : HANDLE * ph
// Description: This handles the fact that one is in one character set
//              while the name is in another.
// It does NOT presently handle correctly if there are also different
// characterset in the PATH NAME - But maybe one day ... Sep 2001
///////////////////////////////////////////////////////////////////////////////
int  TryAllOpens( char * lpf, HANDLE * ph )
{
   int  flg = FALSE;   // set FAILED
   HANDLE            hFind;
   WIN32_FIND_DATA   fd;
   LPWORKSTR	      pWS = lpWS;
   char *            lpt = &g_szTmpBuf2[0];

#ifdef   ADDTRYFIND2    // these tries FAILED, so for now EXCLUDED
   hFind = FindFirstFile( lpf, &fd );
   if( VH(hFind) )
   {
      FindClose(hFind);
      if(VERBM)
      {
   		strcpy(lpVerb, "ERROR: Found - ");
   		BuildDirString( lpVerb, &fd, 0, FALSE, TRUE, SZLEN );
         prt( lpVerb );	// Out it, adding Cr/Lf
      }
   }
   else
   {
      if( InStr( lpf, " " ) )
      {
         strcpy(lpt, "\"");
         strcat(lpt, lpf);
         strcat(lpt, "\"");
         hFind = FindFirstFile( lpt, &fd );
         if( VH(hFind) )
         {
            FindClose(hFind);
            if(VERBM)
            {
         		strcpy(lpVerb, "ERROR: Found - ");
         		BuildDirString( lpVerb, &fd, 0, FALSE, TRUE, SZLEN );
               prt( lpVerb );	// Out it, adding Cr/Lf
            }
         }
      }
   }
   if( !flg )
   {
		HANDLE		hf;
		OFSTRUCT	   of;
		hf = (HANDLE)OpenFile( lpf, &of, OF_READ );
		*ph = hf;
		if( VH(hf) )
		{
			flg = TRUE;
		}
   }
#endif   // #ifdef   ADDTRYFIND2
   if( !flg )
   {
      char * p;
      strcpy(lpt, lpf);
      p = strrchr(lpt, '\\');
      if(p)
      {
         *p = 0;
         hFind = FindFirstFile(lpt, &fd);
         if( VH(hFind) )
         {
            FindClose(hFind);
            if(VERBM)
            {
               sprintf( lpVerb, "Found %s [%s]"MEOR,
                  ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "<DIR>" : "<File>"),
                  lpt );
               prt(lpVerb);
            }
            p = strrchr(lpf, '\\');
            if( p &&
               ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
               strcat(lpt, "\\*.*");
               p++;  // move up to file name
               hFind = FindFirstFile(lpt, &fd);
               if( VH(hFind) )
               {
                  do
                  {
                     if( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
                     {
                        if( VERBM )
                        {
                     		*lpVerb = 0;
         	            	BuildDirString( lpVerb, &fd, 0, FALSE, TRUE, SZLEN );
                           prt( lpVerb );	// Out it, adding Cr/Lf
                        }
                        if( MatchXHibit( p, &fd.cFileName[0] ) )
                        {
                           // found IT - under slightly different name
                           strcpy( p, &fd.cFileName[0] ); // change the name
                           FindClose(hFind); // close out FIND - it is found
                           hFind = 0;  // signal a GOOD find
                           break;      // and exit the while
                        }
                     }
                  } while ( FindNextFile( hFind, &fd ) );

                  // end of SEARCH - was it successful?
                  if( hFind == 0 )
                  {
               		HANDLE	hdl;     // YES, some worthwhile effort
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
                  }
                  else
                  {
                     FindClose(hFind);
                  }
               }
            }
         }
      }
   }
   return flg;
}
#endif // 000000000000000000000000000000000000000000000000000000000

#if 0 // 00000000000000000000000000000000000000000000000000000000000000000000
int	BuildDirString( char * lpb,		// Destination buffer
				   LPWIN32_FIND_DATA pfd,	// Information
				   int iDepth,		// Indent depth
				   int	iHour24,	// 24-Hour time
				   int	fAddFull,	// Add FULL NAME
				   int	iSizLen );
int	BuildDirString( char * lpb,		// Destination buffer
				   LPWIN32_FIND_DATA pfd,	// Information
				   int iDepth,		// Indent depth
				   int	iHour24,	// 24-Hour time
				   int	fAddFull,	// Add FULL NAME
				   int	iSizLen )
{
	int		i, k;
	char	   c = 0;

	// Fill in DEPTH, if any
	// =====================
	if( iDepth )
	{
		if( (int)strlen( lpb ) < iDepth )
		{
			strcat( lpb, "+" );
			while( (int)strlen( lpb ) < iDepth )
				strcat( lpb, "+" );
		}
	}

	// VERY SPECIAL CASE
	if( pfd->dwFileAttributes == SPLFA )
	{
		sprintf( (lpb + strlen( lpb )),
			" Directory of %s",
			&pfd->cFileName[0] );
		return( strlen( lpb ) );
	}
	// Put in DOS TYPE Name, if any
	// ============================
	if( pfd->cAlternateFileName[0] )
	{
		k = strlen( lpb );
		for( i = 0; i < 8; i++ )
		{
			c = pfd->cAlternateFileName[i];
			if( ( c == '.' ) || ( c == 0 ) )
			{
				if( c == '.' )
				{
					if( k < 2 )
					{
						lpb[k++] = c;
					}
					else
					{
						i++;	// Bump past "."
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				lpb[k++] = c;
			}
		}
		if( c )
		{
			if( k < 8 )
			{
				// Fill with spaces
				while( k < 8 )
					lpb[k++] = ' ';
			}
			lpb[k++] = ' ';		// Dot is a SPACE
			for(  ; i < 14; i++ )
			{
				c = pfd->cAlternateFileName[i];
				if( c > ' ' )
				{
					if( c != '.' )	// DON'T put in the "."
						lpb[k++] = c;
				}
				else
				{
					// Is space or LESS
					break;	// End of NAME
				}
			}
		}
		lpb[k] = 0;
	}
	else
	{
		// NOTE NOTE NOTE
		// It SEEMS if the FindFirstFile/FindNextFile
		// Finds a FILE that is ALL CAPITAL, the name of
		// which it has placed in cFileName[], then
		// this cAlternateFileName[] can be BLANK!!!
		// so there has been a FIX in AddFile()
		// specifically for this BLANK CASE!!!
		strcat( lpb, "*NONE!!!* " );
	}
	while( (int)strlen( lpb ) < (MXDNAME+iDepth) )
	{
		strcat( lpb, " " );
	}

	// Add either <DIR> or SIZE
	// ========================
	if( pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		strcat( lpb, "  <DIR>       " );
	}
	else
	{
		//sprintf( (lpb + strlen( lpb )),
		//	"%13u ",
		//	pfd->nFileSizeLow );
//#ifdef	FA4DBG
		PutThous( (lpb + strlen( lpb )),
			iSizLen,
			pfd->nFileSizeLow );
//#else
//		sprintf( EndBuf(lpb), "%d", pfd->nFileSizeLow );
//#endif	/* FA4DBG y/n */
		strcat( lpb, " " );	// Space AFTER number
	}

	AddDateTime( lpb, pfd, iHour24 );

	// Add Full Name at end, if required
	// =================================
	if( fAddFull && 
		pfd->cFileName[0] )
	{
		sprintf( (lpb + strlen(lpb)),
			" %s",
			&pfd->cFileName[0] );
	}
	return( strlen( lpb ) );
}


#endif // 0000000000000000000000000000000

#if   0  // old code, to be removed sometime
		if(j)
		{
         // FIX20041205 - this appear WRONG with *.htm* par example
			if( ( j < i ) &&
				( (i - j) <= 3 ) )
			{
				k = 0;
				for( l = 0; l < j; l++ )
				{
					c = lpf[l];
					if( c == '.' )
					{
						l++;
						break;
					}
					lpb[k++] = c;
				}
				lpb[k] = 0;
				k = 0;
				for( ; l < i; l++ )
				{
					c = lpf[l];
					if( c != '.' )
						lpe[k++] = c;
				}
				lpe[k] = 0;
			}
			else
			{
				if( lpb )
					strcpy( lpb, lpf );
				if( lpe )
					*lpe = 0;
			}
		}
		else
		{
			if( lpb )
				strcpy( lpb, lpf );
			if( lpe )
				*lpe = 0;
		}
#endif // #if   0  // old code, to be removed sometime

// there seems no body for these
extern	HFILE GMOpen( char * lpf, uint32_t uStyle );
extern	HFILE GMClose( HFILE hFile );
extern	uint32_t	GMWrite( HFILE hFile, char * lpb, uint32_t len );
extern	uint32_t	GMRead( HFILE hFile, char * lpb, uint32_t len );
extern	uint32_t	GMMove( HFILE hFile, uint32_t Off, uint32_t type );
