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
import org.springframework.context.ApplicationContext;

public class ConferenceBeanTest extends TestCase {
    private ApplicationContext m_applicationContext;

    protected void setUp() throws Exception {
        m_applicationContext = TestHelper.getApplicationContext();
    }

    public void testConference() throws Exception {
        Conference conference = (Conference) m_applicationContext.getBean(Conference.BEAN_NAME,
                Conference.class);        
        assertNotNull(conference.getSettings());
    }

    public void testBridge() throws Exception {
        Bridge bridge = (Bridge) m_applicationContext.getBean(Bridge.BEAN_NAME, Bridge.class);
        assertNotNull(bridge.getSettings());
    }

    public void testParticipant() throws Exception {
        Participant participant = (Participant) m_applicationContext.getBean(
                Participant.BEAN_NAME, Participant.class);
        assertNotNull(participant.getSettings());
    }
}
