/*\
 * Fa4Out.c
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * save output in double inked list...
 * printf out list...
 *
\*/
#ifndef _Fa4Out_h
#define _Fa4Out_h

extern void  add2outlist( char * lpo );
extern void prtoutlist( void ); // ouputs to prt(),
extern void killoutlist( void ); // kills list

#endif // _Fa4Out_h
// eof
