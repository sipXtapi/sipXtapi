//  
// Copyright (C) 2017 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpTestCodecPaths_h_
#define _MpTestCodecPaths_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// CONSTANTS

// Setup codec paths..
static const UtlString sCodecPaths[] = {
#ifdef WIN32
                                  "bin",
                                  "..\\bin",
#elif __pingtel_on_posix__
                                  "../../../../bin",
                                  "../../../bin",
                                  "../../bin",
#  ifdef MP_CODECS_DIR
                                  MP_CODECS_DIR,
#  endif
#else
#                                 error "Unknown platform"
#endif
                                  "."
                                 };
static const size_t sNumCodecPaths = sizeof(sCodecPaths)/sizeof(sCodecPaths[0]);

// STRUCTS
// TYPEDEFS

#endif  // _MpTestCodecPaths_h_
