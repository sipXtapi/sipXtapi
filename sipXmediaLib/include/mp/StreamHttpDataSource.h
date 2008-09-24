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


#ifndef _StreamHttpDataSource_h_
#define _StreamHttpDataSource_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"
#include "net/Url.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTask.h"
#include "os/OsBSem.h"


// DEFINES
#define DEFAULT_BUFFER_SIZE   128*1024
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class OsConnectionSocket ;
class HttpMessage ;
// TYPEDEFS
typedef UtlBoolean (*GetDataCallbackProc)(char* pData, 
                                         int iLength, 
                                         void* pOptionalData, 
                                         HttpMessage* pMsg);


/// Defines a stream data source built on top of a Http Stream.
/**
*  Deletion of this class is very problematic, because the thread
*  that pumps the data from the stream may be blocked for a long
*  period of time and most cases the media task is player
*  destroying this resource.  Ideally, we would interrupt the socket
*  read, however, that is not possible.  So, to avoid blocking the 
*  thread context used to destroy this object, we use a variable 
*  mbDeleteOnCompletion to mark if/when we need to destroy this class 
*  once the run method has completed.  This method is wrapped in
*  a static binary semaphore to guard against races.
*/
class StreamHttpDataSource : public StreamDataSource, public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Default constructor
   StreamHttpDataSource(Url url, int iFlags);

     /// Destructor
   virtual ~StreamHttpDataSource();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Opens the data source
   virtual OsStatus open() ;

     /// Closes the data source
   virtual OsStatus close() ;

     /// Destroys and deletes the data source object
   virtual OsStatus destroyAndDelete() ;

     /// Reads iLength bytes of data from the data source and places the
     /// data into the passed szBuffer buffer.
   virtual OsStatus read(char *szBuffer, int iLength, int& iLengthRead) ;
     /**<
     *  @param szBuffer - Buffer to place data
     *  @param iLength - Max length to read
     *  @param iLengthRead - The actual amount of data read.
     */

     /// Identical to read, except the stream pointer is not advanced.
   virtual OsStatus peek(char* szBuffer, int iLength, int& iLengthRead) ;
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
   virtual OsStatus seek(unsigned int iLocation);
     /**<
     *  @param iLocation - The desired seek location
     */


     /// Callback routine that is invoked whenever new data is available from http socket.
   UtlBoolean deliverData(char *szData, int iLength, int iMaxLength) ;

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   /// Gets the length of the stream (if available)
   virtual OsStatus getLength(int& iLength);      

   /// Gets the current position within the stream.
   virtual OsStatus getPosition(int& iPosition) ;

     /// Renders a string describing this data source.
   virtual OsStatus toString(UtlString& string) ;
     // This is often used for debugging purposes.

     /// Gets the amount of data presently buffered.
   virtual int getBufferedLength();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Copy constructor (not supported)
   StreamHttpDataSource(const StreamHttpDataSource& rStreamHttpDataSource);

     /// Assignment operator (not supported)
   StreamHttpDataSource& operator=(const StreamHttpDataSource& rhs);

     /// Thread entry point
   int run(void *pArgs);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   Url        m_url ;                  ///< Source url
   UtlString  mBuffer ;                ///< Buffered content
   OsBSem     mSemNeedData ;           ///< Used block when we need more data
   OsBSem     mSemLimitData ;          ///< Used to block when need to throttle
   OsBSem     mSemGuardData;           ///< Protected data structures
   OsBSem     mSemGuardStartClose;     ///< Protected Opening/Closing the DataSource   
   UtlBoolean  mbDone ;                ///< Is the data source complete
   UtlBoolean  mbQuit ;                ///< Are we quiting?
   UtlBoolean  mbDeleteOnCompletion ;  ///< Should the run method delete the object?
   unsigned int  miMaxData ;           ///< Max amount of data to buffer
   unsigned int  miDSLength ;          ///< Data Stream Length
   unsigned int  miOffset ;            ///< Present offset into the buffer
   unsigned int  miBufferOffset ;      ///< Buffer offset from start of stream
   UtlBoolean  mbFiredThrottledEvent ; ///< Should fire event on next throttle?
   UtlBoolean  mbClosed ;              ///< Have we closed this down?
   UtlBoolean  mbInterrupt ;           ///< Interrupt current operation?

   static OsBSem sSemGuardDelete;      ///< Guard deletion of data sources
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamHttpDataSource_h_
