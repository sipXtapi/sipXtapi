// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/JavaSUAListenerDispatcher.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _JavaSUAListenerDispatcher_h_
#define _JavaSUAListenerDispatcher_h_

// SYSTEM INCLUDES
#include <jni.h>

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsConfigDb.h"
#include "utl/UtlHashMap.h"



// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define TONE_LISTENER_CLASS                 "org/sipfoundry/stapi/event/ToneListener"
#define TONE_LISTENER_CONST                 "<init>"
#define TONE_LISTENER_CONST_SIGNATURE       "()V"
#define NEW_MESSAGE_LISTENER_CLASS          "org/sipfoundry/sip/event/NewSipMessageListener"
#define NEW_MESSAGE_LISTENER_METHOD         "newMessage"
#define NEW_MESSAGE_LISTENER_SIGNATURE      "(Lorg/sipfoundry/sip/event/NewSipMessageEvent;)V"
#define TONE_LISTENER_METHOD                "toneEvent"
#define TONE_LISTENER_METHOD_SIGNATURE      "(Ljava/lang/String;Ljava/lang/String;II)V"
#define NEW_SIP_EVENT_CLASS                 "org/sipfoundry/sip/event/NewSipMessageEvent"
#define NEW_SIP_EVENT_CONST_METHOD          "<init>"
#define NEW_SIP_EVENT_CONST_SIGNATURE       "(Lorg/sipfoundry/sip/SipMessage;Z)V"
#define SIP_REQUEST_CLASS                   "org/sipfoundry/sip/SipRequest"
#define SIP_REQUEST_CONST_METHOD            "<init>"
#define SIP_REQUEST_CONST_SIGNATURE         "(Ljava/lang/String;)V"
#define SIP_RESPONSE_CLASS                  "org/sipfoundry/sip/SipResponse"
#define SIP_RESPONSE_CONST_METHOD           "<init>"
#define SIP_RESPONSE_CONST_SIGNATURE        "(Ljava/lang/String;)V"
#define SIP_LINE_LISTENER_METHOD_ENABLED    "lineEnabled"
#define SIP_LINE_LISTENER_METHOD_FAILED     "lineFailed"
#define SIP_LINE_LISTENER_METHOD_TIMEOUT    "lineTimeout"
#define SIP_LINE_LISTENER_METHOD_ADDED      "lineAdded"
#define SIP_LINE_LISTENER_METHOD_DELETED    "lineDeleted"
#define SIP_LINE_LISTENER_METHOD_CHANGED    "lineChanged"
#define SIP_LINE_LISTENER_METHOD_OUTBOUND   "lineDefault"
#define SIP_LINE_LISTENER_SIGNATURE         "(Lorg/sipfoundry/sip/event/SipLineEvent;)V"
#define SIP_LINE_CLASS                      "org/sipfoundry/sip/SipLine"
#define SIP_LINE_CONST_METHOD               "<init>"
#define SIP_LINE_CONST_SIGNATURE            "(Ljava/lang/String;Ljava/lang/String;)V"
#define SIP_LINE_EVENT_CLASS                "org/sipfoundry/sip/event/SipLineEvent"
#define SIP_LINE_EVENT_CONST_METHOD         "<init>"
#define SIP_LINE_EVENT_CONST_SIGNATURE      "(Lorg/sipfoundry/sip/SipLine;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"


// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent ;
class SipMessage ;
class SipLine ;
class SipLineEvent ;


//:This class is used indirectly by both the Java Layer and SipUserAgent.  The
// SipUserAgent provides SipMessage observations in various forms.  In order to
// expose this feature to Java, this task was created to adapt the C++ call backs
// (in the form of OsMsgs), and redispatch them to java listeners.
class JavaSUAListenerDispatcher : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
private:
   JavaSUAListenerDispatcher() ;
     //:Default private constructor, guards against instantiation.  Use the 
     // getInstance() method to obtain an instance to this dispatch thread

public:
   virtual ~JavaSUAListenerDispatcher() ;
     //:Destructor

public:
   static JavaSUAListenerDispatcher* getInstance() ;
   //:Obtain a reference to the Java SIP UA Listener Dispatcher.  Developers
   // must use this method to access/use the dispatcher.  The underlying task
   // is automatically created and started the first time this method is 
   // called.

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);
   //:Handles incoming messages.  
   
   void addSipJNIListenerMapping(int iKey, jobject jobjValue) ;
   //: Add a mapping between a key and a java object
 
   void removeSipJNIListenerMapping(int iKey) ;
   //: Removes the int key java object from the mapping hash dictionary

   void addLineJNIListenerMapping(int iKey, jobject jobjValue) ;
   //: Add a mapping between a key and a java object

   void removeLineJNIListenerMapping(int iKey) ;
   //: Removes the int key java object from the mapping hash dictionary

   void takeListenerLock() ;
   //: Take the lock protected listener list manipulation

   void releaseListenerLock() ;
   //: Release the lock protected listener list manipulation

   void* createSipRequestObject(const char* szText, void* pEnv) ;
   //:Builds a Java SipRequest object

   void* createSipResponseObject(const char* szText, void* pEnv) ;
   //:Builds a Java SipResponse object



/* ============================ ACCESSORS ================================= */

  jobject getSipJNIListenerMapping(int iKey) ;
  //: Get the mapping of one jobject from an integer key

   jobject getLineJNIListenerMapping(int iKey) ;
  //: Get the mapping of one jobject from an integer key

   jobject getToneListener(void* pEnv) ;
   //: Gets a singleton static instance of ToneListener java object. 

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    
   UtlBoolean dispatchSipMessage(const SipMessage* pMessage, const void* pListener) ;
   //:Dispatches a Sip Response to the specified java object/listener

   void* createSipEventObject(const SipMessage* pMessage, void* pEnv) ;
   //:Create the java event object that will be later delievered to 
   // java NewSipMessageListeners

   UtlBoolean dispatchLineEvent(SipLineEvent* pEvent) ;
   //:Dispatches a Sip Response to the specified java object/listener

   void* createSipLineObject(const char* szIdentity, const char* szDisplay, void* pEnv) ;
   //:Builds a Java Sip Line Object

   void* createSipLineEventObject(SipLineEvent& event, void* pEnv) ;
   //:Builds a Java Sip Line Event Object

   UtlBoolean processDtmfDigit(const char* szCallId, 
                              const char* szConnection,
                              const int   iDigit,
                              const int   iDuration) ;
     //:Processes a remote DTMF digit press

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   JavaSUAListenerDispatcher(const JavaSUAListenerDispatcher& rJavaSUAListenerDispatcher);
     //:Copy constructor disabled

   JavaSUAListenerDispatcher& operator=(const JavaSUAListenerDispatcher& rhs);
     //:Assignment operator disabled


/* //////////////////////////// ATTRIBUTES //////////////////////////////// */
   static JavaSUAListenerDispatcher* m_spInstance ; // pointer to the single
                                                    // instance of this class
   static OsBSem   m_sLock;   // semaphore used to ensure that there is only one
                              // instance of this class
   static UtlString m_sTaskName ; // The name of this task, used when creating
                                 // the os server task and for debugging 
                                 // purposes

   static OsMutex  m_sMutexListeners ;

   UtlHashMap m_htSipListeners ;  // Active Sip listeners
   UtlHashMap m_htLineListeners ;  // Active Line listeners

};

/* ============================ INLINE METHODS ============================ */

#endif  // _JavaSUAListenerDispatcher_h_
