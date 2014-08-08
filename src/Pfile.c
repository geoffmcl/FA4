

#include	"Fa4.h"		// All incusive include

//#define DEF_IN_FILE     "winerror.c"
//#define DEF_IN_FILE     "d:\\work\\t3\\winerror.c"
#define DEF_IN_FILE     "c:\\msdev\\include\\winerror.h"
#define PEOR        "\n"
#define MXINBUF         4096
#define MXOUTBUF         4096

extern  int	OpenReadFile( char * lpf, HANDLE * ph );
extern  HANDLE	CreateAFile( char * lpf );

#define CreateTTYFile       CreateAFile

void	chkmiss( char * lpt )
{
	if( ( lpt ) &&
		( *lpt ) )
		printf(lpt);
}

HANDLE  OpenTTYFile( char * lpf )
{
    HANDLE  h1, h2;
    h2 = 0;
    if( ( OpenReadFile( lpf, &h1 ) ) &&
        ( h1 ) &&
        ( h1 != (HANDLE)-1 ) )
        h2 = h1;
    return h2;
}

#define		MXERRBUF		256
#define		MXTXTBUF		1024

char    ibuf[MXINBUF+32];
char    obuf[MXOUTBUF+32];
char    ebuf[MXERRBUF+32];
char    lbuf[1024];
char	tbuf[MXTXTBUF+32];

int tty_cmpstg4n( char * p1, char * p2, int j )
{
   int   i, k;
   k = 0;
   for( i = 0; i < j; i++ )
   {
      if( p1[i] != p2[i] )
         break;
   }
   if( i == j )
      k = j;
   return k;
}
int tty_cmpstg4ni( char * p1, char * p2, int j )
{
   int   i, k;
   k = 0;
   for( i = 0; i < j; i++ )
   {
      if( toupper(p1[i]) != toupper(p2[i]) )
         break;
   }
   if( i == j )
      k = j;
   return k;
}


int   tty_bgnstgi( char * pStg, char * pCmp )
{
   int i = 0;
   int      j, k, l;

   j = k = l = 0;
   if( (  pStg ) &&
      ( pCmp ) )
   {
      j = strlen(pStg);
      k = strlen(pCmp);
   }
   if( ( j && k ) &&
      ( k <= j ) )
   {
      l = tty_cmpstg4ni( pStg, pCmp, k ); 
      if(l)
      {
         i = l;
      }
   }
   return i;
}

int   tty_bgnstg( char * pStg, char * pCmp )
{
   int i = 0;
   int      j, k, l;

   j = k = l = 0;
   if( ( pStg ) &&
      ( pCmp ) )
   {
      j = strlen(pStg);
      k = strlen(pCmp);
   }
   if( ( j && k ) &&
      ( k <= j ) )
   {
      l = tty_cmpstg4n( pStg, pCmp, k ); 
      if(l)
      {
         i = l;
      }
   }
   return i;
}


#define		FNDMIN		48
#define		subrd(a) \
	if( rd >  a ) \
		rd -= a;  \
	else		  \
		rd = 0


int	condtbuf( char * lpt )
{
	int	i, j, k;
	char	c, d;

	j = k = 0;
	if( lpt )
      j = strlen( lpt );
   if(j)
	{
		i = j - 1;
		// 1 - remove trailing spacey stuff
		while( i )
		{
			if( lpt[i] > ' ' )
				break;
			lpt[i] = 0;
			if( j )
				j--;
			i--;
		}
		if( j )
		{
			k = 0;
			d = ' ';
			for( i = 0; i < j; i++ )
			{
				c = lpt[i];
				if( c <= ' ' )
				{
					if( d > ' ' )
					{
						lpt[k++] = ' ';
					}
				}
				else
				{
					lpt[k++] = c;
				}
				d = c;
			}
		}
	}
	return k;
}


