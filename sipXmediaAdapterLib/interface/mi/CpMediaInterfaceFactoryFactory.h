//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaInterfaceFactoryFactory_h_
#define _CpMediaInterfaceFactoryFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "CpMediaInterfaceFactory.h"
#include "CpMediaInterfaceFactoryImpl.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

class OsConfigDb; // forward declaration

/**
 * Well known function that is called to create a CpMediaInterfaceFactory object,
 * which owns a CpMediaInterfaceFactoryImpl object, which this function also
 * creates.
 * This function must be implemented for all "plug-in" static sipx media processing
 * libraries. 
 */
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb, 
                                                            uint32_t fameSizeMs = 0, 
                                                            uint32_t maxSamplesPerSec = 0,
                                                            uint32_t defaultSamplesPerSec = 0);
  /**<
  *  @param pConfigDb - a configuration database to pass user-settable config
  *         parameters to sipXmediaLib.  TODO: Someone that knows more, document this better!
  *  @param frameSizeMs - This parameter is used for determining the 
  *         frame size (in milliseconds) that the media subsystem will use.
  *         It is used for initializing the size of audio buffers, and for 
  *         configuring a default value for samples per frame in device 
  *         managers (so that when devices are enabled without specifying 
  *         samples per frame, the value configured here will be used).
  *  @param maxSamplesPerSec - This is used for initializing audio buffers.
  *         Lower sample rates can indeed be used for individual media 
  *         interfaces (set later), since a lesser amount of these buffers 
  *         can be used (i.e. not fully utilized).  Higher sample rates can 
  *         be used by passing params here, but this will result in more 
  *         memory being used.  For low-memory environments that do not 
  *         require wideband support, one may wish to pass 8000kHz here, as 
  *         the default is 16000kHz.
  *  @param defaultSamplesPerSec - The sample rate that device managers and 
  *         flowgraphs will use when no sample rate is specified.
  */

/**
 * Destroy the singleton media factory
 */
extern "C" void sipxDestroyMediaFactoryFactory() ;

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif
