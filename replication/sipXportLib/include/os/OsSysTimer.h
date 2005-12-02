//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsSysTimer_h_
#define _OsSysTimer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;

//:Base class for low-level, OS-dependent timer management

class OsSysTimerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsSysTimerBase();
     //:Default constructor

   virtual
   ~OsSysTimerBase();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static void showTimerInfo(void);
     //:Display information about the active timers in the system.

   virtual void startTimer(OsTimer* pTimer) = 0;
     //:Start a low-level system timer

   virtual void stopTimer(OsTimer* pTimer) = 0;
     //:Stop and release a low-level system timer

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSysTimerBase(const OsSysTimerBase& rOsSysTimerBase);
     //:Copy constructor (not implemented for this class)

   OsSysTimerBase& operator=(const OsSysTimerBase& rhs);
     //:Assignment operator (not implemented for this class)
};

/* ============================ INLINE METHODS ============================ */

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsSysTimer)
// with the OS-dependent realization of that type (e.g., OsSysTimerWnt).
#if defined(_WIN32)
#  include "os/shared/OsSysSoftTimer.h"
   typedef class OsSysSoftTimer OsSysTimer;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsSysTimerVxw.h"
   typedef class OsSysTimerVxw OsSysTimer;
#elif defined(__pingtel_on_posix__)
#  include "os/shared/OsSysSoftTimer.h"
   typedef class OsSysSoftTimer OsSysTimer;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsSysTimer_h_

