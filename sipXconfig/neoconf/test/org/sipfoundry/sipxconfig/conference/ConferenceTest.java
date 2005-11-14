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

    public void testGenerateAccessCodes() {
        Conference conf = (Conference) TestHelper.getApplicationContext().getBean(
                "conferenceConference", Conference.class);
        conf.generateAccessCodes();

        assertTrue(conf.getSettingValue(Conference.ORGANIZER_CODE).toString().length() > 0);
        assertTrue(conf.getSettingValue(Conference.PARTICIPANT_CODE).toString().length() > 0);
    }
}
