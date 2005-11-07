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

    public void testInsertRemoveParticipant() {
        Participant p = new Participant();
        p.setUniqueId();

        Conference c = new Conference();
        assertTrue(c.getParticipants().isEmpty());
        c.insertParticipant(p);

        assertEquals(1, c.getParticipants().size());
        assertSame(p, c.getParticipants().iterator().next());

        assertSame(c, p.getConference());
    }

    public void testRemoveParticipant() {
        Participant p = new Participant();
        p.setUniqueId();

        Participant p1 = new Participant();
        p1.setUniqueId();

        Conference c = new Conference();
        assertTrue(c.getParticipants().isEmpty());
        c.insertParticipant(p);

        c.removeParticipant(p1);
        assertEquals(1, c.getParticipants().size());

        c.removeParticipant(p);
        assertTrue(c.getParticipants().isEmpty());
        assertNull(p.getConference());
    }

    public void testGenerateAccessCodes() {
        Conference conf = (Conference) TestHelper.getApplicationContext().getBean(
                "conferenceConference", Conference.class);
        conf.generateAccessCodes();

        assertTrue(conf.getSettingValue(Conference.ORGANIZER_CODE).toString().length() > 0);
        assertTrue(conf.getSettingValue(Conference.PARTICIPANT_CODE).toString().length() > 0);
    }
}
