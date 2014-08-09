/*\
 * Fa4Con.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
 * console OS entry ...
 *
\*/
#ifndef	_Fa4Con_H
#define	_Fa4Con_H

extern	jmp_buf mark;		// Address for long jump to jump to
// this is the 2nd main( ... ) CONSOLE OS entry.
extern
int JMP_main( int argc, char **argp )


// ONLY FOR CONSOLE APPLICATION
// int main( int argc, char **argp )

#endif	/* _Fa4Con_H */
// eof - Fa4Con.h

