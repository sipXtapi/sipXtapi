// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORPICKUP_H
#define SIPREDIRECTORPICKUP_H

// SYSTEM INCLUDES
//#include <sys/time.h>

// APPLICATION INCLUDES
#include "SipRedirector.h"
#include "os/OsServerTask.h"
#include "os/OsTimer.h"
#include "xmlparser/tinyxml.h"

// DEFINES
#define ALL_CREDENTIALS_USER "*allcredentials"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRedirectorPickUpTask;

/**
 * SipRedirectorPickUp is a singleton class whose object implements
 * the redirections needed for call pick-up.
 *
 * There are several redirections implemented:
 *
 * [directed call pick-up][extension]
 *    is mapped into a selected early dialog discovered by querying [extension]
 * [global call pick-up]
 *    is mapped into a selected early dialog discovered by querying the special
 *    SIP user "*allcredentials"
 * *allcredentials (ALL_CREDENTIALS_USER)
 *    is mapped into all extensions mentioned in the credential table
 * [call retrieve][extension]
 *    is mapped into a selected confirmeddialog discovered by querying
 *    [extension].  But [extension] must be listed as a call park orbit in
 *    orbits.xml.
 * [extension]
 *    When [extension] is listed in orbits.xml, redirects to
 *    "[extension]@SIP_REGISTRAR_ORBIT_SERVER".
 *
 * More information on their processing and usage can be found in
 * ../doc/Redirection.txt.
 */

class SipRedirectorPickUp : public SipRedirector
{
  public:

   SipRedirectorPickUp();

   ~SipRedirectorPickUp();

   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo);

   virtual void finalize();

   virtual SipRedirector::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage);

   // Enum for values that describe the states of dialogs, and also
   // describe filtering criteria for states of dialogs.
   typedef enum {
      // State is unknown.  (Used only as a description.)
      stateUnknown,
      // Matches any state.  (Used only as a matching criterion.)
      stateDontCare,
      // Any other state value not classified.  (Used only as a description.)
      stateOther,
      // "early"
      stateEarly,
      // "confirmed"
      stateConfirmed
   } State;

   // Get the top-level text content of an XML element.
   static void textContentShallow(UtlString& string,
                                  TiXmlElement *element);
   // Get the complete text content of an XML element (including sub-elements).
   static void textContentDeep(UtlString& string,
                               TiXmlElement *element);
   // Service function for textContentDeep.
   static void textContentDeepRecursive(UtlString& string,
                                        TiXmlElement *element);

  protected:

   // The SIP user agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpSipUserAgent;

   // The OsTask that processes NOTIFYs.
   SipRedirectorPickUpTask* mTask;

   // The feature code (SIP user prefix) for directed pick-up.
   UtlString mCallPickUpCode;

   // The two SIP users that are excluded from being considered
   // pick-up requests, so we can use them for other features.
   UtlString mExcludedUser1;
   UtlString mExcludedUser2;

   // The SIP username for global pick-up.
   UtlString mGlobalPickUpCode;

   // The SIP username for call retrieval.
   UtlString mCallRetrieveCode;

   // The SIP domain of the park server.
   UtlString mParkServerDomain;

   // The full name of the orbit.xml file, or "" if there is none,
   // or if some other problem (e.g., no domain name for the park
   // server is specified) diables orbit recognition.
   UtlString mOrbitFileName;

   // A hash map that has as keys all the call parking orbit users.
   UtlHashMap mOrbitList;

   // The last time we checked the modification time of mOrbitFileName.
   unsigned long mOrbitFileLastModTimeCheck;

   // The last known modification time of mOrbitFileName, or OS_INFINITY
   // if it did not exist.
   OsTime mOrbitFileModTime;

   // The SIP domain we are operating in.
   UtlString mDomain;

   // Time to wait for NOTIFYs, in seconds and microseconds.
   int mWaitSecs;
   int mWaitUSecs;

   // Counter for CSeq for SUBSCRIBES.
   unsigned int mCSeq;

   // Switch for "no early-only" workaround.
   UtlBoolean mNoEarlyOnly;

   // Switch for "reversed Replaces" workaround.
   UtlBoolean mReversedReplaces;

   // Switch for the "1 second subscription" workaround.
   UtlBoolean mOneSecondSubscription;

   // Support functions.
   SipRedirector::LookUpStatus lookUpDialog(
      const UtlString& requestString,
      SipMessage& response,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      const char* subscribeUser,
      State stateFilter);

   // Read and parse the call parking orbit description file.
   OsStatus parseOrbitFile(UtlString& fileName);

   // Return TRUE if the argument is an orbit name listed in the orbits.xml file.
   // (This function takes some care to avoid re-reading orbits.xml when it has
   // not changed since the last call.)
   UtlBoolean findInOrbitList(UtlString&);
};

