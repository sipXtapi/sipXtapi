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
#include "mp/StreamFormatDecoder.h"
#include "mp/StreamDecoderListener.h"
#include "mp/StreamDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamFormatDecoder::StreamFormatDecoder(StreamDataSource* pDataSource)
   : mpListener(NULL)
{
   mpDataSource = pDataSource ;
}

// Destructor
StreamFormatDecoder::~StreamFormatDecoder()
{
}

/* ============================ MANIPULATORS ============================== */

// Sets a listener to receive StreamDecoderEvents.
void StreamFormatDecoder::setListener(StreamDecoderListener* pListener)
{
   mpListener = pListener ;
}

/* ============================ ACCESSORS ================================= */

// Gets the the data source for this decoder
StreamDataSource* StreamFormatDecoder::getDataSource() 
{
   return mpDataSource ;
}

/* ============================ INQUIRY =================================== */

/* ============================ TESTING =================================== */


#ifdef MP_STREAM_DEBUG /* [ */
const char* StreamFormatDecoder::getEventString(StreamDecoderEvent event)
{
   switch (event)
   {
      case DecodingStartedEvent:
         return "DecodingStartedEvent" ;
         break;
      case DecodingUnderrunEvent:
         return "DecodingUnderrunEvent" ;
         break ;
      case DecodingThrottledEvent:
         return "DecodingThrottledEvent" ;
         break ;
      case DecodingCompletedEvent:
         return "DecodingCompletedEvent" ;
         break ;
      case DecodingErrorEvent:
         return "DecodingErrorEvent" ;
         break ;
      default:
         return "DecodingUnknownEvent" ;
         break ;
   }
}

#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamFormatDecoder::StreamFormatDecoder(const StreamFormatDecoder& rStreamFormatDecoder)
{
}

// Assignment operator (not supported)
StreamFormatDecoder& 
StreamFormatDecoder::operator=(const StreamFormatDecoder& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Fire an event to an interested consumer.
void StreamFormatDecoder::fireEvent(StreamDecoderEvent event)
{
#ifdef MP_STREAM_DEBUG /* [ */
   osPrintf("StreamFormatDecoder: %s\n", getEventString(event)) ;
#endif /* MP_STREAM_DEBUG ] */

   if (mpListener != NULL)
   {
      mpListener->decoderUpdate(this, event) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	  osPrintf("StreamFormatDecoder: Null listener for event %s\n", getEventString(event)) ;
   }
#endif /* MP_STREAM_DEBUG ] */
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */


#endif
