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

#include "B2BUA.h"

#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "os/OsTask.h"

#include "ConfContext.h"
#include "LineContextManager.h"

/****************************************************************************/

B2BUA* B2BUA::mpInstance = NULL ;

/****************************************************************************/

int main(int argc, char* argv[])

{
    B2BUA B2BUA ;

    if (B2BUA.initialize())
    {        
        // TODO:: Need a way to sync shutdown
        while (1)
        {            
            OsTask::delay(1000) ;
        }
    }

    return 0 ;
}

/****************************************************************************/

B2BUA::B2BUA()
    : mCallContextMgr(MAX_CALLS)
    , mConfContextMgr(MAX_CONFS)
    , mLineContextMgr(MAX_LINES + MAX_CALLS)
{
    mIdentityUrl = "\"B2BUA\" <B2BUA@localhost>" ;
    m_iStunPort = 3478 ;
    m_iSipUdpPort = 5060 ;
    m_iSipTcpPort = 5060 ;
    m_iRtpStartPort = 16384 ;
    m_iRegisterPeriodSecs = 50 ;
    m_logFile = "B2BUA.log" ;
    m_eStunState = STUN_STATE_UNKNOWN ;
    m_bInitialized = false ;

    mpInstance = this ;
    mhInst = NULL ;    
}

/****************************************************************************/

B2BUA::~B2BUA() 
{
    if (mhInst)
    {
        sipxUnInitialize(mhInst, true) ;
    }
}

/****************************************************************************/

bool B2BUA::initStun() 
{
    bool bSuccess = true ;

    if (m_stunServer.length() > 0)
    {
        sipxConfigEnableStun(mhInst, m_stunServer, m_iStunPort, 28) ;               

        // Wait for STUN events for upto 20 secs
        printf("Polling for STUN state ") ;
        int iTries = 0 ;
        while ((m_eStunState == STUN_STATE_UNKNOWN) && iTries < 40)
        {
            OsTask::delay(250) ;
            iTries++ ;
        }
        
        if (m_eStunState != STUN_STATE_SUCCESS)
        {
            printf(" [FAILED]\n") ;                    
            OsSysLog::add(FAC_APP, PRI_ERR, "Stun lookup via %s:%d failed",
                    m_stunServer.data(), m_iStunPort) ;
            bSuccess = false ;
        }
        else
        {
            printf(" [OK]\n") ;
        }
    }

    return bSuccess ;
}

/****************************************************************************/

bool B2BUA::initLines() 
{
    bool bSuccess = false ;
                
    /*
     * Init lines
     */
    printf("Polling for Line state ") ;
    mLineContextMgr.setInstance(mhInst) ;
    if (mLineContextMgr.initLines())
    {
        if (mLineContextMgr.waitForRegistrations())
        {
            bSuccess = true ;
        }        
    }

    if (bSuccess)
        printf(" [OK]\n") ;
    else
        printf(" [FAILED]\n") ;

    return bSuccess ;
}

/****************************************************************************/

bool B2BUA::initialize() 
{
    bool bSuccess = false ;

    char cVersion[80] ;
    sipxConfigGetVersion(cVersion, sizeof(cVersion)) ;
    printf("sipX B2BUA using %s\n", cVersion) ;
    printf("Copyright (c) 2006 Robert J. Andreasen, Jr.  All rights reserved.\n\n") ;

    if (loadConfiguration())
    {
        if (m_logFile.length() > 0)
        {
            // Enable logging
            sipxConfigSetLogLevel(LOG_LEVEL_INFO) ;
            sipxConfigSetLogFile(m_logFile);
        }

        if (sipxInitialize(&mhInst, m_iSipUdpPort, m_iSipTcpPort, PORT_NONE, 
                m_iRtpStartPort, MAX_CALLS, mIdentityUrl, "0.0.0.0") == SIPX_RESULT_SUCCESS)
        {
            bSuccess = true ;      

            sipxConfigEnableRport(mhInst, true) ;
            sipxConfigSetRegisterExpiration(mhInst, m_iRegisterPeriodSecs) ;

            sipxEventListenerAdd(mhInst, EventCallBack, NULL) ;

            if (m_outputProxy.length() > 0)
            {
                sipxConfigSetOutboundProxy(mhInst, m_outputProxy);
            }

            if (!initStun() || !initLines())
            {
                bSuccess = false ;
            }
            else
            {
                m_bInitialized = true ;
            }            
        }
    }

    return bSuccess ;
}

/****************************************************************************/