void    pfile2( void )
{
    HANDLE  h1, h2;
    uint32_t   rd, dwi, dwr, dwg, dwl, dwf;
    char    c;
    char *  cp1;
	char *	cp2;
	uint32_t	dwe, dwt, dwo, wtn;
	char *lpe, *lpt, *lpo;

    h1 = h2 = 0;
	dwl = dwf = 0;
	lpe = &ebuf[0];
	lpt = &tbuf[0];
	lpo = &obuf[0];
   h1 = OpenTTYFile( DEF_IN_FILE );
   h2 = CreateTTYFile( "tempw.c" );
   if( ( VH(h1) ) &&
       ( VH(h2) ) )
   {
      ReadFile( h1, &ibuf[0], MXINBUF, &rd, NULL );
		dwi = 0;
        while( rd > FNDMIN )
        {
			c   = ibuf[dwi];
			cp1 = &ibuf[dwi];
			rd--;
			dwi++;
			if( c == '\n' )
			{
				dwl++;
				memcpy( &ibuf[0], &ibuf[dwi], rd );
				dwr = MXINBUF - rd;
				dwg = 0;
				if( VH(h1) )
				{
					ReadFile( h1, &ibuf[rd], dwr, &dwg, NULL );
					if( dwg < dwr )
					{
						CloseHandle(h1);
						h1 = 0;
					}
				}
				else
				{
					memset( &ibuf[rd], 0, dwr );
				}
				dwi = 0;
				rd  += dwg;
				if( rd < FNDMIN )
					break;
				c   = ibuf[dwi];
				cp1 = &ibuf[dwi];
				rd--;
				dwi++;
			}
//			if( tty_bgnstgi( cp1, "MessageId:" ) )
//				dwf++;

			if( ( tty_bgnstgi( cp1, "MessageId:" ) ) &&
				( cp1[10] <= ' ' ) &&
				( cp1[11] >  ' ' ) )
			{
				// got first piece of puzzle
				cp1 += 11;
				dwi += 11;
				rd  -= 11;
				dwe = 0;
				while( ( (c = cp1[dwe]) > ' ' ) &&
					( dwe < MXERRBUF ) &&
					( rd ) )
				{
					ebuf[dwe++] = c;
					dwi++;
					rd--;
				}
				ebuf[dwe] = 0;
				while( rd > dwe )
				{
					c   = ibuf[dwi];
					cp2 = &ibuf[dwi];
					if( c == '\n' )
						dwl++;
					if( c == '#' )
					{
						chkmiss( "Yuk! Never found MessageText:" );
						break;
					}
					if( tty_bgnstgi( cp2, "MessageText:" ) )
					{
						dwi += 12;
						//rd  -= 12;
						subrd(12);
						cp2 = &ibuf[dwi];
						while( ( ( (c = *cp2) <= ' ' ) || ( c == '/' ) ) &&
							( rd > dwe ) )
						{
							if( c == '#' )
								break;
							if( c == '\n' )
								dwl++;
							cp2++;
							dwi++;
							rd-- ;
						}
						if( ( c > ' ' ) &&
							( c != '#' ) )
						{
							dwt = 0;
							while( ( dwt < MXTXTBUF ) &&
								( ( c = *cp2 ) != 0  ) &&
								( rd > dwe           ) )
							{
								if( c == '#' )
								{
									break;
								}
								else if( ( c < ' ' ) || ( c == '/' ) )
								{
									// skip these
								}
								else
								{
									tbuf[dwt++] = c;
								}
								cp2++;
								dwi++;
								rd --;
							}
							if( c == '#' )
							{
								tbuf[dwt] = 0;
								if( condtbuf( lpt ) )
								{
									strcpy( lpo, "\t{ " );
									strcat( lpo, lpe );
									strcat( lpo, ", \"" );
									strcat( lpo, lpt );
									strcat( lpo, "\", \"" );
									strcat( lpo, lpe );
									strcat( lpo, "\" },\r\n" );
									dwo = strlen(lpo); 
									if(dwo)
									{
					                    WriteFile( h2, lpo, dwo, &wtn, NULL );
									}
									else
									{
										chkmiss( "tbuf len is ZERO!" );
									}
								}
								else
								{
									chkmiss( "condtbuf returned 0!" );
								}
							}
							else
							{
								chkmiss( "No #define!!!" );
							}
						}
						else
						{
							chkmiss( "Yeek! Appear missing text!" );
						}
						cp2 = &ibuf[dwi];
						while( ( ( c = *cp2 ) != 0 ) &&
							( c != '\n' ) &&
							( rd ) )
						{
							cp2++;
							dwi++;
							rd --;
						}
						break;
					}	// if we find MessageText:
					dwi++;
					rd --;
				}	// while( rd > dwe )
			}
			else if( tty_bgnstgi( cp1, "MessageId:" ) )
			{
				chkmiss( "Huh? We have MessageId: but no space and text!" );
			}
		}	// while data greater than minimum
    }
    if( VH(h1) )
        CloseHandle(h1);
    if( VH(h2) )
        CloseHandle(h2);

}

