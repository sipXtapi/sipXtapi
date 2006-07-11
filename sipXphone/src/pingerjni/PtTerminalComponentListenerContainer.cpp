// $Id: //depot/OPENDEV/sipXphone/src/pingerjni/PtTerminalComponentListenerContainer.cpp#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include "jni.h"
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "pingerjni/PtTerminalComponentListenerContainer.h"
#include "ptapi/PtTerminalComponentListener.h"
#include "ptapi/PtComponentIntChangeEvent.h"
#include "ptapi/PtComponentStringChangeEvent.h"
#include "ptapi/PtTerminalComponentEvent.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtEventMask.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneDisplay.h"
#include "pingerjni/debug_jni.h"
#include "pingerjni/JNIHelpers.h"
#include "pingerjni/CallListenerHelpers.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtTerminalComponentListenerContainer::PtTerminalComponentListenerContainer(const char* name, PtEventMask* pMask, jobject terminalListener, void* pEnv) 
: PtTerminalComponentListener(name, pMask)
{
   m_classIntComponentEventClass = NULL ;
   m_classComponentEventClass = NULL ;

   for (int i=0; i<CACHEID_MAX_INDEX; i++) 
   {
      m_cachedMethodIDs[i] = NULL ;
   }

   // Store a reference to our java listener object   
   if (terminalListener != NULL) 
   {
      if (pEnv == NULL)
      {         
         JavaVM* pVM ;
         JNIEnv* pJNIEnv ;
      
         if (jniGetVMReference(&pVM, &pJNIEnv))
         {
            mjobjListener = pJNIEnv->NewGlobalRef(terminalListener) ;
            initializeMethodIdCache(pJNIEnv) ;
            jniReleaseVMReference(pVM) ; 
         }
      }
      else
      {
         mjobjListener = ((JNIEnv*) pEnv)->NewGlobalRef(terminalListener) ;
         initializeMethodIdCache((JNIEnv*) pEnv) ;
      }
   }
}


// Copy constructor
PtTerminalComponentListenerContainer::PtTerminalComponentListenerContainer(const PtTerminalComponentListenerContainer& rPtTerminalComponentListener)
{
   // Not Implemented
}


// Destructor
PtTerminalComponentListenerContainer::~PtTerminalComponentListenerContainer()
{
   // Free the global reference to our java listener object
   if (mjobjListener != NULL) 
   {             
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         pEnv->DeleteGlobalRef(mjobjListener) ;
       mjobjListener = NULL ;
         cleanup(pEnv) ;
         jniReleaseVMReference(pVM) ; 
      }
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtTerminalComponentListenerContainer& 
PtTerminalComponentListenerContainer::operator=(const PtTerminalComponentListenerContainer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


void PtTerminalComponentListenerContainer::phoneRingerVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneRingerVolumeChanged", CACHEID_PHONERINGER_VOLUMECHANGED, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneRingerPatternChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneRingerPatternChanged", CACHEID_PHONERINGER_PATTERNCHANGED, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneRingerInfoChanged(const PtComponentStringChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       osPrintf("PtTerminalComponentListenerContainer: phoneRingerInfoChanged\n NOT IMPLEMENTED") ;
}


void PtTerminalComponentListenerContainer::phoneSpeakerVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneSpeakerVolumeChanged", CACHEID_PHONESPEAKER_VOLUMECHANGED, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneMicrophoneGainChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneMicrophoneGainChanged", CACHEID_PHONEMICROPHONE_GAINCHANGED,  rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneLampModeChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneLampModeChanged", CACHEID_PHONELAMP_MODECHANGED, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneHandsetVolumeChanged(const PtComponentIntChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentIntChangeEvent(mjobjListener, "phoneHandsetVolumeChanged", CACHEID_PHONEHANDSET_VOLUMECHANGED, rEvent) ;
}



void PtTerminalComponentListenerContainer::phoneButtonInfoChanged(const PtComponentStringChangeEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_BUTTON_EVENTS))
        osPrintf("PtTerminalComponentListenerContainer: phoneButtonInfoChanged **NOT IMPLEMENTED**\n") ;
}


void PtTerminalComponentListenerContainer::phoneButtonUp(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_BUTTON_EVENTS))
       fireComponentEvent(mjobjListener, "phoneButtonUp", CACHEID_PHONEBUTTON_UP,  rEvent) ;

}


