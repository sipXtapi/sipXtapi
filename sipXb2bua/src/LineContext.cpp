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

#include "LineContext.h"
#include "utl/UtlInt.h"
#include "os/OsSysLog.h"

/****************************************************************************/

LineContext::LineContext(UtlString url,
                         UtlString alias,
                         UtlString realm,
                         UtlString username,
                         UtlString password,
                         UtlString mapUrl,
                         UtlString mapUrlAlias,
                         UtlString lineSelect,
                         int       iMaxCalls)
{
    m_hInst = NULL ;
    m_url = url ;        
    m_alias = alias ;
    m_realm = realm ;
    m_username = username ;
    m_password = password ;
    m_mapUrl = mapUrl ;
    m_mapUrlAlias = mapUrlAlias ;  
    m_iMaxCalls = iMaxCalls ;
    m_lineSelect = lineSelect ;

    m_eLineContextState = LCS_UNKNOWN ;
    m_hLineUrl = SIPX_LINE_NULL ;
    m_hLineMapUrl = SIPX_LINE_NULL ;
}

/****************************************************************************/

LineContext::~LineContext() 
{

}

/****************************************************************************/

bool LineContext::initialize(SIPX_INST hInst) 
{
    bool bRC = false ;
    SIPX_RESULT sipxRC ;
    SIPX_CONTACT_ID registerContactId = CONTACT_AUTO ;
    SIPX_CONTACT_ID provisionContactId = CONTACT_AUTO ;

    // Store hInst
    m_hInst = hInst ;

    assert(m_hLineUrl == SIPX_LINE_NULL) ;
    assert(m_hLineMapUrl == SIPX_LINE_NULL) ;
    assert(m_hInst != NULL) ;
    
    if ((m_hLineUrl == SIPX_LINE_NULL) && (m_hLineMapUrl == SIPX_LINE_NULL))
    {
        bRC = true ;

        /*
         * Pick the best contacts for the line -- default to STUNed contact for
         * registering side and local contact for provisioned side.
         */
        size_t nAddresses ;
        SIPX_CONTACT_ADDRESS addresses[32] ;
        sipxConfigGetLocalContacts(m_hInst, addresses, 32, nAddresses) ;
        for (size_t i=0; i<nAddresses; i++)
        {
            if (addresses[i].eTransportType == TRANSPORT_UDP)
            {
                switch (addresses[i].eContactType)
                {
                    case CONTACT_LOCAL:
                        provisionContactId = addresses[i].id ;
                        if (registerContactId == CONTACT_AUTO)
                        {
                            registerContactId = addresses[i].id ;
                        }
                        break ;
                    case CONTACT_NAT_MAPPED:
                        registerContactId = addresses[i].id ;
                        break ;
                    default:
                        // Ignore for now
                        break ;
                }
            }
        }

        // TODO:: Should log contact id details -- very useful

        /*
         * Add registering line
         */
        sipxRC = sipxLineAdd(m_hInst, m_url, &m_hLineUrl, registerContactId) ;
        if (sipxRC == SIPX_RESULT_SUCCESS)
        {
            m_eLineContextState = LCS_PROVISIONED ;
            OsSysLog::add(FAC_APP, PRI_INFO,
                    "Register Line %s added (hLine=%d)",
                    m_url.data(), m_hLineUrl) ;

            // Add Alias
            if (!m_alias.isNull())
            {
                sipxRC = sipxLineAddAlias(m_hLineUrl, m_alias) ;
                if (sipxRC == SIPX_RESULT_SUCCESS)
                {
                    OsSysLog::add(FAC_APP, PRI_INFO,
                            "Alias %s added for line %s (hLine=%d)",
                            m_alias.data(), m_url.data(), m_hLineUrl) ;

                }
                else
                {
                    bRC = false ;
                    OsSysLog::add(FAC_APP, PRI_ERR, 
                            "Unable to add alias %s for line %s (error=%d)", 
                            m_alias.data(), m_url.data(), sipxRC) ;
                }
            }

            // Add credentials
            if (!m_username.isNull() && !m_password.isNull() &&
                    !m_realm.isNull())
            {
                sipxRC = sipxLineAddCredential(m_hLineUrl, m_username,
                        m_password, m_realm) ;
                if (sipxRC == SIPX_RESULT_SUCCESS)
                {
                    OsSysLog::add(FAC_APP, PRI_INFO,
                            "Auth credentials (r=%s, u=%s) added "
                            "for line %s (hLine=%d)",
                            m_realm.data(), m_username.data(), 
                            m_url.data(), m_hLineUrl) ;

                    m_eLineContextState = LCS_REGISTERING ;
                    sipxRC = sipxLineRegister(m_hLineUrl, true) ;
                    if (sipxRC == SIPX_RESULT_SUCCESS)
                    {
                        OsSysLog::add(FAC_APP, PRI_INFO,
                                "Request Line registration "
                                "for line %s (hLine=%d)",
                                m_url.data(), m_hLineUrl) ;
                    }
                    else
                    {
                        m_eLineContextState = LCS_FAILED ;
                        OsSysLog::add(FAC_APP, PRI_ERR, 
                                "Unable to add request register "
                                "for line %s (error=%d)", 
                                m_url.data(), sipxRC) ;
                        bRC = false ;
                    }
                }
                else
                {
                    OsSysLog::add(FAC_APP, PRI_ERR, 
                            "Unable to add credentials for line %s (error=%d)", 
                            m_url.data(), sipxRC) ;
                    bRC = false ;
                }
            }
        }
        else
        {
            OsSysLog::add(FAC_APP, PRI_ERR, 
                    "Unable to create registration line for %s (error=%d)", 
                    m_url.data(), sipxRC) ;
            bRC = false ;
        }


        /*
         * Add provisioned line
         */
        sipxRC = sipxLineAdd(m_hInst, m_mapUrl, &m_hLineMapUrl,
                provisionContactId) ;
        if (sipxRC == SIPX_RESULT_SUCCESS)
        {
            OsSysLog::add(FAC_APP, PRI_INFO,
                    "Provisioned Line %s added (hLine=%d)",
                    m_mapUrl.data(), m_hLineMapUrl) ;

            // Add Alias
            if (!m_mapUrlAlias.isNull())
            {
                sipxRC = sipxLineAddAlias(m_hLineUrl, m_mapUrlAlias) ;
                if (sipxRC == SIPX_RESULT_SUCCESS)
                {
                    OsSysLog::add(FAC_APP, PRI_INFO,
                            "Alias %s added for line %s (hLine=%d)",
                            m_mapUrlAlias.data(), m_mapUrl.data(),
                            m_hLineMapUrl) ;

                }
                else
                {
                    OsSysLog::add(FAC_APP, PRI_ERR, 
                            "Unable to add alias %s for line %s (error=%d)", 
                            m_alias.data(), m_mapUrl.data(), sipxRC) ;
                    bRC = false ;
                }
            }
        }
        else
        {
            OsSysLog::add(FAC_APP, PRI_ERR, 
                    "Unable to create registration line for %s (error=%d)", 
                    m_mapUrl.data(), sipxRC) ;
            // bRC = false ;
        }
    }
    
    return bRC ;
}

