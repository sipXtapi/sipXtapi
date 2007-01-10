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


#ifndef _StreamDataSource_h_
#define _StreamDataSource_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "mp/StreamDefs.h"
#include "os/OsStatus.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef enum                  // Data source event definitions
{
   LoadingStartedEvent,       // Data source statred loading
   LoadingThrottledEvent,     // Data source throttled
   LoadingCompletedEvent,     // Data source completed loading
   LoadingErrorEvent          // Data source error

} StreamDataSourceEvent;

// FORWARD DECLARATIONS
class StreamDataSourceListener;


/**
*  @brief An abstraction definition of a stream data source
*/
class StreamDataSource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructors a StreamDataSource given optional flags.
   StreamDataSource(int iFlags = 0);
     /**<
     *  @see StreamDefs.h for a description of available flags.
     */

     /// Destructor
   virtual ~StreamDataSource();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Opens the data source
   virtual OsStatus open() = 0 ;

     /// Closes the data source
   virtual OsStatus close() = 0 ;

     /// Destroys and deletes the data source object
   virtual OsStatus destroyAndDelete() = 0 ;

     /// Reads iLength bytes of data from the data source and places the
     /// data into the passed szBuffer buffer.
   virtual OsStatus read(char *szBuffer, int iLength, int& iLengthRead) = 0;
     /**<
     *  @param szBuffer - Buffer to place data
     *  @param iLength - Max length to read
     *  @param iLengthRead - The actual amount of data read.
     */

     /// Identical to read, except the stream pointer is not advanced.
   virtual OsStatus peek(char* szBuffer, int iLength, int& iLengthRead) = 0;
     /**<
     *  @param szBuffer - Buffer to place data
     *  @param iLength - Max length to read
     *  @param iLengthRead - The actual amount of data read.
     */
   
     /// Interrupts any time consuming operation.
   virtual OsStatus interrupt() ;
     /**<
     *  For example, some data sources may require network access (e.g. http)
     *  to read or fetch data.  Invoking an interrupt() will cause any
     *  time consuming or blocking calls to exit with more quickly with an 
     *  OS_INTERRUPTED return code.
     */

     /// Moves the stream pointer to the an absolute location.
   virtual OsStatus seek(unsigned int iLocation) = 0 ;
     /**<
     *  @param iLocation - The desired seek location
     */

     /// Sets a listener to receive StreamDataSourceEvent events for this
     /// data source.
   void setListener(StreamDataSourceListener* pListener);
   
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Gets the length of the stream (if available)
   virtual OsStatus getLength(int& iLength) = 0 ;

     /// Gets the current position within the stream.
   virtual OsStatus getPosition(int& iPosition) = 0 ;

     /// Renders a string describing this data source.  
   virtual OsStatus toString(UtlString& string) = 0 ;
     // This is often used for debugging purposes.
      
     /// Gets the flags specified at time of construction
   int getFlags() ;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* ============================ TESTING =================================== */

#ifdef MP_STREAM_DEBUG /* [ */
static const char* getEventString(StreamDataSourceEvent event);
#endif /* MP_STREAM_DEBUG ] */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Copy constructor (not supported)
   StreamDataSource(const StreamDataSource& rStreamDataSource);

     /// Assignment operator (not supported)
   StreamDataSource& operator=(const StreamDataSource& rhs);

     /// Fires a data source event to the interested consumer.
   void fireEvent(StreamDataSourceEvent event);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   StreamDataSourceListener* mpListener; ///< data source listener
   int                       miFlags;    ///< flags specified during construction
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamDataSource_h_
