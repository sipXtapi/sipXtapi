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

#include "tao/TaoReference.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoReference::TaoReference() : mLock(OsRWMutex::Q_FIFO)
{
        mRef = 0;
}

TaoReference::~TaoReference()
{
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned int TaoReference::add()
{
        unsigned int ref;

        mLock.acquireWrite();
        if (mRef >= DEF_TAO_VERY_BIG_NUMBER)
        {
                reset();
        }
        else
        {
                mRef++;
        }
        ref = mRef;
        mLock.releaseWrite();

        return ref;
}

TaoStatus TaoReference::reset()
{
        mLock.acquireWrite();
        mRef = 0;
        mLock.releaseWrite();

        return TAO_SUCCESS;
}

TaoStatus TaoReference::release()
{
        mLock.acquireWrite();
        if (--mRef < 0)
        {
                mRef = 0;
        }
        mLock.releaseWrite();

        return TAO_SUCCESS;
}
