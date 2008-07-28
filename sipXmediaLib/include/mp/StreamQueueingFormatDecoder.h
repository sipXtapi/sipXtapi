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
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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

#ifndef _StreamQueueingFormatDecoder_h_
#define _StreamQueueingFormatDecoder_h_

// SYSTEM INCLUDES
#include "time.h"

// APPLICATION INCLUDES
#include "mp/StreamFormatDecoder.h"
#include "os/OsDefs.h"
#include "os/OsMsgPool.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The Stream Queueing Format Decoder builds upon the abstract 
//:StreamFormatDecoder by adding a mechanism to queue a max number
//:of rendered frames.
class StreamQueueingFormatDecoder : public StreamFormatDecoder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   StreamQueueingFormatDecoder(StreamDataSource* pDataSource, 
                               int               iQueueLength);
     //:Constructs a queueing format decoder given a data source and queue
     //:length

   virtual
   ~StreamQueueingFormatDecoder();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   virtual OsStatus getFrame(unsigned short* samples);
     //: Gets the next available frame
     //! returns OS_SUCCESS if a frame is available

   virtual OsStatus queueFrame(const unsigned short* pSamples);
     //: Queues a frame of data

   virtual OsStatus queueEndOfFrames();
     //: Queues an end of frame marker.  This informs MprFromStream that the
     //: Stream has ended.

   virtual OsStatus drain();
     //: Drains any queued frames

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   int getMaxQueueLength() ;
     //: Gets the maximum number of frames that can be queued before the
     //: queueing routines will block.

   int getNumQueuedFrames() ;
     //: Gets the current number of queued frames.

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamQueueingFormatDecoder(const StreamQueueingFormatDecoder& rStreamQueueingFormatDecoder);
     //:Copy constructor (not supported)

   StreamQueueingFormatDecoder& operator=(const StreamQueueingFormatDecoder& rhs);
     //:Assignment operator (not supported)
      
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMsgQ    mMsgqFrames ;        // Queue of frames
   OsMsgPool mMsgPool;            // Pool for msg containers
   int       miMaxQueueLength ;   // Max size of the queue length
   UtlBoolean mbReportThrottle ;   // Should this report throttles?
   UtlBoolean mbDraining ;

   static OsMutex mMutReport ;// Thread safety for performance monitoring
   
   static time_t       sLastReported ;  // When we last reported status

   static unsigned int sDeltaFrames ;	// Frames since last report
   static unsigned int sDeltaStreams ;	// Streams since last report
   static unsigned int sDeltaUnderruns ;// Underruns since last report
   static unsigned int sDeltaThrottles ;// Throttles since last report

   static unsigned int sTotalFrames ;	// Cumulative number of frames
   static unsigned int sTotalStreams ;	// Cumulative number of streams
   static unsigned int sTotalUnderruns ;// Cumulative number of underruns
   static unsigned int sTotalThrottles ;// Cumulative number of throttles

   static void reportFrame(UtlBoolean bUnderrun) ;
     //:Reports that a frame has been processed by media processing.

   static void reportThrottle() ;
    //: Reports that the decoder has been throttled (decoding faster 
    //: then data is being requested).

   static void reportStream() ;
     //:Reports that a stream has been created
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamQueueingFormatDecoder_h_

#endif