bool B2BUA::loadConfiguration()
{
    bool bRC = true ;
    UtlString value ;
    int       iValue ;
    OsConfigDb db ;

    if (db.loadFromFile("B2BUA.conf") == OS_SUCCESS)
    {
        /*
         * General Settings
         */
        if (db.get("OUTBOUND_PROXY", value) == OS_SUCCESS)
        {
            m_outputProxy = value ;
        }
        if (db.get("STUN_SERVER", value) == OS_SUCCESS)
        {
            m_stunServer = value ;
        }
        if (db.get("STUN_PORT", iValue) == OS_SUCCESS)
        {
            m_iStunPort = iValue ;
        }
        if (db.get("SIP_UDP_PORT", iValue) == OS_SUCCESS)
        {
            m_iSipUdpPort = iValue ;
        }
        if (db.get("SIP_TCP_PORT", iValue) == OS_SUCCESS)
        {
            m_iSipTcpPort = iValue ;
        }
        if (db.get("RTP_START_PORT", iValue) == OS_SUCCESS)
        {
            m_iRtpStartPort = iValue ;
        }
        if (db.get("LOG_FILE", value) == OS_SUCCESS)
        {
            m_logFile = value ;
        }
        if (db.get("REGISTER_PERIOD_SECS", iValue) == OS_SUCCESS)
        {
            m_iRegisterPeriodSecs = iValue ;
        }


        /*
         * Lines
         */
        if (db.get("NUM_LINES", iValue) == OS_SUCCESS)
        {
            m_iNumLines = iValue ;
        }

        if (m_iNumLines > 0)
        {
            int iAddIndex = 0 ;
            for (int i=1; i<=m_iNumLines; i++)
            {
                char      cKey[128] ;
                UtlString url ;           // Required
                UtlString alias ;         // Optional
                UtlString realm ;         // Optional
                UtlString username ;      // Optional
                UtlString password ;      // Optional
                UtlString mapUrl ;        // Required
                UtlString mapUrlAlias ;   // Optional
                UtlString lineSelect ;    // Optional
                int       iMaxCalls = 0 ; // Required

                sprintf(cKey, "LINE_URL.%d", i) ;
                db.get(cKey, url) ;

                sprintf(cKey, "LINE_ALIAS.%d", i) ;
                db.get(cKey, alias) ;

                sprintf(cKey, "LINE_REALM.%d", i) ;
                db.get(cKey, realm) ;

                sprintf(cKey, "LINE_USERNAME.%d", i) ;
                db.get(cKey, username) ;

                sprintf(cKey, "LINE_PASSWORD.%d", i) ;
                db.get(cKey, password) ;

                sprintf(cKey, "LINE_SELECT.%d", i) ;
                db.get(cKey, lineSelect) ;

                sprintf(cKey, "LINE_MAP_URL.%d", i) ;
                db.get(cKey, mapUrl) ;

                sprintf(cKey, "LINE_MAP_URL_ALIAS.%d", i) ;
                db.get(cKey, mapUrlAlias) ;

                sprintf(cKey, "LINE_MAX_CALLS.%d", i) ;
                db.get(cKey, iMaxCalls) ;
                
                if (url.isNull() || mapUrl.isNull() || iMaxCalls == 0)
                {
                    printf("ERROR: Invalid Line configuration for entry %d, missing required data\n", 
                            i) ;
                    bRC = false ;
                    break ;
                }
                else
                {
                    LineContext* pContext = new LineContext(url, 
                            alias, realm, username, password, mapUrl, 
                            mapUrlAlias, lineSelect, iMaxCalls) ;
                    assert(pContext) ;
                    if (pContext)
                    {
                        if (!mLineContextMgr.addLineContext(pContext))
                        {
                            OsSysLog::add(FAC_APP, PRI_ERR,
                                    "Unable to add line (max lines): %s",
                                    url.data()) ;
                            delete pContext ;
                        }
                    }
                }
            }
        }
        else
        {
            printf("ERROR: No lines are configured (param NUM_LINES)") ;

            bRC = false ;
        }
    }
    else
    {
        db.set("OUTBOUND_PROXY", m_outputProxy) ;
        db.set("STUN_SERVER", m_stunServer) ;
        db.set("STUN_PORT", m_iStunPort) ;
        db.set("SIP_UDP_PORT", m_iSipUdpPort) ;
        db.set("SIP_TCP_PORT", m_iSipTcpPort) ;
        db.set("RTP_START_PORT", m_iRtpStartPort) ;
        db.set("LOG_FILE", m_logFile) ;
        db.set("REGISTER_PERIOD_SECS", m_iRegisterPeriodSecs) ;
        db.set("NUM_LINES", 0) ;

        db.storeToFile("B2BUA.conf") ;

        bRC = false ;
    }
    return bRC ;
}

