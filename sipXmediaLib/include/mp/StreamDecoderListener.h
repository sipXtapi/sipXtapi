//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _StreamDecoderListener_h_
#define _StreamDecoderListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamFormatDecoder.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class StreamDecoderListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamDecoderListener();
     //:Default constructor

   virtual
   ~StreamDecoderListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void decoderUpdate(StreamFormatDecoder* pDecoder, 
                              StreamDecoderEvent event) = 0 ;
     //: Informs the listener when the decoder has an event to publish.
     //! param pDecoder - Decoder publishing the state change
     //! param event - The new decoder event state

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamDecoderListener(const StreamDecoderListener& rStreamDecoderListener);
     //:Copy constructor

   StreamDecoderListener& operator=(const StreamDecoderListener& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamDecoderListener_h_
