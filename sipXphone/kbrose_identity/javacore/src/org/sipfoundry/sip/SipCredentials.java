/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipCredentials.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
 
package org.sipfoundry.sip;

/**
 * SipCredentials.java
 *
 *SIP Credentials consist of following items:
 *realm, userId, type, passToken
 *It has accessor methods to access those fields.
 * Created: Fri Feb 15 16:07:39 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class SipCredentials {

    private String m_strRealm;
    private String m_strUserID;
    private String m_strType;
    private String m_strPassToken;

    private SipCredentials (){
    }

    /**
     * @param realm realm
     * @param userid userid
     * @param type type
     * @param passwordToken passToken
     */
    public SipCredentials( String realm, String userid,
                                String type, String passwordToken ){

        m_strRealm  = realm;
        m_strUserID = userid;
        m_strType   = type;
        m_strPassToken = passwordToken;
    }

    /**
     * @return realm
     */
    public String getRealm(){
        return m_strRealm;
    }

    /**
     * @return userid
     */
    public String getUserID(){
        return m_strUserID;
    }

    /**
     * @return type
     */
    public String getType(){
        return m_strType;
    }

    /**
     * @return passToken
     */
    public String getPasswordToken(){
        return m_strPassToken;
    }

}// SipCredentials
