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
//#include <...>

// APPLICATION INCLUDES
#include "mp/StreamRAWFormatDecoder.h"
#include "mp/StreamDataSource.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StreamRAWFormatDecoder::StreamRAWFormatDecoder(StreamDataSource* pDataSource)
   : StreamQueueingFormatDecoder(pDataSource, 1600)
   , OsTask("RawDecoder-%d")
   , mSemExited(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
}


// Destructor
StreamRAWFormatDecoder::~StreamRAWFormatDecoder()
{
}

/* ============================ MANIPULATORS ============================== */

// Initializes the decoder
OsStatus StreamRAWFormatDecoder::init()
{
   return OS_SUCCESS ;
}


// Frees all resources consumed by the decoder
OsStatus StreamRAWFormatDecoder::free()
{
   return OS_SUCCESS ;
}


// Begins decoding
OsStatus StreamRAWFormatDecoder::begin()
{
   mbEnd = FALSE ;
   mSemExited.acquire() ;   

   fireEvent(DecodingStartedEvent) ;   
   if (start() == FALSE)
   {
      syslog(FAC_STREAMING, PRI_CRIT, "Failed to create thread for StreamWAVFormatDecoder") ;

      // If we fail to create the thread, send out failure events
      // and clean up
      mbEnd = TRUE ;
      fireEvent(DecodingErrorEvent) ;
      fireEvent(DecodingCompletedEvent) ;
      mSemExited.release() ;
   }   
  
   return OS_SUCCESS ;
}


// Ends decoding
OsStatus StreamRAWFormatDecoder::end()
{
   mbEnd = TRUE ;

   // Interrupt any inprocess reads/seeks.  This speeds up the end.
   StreamDataSource* pSrc = getDataSource() ;
   if (pSrc != NULL)
   {
       pSrc->interrupt() ;
   }

   // Draw the decoded queue
   drain() ;

   // Wait for the run method to exit.
   mSemExited.acquire() ;

   // Draw the decoded queue again to verify that nothing is left.
   drain() ;
   
   mSemExited.release() ;

   return OS_SUCCESS ;
}


/* ============================ ACCESSORS ================================= */

// Renders a string describing this decoder.  
OsStatus StreamRAWFormatDecoder::toString(UtlString& string)
{
   string.append("RAW") ;

   return OS_SUCCESS ;
}

/* ============================ INQUIRY =================================== */


// Gets the decoding status.
UtlBoolean StreamRAWFormatDecoder::isDecoding()
{
   return (isStarted() || isShuttingDown());
}


// Determines if this is a valid decoder given the associated data source.
UtlBoolean StreamRAWFormatDecoder::validDecoder()
{
   return TRUE ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Copy constructor (not supported)
StreamRAWFormatDecoder::StreamRAWFormatDecoder(const StreamRAWFormatDecoder& rStreamRAWFormatDecoder)
   : StreamQueueingFormatDecoder(NULL, 1600)
   , mSemExited(OsBSem::Q_PRIORITY, OsBSem::FULL)
{

}

// Assignment operator (not supported)
StreamRAWFormatDecoder& 
StreamRAWFormatDecoder::operator=(const StreamRAWFormatDecoder& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// Thread entry point
int StreamRAWFormatDecoder::run(void* pArgs)
{
   int iRead ;
   int iLength = sizeof(unsigned short) * 80;
      
   StreamDataSource* pSrc = getDataSource() ;
   if (pSrc != NULL)
   {
      char* pEventSamples = new char[iLength]; 
      while ((pSrc->read(pEventSamples, iLength, iRead) == OS_SUCCESS) && !mbEnd)
      {
         queueFrame((const unsigned short*) pEventSamples) ;
      }
      delete pEventSamples ;

      queueEndOfFrames() ;
      pSrc->close();
   }
   
   fireEvent(DecodingCompletedEvent) ;

   mSemExited.release() ;

   return 0 ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

#endif 
