//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ps/linux/PsHookswDevLinux.h"
#include "ps/PsHookswTask.h"
#include "ps/PsMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
int PsHookswDevLinux::sHookSwitchState = PsHookswTask::ON_HOOK;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor.
// Initialize the interrupt controller for hookswitch interrupt handling
PsHookswDevLinux::PsHookswDevLinux(PsHookswTask* pHookswTask)
:  PsHookswDev(pHookswTask)
{
   // disable interrupt
   disableIntr();
}

// Destructor
PsHookswDevLinux::~PsHookswDevLinux()
{
   // disable the interrupt
   disableIntr();
}

/* ============================ MANIPULATORS ============================== */

// Disable hook switch interrupts
void PsHookswDevLinux::disableIntr(void)
{
}

// Enable hook switch interrupts
void PsHookswDevLinux::enableIntr(UtlBoolean lookForOffHook)
{
}

/* ============================ ACCESSORS ================================= */
void PsHookswDevLinux::setHookState(int hookState)
{
        sHookSwitchState = hookState;
}
/* ============================ INQUIRY =================================== */

// Return TRUE if the hookswitch is "off hook", otherwise FALSE.
UtlBoolean PsHookswDevLinux::isOffHook(void)
{
        return (sHookSwitchState == PsHookswTask::OFF_HOOK );
}

// Return TRUE if the hookswitch is "on hook", otherwise FALSE.
UtlBoolean PsHookswDevLinux::isOnHook(void)
{
        return (sHookSwitchState == PsHookswTask::ON_HOOK );
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
