/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/event/SipLineEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sip.event ;

import org.sipfoundry.sip.* ;

public class SipLineEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private SipLine m_line ;
    private int m_iSipResponseCode ;
    private String m_strSipResponseText ;
    private String m_strAuthenticationRealm ;
    private String m_strAuthenticationScheme ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SipLineEvent(SipLine line,
                        int iResponseCode,
                        String strSipResponseText,
                        String strAuthenticationRealm,
                        String strAuthenticationScheme)
    {
        m_line = line ;
        m_iSipResponseCode = iResponseCode ;
        m_strSipResponseText = strSipResponseText ;
        m_strAuthenticationRealm = strAuthenticationRealm ;
        m_strAuthenticationScheme = strAuthenticationScheme ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public SipLine getLine()
    {
        return m_line ;
    }


    public int getSipResponseCode()
    {
        return m_iSipResponseCode ;
    }


    public String getSipResponseText()
    {
        return m_strSipResponseText ;
    }


    public String getAuthenticationRealm()
    {
        return m_strAuthenticationRealm ;
    }


    public String getAuthenticationScheme()
    {
        return m_strAuthenticationScheme ;
    }
}