void PtTerminalComponentListenerContainer::phoneButtonDown(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_BUTTON_EVENTS))
       fireComponentEvent(mjobjListener, "phoneButtonDown", CACHEID_PHONEBUTTON_DOWN,  rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneButtonRepeat(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_BUTTON_EVENTS))
       fireComponentEvent(mjobjListener, "phoneButtonRepeat", CACHEID_PHONEBUTTON_REPEAT, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneHookswitchOffhook(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_HOOKSWITCH_EVENTS))
       fireComponentEvent(mjobjListener, "phoneHookswitchOffhook", CACHEID_PHONEHOOKSWITCH_OFFHOOK,  rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneHookswitchOnhook(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_HOOKSWITCH_EVENTS))
       fireComponentEvent(mjobjListener, "phoneHookswitchOnhook", CACHEID_PHONEHOOKSWITCH_ONHOOK, rEvent) ;
}


void PtTerminalComponentListenerContainer::phoneDisplayChanged(const PtTerminalComponentEvent& rEvent)
{
    if (!(mlEventFilter & IGNORE_VOLUME_EVENTS))
       fireComponentEvent(mjobjListener, "phoneDisplayChanged", CACHEID_PHONEDISPLAY_CHANGED, rEvent) ;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

PT_IMPLEMENT_CLASS_INFO(PtTerminalComponentListenerContainer, PtTerminalComponentListener) 


/* //////////////////////////// PROTECTED ///////////////////////////////// */


void PtTerminalComponentListenerContainer::initializeMethodIdCache(JNIEnv* pEnv)
{
   //
   // First Clear all of the method ids
   //
   for (int i=0; i<CACHEID_MAX_INDEX; i++)
   {
      m_cachedMethodIDs[i] = 0 ;
   }


   //
   // Second, Load the two event classes
   //
   jclass classIntEvent = pEnv->FindClass("org/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent") ;
   if (classIntEvent != NULL)
   {
      m_classIntComponentEventClass = (jclass) pEnv->NewGlobalRef(classIntEvent) ;
      pEnv->DeleteLocalRef(classIntEvent) ;
      // CACHEID_INTCOMPONENT_EVENT_INIT
      m_cachedMethodIDs[CACHEID_INTCOMPONENT_EVENT_INIT] = pEnv->GetMethodID(
            m_classIntComponentEventClass, 
            "<init>", 
            "(JJII)V") ;
      if (m_cachedMethodIDs[CACHEID_INTCOMPONENT_EVENT_INIT] <= 0)
         LOOKUP_FAILURE() ;
   }
   else
   {
      LOOKUP_FAILURE() ;
   }

   jclass classEvent = pEnv->FindClass("org/sipfoundry/telephony/phone/event/PtTerminalComponentEvent") ;
   if (classEvent != NULL)
   {
      m_classComponentEventClass = (jclass) pEnv->NewGlobalRef(classEvent) ;
      pEnv->DeleteLocalRef(classEvent) ;
      // CACHEID_COMPONENT_EVENT_INIT
      m_cachedMethodIDs[CACHEID_COMPONENT_EVENT_INIT] = pEnv->GetMethodID(   
            m_classComponentEventClass, 
            "<init>", 
            "(JJ)V") ;
      if (m_cachedMethodIDs[CACHEID_COMPONENT_EVENT_INIT] <= 0)
         LOOKUP_FAILURE() ;
   }   
   else
   {
      LOOKUP_FAILURE() ;
   }


   // Lastly cache the method ids
   jclass classListener = pEnv->FindClass("org/sipfoundry/telephony/phone/PtTerminalComponentListener") ;
   if (classListener != NULL)
   {
      // CACHEID_PHONEBUTTON_UP                    (norm)
      m_cachedMethodIDs[CACHEID_PHONEBUTTON_UP] = pEnv->GetMethodID(
            classListener, 
            "phoneButtonUp",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEBUTTON_UP] <= 0)
         LOOKUP_FAILURE() ;
   
      // CACHEID_PHONEBUTTON_DOWN                  (norm)
      m_cachedMethodIDs[CACHEID_PHONEBUTTON_DOWN] = pEnv->GetMethodID(
            classListener, 
            "phoneButtonDown",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEBUTTON_DOWN] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEBUTTON_REPEAT                (norm)
      m_cachedMethodIDs[CACHEID_PHONEBUTTON_REPEAT] = pEnv->GetMethodID(
            classListener, 
            "phoneButtonRepeat",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEBUTTON_REPEAT] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEBUTTON_INFOCHANGED           (ni)


      // CACHEID_PHONEHOOKSWITCH_OFFHOOK           (norm)
      m_cachedMethodIDs[CACHEID_PHONEHOOKSWITCH_OFFHOOK] = pEnv->GetMethodID(
            classListener, 
            "phoneHookswitchOffhook",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEHOOKSWITCH_OFFHOOK] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEHOOKSWITCH_ONHOOK            (norm)
      m_cachedMethodIDs[CACHEID_PHONEHOOKSWITCH_ONHOOK] = pEnv->GetMethodID(
            classListener, 
            "phoneHookswitchOnhook",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEHOOKSWITCH_ONHOOK] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEDISPLAY_CHANGED              (norm)
      m_cachedMethodIDs[CACHEID_PHONEDISPLAY_CHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneDisplayChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtTerminalComponentEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEDISPLAY_CHANGED] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONERINGER_VOLUMECHANGED         (int)
      m_cachedMethodIDs[CACHEID_PHONERINGER_VOLUMECHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneRingerVolumeChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONERINGER_VOLUMECHANGED] <= 0)
         LOOKUP_FAILURE() ;


      // CACHEID_PHONERINGER_PATTERNCHANGED        (int)
      m_cachedMethodIDs[CACHEID_PHONERINGER_PATTERNCHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneRingerPatternChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONERINGER_PATTERNCHANGED] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONERINGER_INFOCHANGED           (ni)
   

      // CACHEID_PHONESPEAKER_VOLUMECHANGED        (int)
      m_cachedMethodIDs[CACHEID_PHONESPEAKER_VOLUMECHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneSpeakerVolumeChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONESPEAKER_VOLUMECHANGED] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEMICROPHONE_GAINCHANGED       (int)
      m_cachedMethodIDs[CACHEID_PHONEMICROPHONE_GAINCHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneMicrophoneGainChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEMICROPHONE_GAINCHANGED] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONELAMP_MODECHANGED             (int)
      m_cachedMethodIDs[CACHEID_PHONELAMP_MODECHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneLampModeChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONELAMP_MODECHANGED] <= 0)
         LOOKUP_FAILURE() ;

      // CACHEID_PHONEHANDSET_VOLUMECHANGED        (int)
      m_cachedMethodIDs[CACHEID_PHONEHANDSET_VOLUMECHANGED] = pEnv->GetMethodID(
            classListener, 
            "phoneHandsetVolumeChanged",
            "(Lorg/sipfoundry/telephony/phone/event/PtComponentIntChangeEvent;)V") ;
      if (m_cachedMethodIDs[CACHEID_PHONEHANDSET_VOLUMECHANGED] <= 0)
         LOOKUP_FAILURE() ;

      pEnv->DeleteLocalRef(classListener) ;
   }
   else
   {
      LOOKUP_FAILURE() ;
   }
}

