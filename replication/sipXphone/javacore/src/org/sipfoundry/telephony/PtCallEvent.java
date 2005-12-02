/*
 * $Id$
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

public class PtCallEvent extends PtEvent implements CallEvent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** @deprecated do not expose */
    public static final int LS_REMOTE  = 0 ;
    /** @deprecated do not expose */
    public static final int LS_UNKNOWN = -1 ;
    /** @deprecated do not expose */
    public static final int LS_LOCAL   = 1 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the call associated with this event */
    protected Call m_call ;
    /** local state */
    protected int m_iLocalState ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking originating call.  This is an immutable object
     * and thus no set accessors are defined.
     */
    public PtCallEvent(Call call, int iLocalState)
    {
        m_iLocalState = iLocalState ;
        m_call = call ;
    }


    /**
     * JNI constructor taking native call object.  This is an immutable object
     * and thus no set accessors are defined.
     */
    public PtCallEvent(long lCall, int iLocalState)
    {
        m_iLocalState = iLocalState ;
        m_call = PtCall.fromNativeObject(lCall) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public Call getCall()
    {
        return m_call ;
    }


    /**
     * @deprecated do not expose
     */
    public int getLocalState()
    {
        return m_iLocalState ;
    }
}
