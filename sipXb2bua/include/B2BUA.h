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

#ifndef _B2BUA_H
#define _B2BUA_H

/****************************************************************************/

#include "utl/UtlString.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#include "LineContextManager.h"
#include "CallContextManager.h"
#include "ConfContextManager.h"

/****************************************************************************/

#define MAX_CONFS           32
#define MAX_CALLS           32
#define MAX_LINES           32 

/****************************************************************************/

typedef enum STUN_STATE
{
    STUN_STATE_UNKNOWN,
    STUN_STATE_SUCCESS,
    STUN_STATE_FAILURE 
} ;

/****************************************************************************/

class B2BUA
{
public:
    B2BUA() ;
    virtual ~B2BUA() ;

    bool initialize() ;

    static bool SIPX_CALLING_CONVENTION EventCallBack(SIPX_EVENT_CATEGORY category, 
                                                      void*               pInfo, 
                                                      void*               pUserData);
protected:
    bool loadConfiguration() ;
    bool initStun() ;
    bool initLines() ;

    void rejectCall(SIPX_CALLSTATE_INFO* pCallInfo,
                    int                  iCause,
                    const char*          szCause) ;
    void acceptCall(SIPX_CALLSTATE_INFO* pCallInfo) ;
    
    void handleLineEvent(SIPX_LINESTATE_INFO* pLineInfo) ;
    void handleConfigEvent(SIPX_CONFIG_INFO* pConfigInfo) ;
	void handleCallEvent(SIPX_CALLSTATE_INFO* pCallInfo) ;
    void handleMediaEvent(SIPX_MEDIA_INFO* pMediaInfo) ;

protected:

    SIPX_INST mhInst ;
    SIPX_LINE mhLine ;
    static B2BUA* mpInstance ;

    CallContextManager mCallContextMgr ;
    ConfContextManager mConfContextMgr ;
    LineContextManager mLineContextMgr ;

    // Global Configuration Data
    UtlString m_outputProxy ;
    UtlString m_stunServer ;
    int       m_iStunPort ;
    int       m_iSipUdpPort ;
    int       m_iSipTcpPort ;
    int       m_iRtpStartPort ;
    int       m_iRegisterPeriodSecs ;
    UtlString m_logFile ;
    int       m_iNumLines ;
    bool      m_bInitialized ;

    UtlString mIdentityUrl ;
    UtlString mIdentityRealm ;
    UtlString mIdentityUsername ;
    UtlString mIdentityPassword ;

    STUN_STATE m_eStunState ;
} ;

/****************************************************************************/

#endif // _B2BUA_H