/****************************************************************************/

void B2BUA::handleLineEvent(SIPX_LINESTATE_INFO* pLineInfo) 
{
    mLineContextMgr.handleLineEvent(pLineInfo) ;
}

/****************************************************************************/

void B2BUA::handleConfigEvent(SIPX_CONFIG_INFO* pConfigInfo) 
{
    SIPX_CONTACT_ADDRESS contacts[16] ;
    size_t nContacts ;

    switch (pConfigInfo->event)
    {
        case CONFIG_STUN_SUCCESS:
            m_eStunState = STUN_STATE_SUCCESS ;
            break ;
        case CONFIG_STUN_FAILURE:
            m_eStunState = STUN_STATE_FAILURE ;
            break ;
        case CONFIG_UNKNOWN:
        default:
            // Ignore for now
            break ;
    }

    SIPX_RESULT status = sipxConfigGetLocalContacts(mhInst, contacts, 10, nContacts) ;
    if (status == SIPX_RESULT_SUCCESS)
    {
        for (size_t i = 0; i<nContacts; i++)
        {
            const char* szTransport = "UNKNOWN" ;
            switch (contacts[i].eTransportType)
            {
                case TRANSPORT_UDP:
                    szTransport = "UDP" ;
                    break ;
                case TRANSPORT_TCP:
                    szTransport = "TCP" ;
                    break ;
                case TRANSPORT_TLS:
                    szTransport = "TLS" ;
                    break ;
                default:
                    assert(false);
                    break ;
            }
            const char* szType = "UNKNOWN" ;
            switch (contacts[i].eContactType)
            {               
                case CONTACT_LOCAL:
                    szType = "LOCAL" ;
                    break ;
                case CONTACT_NAT_MAPPED:
                    szType = "NAT_MAPPED" ;
                    break ;
                case CONTACT_RELAY:
                    szType = "RELAY" ;
                    break ;
                case CONTACT_CONFIG:
                    szType = "CONFIG" ;
                    break ;
                default:
                    assert(false) ;
                    break ;
            }

            OsSysLog::add(FAC_APP, PRI_INFO, 
                    "Type %s, Interface: %s, Ip %s (%s), Port %d",
                    szType, contacts[i].cInterface, contacts[i].cIpAddress,
                    szTransport, contacts[i].iPort) ;
        }
    }
    else
    {
        OsSysLog::add(FAC_APP, PRI_ERR, 
                "Unable to query local contact addresses") ;
    }
}

/****************************************************************************/

void B2BUA::rejectCall(SIPX_CALLSTATE_INFO* pCallInfo,
                       int                  iCause,
                       const char*          szCause)
{
    SIPX_RESULT rc ;

    OsSysLog::add(FAC_APP, PRI_INFO, "Call rejected hCall=%d  %d:%s",
            pCallInfo->hCall, iCause, szCause) ;

    rc = sipxCallReject(pCallInfo->hCall, iCause, szCause) ;
    assert(rc == SIPX_RESULT_SUCCESS) ;
}

/****************************************************************************/

void B2BUA::acceptCall(SIPX_CALLSTATE_INFO* pCallInfo)
{
    CallContext* pCallContext ;
    SIPX_RESULT rc ;
   
    OsSysLog::add(FAC_APP, PRI_INFO, "Call accepted hCall=%d  %d:%s",
            pCallInfo->hCall) ;

    pCallContext = mCallContextMgr.allocCallContext(pCallInfo->hCall, 
            CALL_CONTEXT_INBOUND) ;
    if (pCallContext)
        rc = sipxCallAccept(pCallInfo->hCall, false) ;
    else
        rejectCall(pCallInfo, 500, "too many users") ;

    assert(rc == SIPX_RESULT_SUCCESS) ;
}

/****************************************************************************/