/****************************************************************************/

void LineContext::handleLineEvent(SIPX_LINESTATE_INFO* pLineInfo) 
{
    if (pLineInfo->hLine == m_hLineUrl)
    {
        switch (pLineInfo->event)
        {
            case LINESTATE_REGISTERED:
                m_eLineContextState = LCS_REGISTERED ;                    
                break ;
            case LINESTATE_REGISTER_FAILED:
                m_eLineContextState = LCS_FAILED ;                    
                break ;
            case LINESTATE_PROVISIONED:
            case LINESTATE_REGISTERING:
            case LINESTATE_UNREGISTERING:
            case LINESTATE_UNREGISTERED:        
            case LINESTATE_UNREGISTER_FAILED:        
            case LINESTATE_UNKNOWN:
            default:
                // Ignored for now
                break ;        
        }
    }
}

/****************************************************************************/

bool LineContext::isHandlingLine(SIPX_LINE hLine)
{
    bool bRC = false ;

    assert(hLine != SIPX_LINE_NULL) ;

    if (hLine != SIPX_LINE_NULL)
    {
        if ((hLine == m_hLineUrl) || (hLine == m_hLineMapUrl))
        {
            bRC = true ;
        }
    }
    return bRC ;
}

/****************************************************************************/

bool LineContext::isHandlingLine(const char* szLineSelect) 
{
    bool bRC = false ;

    assert(szLineSelect != NULL) ;

    if (szLineSelect != NULL)
    {
        if (m_lineSelect.compareTo(szLineSelect, UtlString::ignoreCase) == 0)
        {
            bRC = true ;
        }
    }
    return bRC ;
}

/****************************************************************************/

bool LineContext::getForwardingUrl(SIPX_CALLSTATE_INFO* pCallInfo,
                                   UtlString&           forwardUrl,
                                   SIPX_LINE&           forwardLine)
{
    if (pCallInfo->hLine == m_hLineUrl)
    {
        /*
         * Inbound call from the service provider to the PBX
         */
        forwardUrl = m_mapUrl ;

        // We need to create a temp line to manage the the caller id.  This
        // line is associated with the inbound call handle and freed upon the 
        // call's exit (actually, reference counted...)
        forwardLine = m_hLineMapUrl ;
    }
    else 
    {
        /*
         * Inbound call from the PBX to the service provider
         */
        char szToUrl[256] ;
        char szUsername[256] ;

        sipxCallGetLocalID(pCallInfo->hCall, szToUrl, sizeof(szToUrl)) ;
        sipxUtilUrlParse(szToUrl, szUsername, NULL, NULL) ;

        size_t nUrl = sizeof(szToUrl) ;
        strcpy(szToUrl, m_url) ;
        sipxUtilUrlUpdate(szToUrl, nUrl, szUsername, NULL, -1) ;        

        forwardUrl = szToUrl ;
        forwardLine = m_hLineUrl ;
    }

    return true ;
}

/****************************************************************************/

LINE_CONTEXT_STATE LineContext::getLineContextState()
{
    return m_eLineContextState ;
}

/****************************************************************************/

bool LineContext::deinitialize() 
{
    // TODO :: Need to unregister lines
	return false ;
}

/****************************************************************************/

void LineContext::addActiveCall(SIPX_CALL hCall) 
{
    m_activeCalls.insert(new UtlInt(hCall)) ;
}

/****************************************************************************/

bool LineContext::isActiveCall(SIPX_CALL hCall) 
{
    UtlInt key(hCall) ;

    return (m_activeCalls.contains(&key) == TRUE) ;
}

/****************************************************************************/

void LineContext::removeActiveCall(SIPX_CALL hCall) 
{
    UtlInt key(hCall) ;

    m_activeCalls.destroy(&key) ;
}

/****************************************************************************/

size_t LineContext::getNumActiveCalls()
{
    return m_activeCalls.entries() ;
}

/****************************************************************************/

bool LineContext::canAddCall() 
{
    return (bool) (getNumActiveCalls() < m_iMaxCalls) ;
}

/****************************************************************************/

