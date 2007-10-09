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
// and constant macros.
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#ifdef _MSC_VER
// Microsoft Visual Studio C/C++ compiler
#  include <os/msinttypes/inttypes.h>

#elif __GNUC__
// GNU C/C++ compiler
#  include <inttypes.h>

#else
#  error Unknown compiler. Check does your compiler support C99 "inttypes.h" and edit this file.
#endif

#endif  // _OsIntTypes_h_
