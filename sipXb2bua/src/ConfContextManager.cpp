/** 
 * Copyright (c) 2006 BreakPoint Software, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are 
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer. 
 * 
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * 
 * Neither the name of the BreakPoint Software, Inc. nor the names of its 
 * contributors may be used to endorse or promote products derived from this 
 * software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author Bob Andreasen (rjandreasen gmail com)
 */

#include <assert.h>

#include "ConfContextManager.h"
#include "ConfContext.h"

#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"


/****************************************************************************/

ConfContextManager::ConfContextManager(int iMaxConfs)
    : mConfContextLock(OsMutex::Q_FIFO)
{
    assert(iMaxConfs > 0) ;
    if (iMaxConfs > 0)
        mMaxConfs = iMaxConfs ;
    else
        mMaxConfs = 32 ;

    mpConfContexts = new ConfContext[mMaxConfs] ;
    assert(mpConfContexts != NULL) ;
}

/****************************************************************************/

ConfContextManager::~ConfContextManager()
{
    delete [] mpConfContexts ;
}

/****************************************************************************/

ConfContext* 
ConfContextManager::allocConfContext(SIPX_CONF           hConf, 
                                     CallContextManager* pCallContextMgr)
{    
    OsLock lock(mConfContextLock) ;

    ConfContext* pRC = getConfContext(hConf) ;
    if (pRC == NULL)
    {
        pRC = getConfContext(SIPX_CONF_NULL) ;
        if (pRC)
        {
            pRC->setConfHandle(hConf) ;
            pRC->setCallContextManager(pCallContextMgr) ;
            printf("<-> Conf Context created for conf %d\n", hConf) ;
        }
    }
    return pRC ;
}

/****************************************************************************/

void ConfContextManager::freeConfContext(SIPX_CONF hConf) 
{
    OsLock lock(mConfContextLock) ;

    ConfContext* pRC = getConfContext(hConf) ;
    if (pRC != NULL && pRC->isInUse())
    {
        pRC->stopTone(SIPX_CALL_NULL) ;
        pRC->reset() ;
        printf("<-> Conf Context released for conf %d\n", hConf) ;
    }

    sipxConferenceDestroy(hConf) ;
}

/****************************************************************************/

ConfContext* ConfContextManager::getConfContext(SIPX_CONF hConf) 
{
    ConfContext* pRC = NULL ;
    OsLock lock(mConfContextLock) ;

    for (int i=0; i<mMaxConfs; i++)
    {
        if (mpConfContexts[i].getConfHandle() == hConf)
        {
            pRC = &mpConfContexts[i] ;
            break ;
        }
    }
    return pRC ;
}

/****************************************************************************/

ConfContext* ConfContextManager::getConfContextByCall(SIPX_CALL hCall) 
{
    ConfContext* pRC = NULL ;
    OsLock lock(mConfContextLock) ;

    for (int i=0; i<mMaxConfs; i++)
    {
        if (mpConfContexts[i].isInUse() && 
                mpConfContexts[i].containsCall(hCall))
        {
            pRC = &mpConfContexts[i] ;
            break ;
        }
    }
    return pRC ;
}

/****************************************************************************/

void ConfContextManager::handleMediaEvent(SIPX_MEDIA_INFO* pMediaInfo)
{
    ConfContext* pConfContext = NULL ;

    switch (pMediaInfo->event)
    {
    case MEDIA_REMOTE_DTMF:
        if (pConfContext = getConfContextByCall(pMediaInfo->hCall))
        {
            switch (pMediaInfo->cause)
            {
            case MEDIA_CAUSE_DTMF_START:
                pConfContext->startTone(pMediaInfo->hCall, pMediaInfo->toneId) ;
                break ;
            case MEDIA_CAUSE_DTMF_STOP:
                pConfContext->stopTone(pMediaInfo->hCall) ;
                break ;
            default:
                break ;
            }
        }
        break ;
    default:
        // Ignore for now
        break ;        
    }  
}

/****************************************************************************/

