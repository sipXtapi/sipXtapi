/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestHelper;

public class AddExternalLineTestUi extends WebTestCase {
    
    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(AddExternalLineTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
        SiteTestHelper.seedUser(tester);
        m_helper.seedPhone(1);
        clickLink("ManagePhones");
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testDisplay() {
        clickLink("AddExternalLine");        
        SiteTestHelper.assertNoException(tester);
        assertFormPresent("form:externalLine");
    }
    
    public void testAddExternalLine() {
        clickLink("AddExternalLine");        
        setFormElement("displayName", "Dil Bert");
        setFormElement("userId", "dilbert");
        setFormElement("password", "1234");
        setFormElement("registrationServer", "frakenberry.org");
        setFormElement("voiceMail", "2000");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        assertTextPresent("\"Dil Bert\"&lt;sip:dilbert@frakenberry.org&gt;");
    }
}
