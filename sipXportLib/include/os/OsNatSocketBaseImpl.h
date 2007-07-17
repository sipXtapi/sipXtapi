// $Id$
//
// Copyright (C) 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatSocketBaseImpl_h_
#define _OsNatSocketBaseImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "os/IStunSocket.h"
#include "os/OsNotification.h"

// The follow defines are used to keep track of what has been recorded for
// various time-based metrics.
#define ONDS_MARK_NONE           0x00000000
#define ONDS_MARK_FIRST_READ     0x00000001
#define ONDS_MARK_LAST_READ      0x00000002
#define ONDS_MARK_FIRST_WRITE    0x00000004
#define ONDS_MARK_LAST_WRITE     0x00000008

class OsNatSocketBaseImpl : public IStunSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    OsNatSocketBaseImpl() ;
    virtual ~OsNatSocketBaseImpl() ;

    /**
     * Set a notification object to be signaled when the first the data 
     * packet is received from the socket.  Once this is signaled, the 
     * notification object is discarded.
     */
    virtual void setReadNotification(OsNotification* pNotification) ;
    
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void markReadTime() ;
    void markWriteTime() ;

   /**
    * Get the timestamp of the first read data packet (excluding any 
    * STUN/TURN/NAT packets).
    */
   virtual bool getFirstReadTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the last read data packet (excluding any 
    * STUN/TURN/NAT packets).
    */
   virtual bool getLastReadTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the first written data packet (excluding any
    * STUN/TURN/NAT packets).
    */
   virtual bool getFirstWriteTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the last written data packet (excluding any
    * STUN/TURN/NAT packets).
    */
   virtual bool getLastWriteTime(OsDateTime& time) ;

    /**
     * Handle/process an inbound STUN message.
     */
    virtual void handleStunMessage(char* pBuf, int length, UtlString& fromAddress, int fromPort) ;


    /**
     * Handle/process an inbound TURN message.
     */
    virtual void handleTurnMessage(char*        pBuf, 
                                   int          length, 
                                   UtlString&   fromAddress, 
                                   int          fromPort) ;
    
    /**
     * Pull a data indication out of the buffer
     */
    virtual int handleTurnDataIndication(char*      buffer, 
                                         int        bufferLength,
                                         UtlString* pRecvFromIp,
                                         int*       pRecvFromPort) ;

    /**
     *  Handle/process an inbound STUN or TURN message.
     */
    virtual bool handleSturnData(char*      buffer, 
                                 int&       bufferLength,
                                 UtlString& receivedIp,
                                 int&       receivedPort) ; 

    virtual OsSocket* getSocket();                                 

protected:
    unsigned int          miRecordTimes ;   // Bitmask populated w/ ONDS_MARK_*
    OsDateTime            mFirstRead ;
    OsDateTime            mLastRead ;
    OsDateTime            mFirstWrite ;
    OsDateTime            mLastWrite ;
    OsMutex               mReadNotificationLock ;
    OsNotification*       mpReadNotification ;
    
};

/* ============================ INLINE METHODS ============================ */


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */



#endif  // _OsNatSocketBaseImpl_h_

