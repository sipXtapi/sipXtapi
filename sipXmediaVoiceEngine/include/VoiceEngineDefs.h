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

#ifdef USE_GIPS
#include "GIPSVEBase.h"
#include "GIPSVECallReport.h"
#include "GIPSVECodec.h"
#include "GIPSVECommon.h"
#include "GIPSVEDTMF.h"
#include "GIPSVEEncryption.h"
#include "GIPSVEErrors.h"
#include "GIPSVEExternalMedia.h"
#include "GIPSVEFile.h"
#include "GIPSVEHardware.h"
#include "GIPSVENetEqStats.h"
#include "GIPSVENetwork.h"
#include "GIPSVEPTT.h"
#include "GIPSVERTP_RTCP.h"
#include "GIPSVEVideoSync.h"
#include "GIPSVEVolumeControl.h"
#include "GIPSVEVQE.h"
#include "GIPS_common_types.h"
#else
# include "include/stubbed_types.h"
# include "include/StubbedVoiceEngineLib.h"
#endif // USE_GIPS

#define GIPSVE_CodecInst GIPS_CodecInst

#  if defined(_WIN32)
#    include "GipsVideoEngine.h"
#    include <windows.h>
#    include "GipsVideoEngineWindows.h"
#    include "GIPS_common_video_types.h"
#  elif defined(__MACH__)
#    include "mac/GipsVideoEngine.h"
#    include <carbon/Carbon.h>
#    include "mac/GipsVideoEngineMac.h"
#    include "mac/GIPS_common_video_types.h"
#  else
#    error("Unknown platform") ;
#  endif

#  if defined(_WIN32)
typedef GipsVideoEngineWindows GipsVideoEnginePlatform ;
#  elif defined(__MACH__)
typedef GipsVideoEngineMac GipsVideoEnginePlatform;
#  else
#    error("Unknown platform") ;
#  endif

/** Optional Settings/tweaks/modes **/

#define ENCODE_GIPS_LOGS
#undef ENCODE_GIPS_LOGS           // Comment to use ENCODE_GIPS_LOGS

#define GIPS_LOG_FLUSH
#undef GIPS_LOG_FLUSH             // Comment flush logs after every GIPS calls

#define USE_GLOBAL_VOICE_ENGINE
#undef USE_GLOBAL_VOICE_ENGINE    // Comment to use USE_GLOBAL_VOICE_ENGINE

#define ENABLE_GIPS_VQMON
#undef ENABLE_GIPS_VQMON       // Comment to disable VQMon

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
#  define GIPS_EXPIRE_MONTH   0
#  define GIPS_EXPIRE_DAY     0
#  define GIPS_EXPIRE_YEAR    0
#  pragma comment( lib, "GIPSVideoEngineWindows_MT.lib" )

#endif // _VoiceEngineDefs_h
