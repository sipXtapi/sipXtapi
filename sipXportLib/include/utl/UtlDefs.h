// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
// Copyright (C) 2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlDefs_h_
#define _UtlDefs_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
#ifndef FALSE
#define FALSE ((UtlBoolean)(1==0))
#endif

#ifndef TRUE
#define TRUE ((UtlBoolean)(1==1))
#endif

#ifndef NULL
#define NULL 0
#endif

#define UTL_NOT_FOUND ((size_t)-1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef int UtlBoolean ;

typedef long long int intll;

/** FORMAT_INTLL is a string containing the format length specifier
 *  for printing an intll with the 'd', 'x', etc. format specifiers.  E.g.:
 *      intll xyz;
 *      printf("The value is %" FORMAT_INTLL "d", xyz);
 *  Note that the '%' before and the format specifier after must be provided.
 *  This must be a #define, since this specifier isn't standardized.
 */
#if defined(_WIN32)
#  define   FORMAT_INTLL   "I64"
#elif defined(__pingtel_on_posix__)
#  define   FORMAT_INTLL   "ll"
#else
#  error Unsupported target platform.
#endif

typedef const char* const UtlContainableType ;

// FORWARD DECLARATIONS

#endif // _UtlDefs_h_

