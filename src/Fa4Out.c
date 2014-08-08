
// Fa4Out.c

#include "Fa4.h"

void  add2list( PLE ph, char * lpo ) // like PLE ph = &g_sOutList;
{
   PLE      pn;
   char *   lpf;
   uint32_t    dwl = strlen( lpo );

   if( dwl == 0 )
      return; // quietly forget adding nothing!!!

   pn  = (PLE)MALLOC( LPTR, (sizeof(LIST_ENTRY) + dwl + 1) );

   if(!pn) { prt( "ERROR: Memory allocation FAILED!"MEOR ); exit(0); }

   lpf = (char *)pn;
   lpf += sizeof(LIST_ENTRY);

   strcpy(lpf, lpo); // get the data

   InsertTailList(ph,pn); // add to list

}

void  add2outlist( char * lpo )
{
   PLE ph = &g_sOutList;
   add2list( ph, lpo );
}

void killoutlist( void )
{
   PLE ph = &g_sOutList;
   KillLList(ph);
}

void prtoutlist( void )
{
   PLE ph = &g_sOutList;
   PLE pn;
   Traverse_List( ph, pn ) {
      char * pd = (char *)pn;
      pd += sizeof(LIST_ENTRY);
      prt(pd);
   }
}

// eof - Fa4Out.c