void    pfile( void )
{
    HANDLE  h1, h2;
    uint32_t   rd, dwi, dwk, wtn, dwd, dwe;
    char    c;
    char *  cp1;
    char *  cp2;
    uint32_t   dwr, dwg;

    h1 = h2 = 0;
    rd = 0;
    h1 = OpenTTYFile( DEF_IN_FILE );
    if( VH(h1) )
       h2 = CreateTTYFile( "tempw.c" );
    if( VH(h2) )
    {
        dwk = 0;
        dwi = (uint32_t)-1;
        while( ( ReadFile( h1, &ibuf[0], 4096, &rd, NULL ) ) &&
            ( rd ) )
        {
            for( dwi = 0; dwi < rd; dwi++ )
            {
                c = ibuf[dwi];
                if( ( c == 0x0a ) &&
                    ( ibuf[dwi+1] == '}' ) &&
                    ( ibuf[dwi+2] == ';' ) )
                {
                    break;
                }
                obuf[dwk++] = c;
                if( dwk == 4096 )
                {
                    WriteFile( h2, &obuf[0], 4096, &wtn, NULL );
                    dwk = 0;
                }
            }
            if( dwi < rd )
                break;
        }
        if( dwi < rd )
        {
            cp2 = &ibuf[dwi];
            while( dwi < rd )
            {
                if( dwk )
                {
                    WriteFile( h2, &obuf[0], dwk, &wtn, NULL );
                    dwk = 0;
                }
                obuf[dwk++] = 0x09;
                obuf[dwk++] = '{';
                obuf[dwk++] = ' ';
                dwd = rd - dwi;
                cp1 = &ibuf[dwi];
                printf( "Copying bal %d from %d to beginning ..."PEOR,
                    dwd, dwi );
                memcpy( &ibuf[0], &ibuf[dwi], dwd );
                dwr = 4096 - dwd;
                dwg = 0;
                if( VH(h1) )
                {
                    printf( "Reading in %d to %d to end ...(%d)"PEOR,
                        dwr, dwd, (dwd+dwr) );
                    ReadFile( h1, &ibuf[dwd], dwr, &dwg, NULL );
                    if( dwg < dwr )
                    {
                        CloseHandle(h1);
                        h1 = 0;
                    }
                }
                dwi = 0;
                rd  = dwd + dwg;
                for( dwi = 0; dwi < rd; dwi++ )
                {
                    c = ibuf[dwi];
                    cp1 = &ibuf[dwi];
//                    if( tty_bgnstg( cp1, "Id: ERROR" ) )
                    if( tty_bgnstg( cp1, "MessageId: " ) )
                    {
                        //dwi += 4;
                        dwi += 11;
                        cp1 = &ibuf[dwi];
                        dwe = 0;
                        while( cp1[dwe] > ' ' )
                        {
                            obuf[dwk++] = cp1[dwe];
                            ebuf[dwe++] = cp1[dwe];
                            if( dwk >= 4096 )
                            {
                                WriteFile( h2, &obuf[0], dwk, &wtn, NULL );
                                dwk = 0;
                            }
                            //cp1++;
                            dwi++;
                            if(dwe > 256)
                                break;
                        }
                        ebuf[dwe] = 0;  // terminate ERROR NAME
                        break;
                    }
                }
                while( dwi < rd )
                {
                    cp2 = &ibuf[dwi];
                    if( tty_bgnstg( cp2, "Text:" ) )
                    {
                        dwi += 5;
                        cp2 = &ibuf[dwi];
                        while( ( ( *cp2 <= ' ' ) || ( *cp2 == '/' ) ) &&
                            ( dwi < rd ) )
                        {
                            cp2++;
                            dwi++;
                        }
                        break;
                    }
                    dwi++;
                }
                if( dwi < rd )
                {
                    obuf[dwk++] = ',';
                    obuf[dwk++] = ' ';
                    obuf[dwk++] = 0x22;
                    if( dwk >= 4096 )
                    {
                        WriteFile( h2, &obuf[0], dwk, &wtn, NULL );
                        dwk = 0;
                    }
                }
                while( dwi < rd )
                {
                    c = *cp2;
                    if( c == '#' )
                    {
                        break;
                    }
                    else if( c < ' ' )
                    {
                        while( ( ( c = *cp2 ) != 0 ) &&
                            ( ( c <= ' ' ) || ( c == '/' ) ) )
                        {
                            cp2++;
                            dwi++;
                        }
                    }
                    else
                    {
                        obuf[dwk++] = c;
                        if( dwk >= 4096 )
                        {
                            WriteFile( h2, &obuf[0], dwk, &wtn, NULL );
                            dwk = 0;
                        }
                    }
                    if( c == '#' )
                    {
                        break;
                    }
                    cp2++;
                    dwi++;
                }
//                cp1 = &ibuf[dwi];

                // end of text
                obuf[dwk++] = 0x22;
                obuf[dwk++] = ' ';
                obuf[dwk++] = '}';
                obuf[dwk++] = ',';
                obuf[dwk++] = 0x0d;
                obuf[dwk++] = 0x0a;

            }
            if( dwk )
            {
                WriteFile( h2, &obuf[0], dwk, &wtn, NULL );
                dwk = 0;
            }
        }

    }
    if( VH(h1) )
        CloseHandle(h1);
    if( VH(h2) )
        CloseHandle(h2);

}

