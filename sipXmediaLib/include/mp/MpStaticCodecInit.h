//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpStaticCodecInit_h_
#define _MpStaticCodecInit_h_

#ifdef __pingtel_on_posix__  /* [ */
#  include <config.h>
#else  /* ] [ */
// Set interesting codec to be compiled statically


#endif /* __pingtel_on_posix__ ] */

void mpStaticCodecInitializer(void);

#endif
