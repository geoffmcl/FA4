

// Fa4Help.h
#ifndef	_Fa4Help_H
#define	_Fa4Help_H

extern void	Usage( WS, uint32_t Err );

extern   int	ProcessArgs( LPWORKSTR pWS, int argc, char **argp, uint32_t level );

typedef enum {   // date styles
   od_dateup = 1,
   od_datedown = 2,
   od_sizeup = 3,
   od_sizedown = 4
}OrderStyle;

#endif	/* _Fa4Help_H */
// eof - Fa4Help.h