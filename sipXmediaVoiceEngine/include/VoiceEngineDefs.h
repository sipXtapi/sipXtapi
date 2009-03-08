// Copyright 2007,2008 AOL LLC.
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
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#ifndef _VoiceEngineDefs_h
#define _VoiceEngineDefs_h

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#ifdef USE_GIPS
# if defined(_WIN32)
#   include "GIPS_common_types.h"
#   include "GipsVoiceEngineLib.h"
# elif defined (__MACH__)
#   include "mac/GipsVoiceEngineLib.h"
# else
#   include "GipsVoiceEngineLibLinux.h"
# endif
#else
# include "include/stubbed_types.h"
# include "include/StubbedVoiceEngineLib.h"
#endif // USE_GIPS

#define GIPSVE_CodecInst GIPS_CodecInst

# if defined (USE_GIPS)
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

#if defined (USE_GIPS)
#  if defined(_WIN32)
typedef GipsVideoEngineWindows GipsVideoEnginePlatform ;
#  elif defined(__MACH__)
typedef GipsVideoEngineMac GipsVideoEnginePlatform;
#  else
#    error("Unknown platform") ;
#  endif
#elif 
typedef GipsVideoEngineStub GipsVideoEnginePlatform ;
#else
typedef void* GipsVideoEnginePlatform ;
#endif

/** Optional Settings/tweaks/modes **/

#define ENCODE_GIPS_LOGS
#undef ENCODE_GIPS_LOGS           // Comment to use ENCODE_GIPS_LOGS

#define GIPS_LOG_FLUSH
#undef GIPS_LOG_FLUSH             // Comment flush logs after every GIPS calls

#define USE_GLOBAL_VOICE_ENGINE
#undef USE_GLOBAL_VOICE_ENGINE    // Comment to use USE_GLOBAL_VOICE_ENGINE

#ifdef _WIN32
#  define ENABLE_GIPS_VQMON
//#undef ENABLE_GIPS_VQMON       // Comment to enable VQMon
#else
#  undef ENABLE_GIPS_VQMON
#endif

#define USE_GIPS_DLL
#undef USE_GIPS_DLL            // Comment to use the DLL version of GIPS


// Support resolutions
#define INCLUDE_SQCIF_RESOLUTION
#define INCLUDE_QCIF_RESOLUTION
#define INCLUDE_QVGA_RESOLUTION
#define INCLUDE_CIF_RESOLUTION
#define INCLUDE_VGA_RESOLUTION
//#define INCLUDE_4CIF_RESOLUTION
//#define INCLUDE_16CIF_RESOLUTION

#if defined(USE_GIPS) && defined(_WIN32)
# ifdef USE_GIPS_DLL
#  define GIPS_KEY            "--INSERTKEYHERE--"
#  define GIPS_EXPIRE_MONTH   0
#  define GIPS_EXPIRE_DAY     0
#  define GIPS_EXPIRE_YEAR    0
#  pragma comment( lib, "GipsVideoEngineWindowsDLL_MT.lib" )
#  pragma comment( lib, "GIPSVoiceEngineDLL.lib" )
# else
#  undef GIPS_KEY
#  define GIPS_EXPIRE_MONTH   0
#  define GIPS_EXPIRE_DAY     0
#  define GIPS_EXPIRE_YEAR    0
#  pragma comment( lib, "GIPSVoiceEngineMultimediaWindows_MT.lib" )
# endif
#else
#  undef GIPS_KEY
#  define GIPS_EXPIRE_MONTH   0
#  define GIPS_EXPIRE_DAY     0
#  define GIPS_EXPIRE_YEAR    0
#endif // USE_GIPS && _WIN32


#endif // _VoiceEngineDefs_h
