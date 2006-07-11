//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
#ifndef _RingTone_h_
#define _RingTone_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
#define RING_STATE_QUIET    0
#define RING_STATE_RINGING  1
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
#include <os/OsTimer.h>
#include <utl/UtlString.h>


// Helper class for ring tone specific notification
class RingToneNotification : public OsNotification
{
public:
    RingToneNotification(void (*fn)());
    virtual ~RingToneNotification();

    OsStatus signal(const int eventData);

private:
    void (*mCallback)();
};


/**
 * RingTone generation
 */
class RingTonePlayer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    static RingTonePlayer *getRingTonePlayer();
    static void releaseRingTonePlayer();

    void startRinging();
    void stopRinging();

    void enableRinging(UtlBoolean bEnable);

    void setRingToneFile(char* sFileName);

    static void Callback();

    RingToneNotification mNotification;
    OsTimer mTimer;


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /**
    * RingTone contructor.
    */
    RingTonePlayer();

   /**
    * RingTone destructor.
    */
    virtual ~RingTonePlayer();

    static RingTonePlayer* mInstance;

    static UtlString       mRingToneFile;
    static int             mState;
};


#endif
