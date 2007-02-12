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

#include "TempLineContext.h"

#include "os/OsSysLog.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "utl/UtlHashBag.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

/****************************************************************************/

TempLineContext::TempLineContext(SIPX_INST       hInst,
                                 UtlString       url,
                                 SIPX_CONTACT_ID contactID) 
{
    m_hInst = hInst ;
    m_hLine = SIPX_LINE_NULL ;
    m_iCalls = 0 ;

    sipxLineAdd(hInst, url, &m_hLine, contactID) ;
}

/****************************************************************************/

TempLineContext::~TempLineContext() 
{
    if (m_hLine != SIPX_LINE_NULL)
    {
        sipxLineRemove(m_hLine) ;
    }
}

/****************************************************************************/

bool TempLineContext::isHandlingLine(SIPX_CALL hLine) 
{
    return (hLine != SIPX_LINE_NULL && hLine == m_hLine) ;
}

/****************************************************************************/

void TempLineContext::addCallRef() 
{
    m_iCalls++ ;
}

/****************************************************************************/

void TempLineContext::removeCallRef() 
{
    m_iCalls-- ;
    assert(m_iCalls >= 0) ;
}

/****************************************************************************/

int TempLineContext::getCallRef() 
{
    return m_iCalls ;
}

/****************************************************************************/
