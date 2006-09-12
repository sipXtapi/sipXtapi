// $Id$
//
// Copyright (C) 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatSocketBaseImpl_h_
#define _OsNatSocketBaseImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/IStunSocket.h"

class OsNatSocketBaseImpl : public IStunSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
protected:
    OsSocket* mpSocket;

};

/* ============================ INLINE METHODS ============================ */


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */



#endif  // _OsNatSocketBaseImpl_h_

