/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class ConferenceTest extends TestCase {
    private Conference m_conf;

    protected void setUp() throws Exception {
        m_conf = (Conference) TestHelper.getApplicationContext().getBean("conferenceConference",
                Conference.class);
    }

    public void testGenerateAccessCodes() {
        m_conf.generateAccessCodes();

        assertTrue(m_conf.getSettingValue(Conference.ORGANIZER_CODE).toString().length() > 0);
        assertTrue(m_conf.getSettingValue(Conference.PARTICIPANT_CODE).toString().length() > 0);
    }

    public void testGetUri() {
        m_conf.setName("weekly.marketing");

        Bridge bridge = new Bridge();
        bridge.setHost("bridge1.sipfoundry.org");
        bridge.insertConference(m_conf);

        assertEquals("sip:weekly.marketing@bridge1.sipfoundry.org", m_conf.getUri());

        bridge.setHost("abc.domain.com");
        assertEquals("sip:weekly.marketing@abc.domain.com", m_conf.getUri());
    }

    public void testGenerateRemoteAdmitSecret() {
        assertNull(m_conf.getRemoteAdmitSecret());
        m_conf.generateRemoteAdmitSecret();
        assertTrue(m_conf.getRemoteAdmitSecret().length() > 0);
    }

}
