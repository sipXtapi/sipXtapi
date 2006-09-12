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

#ifndef _CONF_CONTEXT_H
#define _CONF_CONTEXT_H

/****************************************************************************/

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

#define MAX_CONF_PARTICIPANTS   4

/****************************************************************************/

class CallContextManager ;
class CallContext ;

/****************************************************************************/

class ConfContext
{
public:
	ConfContext() ;

    void setCallContextManager(CallContextManager* pCallContextMgr) ;

	void setConfHandle(SIPX_CONF hConf) ;
	SIPX_CONF getConfHandle() ;

    bool isInUse() ;
    void reset() ;

    CallContext* getInboundCall() ;
    CallContext* getOutboundCall() ;

    bool containsCall(SIPX_CALL hCall) ;

    size_t getNumCalls() ;

    void startTone(SIPX_CALL hOriginatingCall, SIPX_TONE_ID toneId) ;
    void stopTone(SIPX_CALL hOriginatingCall) ;

    void startRingback() ;
    void stopRingback() ;

    void startBusy() ;
    void stopBusy() ;    

protected:
    CallContextManager* mpCallContextMgr ;
	SIPX_CONF           m_hConf ;
    bool                m_bInUse ;
} ;

/****************************************************************************/

#endif // _CONF_CONTEXT_H

