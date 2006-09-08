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

#include "ConfContext.h"
#include "CallContextManager.h"

#include "os/OsSysLog.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "RingbackTone.h"
#include "BusyTone.h"

/****************************************************************************/

ConfContext::ConfContext() 
{
	reset() ;
}

/****************************************************************************/

void ConfContext::setCallContextManager(CallContextManager* pCallContextMgr)
{
    mpCallContextMgr = pCallContextMgr ;
}

/****************************************************************************/

void ConfContext::setConfHandle(SIPX_CONF hConf)
{
	assert(m_hConf == SIPX_CONF_NULL) ;
	m_hConf = hConf ;
    m_bInUse = true ;
}

/****************************************************************************/

SIPX_CONF ConfContext::getConfHandle() 
{
	return m_hConf ;
}

/****************************************************************************/

void ConfContext::reset()
{   
    m_hConf = SIPX_CONF_NULL ;
    m_bInUse = false ;
}

/****************************************************************************/

CallContext* ConfContext::getInboundCall()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;
    CallContext* pRC = NULL ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            CallContext* pCheck = 
                    mpCallContextMgr->getCallContext(hCalls[i]) ;
            if (pCheck && pCheck->getOrigin() == CALL_CONTEXT_INBOUND)
            {
                pRC = pCheck ;
                break ;
            }
        }
    }

    return pRC ;
}

/****************************************************************************/

CallContext* ConfContext::getOutboundCall()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;
    CallContext* pRC = NULL ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            CallContext* pCheck = 
                    mpCallContextMgr->getCallContext(hCalls[i]) ;
            if (pCheck && pCheck->getOrigin() == CALL_CONTEXT_OUTBOUND)
            {
                pRC = pCheck ;
                break ;
            }
        }
    }
    return pRC ;      
}

/****************************************************************************/

bool ConfContext::containsCall(SIPX_CALL hCall)
{
    bool      bRC = false ;
    SIPX_CALL hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t    nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            if (hCalls[i] == hCall)
            {
                bRC = true ;
                break ;
            }
        }
    }

    return bRC ;      
}


/****************************************************************************/

size_t ConfContext::getNumCalls()
{
    bool      bRC = false ;
    SIPX_CALL hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t    nCalls = 0 ;
    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) != SIPX_RESULT_SUCCESS)
    {
        nCalls = 0 ;
    }

    return nCalls ;
}


/****************************************************************************/

void ConfContext::startTone(SIPX_CALL hOriginatingCall, SIPX_TONE_ID toneId)
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            if (hCalls[i] != hOriginatingCall)
            {
                sipxCallStartTone(hCalls[i], toneId, false, true) ;
            }
        }
    }
}

/****************************************************************************/

void ConfContext::stopTone(SIPX_CALL hOriginatingCall)
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            if (hCalls[i] != hOriginatingCall)
            {
                sipxCallStopTone(hCalls[i]) ;
            }
        }
    }       
}


/****************************************************************************/

void ConfContext::startRingback()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            sipxCallPlayBufferStart(hCalls[i], 
                    (const char*) g_ringbackTone, 
                    sizeof(g_ringbackTone), 
                    RAW_PCM_16, 
                    true, 
                    false, 
                    true) ;
        }
    }
}

/****************************************************************************/

void ConfContext::stopRingback()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            sipxCallPlayBufferStop(hCalls[i]) ;
        }
    }
}

/****************************************************************************/

void ConfContext::startBusy()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            sipxCallPlayBufferStart(hCalls[i], 
                    (const char*) g_busyTone, 
                    sizeof(g_busyTone), 
                    RAW_PCM_16, 
                    true, 
                    false, 
                    true) ;
        }
    }
}

/****************************************************************************/

void ConfContext::stopBusy()
{
    SIPX_CALL    hCalls[MAX_CONF_PARTICIPANTS] ;
    size_t       nCalls = 0 ;

    if (sipxConferenceGetCalls(m_hConf, hCalls, MAX_CONF_PARTICIPANTS, 
            nCalls) == SIPX_RESULT_SUCCESS)
    {
        for (size_t i=0; i<nCalls; i++)
        {
            sipxCallPlayBufferStop(hCalls[i]) ;
        }
    }
}

/****************************************************************************/

bool ConfContext::isInUse()
{
    return m_bInUse ;
}

/****************************************************************************/