// Cleanup the listener by clearing all JNI caches
void PtTerminalComponentListenerContainer::cleanup(JNIEnv* pEnv)
{
   for (int i=0; i<CACHEID_MAX_INDEX; i++)
   {
      m_cachedMethodIDs[i] = 0 ;
   }

   if (m_classIntComponentEventClass != NULL)
   {
      pEnv->DeleteGlobalRef(m_classIntComponentEventClass) ;
      m_classIntComponentEventClass = NULL ;
   }

   if (m_classComponentEventClass != NULL)
   {
      pEnv->DeleteGlobalRef(m_classComponentEventClass) ;
      m_classComponentEventClass = NULL ;
   }
}
     

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


void PtTerminalComponentListenerContainer::fireComponentIntChangeEvent( const jobject jobjListener,
                                                                const char* szMethod,
                                                                const int iCacheID,
                                                                const PtComponentIntChangeEvent& rEvent)
{
   int iPriority = jniGetCurrentPriority() ;

#if DEBUG_FIRE_COMPONENT_LISTENERS
   if (IsJNIDebuggingEnabled())
      osPrintf("BEGIN fireComponentIntChangeEvent: %s\n", szMethod) ;
#endif   

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {
         if ((m_cachedMethodIDs[iCacheID] != NULL) && 
                  (m_classIntComponentEventClass != NULL))
         {
            // Get our data
            PtComponent* pComponentTemp = NULL ;
            PtTerminal*  pTerminal = new PtTerminal() ;
            int iOldValue = 0 ; 
            int iNewValue = 0 ; 

            PtStatus status1 = ((PtTerminalComponentEvent &) rEvent).getComponent(pComponentTemp) ;
             PtComponent* pComponent = createComponentCopy(pComponentTemp) ;            
            if (status1 != 0)
               API_FAILURE(status1) ;
               
            PtStatus status2 = ((PtTerminalEvent &) rEvent).getTerminal(*pTerminal) ;      
            if (status2 != 0)
               API_FAILURE(status2) ;
            
            jobject objEvent = pEnv->NewObject(m_classIntComponentEventClass, 
                     m_cachedMethodIDs[CACHEID_INTCOMPONENT_EVENT_INIT], 
                     (jlong) (int) pTerminal, (jlong) (int) pComponent, 
                     (jint) iOldValue, (jint) iNewValue) ;
            if (objEvent != NULL) 
            {
               pEnv->CallVoidMethod(jobjListener, m_cachedMethodIDs[iCacheID], objEvent) ;                     
               if (pEnv->ExceptionOccurred()) 
               {
                  pEnv->ExceptionDescribe() ;
                  pEnv->ExceptionClear() ;
               }
               pEnv->DeleteLocalRef(objEvent) ;
            }
            else
            {
               LOOKUP_FAILURE() ;
            }
         }
         else
         {
            LOOKUP_FAILURE() ;
         }
         jniReleaseVMReference(pVM) ;
      }      
   }
   jniResetPriority(iPriority) ;

