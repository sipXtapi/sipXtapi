// $Id: //depot/OPENDEV/sipXphone/include/pingerjni/PtTerminalComponentListenerContainer.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PtTerminalComponentListenerContainer_h_
#define _PtTerminalComponentListenerContainer_h_

// SYSTEM INCLUDES
#include "jni.h"
// APPLICATION INCLUDES
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtComponent.h"
// DEFINES
#define CACHEID_PHONEBUTTON_UP					0
#define CACHEID_PHONEBUTTON_DOWN				   1
#define CACHEID_PHONEBUTTON_REPEAT				2
#define CACHEID_PHONEBUTTON_INFOCHANGED		3
#define CACHEID_PHONEHOOKSWITCH_OFFHOOK		4
#define CACHEID_PHONEHOOKSWITCH_ONHOOK			5
#define CACHEID_PHONEDISPLAY_CHANGED			6
#define CACHEID_PHONERINGER_VOLUMECHANGED		7
#define CACHEID_PHONERINGER_PATTERNCHANGED	8
#define CACHEID_PHONERINGER_INFOCHANGED		9
#define CACHEID_PHONESPEAKER_VOLUMECHANGED	10
#define CACHEID_PHONEMICROPHONE_GAINCHANGED	11
#define CACHEID_PHONELAMP_MODECHANGED			12
#define CACHEID_PHONEHANDSET_VOLUMECHANGED	13
#define CACHEID_COMPONENT_EVENT_INIT			14
#define CACHEID_INTCOMPONENT_EVENT_INIT		15

#define CACHEID_MAX_INDEX						   16


#define IGNORE_NONE                             0x00000000    
#define IGNORE_BUTTON_EVENTS                    0x00000001
#define IGNORE_HOOKSWITCH_EVENTS                0x00000002
#define IGNORE_VOLUME_EVENTS                    0x00000004 


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtComponentIntChangeEvent;
class PtComponentStringChangeEvent;
class PtTerminalComponentEvent;
class PtEventMask;

//:The PtTerminalComponentListenerContainer is used to register with and receive 
//:events from PtTerminalComponentListener objects.

class PtTerminalComponentListenerContainer : public PtTerminalComponentListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtTerminalComponentListenerContainer(const char* name = NULL, PtEventMask* pMask = NULL, jobject terminalListener = NULL, void* pEnv = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtTerminalComponentListenerContainer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void phoneRingerVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_RINGER_VOLUME_CHANGED 
     //:indicating that the PtPhoneRinger volume has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneRingerPatternChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_RINGER_PATTERN_CHANGED 
     //:indicating that the PtPhoneRinger audio pattern to be played when ringing has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneRingerInfoChanged(const PtComponentStringChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_RINGER_INFO_CHANGED 
     //:indicating that the PtPhoneRinger info string has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneSpeakerVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_SPEAKER_VOLUME_CHANGED 
     //:indicating that the associated PtPhoneSpeaker volume has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneMicrophoneGainChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_MICROPHONE_GAIN_CHANGED 
     //:indicating that the associated PtPhoneMicrophone gain has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneLampModeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_LAMP_MODE_CHANGED 
     //:indicating that the associated PtPhoneLamp mode has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonInfoChanged(const PtComponentStringChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_BUTTON_INFO_CHANGED 
     //:indicating that the associated PtPhoneButton info string has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonUp(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_BUTTON_UP 
     //:indicating that the associated PtPhoneButton has changed to the up 
     //:(released) position.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonDown(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_BUTTON_DOWN 
     //:indicating that the associated PtPhoneButton has changed to the down 
     //:(pressed) position.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonRepeat(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_BUTTON_DOWN 
     //:indicating that the associated PtPhoneButton has changed to the down 
     //:(pressed) position.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHookswitchOffhook(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_HOOKSWITCH_OFFHOOK 
     //:indicating that the PtPhoneHookswitch has changed to the offhook state.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHookswitchOnhook(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_HOOKSWITCH_ONHOOK 
     //:indicating that the PtPhoneHookswitch has changed to the onhook state.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneDisplayChanged(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_DISPLAY_CHANGED 
     //:indicates that the PtPhoneDisplay has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHandsetVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id = 
     //:PHONE_HANDSET_VOLUME_CHANGED 
     //:indicating that the phone handset speaker volume has changed.
     // The event parameter is valid only within 
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

   void cleanup(JNIEnv* pEnv) ;
     //:Cleanup the listener by clearing all JNI caches

/* ============================ ACCESSORS ================================= */
   static const char* className();
     //:Returns the name of this class
     //!returns: Returns the string representation of the name of this class

   jobject getTerminalListener() { return mjobjListener ; } ;
     //:Returns the terminal listener object for this container

   void setEventFilter(long lEventFilter) ;
     //: set the event filter for this container.

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if the given string contains the class name of this class.
     //!retcode: FALSE - if the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from 
     //:the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - if this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PtTerminalComponentListenerContainer(const PtTerminalComponentListenerContainer& rPtTerminalComponentListenerContainer);
     //:Copy constructor

   PtTerminalComponentListenerContainer& operator=(const PtTerminalComponentListenerContainer& rhs);
     //:Assignment operator   

   PtComponent* createComponentCopy(PtComponent *pOrig) ;
     //:Create a copy of the passed component

   void fireComponentIntChangeEvent(const jobject jobjListener,
								     const char* szMethod,
								     const int iCacheID,
								     const PtComponentIntChangeEvent& rEvent) ;
     //:Helper method that fires off an component int change event

   void fireComponentEvent(const jobject jobjListener,
						    const char* szMethod,
						    const int iCacheID,
						    const PtTerminalComponentEvent& rEvent) ;
     //:Helper method that fires off component change event

   void initializeMethodIdCache(JNIEnv* pEnv) ;
     //:Initialize the methodId cache



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   jobject	mjobjListener ;

   jmethodID m_cachedMethodIDs[CACHEID_MAX_INDEX] ;
   //: Cache of methods 
   jclass    m_classIntComponentEventClass ;
   jclass    m_classComponentEventClass ;

   unsigned long mlEventFilter ;
   //: Event Filters
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalComponentListenerContainer_h_
