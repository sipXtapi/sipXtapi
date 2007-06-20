// 
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef QSTYPES__H__
#define QSTYPES__H__

/*-----------------------------------------------------------------------------
 * Module history
 *
 * Date			Description
 * ----			-----------
 * 21 Jun 07      First attempt to remove this file and switch to C99 integer types.
 * 26 Feb 05		Initial version of the file.
 *-----------------------------------------------------------------------------
 */

//
//                       !!!!!!!! ATTENTION !!!!!!!!
//
// Please, do not use these UInt64 and Int64. Use uint64_t and int64_t instead.
// And, please, replace all existing references to UInt64 and Int64 with
// uint64_t and int64_t.


#include <os/OsIntTypes.h>

/* 64 bit integers */
typedef uint64_t UInt64;
typedef int64_t  Int64;

#endif /* QSTYPES__H__ */