/**
 * Task to receive and act on NOTIFYs.
 */

class SipRedirectorPickUpTask : public OsServerTask
{
  public:

   SipRedirectorPickUpTask(
      SipUserAgent* pSipUserAgent,
      int redirectorNo);

   ~SipRedirectorPickUpTask();

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

  protected:

  private:

   // The SIP user agent to send SUBSCRIBEs and receive NOTIFYs.
   SipUserAgent* mpSipUserAgent;

   // Our redirector number.
   int mRedirectorNo;
};

/**
 * Notifications for when our timers fire.
 */
class SipRedirectorPickUpNotification : public OsNotification
{
  public:

   SipRedirectorPickUpNotification(
      RequestSeqNo requestSeqNo,
      int redirectorNo);

   OsStatus signal(const int eventData);

  private:

   RequestSeqNo mRequestSeqNo;
   int mRedirectorNo;
};

/**
 * Private storage for pick-up suspensions.
 */
class SipRedirectorPrivateStoragePickUp : public SipRedirectorPrivateStorage
{
   friend class SipRedirectorPickUp;
   friend class SipRedirectorPickUpTask;

  public:
   
   SipRedirectorPrivateStoragePickUp(RequestSeqNo requestSeqNo,
                                     int redirectorNo);

   virtual ~SipRedirectorPrivateStoragePickUp();

   virtual UtlContainableType getContainableType() const;

  protected:

   // CallId of the SUBSCRIBE we sent.
   UtlString mSubscribeCallId;

   // State filtering criteria.
   SipRedirectorPickUp::State mStateFilter;

    static UtlContainableType TYPE;    /** < Class type used for runtime checking */

  private:

   // Notification.
   SipRedirectorPickUpNotification mNotification;

   // Timer.
   OsTimer mTimer;

   // When the SUBSCRIBE was sent.
   OsTime mSubscribeSendTime;

   // Process a NOTIFY body and record the calls as appropriate.
   void processNotify(const char* body);
   void processNotifyDialogElement(TiXmlElement* dialog);
   void processNotifyLocalRemoteElement(TiXmlElement* element,
                                        UtlString& identity,
                                        UtlString& target);

   // Information about the best dialog to pick up that we have seen so far.
   // Call-Id
   UtlString mTargetDialogCallId;
   // Duration, or 0 if none was given for the dialog, or the special value
   // TargetDialogDurationAbsent if no dialog is recorded.
   // If no <duration> is specified in a dialog event notice (and many
   // event notices do not contain it), use 0.
   // Since we select the call with the largest duration, this puts
   // phones that do not provide <duration> at a disadvantage to get
   // their calls picked up.
   int mTargetDialogDuration;
   // Note that the special value must be less than any legitimate duration.
   static const int TargetDialogDurationAbsent;
   // Local tag (from the point of view of the UA we are picking up from)
   UtlString mTargetDialogLocalTag;
   // Remote tag
   UtlString mTargetDialogRemoteTag;
   // URI at which to contact the remote end.
   UtlString mTargetDialogRemoteURI;
   // URI at which to contact the local end
   UtlString mTargetDialogLocalURI;
   // Local Identity
   UtlString mTargetDialogLocalIdentity;
};

#endif // SIPREDIRECTORPICKUP_H
