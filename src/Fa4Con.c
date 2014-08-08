
// Fa4Con.c
#undef		UWEACC1		// special version

#include	"Fa4.h"		// All incusive include
extern	void	DisableDiagFile( void );
extern	void	EnableDiagFile( void );
extern	int		Fa4_main( int argc, char **argp );
//extern	void	Pgm_Exit( WS );	// ALL should come here to exit
extern	int		iCritErr;	// Program EXIT value
extern	int		iMainRet;

int	iRetVal = (int)-1;
char *g_ProgramName = (char *)"dummy";

jmp_buf mark;		// Address for long jump to jump to

// this is the 2nd main( ... ) CONSOLE OS entry.
int JMP_main( int argc, char **argp )
{
	int		iret;
	int		jmpret;

	jmpret = 0;
	jmpret = setjmp( mark );
	if( jmpret == 0 )
	{
		Fa4_main( argc, argp );
		Pgm_Exit( lpWS );	// Do the CLEANUP
	}
	if( iCritErr )
		iret = iCritErr;
	else
		iret = iMainRet;

	return iret;	// the ONLY return to the OS

}

#ifdef	UWEACC1

#define	MXDIAGBUF		1024
char	g_szDiag[MXDIAGBUF+32];
char	g_szDiag2[MXDIAGBUF+32];
int	g_iDiag2 = 0;

// Show Registry entry for EXCELL
// HKU
#define	DEF_REG1	 "Software\\Microsoft\\Windows\\CurrentVersion\\Extensions"
char	g_szReg1[] = DEF_REG1;

void	ShowReg( void )
{
	HKEY		hKey = 0;
	LONG		lg;
	uint32_t		dwType;
	char *	lpd = &g_szDiag[0];
	uint32_t		dws = MXDIAGBUF;
	uint32_t		dwi;
	char *	lpb;

	lg = RegOpenKey( HKEY_CURRENT_USER,
		&g_szReg1[0],
		&hKey );
	if( ( lg == ERROR_SUCCESS ) &&
		( hKey ) &&
		( hKey != (HKEY)-1 ) )
	{
		lg = RegQueryValueEx(
			hKey,    // handle to key
			"xls",	// value name
			0,			// reserved
			&dwType,	// type buffer
			lpd,	// data buffer
			&dws );	// size of data buffer
		if( ( lg == ERROR_SUCCESS ) &&
			( dwType == REG_SZ     ) &&
			( dwi = strlen(lpd)   ) )
		{
			if( g_iDiag2 )
			{
				// maybe already here
				int	i;
				lpb = &g_szDiag2[0];
				while( i = strlen(lpb) )
				{
					if( strcmpi(lpb,lpd) == 0 )
					{
						//*lpd = 0;
						dwi = 0;
						break;
					}
					lpb += (i + 1);
				}
			}

			printf("\n");
			printf(lpd);
			if( dwi )
			{
				// ==================================
				strcpy( &g_szDiag2[g_iDiag2], lpd );
				g_iDiag2 += (dwi + 1);
				g_szDiag2[g_iDiag2] = 0;
				// ==================================
				printf( " (Added)" );
			}
		}
		RegCloseKey(hKey);
	}
}


// Get Environment
void	ShowEnv( void )
{
	uint32_t	dwi, dwj, dwk;
	char *	lpd = &g_szDiag[0];
	char *	lpb;
	char		c;
	WIN32_FIND_DATA	fd;
	HANDLE	hFind;

	dwi = GetEnvironmentVariable(
		"PATH",	// environment variable name
		lpd,		// buffer for variable value
		MXDIAGBUF );	// size of buffer
	if( dwi )
	{
		printf(lpd);
		lpb = &lpd[dwi+8];
		dwk = 0;
		for( dwj = 0; dwj < dwi; dwj++ )
		{
			c = lpd[dwj];
			if( c == ';' )
			{
				if( dwk )
				{
					lpb[dwk] = 0;
					if( lpb[dwk-1] != '\\' )
						strcat(lpb,"\\");
					dwk = strlen(lpb);
					strcat(lpb,"EXCEL*.*");
					if( ( hFind = FindFirstFile(lpb,&fd) ) &&
						( hFind != INVALID_HANDLE_VALUE ) ) //(HANDLE)-1 ) )
					{
						if( fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )
						{
							printf("\n");
							printf(lpb);
						}
						FindClose(hFind);
					}
				}
				dwk = 0;
			}
			else
			{
				if( c != '"' )
					lpb[dwk++] = c;
			}
		}
	}
	else
	{
		printf("What FAILED!!!!");
	}
	// and in the REGISTRY
}
int main( int argc, char **argp )
{
	int	iret = 0;
	ShowEnv();
	ShowReg();
	return iret;
}

#else	/* !UWEACC1 */


// ONLY FOR CONSOLE APPLICATION
int main( int argc, char **argp )
{
	int	iret;
    g_ProgramName = argp[0];
//    pfile2();
//    outvals();
#ifdef	NDEBUG
	DisableDiagFile();
#endif	/* NDEBUG = NO DEBUG */
	iret = JMP_main( argc, argp );
	return iRetVal;
//	return iret;
}

#endif	/* UWEACC1 */

// eof - Fa4Con.c
