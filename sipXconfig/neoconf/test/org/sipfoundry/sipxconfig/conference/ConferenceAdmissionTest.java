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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;

public class ConferenceAdmissionTest extends XMLTestCase {
    private List m_conferences;

    private static String[][] DATA = {
        {
            "disable", "101", "0000", "000", "000", ""
        },
        {
            "sales", "400", "400111", "400222", "400AAA", "sip:sales@bridge.sipfoundry.org"
        },
        {
            "marketing", "500", "500111", "500222", "500AAA",
            "sip:marketing@bridge.sipfoundry.org"
        }
    };

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);

        m_conferences = new ArrayList(DATA.length);
        for (int i = 0; i < DATA.length; i++) {
            IMocksControl conferenceCtrl = EasyMock.createControl();
            Conference conference = conferenceCtrl.createMock(Conference.class);

            conference.getName();
            conferenceCtrl.andReturn(DATA[i][0]).anyTimes();

            conference.getExtension();
            conferenceCtrl.andReturn(DATA[i][1]);

            conference.getOrganizerAccessCode();
            conferenceCtrl.andReturn(DATA[i][2]);

            conference.getParticipantAccessCode();
            conferenceCtrl.andReturn(DATA[i][3]);

            conference.getRemoteAdmitSecret();
            conferenceCtrl.andReturn(DATA[i][4]);

            conference.getUri();
            conferenceCtrl.andReturn(DATA[i][5]);

            // the first one is disabled
            conference.isEnabled();
            conferenceCtrl.andReturn(i > 0);

            m_conferences.add(conference);
            conferenceCtrl.replay();
        }

    }
    
    public void testGenerate() throws Exception {
        ConferenceAdmission admission = new ConferenceAdmission();
        admission.generate(m_conferences);
        String generatedXml = admission.getFileContent();
        System.err.println(generatedXml);
        InputStream referenceXml = ConferenceAdmissionTest.class
                .getResourceAsStream("conferences.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
    }
}
