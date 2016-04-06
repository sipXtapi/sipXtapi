//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _OsIntTypes_h_
#define _OsIntTypes_h_

/** @file
*  @brief Include this file if you want use C99 integer types with specified
*  width and corresponding set of macros.
*
*  This file is just a dispatcher, including one or other implementation
*  of C99 <stdint.h> and <inttypes.h>. It is created to simplify porting
*  to different platforms and compilers, some of them have no C99 integer
*  types implemented. 
*/

// Define these macros to include support for minimum/maximum constants
// and constant macros, if they have not already been defined on the compiler
// command line by the build system.
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifdef _MSC_VER
#if (_MSC_VER < 1900)
// Microsoft Visual Studio C/C++ compiler
#  include <os/msinttypes/inttypes.h>
#else
#  include <limits.h>
#  include <inttypes.h>
#endif

#elif __GNUC__
// GNU C/C++ compiler
#  include <limits.h>
#  include <inttypes.h>

#else
#  error Unknown compiler. Check does your compiler support C99 "inttypes.h" and edit this file.
#endif

/* Test to see whether this file got included soon enough - if the __STDC macros
 * weren't defined on the compiler command line, then we have to include this
 * file before anything else includes inttypes.h or stdint.h for the __STDC
 * macros to have any effect. Use INT16_MAX as a simple check for success. */
#ifndef INT16_MAX
#warning #include os/OsIntTypes.h before stdint.h/inttypes.h
#define _STDINT_H help_find_first_include_of_stdint.h
#define _STDINT_H_ help_find_first_include_of_stdint.h
#endif

#endif  // _OsIntTypes_h_
