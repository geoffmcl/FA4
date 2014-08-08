# CMakeLists.txt for FA4 project - 20140727

cmake_minimum_required( VERSION 2.8.8 )
include (CheckIncludeFile)

project( FA4 )

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Set ON to build and use shared library"         OFF )
option( USE_DT4_MODULE   "Set ON to add DT4 modules to library"           OFF )

# to distinguish between debug and release lib
set( CMAKE_DEBUG_POSTFIX d )

if(CMAKE_COMPILER_IS_GNUCXX)
    set( WARNING_FLAGS -Wall )
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    #    set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
    # endforeach()
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
else()
    #   what to add for gcc? Maybe
    set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-unused-but-set-variable")
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )
#####################################################################
## configuration
check_include_file(dirent.h HAVE_DIRENT_H)

#####################################################################
## general utility library
#########################
set(name grmLib)
set(dir utils)
include_directories( ${dir} )
set( ${name}_SRCS
    ${dir}/grmLib.c
    ${dir}/grmOut.c
    ${dir}/gmUtil.c
    )
set( ${name}_HDRS
    ${dir}/grmLib.h
    ${dir}/grmOut.h
    ${dir}/gmFile.h
    ${dir}/gmUtils.h
    )
if (USE_DT4_MODULE)
    list(APPEND ${name}_SRCS ${dir}/gmgetDT4.c)
    add_definitions( -DUSE_DT4_MODULE )
endif ()    
if (HAVE_DIRENT_H)
    add_definitions( -DHAVE_DIRENT_H )
else ()
    list(APPEND ${name}_SRCS dirent/dirent.c)
    list(APPEND ${name}_HDRS dirent/dirent.h)
    include_directories( dirent )
endif ()
add_library( ${name} ${LIB_TYPE} ${${name}_SRCS} ${${name}_HDRS} )
set_target_properties( ${name} PROPERTIES COMPILE_FLAGS "-DADD_STGBUF=1" )
list(APPEND add_LIBS ${name})

#####################################################################
## pcre utility library
#########################
set(name pcre)
set(dir pcre)
include_directories( ${dir} )
set( ${name}_SRCS
    ${dir}/pcre_chartables.c
    ${dir}/pcre_compile.c
    ${dir}/pcre_config.c
    ${dir}/pcre_dfa_exec.c
    ${dir}/pcre_exec.c
    ${dir}/pcre_fullinfo.c
    ${dir}/pcre_get.c
    ${dir}/pcre_globals.c
    ${dir}/pcre_info.c
    ${dir}/pcre_maketables.c
    ${dir}/pcre_newline.c
    ${dir}/pcre_ord2utf8.c
    ${dir}/pcre_refcount.c
    ${dir}/pcre_study.c
    ${dir}/pcre_tables.c
    ${dir}/pcre_try_flipped.c
    ${dir}/pcre_ucp_searchfuncs.c
    ${dir}/pcre_valid_utf8.c
    ${dir}/pcre_version.c
    ${dir}/pcre_xclass.c
    ${dir}/pcreposix.c
    )
set( ${name}_HDRS
    ${dir}/config.h
    ${dir}/pcre.h
    ${dir}/pcrecpp.h
    ${dir}/pcreposix.h
    ${dir}/pcre_internal.h
    ${dir}/pcre_scanner.h
    ${dir}/ucp.h
    ${dir}/ucpinternal.h    
    )
add_library( ${name} ${LIB_TYPE} ${${name}_SRCS} ${${name}_HDRS} )
list(APPEND add_LIBS ${name})

set(name fa4)
set(dir src)
set( ${name}_SRCS
    ${dir}/Fa4.c
    ${dir}/Fa4Comp.c
    ${dir}/Fa4Con.c
    ${dir}/Fa4Find.c
    ${dir}/Fa4Help.c
    ${dir}/Fa4Out.c
    ${dir}/Fa4Show.c
    ${dir}/Fa4Util.c
    ${dir}/Fa4Wild.c
    ${dir}/Pfile.c
    )
set( ${name}_HDRS
    ${dir}/Fa4.h
    ${dir}/Fa4CWin.h
    ${dir}/Fa4Con.h
    ${dir}/Fa4Help.h
    ${dir}/Fa4List.h
    ${dir}/Fa4Out.h
    ${dir}/Fa4Util.h
    ${dir}/Fa4Vers.h
    ${dir}/Fa4Wild.h
    ${dir}/Fa4Work.h
    ${dir}/FastComp.h
    )
if (WIN32)
    list(APPEND ${name}_SRCS ${dir}/FA4.rc ${dir}/resource.h ${dir}/winerror.c ${dir}/WinErr.h )
endif ()
add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
target_link_libraries( ${name} ${add_LIBS} )
if (MSVC)
    set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
    install( TARGETS ${name} DESTINATION C:/MDOS )
else ()
    install( TARGETS ${name} DESTINATION bin )
endif ()

# eof
