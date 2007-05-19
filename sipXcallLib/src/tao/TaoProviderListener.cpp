//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "os/OsConnectionSocket.h"
#include "tao/TaoProviderListener.h"
#include "tao/TaoTransportTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoProviderListener::TaoProviderListener(PtEventMask* pMask)
: PtProviderListener(pMask)
{
        mTerminalName = 0;
        mpConnectionSocket = 0;
}

TaoProviderListener::TaoProviderListener(int objId,
                                                                                 TaoObjHandle clientSocket,
                                                                                 TaoTransportTask* pSvrTransport,
                                                                                 const char * terminalName)
: PtProviderListener(NULL)
{
        mObjId = objId;
        mhClientSocket = clientSocket;
        mpSvrTransport = pSvrTransport;
        int len = strlen(terminalName);
        if (len > 0)
        {
                mTerminalName = new char[len + 1];
                strcpy(mTerminalName, terminalName);
        }
        else
        {
                mTerminalName = 0;
        }

        mpConnectionSocket = new OsConnectionSocket(DEF_TAO_EVENT_PORT, mTerminalName);

}

TaoProviderListener::~TaoProviderListener()
{
        if (mTerminalName)
        {
                delete[] mTerminalName;
                mTerminalName = 0;
        }

        if (mpConnectionSocket)
        {
                delete mpConnectionSocket;
                mpConnectionSocket = 0;
        }
}