#if DEBUG_FIRE_COMPONENT_LISTENERS
   if (IsJNIDebuggingEnabled())
      osPrintf("END   fireComponentIntChangeEvent: %s\n", szMethod) ;
#endif   

}


void PtTerminalComponentListenerContainer::fireComponentEvent(  const jobject jobjListener,
                                                          const char* szMethod,
                                                          const int iCacheID,
                                                          const PtTerminalComponentEvent& rEvent)
{
   int iPriority = jniGetCurrentPriority() ;

#if DEBUG_FIRE_COMPONENT_LISTENERS
   if (IsJNIDebuggingEnabled())
      osPrintf("BEGIN fireComponentIntChangeEvent: %s\n", szMethod) ;
#endif   

   if (jobjListener != NULL) 
   {
      JavaVM* pVM ;
      JNIEnv* pEnv ;
      
      if (jniGetVMReference(&pVM, &pEnv))
      {         
         if ((m_cachedMethodIDs[iCacheID] != NULL) && 
                  (m_classComponentEventClass != NULL))
         {
            // Get our data
            PtComponent* pComponentTemp = NULL ;
            PtTerminal*  pTerminal = new PtTerminal() ;

            PtStatus status1 = ((PtTerminalComponentEvent &) rEvent).getComponent(pComponentTemp) ;
             PtComponent* pComponent = createComponentCopy(pComponentTemp) ;            
            if (status1 != 0)
               API_FAILURE(status1) ;
               
            PtStatus status2 = ((PtTerminalEvent &) rEvent).getTerminal(*pTerminal) ;      
            if (status2 != 0)
               API_FAILURE(status2) ;
            
            jobject objEvent = pEnv->NewObject(m_classComponentEventClass, 
                     m_cachedMethodIDs[CACHEID_COMPONENT_EVENT_INIT], 
                     (jlong) (int) pTerminal, (jlong) (int) pComponent) ;
            if (objEvent != NULL) 
            {
               pEnv->CallVoidMethod(jobjListener, m_cachedMethodIDs[iCacheID], objEvent) ;
               if (pEnv->ExceptionOccurred()) 
               {
                  pEnv->ExceptionDescribe() ;
                  pEnv->ExceptionClear() ;
               }
               pEnv->DeleteLocalRef(objEvent) ;
            }
            else
            {
               LOOKUP_FAILURE() ;
            }
         }
         else
         {
            LOOKUP_FAILURE() ;
         }
         jniReleaseVMReference(pVM) ;
      }
   }
   jniResetPriority(iPriority) ;

#if DEBUG_FIRE_COMPONENT_LISTENERS
   if (IsJNIDebuggingEnabled())
      osPrintf("END  fireComponentEvent: %s\n", szMethod) ;
#endif
}


PtComponent* PtTerminalComponentListenerContainer::createComponentCopy(PtComponent *pOrig)
{
   PtComponent* pComponent = NULL ;
   int iType ;

   pOrig->getType(iType) ;

   switch (iType) {
      case PtComponent::BUTTON:
         pComponent = new PtPhoneButton(*((PtPhoneButton*)pOrig)) ;
         break ;
      case PtComponent::HOOKSWITCH:
         pComponent = new PtPhoneHookswitch(*((PtPhoneHookswitch*)pOrig)) ;
         break ;
      case PtComponent::LAMP:
         pComponent = new PtPhoneLamp(*((PtPhoneLamp*)pOrig)) ;
         break ;
      case PtComponent::MICROPHONE:
         pComponent = new PtPhoneMicrophone(*((PtPhoneMicrophone*)pOrig)) ;
         break ;
      case PtComponent::RINGER:
         pComponent = new PtPhoneRinger(*((PtPhoneRinger*)pOrig)) ;
         break ;
      case PtComponent::SPEAKER:
         pComponent = new PtPhoneSpeaker(*((PtPhoneSpeaker*)pOrig)) ;
         break ;
      case PtComponent::DISPLAY:
         pComponent = new PtPhoneDisplay(*((PtPhoneDisplay*)pOrig)) ;
         break ;
      default:
         osPrintf("JNI: getComponents: Unexpected component type: %d\n", iType) ;
   }

   return pComponent ;
}



void PtTerminalComponentListenerContainer::setEventFilter(long lEventFilter)
{
    mlEventFilter = lEventFilter ;

}
    