void B2BUA::handleCallEvent(SIPX_CALLSTATE_INFO* pCallInfo) 
{
    LineContext* pLineContext = NULL ;
    CallContext* pCallContext = NULL ;
    SIPX_RESULT rc ;

    switch (pCallInfo->event)
    {
        case CALLSTATE_OFFERING:
            if (!m_bInitialized)
            {
                OsSysLog::add(FAC_APP, PRI_ERR, 
                        "received call during startup hCall=%d: %s",
                        pCallInfo->hCall) ;

                rejectCall(pCallInfo, 500, "starting up; callback later") ;
            }
            else if ((pLineContext = mLineContextMgr.getLineContext(pCallInfo)) != NULL)
            {
                // Maintain call accounting (needed to limit max number of calls)
                if (pLineContext->canAddCall())
                {
                    pLineContext->addActiveCall(pCallInfo->hCall) ;
                    acceptCall(pCallInfo) ;
                }
                else
                {
                    rejectCall(pCallInfo, 404, "line busy") ;
                }
            }
            else
            {                                        
                rejectCall(pCallInfo, 404, "Unknown identity") ;
            }
            break ;
        case CALLSTATE_ALERTING:
            pCallContext = mCallContextMgr.getCallContext(pCallInfo->hCall) ;
            if (pCallContext)
            {
                SIPX_CONF hConf ;
                rc = sipxConferenceCreate(mhInst, &hConf) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
                rc = sipxConferenceJoin(hConf, pCallInfo->hCall) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
                mConfContextMgr.allocConfContext(hConf, &mCallContextMgr) ;
                rc = sipxCallAnswer(pCallInfo->hCall) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
            }
            else
            {
                OsSysLog::add(FAC_APP, PRI_ERR, 
                        "dropping unknown call hCall=%d: CALLSTATE_ALERTING",
                        pCallInfo->hCall) ;

                SIPX_CALL hDestroy = pCallInfo->hCall ;            
                rc = sipxCallDestroy(hDestroy) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
            }
            break ;

        case CALLSTATE_CONNECTED:
        case CALLSTATE_BRIDGED:
            pLineContext = mLineContextMgr.getLineContext(pCallInfo) ;
            pCallContext = mCallContextMgr.getCallContext(pCallInfo->hCall) ;
            if (pCallContext && pLineContext)
            {
                if (!pCallContext->isConnected())
                {
					pCallContext->setConnected(true) ;
                    if (pCallContext->getOrigin() == CALL_CONTEXT_INBOUND)
                    {
                        ConfContext* pConfContext = 
                                mConfContextMgr.getConfContextByCall(pCallInfo->hCall) ;

                        if (pConfContext)
                        {
                            pConfContext->startRingback() ;

                            SIPX_CONTACT_ID contactId ;
                            SIPX_LINE forwardLine ;
                            UtlString forwardingUrl ;
                            pLineContext->getForwardingUrl(pCallInfo, forwardingUrl, forwardLine) ;
                            if (forwardLine == SIPX_LINE_NULL)
                            {
                                char szFromUrl[256] ;
                                sipxCallGetRemoteID(pCallInfo->hCall, szFromUrl, sizeof(szFromUrl)) ;
                                if (sipxLineFindByURI(mhInst, szFromUrl, forwardLine) == SIPX_RESULT_SUCCESS)
                                {
                                    // mLineContextMgr.addCallRef(forwardLine) ;
                                }
                                else
                                {

                                }
                            }

                            printf("<--> Forwarding URL: %s\n", forwardingUrl.data()) ;

                            sipxLineGetContactId(forwardLine, contactId) ;  

                            OsSysLog::add(FAC_APP, PRI_INFO, 
                                    "forwarding hCall=%d to %s with contactId=%d",
                                    pCallInfo->hCall, forwardingUrl.data(), contactId) ;

                            SIPX_CALL hNewCall ;
                            rc = sipxConferenceAdd(
                                    pConfContext->getConfHandle(),
                                    forwardLine,
                                    forwardingUrl,
                                    &hNewCall,
                                    contactId,
                                    NULL,
                                    NULL) ;

                            assert(rc == SIPX_RESULT_SUCCESS) ;
                            mCallContextMgr.allocCallContext(hNewCall, CALL_CONTEXT_OUTBOUND) ;
                            //TODO FAILURE CASE
                        }
                    } 
                    else if (pCallContext->getOrigin() == CALL_CONTEXT_OUTBOUND)
                    {
                        ConfContext* pConfContext = 
                                mConfContextMgr.getConfContextByCall(pCallInfo->hCall) ;

                        if (pConfContext)
                        {
                            pConfContext->stopRingback() ;
                        }        
                    }
                }
            }
            else
            {
                OsSysLog::add(FAC_APP, PRI_ERR, 
                        "dropping unknown call hCall=%d: CALLSTATE_CONNECTED",
                        pCallInfo->hCall) ;

                SIPX_CALL hDestroy = pCallInfo->hCall ;
                rc = sipxCallDestroy(hDestroy) ;
                assert(rc == SIPX_RESULT_SUCCESS) ;
            }
            break ;

        case CALLSTATE_DISCONNECTED:
            {
                SIPX_CONF hConf = SIPX_CONF_NULL ;
                SIPX_CALL hDestroy = pCallInfo->hCall ;
                if (sipxCallGetConference(pCallInfo->hCall, hConf) == 
                        SIPX_RESULT_SUCCESS)
                {
                    ConfContext* pConfContext = mConfContextMgr.getConfContext(hConf) ;
                    if (pConfContext)
                    {
                        CallContext* pCallContext = 
                                mCallContextMgr.getCallContext(pCallInfo->hCall) ;
                        if ((pCallContext->getOrigin() == CALL_CONTEXT_OUTBOUND) &&
                            !pCallContext->isConnected())
                        {                           
                            sipxCallDestroy(hDestroy) ;
                            pConfContext->startBusy() ;
                        }
                        else
                        {
                            sipxCallDestroy(hDestroy) ;
                            if (pCallContext->getOrigin() == CALL_CONTEXT_INBOUND ||
                                pConfContext->getNumCalls() < 2)
                            {
                                mConfContextMgr.freeConfContext(hConf) ;
                            }
                        }
                    }
                    else
                    {
                        sipxCallDestroy(hDestroy) ;
                    }
                }
                else
                {
                    sipxCallDestroy(hDestroy) ;
                }

                // Maintain call accounting (needed to limit max number of calls)
                mLineContextMgr.handleCallDestroyed(pCallInfo->hCall) ;
            }
            break ;

        case CALLSTATE_DESTROYED:
            mCallContextMgr.freeCallContext(pCallInfo->hCall) ;
            break ;

        case CALLSTATE_NEWCALL:
            if (pCallInfo->cause == CALLSTATE_CAUSE_TRANSFER)
            {
                mCallContextMgr.allocCallContext(pCallInfo->hCall, CALL_CONTEXT_OUTBOUND) ;
            }
            break ;

        case CALLSTATE_REMOTE_ALERTING:
            if (pCallInfo->cause == CALLSTATE_CAUSE_EARLY_MEDIA)
            {
                SIPX_CONF hConf = SIPX_CONF_NULL ;
                if (sipxCallGetConference(pCallInfo->hCall, hConf) == 
                        SIPX_RESULT_SUCCESS)
                {
                    ConfContext* pConfContext = mConfContextMgr.getConfContext(hConf) ;
                    if (pConfContext)
                    {
                        pConfContext->stopRingback() ;
                    }
                }
            }
            break ;

        case CALLSTATE_UNKNOWN:
	    case CALLSTATE_DIALTONE:
	    case CALLSTATE_REMOTE_OFFERING:	    
        case CALLSTATE_HELD:
        case CALLSTATE_REMOTE_HELD:
        case CALLSTATE_TRANSFER_EVENT:
            // Ignore for now
            break ;
    }
}

