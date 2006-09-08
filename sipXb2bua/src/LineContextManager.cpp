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

#include "LineContextManager.h"
#include "os/OsSysLog.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsTask.h"
#include "utl/UtlHashBag.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

/****************************************************************************/

LineContextManager::LineContextManager(int iMaxLines)
    : m_lineContextLock(OsMutex::Q_FIFO)
{
    m_hInst = NULL ;

    assert(iMaxLines > 0) ;
    if (iMaxLines > 0)
        m_iMaxLines = iMaxLines ;
    else
        m_iMaxLines = 32 ;

    m_pLineContexts = new LineContext*[m_iMaxLines] ;
    assert(m_pLineContexts != NULL) ;
    for (int i=0; i<m_iMaxLines; i++)
    {
        m_pLineContexts[i] = NULL ;
    }       
}

/****************************************************************************/

LineContextManager::~LineContextManager()
{
    for (int i=0; i<m_iMaxLines; i++)
    {
        if (m_pLineContexts[i] != NULL)
        {
            delete m_pLineContexts[i] ;
            m_pLineContexts[i] = NULL ;
        }
    }
    delete [] m_pLineContexts ;
}

/****************************************************************************/

void LineContextManager::setInstance(SIPX_INST hInst)
{
    m_hInst = hInst ;
}

/****************************************************************************/

bool LineContextManager::initLines()
{
    bool bRC = true ;
    for (int i=0; i<m_iMaxLines; i++)
    {
        if (m_pLineContexts[i] && !m_pLineContexts[i]->initialize(m_hInst))
        {
            bRC = false ;
            break ;
        }
    }
    return bRC ;
}

/****************************************************************************/

bool LineContextManager::waitForRegistrations()
{
    bool bSuccess = true ;
    int iTries = 0 ;
    bool bDone = false ;

    while (!bDone && iTries < 80)
    {
        bDone = true ;
        bSuccess = true ;
        for (int i=0; i<m_iMaxLines; i++)
        {
            if (m_pLineContexts[i])
            {
                LINE_CONTEXT_STATE eState = m_pLineContexts[i]->
                        getLineContextState() ;
                if (eState == LCS_REGISTERING)
                {
                    bDone = false ;
                }
            }
        }

        iTries++ ;
        if (!bDone)
        {
            OsTask::delay(250) ;
        }
    }

    for (int i=0; i<m_iMaxLines; i++)
    {
        if (m_pLineContexts[i])
        {
            LINE_CONTEXT_STATE eState = m_pLineContexts[i]->
                    getLineContextState() ;
            if (eState == LCS_REGISTERING) 
            {
                bSuccess = false ;
                OsSysLog::add(FAC_APP, PRI_ERR, 
                        "Line %d failed to register (timeout)", i+1) ;
            }
            else if (eState == LCS_FAILED)
            {
                bSuccess = false ;
                OsSysLog::add(FAC_APP, PRI_ERR, 
                        "Line %d failed to register (check logs)", i+1) ;
            }
        }
    }

    return bSuccess ;
}

/****************************************************************************/

bool LineContextManager::addLineContext(LineContext* pContext) 
{
    OsLock lock(m_lineContextLock) ;
    bool bRC = false ;

    for (int i=0;i<m_iMaxLines; i++)
    {
        if (m_pLineContexts[i] == NULL)
        {
            m_pLineContexts[i] = pContext ;
            bRC = true ;
            break ;
        }
    }

    return bRC ;
}

/****************************************************************************/

bool LineContextManager::removeLineContext(LineContext* pContext) 
{
    OsLock lock(m_lineContextLock) ;

    bool bRC = false ;
    for (int i=0;i<m_iMaxLines; i++)
    {
        if (m_pLineContexts[i] == pContext)
        {
            m_pLineContexts[i] = NULL ;
            bRC = true ;
            break ;
        }
    }
    return bRC ;
}

/****************************************************************************/

LineContext* LineContextManager::getLineContext(SIPX_LINE hLine) 
{
    LineContext* pRC = NULL ;

    if (hLine != SIPX_LINE_NULL)
    {
        for (int i=0; i<m_iMaxLines; i++)
        {
            if (m_pLineContexts[i] && m_pLineContexts[i]->
                    isHandlingLine(hLine))
            {
                pRC = m_pLineContexts[i] ;
                break ;
            }
        }
    }
    return pRC ;
}

/****************************************************************************/

LINE_CONTEXT_STATE LineContextManager::getLineContextState(SIPX_LINE hLine)
{
    LINE_CONTEXT_STATE rc = LCS_UNKNOWN ;

    OsLock lock(m_lineContextLock) ;
    LineContext* pContext = getLineContext(hLine) ;
    if (pContext)
    {
        rc = pContext->getLineContextState() ;
    }

    return rc ;
}

/****************************************************************************/

LineContext* 
LineContextManager::getLineContext(SIPX_CALLSTATE_INFO* pCallInfo) 
{
    // 1. Search for a direct map on the service provide side
    // 2. Search for LINE_SELECT on the PBX side (shared-mode)
    // 3. Search for direct map PBX side match (DID-mode)

    LineContext* pRC = NULL ;
    SIPX_LINE hLine = pCallInfo->hLine ;

    if (hLine != SIPX_LINE_NULL)
    {
        // 1. Direct Map
        pRC = getLineContext(hLine) ;
    }
    else
    {
        // 2. Line Select on to:
        char cLocalAddress[128] ;
        char cLineSelect[64] ;
        sipxCallGetLocalID(pCallInfo->hCall, cLocalAddress, 
                    sizeof(cLocalAddress)) ;
        if (sipxUtilUrlGetUrlParam(cLocalAddress, "lineSelect", 0, 
                cLineSelect, 
                sizeof(cLineSelect)) == SIPX_RESULT_SUCCESS)
        {
            for (int i=0; i<m_iMaxLines; i++)
            {
                if (m_pLineContexts[i] && 
                        m_pLineContexts[i]->isHandlingLine(cLineSelect))
                {
                    pRC = m_pLineContexts[i] ;
                    break ;
                }
            }

            if (pRC == NULL)
            {
                OsSysLog::add(FAC_APP, PRI_ERR,
                        "Unable to match lineSelect: %s", cLineSelect) ;
            }
        }
        else
        {
            // 3. DID map on From
            char cRemoteId[128] ;
            if (sipxCallGetRemoteID(pCallInfo->hCall, cRemoteId, sizeof(cRemoteId)) 
                    == SIPX_RESULT_SUCCESS) 
            {
                if (sipxLineFindByURI(m_hInst, cRemoteId, hLine) == 
                        SIPX_RESULT_SUCCESS)
                {
                    pRC = getLineContext(hLine) ;
                }
            }
        }
    }

    return pRC ;
}

/****************************************************************************/

void LineContextManager::handleCallDestroyed(SIPX_CALL hCall)
{
    OsLock lock(m_lineContextLock) ;

    for (int i=0;i<m_iMaxLines; i++)
    {
        if (    (m_pLineContexts[i] != NULL) && 
                m_pLineContexts[i]->isActiveCall(hCall))
        {
            m_pLineContexts[i]->removeActiveCall(hCall) ;
            break ;
        }
    }
}

/****************************************************************************/

void LineContextManager::handleLineEvent(SIPX_LINESTATE_INFO* pLineInfo) 
{
    LineContext* pContext = getLineContext(pLineInfo->hLine) ;
    if (pContext)
    {
        pContext->handleLineEvent(pLineInfo) ;
    }
}

/****************************************************************************/

