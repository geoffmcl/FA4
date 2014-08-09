/*\
 * Fa4CWin.h
 *
 * Copyright (c) 1987 - 2014 - Geoff R. McLane
 *
 * Licence: GNU GPL version 2
 * See LICENSE.txt in the source
 *
\*/
//////////////////////////////////////////////////////////////////////////
// Fa4CWin.h - 20140729
// Compatible Windows - Fill in things missing in NOT WIN32 systems
/////////////////////////////////////////////////////////////////////////
#ifndef _FA4CWIN_H_
#define _FA4CWIN_H_

#ifndef WIN32
///////////////////////////////////////////////////
// ONLY for NOT WIN32
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#if !defined(HANDLE_DECLARED)
#define HANDLE_DECLARED
typedef void * HANDLE;
typedef void * HFILE;
#endif // !HANDLE_DECLARED
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct _LIST_ENTRY;

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;


#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (HANDLE)-1
#endif

#ifndef HFILE_ERROR
#define HFILE_ERROR (HANDLE)-1
#endif
/////////////////////////////////////////////////////////////////////
#endif // !WIN32

/////////////////////////////////////////////
// COMMON - Items needed all the time

#ifndef MCDECL
#ifdef WIN32
#define MCDECL _cdecl
#else
#define MCDECL
#endif // WIN32
#endif // MCDECL

#endif // _FA4CWIN_H_
// eof