/****************************************************************************/

void B2BUA::handleMediaEvent(SIPX_MEDIA_INFO* pMediaInfo) 
{    
    mConfContextMgr.handleMediaEvent(pMediaInfo) ;
}

/****************************************************************************/

bool SIPX_CALLING_CONVENTION B2BUA::EventCallBack(SIPX_EVENT_CATEGORY category, 
                                                  void*               pInfo, 
                                                  void*               pUserData)
{
    if (mpInstance)
    {
        switch (category)
        {
            case EVENT_CATEGORY_CALLSTATE:
                mpInstance->handleCallEvent((SIPX_CALLSTATE_INFO*)pInfo) ;
                break ;
            case EVENT_CATEGORY_LINESTATE:
                mpInstance->handleLineEvent((SIPX_LINESTATE_INFO*) pInfo) ;
                break ;
            case EVENT_CATEGORY_CONFIG:            
                mpInstance->handleConfigEvent((SIPX_CONFIG_INFO*)pInfo) ;
                break ;
            case EVENT_CATEGORY_MEDIA:
                mpInstance->handleMediaEvent((SIPX_MEDIA_INFO*)pInfo) ;
                break ;
            case EVENT_CATEGORY_INFO_STATUS:
            case EVENT_CATEGORY_INFO:
            case EVENT_CATEGORY_SUB_STATUS:
            case EVENT_CATEGORY_NOTIFY:
            case EVENT_CATEGORY_SECURITY:
            default:
                break;
        }
    }

    return true ;
}

/****************************************************************************/

#if !defined(_WIN32)
// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.
void JNI_LightButton(long)
{

}
#endif /* !defined(_WIN32) */

/****************************************************************************/

