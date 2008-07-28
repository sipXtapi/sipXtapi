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

#ifdef MP_STREAMING

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"
#include "mp/StreamDataSourceListener.h"
#include "mp/StreamDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamDataSource::StreamDataSource(int iFlags /* = 0 */)
   : mpListener(NULL)
{
   miFlags = iFlags ;
}


// Destructor
StreamDataSource::~StreamDataSource()
{
}

/* ============================ MANIPULATORS ============================== */


// Interrupts any time consuming operation.
OsStatus StreamDataSource::interrupt()
{
    // Default implemenation does nothing.
    return OS_NOT_SUPPORTED ;
}
     

// Sets a listener to receive StreamDataSourceEvent events for this data 
// source.
void StreamDataSource::setListener(StreamDataSourceListener* pListener)
{
   mpListener = pListener ;
}


/* ============================ ACCESSORS ================================= */

// Gets the flags specified at time of construction
int StreamDataSource::getFlags()
{
   return miFlags;
}


/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
const char* StreamDataSource::getEventString(StreamDataSourceEvent event)
{
   switch (event)
   {
      case LoadingStartedEvent:
         return "LoadingStartedEvent" ;
         break;
      case LoadingThrottledEvent:
         return "LoadingThrottledEvent" ;
         break ;
      case LoadingCompletedEvent:
         return "LoadingCompletedEvent" ;
         break ;
      case LoadingErrorEvent:
         return "LoadingErrorEvent" ;
         break ;
      default:
         return "LoadingUnknownEvent" ;
         break ;
   }
}
#endif /* MP_STREAM_DEBUG ] */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Assignment operator (not supported)
StreamDataSource& 
StreamDataSource::operator=(const StreamDataSource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor (not supported)
StreamDataSource::StreamDataSource(const StreamDataSource& rStreamDataSource)
{
}

// Fires a data source event to the interested consumer.
void StreamDataSource::fireEvent(StreamDataSourceEvent event)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("StreamDataSource: %s\n", getEventString(event)) ;
#endif /* MP_STREAM_DEBUG ] */

   if (mpListener != NULL)
   {
      mpListener->dataSourceUpdate(this, event) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
      osPrintf("** WARNING: unable to send event %s -- null listener\n", getEventString(event)) ;
   }
#endif /* MP_STREAM_DEBUG ] */
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

#endif
