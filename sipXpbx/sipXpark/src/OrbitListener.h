// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////

#ifndef _OrbitListener_h_
#define _OrbitListener_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include <tao/TaoDefs.h>
#include <ParkedCallObject.h>
#include <utl/UtlHashMap.h>
#include "OrbitFileReader.h"

// DEFINES
#define ORBIT_CONFIG_FILENAME     "orbits.xml"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CallManager;
class TaoString;
class OrbitData;

//: Dummy DTMF listener.
class DummyListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   DummyListener();
   //:Default constructor

   DummyListener(const DummyListener& rDummyListener);
   //:Copy constructor

   virtual ~DummyListener();
   //:Destructor

/* ============================ MANIPULATORS ============================== */

   DummyListener& operator=(const DummyListener& rhs);
   //:Assignment operator

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

};


//: Object that listens for incoming calls for the Park Server.
//  This object handles an arbitrary set of orbits, described by one orbits.xml
//  file.
class OrbitListener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OrbitListener(CallManager* callManager = NULL);
   //:Default constructor

   ~OrbitListener();

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
     //: Method to process messages which get queued on this
     /**  The is the method that does all the work.  Telephony
       *  events get queued on this which consequently get
       *  processed in handleMessage in the contect of this
       *  task.
       */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void dumpTaoMessageArgs(TaoObjHandle eventId, TaoString& args);

    OsStatus validateOrbit(const UtlString& callId,
                           const UtlString& address,
                           UtlString& orbit,
                           UtlString& audio,
                           int& timeout,
                           int& keycode,
                           int& capacity);
    bool isCallRetrievalInvite(const char* callId, const char* address);

    // Set up the data structures for a new call that isn't a call-retrieval
    // call.
    void setUpParkedCall(const UtlString& callId,
                         const UtlString& address,
                         const UtlString& orbit,
                         const UtlString& audio,
                         int timeout,
                         int keycode,
                         int capacity,
                         const TaoString& arg);

    // Do the work for a call-retrieval call.
    void setUpRetrievalCall(const UtlString& callId,
                            const UtlString& address);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    
    OrbitListener(const OrbitListener& rOrbitListener);
     //:Copy constructor

    OrbitListener& operator=(const OrbitListener& rOrbitListener);
     //:Assignment operator
     
    ParkedCallObject* getOldestCallInOrbit(UtlString& orbit, UtlString& callId,
                                           UtlString& address);

    int getNumCallsInOrbit(UtlString& orbit);

    // Find the ParkedCallObject with a given mSeqNo, or return NULL.
    ParkedCallObject* findBySeqNo(int seqNo);

    CallManager* mpCallManager;

    UtlHashMap mCalls;

    // Object to manage reading and updating orbit file information.
    OrbitFileReader mOrbitFileReader;

    // Dummy DTMF listener.
    DummyListener mListener;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OrbitListener_h_
