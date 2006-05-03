//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _LinePresenceBase_h_
#define _LinePresenceBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;

/**
 * The LinePresenceBase class provides a set of pure virtual interfaces which
 * allow for the setting and retrieval line presence and state information.
 */
class LinePresenceBase {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/**
 * Line Presence state type enumerations.  These are used collectively to
 * represent the possible states of PRESENT, (NOT)PRESENT, ON_HOOK
 * and (NOT)ON_HOOK.
 */
   enum ePresenceStateType {
      PRESENT     = 1,     /**< present state */
      ON_HOOK     = 2,     /**< on/off hook state */
      SIGNED_IN   = 4      /**< ACD SIGNED-IN state */
   };


/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   LinePresenceBase(void) {}

   /**
    * Destructor
    */
   virtual ~LinePresenceBase() {}

/* ============================ MANIPULATORS ============================== */
   /**
    * Update the presence / line state for this line.
    *
    * @param type The type of presence state to be updated.
    * 
    * @param state The state value to be updated.
    */
   virtual void updateState(ePresenceStateType type, bool state) = 0;

/* ============================ ACCESSORS ================================= */
   /**
    * Return the AOR for this line.
    */
   virtual Url* getUri(void) = 0;

   /**
    * Return the presence / line state information for this line.
    */
   virtual bool getState(ePresenceStateType type) = 0;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif  // _LinePresenceBase_h_

