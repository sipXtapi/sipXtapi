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

#ifdef VOICE_ENGINE

#if defined(USE_GIPS)
#if defined(_WIN32)
#   include "GipsVoiceEngineLib.h"
#   include "GIPSAECTuningWizardAPI.h"             
#elif defined (__MACH__)
#   include "mac/GipsVoiceEngineLib.h"
#else
#   include "GipsVoiceEngineLibLinux.h"
#endif
#else
# include "include/stubbed_types.h"
# include "include/StubbedVoiceEngineLib.h"
#endif
#endif

#ifdef VIDEO
# if defined(USE_GIPS)
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
# else
#  include "include/StubbedVideoEngine.h"
# endif
#endif

#if defined (VIDEO) && defined (USE_GIPS)
#  if defined(_WIN32)
typedef GipsVideoEngineWindows GipsVideoEnginePlatform ;
#  elif defined(__MACH__)
typedef GipsVideoEngineMac GipsVideoEnginePlatform;
#  else
#    error("Unknown platform") ;
#  endif
#elif defined (VIDEO)
typedef GipsVideoEngineStub GipsVideoEnginePlatform ;
#else
typedef void* GipsVideoEnginePlatform;
#endif

#endif // _GIPSDEFS_H
