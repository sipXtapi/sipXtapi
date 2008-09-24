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
#include "mi/CpMediaInterfaceFactoryImpl.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

class OsConfigDb; // forward declaration

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


// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif
