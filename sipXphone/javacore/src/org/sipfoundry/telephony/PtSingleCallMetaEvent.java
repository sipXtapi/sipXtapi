/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/PtSingleCallMetaEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.telephony ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

public class PtSingleCallMetaEvent extends PtMetaEvent implements SingleCallMetaEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** What call is this part of? */
    protected PtCall m_call ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor used by JNI
     */
    protected PtSingleCallMetaEvent(long lCall)
    {
        m_call = PtCall.fromNativeObject(lCall) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public Call getCall()
    {
        return m_call ;
    }
}
