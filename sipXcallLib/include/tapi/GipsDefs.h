//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _GIPSDEFS_H
#define _GIPSDEFS_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#ifdef VOICE_ENGINE

#if defined(_WIN32)
#   include "GipsVoiceEngineLib.h"
#   include "GIPSAECTuningWizardAPI.h"             
#elif defined (__MACH__)
#   include "mac/GipsVoiceEngineLib.h"
#else
#   include "GipsVoiceEngineLibLinux.h"
#endif

#ifdef VIDEO
#  if defined(_WIN32)
#    include "GipsVideoEngine.h"
#    include <windows.h>
#    include "GipsVideoEngineWindows.h"
#  elif defined(__MACH__)
#    include "mac/GipsVideoEngine.h"
#    include <carbon/Carbon.h>
#    include "mac/GipsVideoEngineMac.h"
#  else
#    error("Unknown platform") ;
#  endif
#endif

#ifdef VIDEO
#  if defined(_WIN32)
typedef GipsVideoEngineWindows GipsVideoEnginePlatform ;
#  elif defined(__MACH__)
typedef GipsVideoEngineMac GipsVideoEnginePlatform;
#  else
#    error("Unknown platform") ;
#  endif
#endif

#else
typedef void* GipsVideoEnginePlatform;
#endif

#endif // _GIPSDEFS_H
