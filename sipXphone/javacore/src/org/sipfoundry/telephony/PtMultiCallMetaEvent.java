/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/PtMultiCallMetaEvent.java#2 $
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

public class PtMultiCallMetaEvent extends PtMetaEvent implements MultiCallMetaEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** what is the new call? */
    protected PtCall m_callNew = null ;   // WARNING: This attribute is modified in JNI

    /** what are the old calls? */
    protected PtCall m_callsOld[] = null ;   // WARNING: This attribute is modified in JNI



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor used by JNI
     */
    protected PtMultiCallMetaEvent(long lNewCall, long[] lOldCalls)
    {
        m_callNew = PtCall.fromNativeObject(lNewCall) ;

        m_callsOld = new PtCall[lOldCalls.length] ;
        for (int i=0; i<lOldCalls.length; i++)
        {
            m_callsOld[i] = PtCall.fromNativeObject(lOldCalls[i]) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public Call getNewCall()
    {
        return m_callNew ;
    }


    public Call getOldCalls()
    {
        // BUG IN JTAPI 1.3, this should be returnning an ARRAY of calls, not
        // a single call
        Call oldCalls = null ;
        if ((m_callsOld != null) && (m_callsOld.length > 0))
            oldCalls = m_callsOld[0] ;

        return oldCalls ;
    }
}
