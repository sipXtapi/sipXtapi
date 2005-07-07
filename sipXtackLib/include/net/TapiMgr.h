// $Id$
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef SIPXTAPI_EXCLUDE
#ifndef _TapiMgr_h
#define _TapiMgr_h

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The TapiMgr singleton class allows
 * callback funtion-pointers for tapi to be set, 
 * indicating the function that is to be called for
 * the firing of sipXtapi events.
 * This obviates the need for sipXcallLib and 
 * sipXtackLib to link-in the sipXtapi library.
 */
class TapiMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /**
    * Accessor for the single class instance.
    */
    static TapiMgr& getInstance();

   /**
    * Sets the callback function pointer for Call Events (to be fired to the sipXtapi layer)
    */
    void setTapiCallCallback(sipxCallEventCallbackFn fp);
    
   /**
    * Sets the callback function pointer for Line Events (to be fired to the sipXtapi layer)
    */
    void setTapiLineCallback(sipxLineEventCallbackFn fp);
    
    /** 
     * This method calls the Call event callback using the function pointer.
     */    
    void fireCallEvent(const void*          pSrc,
                       const char*		    szCallId,
                       SipSession*          pSession,
				       const char*          szRemoteAddress,                   
				       SIPX_CALLSTATE_MAJOR eMajorState,
				       SIPX_CALLSTATE_MINOR eMinorState) ;
    
    /** 
     * This method calls the Line event callback using the function pointer.
     */    
    void fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINE_EVENT_TYPE_MAJOR major);		
    

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ CREATORS ================================== */
    /**
    * TapiMgr contructor. singleton class.
    */
    TapiMgr();

    /**
    * Copy constructor - should never be used.
    */
    TapiMgr(const TapiMgr& src);


    /**
    * TapiMgr destructor.
    */
    virtual ~TapiMgr();

    /**
     * Private, static pointer, holding on to the singleton class instance.
     */
    static TapiMgr* spTapiMgr;

    /**
     * function pointer for the Call Event callback.
     */
    sipxCallEventCallbackFn sipxCallEventCallbackPtr;

    /**
     * function pointer for the Line Event callback.
     */
    sipxLineEventCallbackFn sipxLineEventCallbackPtr;

};

#endif /* ifndef _TapiMgr_h_ */
#endif /* ifndef SIPXTAPI_EXCLUDE */
