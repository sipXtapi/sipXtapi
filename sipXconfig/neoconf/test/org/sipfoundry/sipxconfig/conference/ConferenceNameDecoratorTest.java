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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConferenceNameDecoratorTest extends TestCase {

    public void testGetProfileName() {

        MockControl settingCtrl = MockControl.createControl(Setting.class);
        Setting setting = (Setting) settingCtrl.getMock();
        setting.getProfileName();
        settingCtrl.setReturnValue("BOSTON_BRIDGE_CONFERENCE_STATUS");
        settingCtrl.setReturnValue("BOSTON_BRIDGE_CONFERENCE.AOR");
        settingCtrl.setReturnValue("BOSTON_BRIDGE_CONFERENCE.REMOTE_ADMIT.SECRET");
        settingCtrl.replay();

        Conference conference = new Conference();
        conference.setName("bongo");
        Conference.ConferenceProfileName handler = new Conference.ConferenceProfileName(
                conference);

        assertEquals("BOSTON_BRIDGE_CONFERENCE_STATUS.bongo", handler.getProfileName(setting)
                .getValue());
        assertEquals("BOSTON_BRIDGE_CONFERENCE.bongo.AOR", handler.getProfileName(setting)
                .getValue());
        assertEquals("BOSTON_BRIDGE_CONFERENCE.bongo.REMOTE_ADMIT.SECRET", handler
                .getProfileName(setting).getValue());

        settingCtrl.verify();
    }

}
