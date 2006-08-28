/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

public class NewPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewPhoneTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testAddPhone() {
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            {
                "000000000000", "", TestPage.TEST_PHONE_MODEL.getLabel()
            },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testSaveAndStay() {
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        checkCheckbox("stay");
        clickButton("phone:ok");
        assertCheckboxSelected("stay");
        // should clear the form
        assertEquals("", getDialog().getFormParameterValue("serialNumber"));

        clickButton("phone:cancel");
        String[][] table = new String[][] {
            {
                "000000000000", "", TestPage.TEST_PHONE_MODEL.getLabel()
            },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testInvalidSerialNumber() {
        clickLink("NewPhone");

        // no digits
        clickButton("phone:ok");
        SiteTestHelper.assertUserError(tester);

        // wrong chars and wrong number
        setFormElement("serialNumber", "x");
        clickButton("phone:ok");
        SiteTestHelper.assertUserError(tester);

        // 12 digits, but not valid chars
        setFormElement("serialNumber", "123456789abx");
        clickButton("phone:ok");
        SiteTestHelper.assertUserError(tester);

        // 16 correct digits - is OK - we accept 12 or more now
        setFormElement("serialNumber", "123456789abcdef");
        clickButton("phone:ok");
        SiteTestHelper.assertNoUserError(tester);

        SiteTestHelper.home(getTester());
        clickLink("NewPhone");
        // finally got it right
        setFormElement("serialNumber", "123456789abc");
        clickButton("phone:ok");
        SiteTestHelper.assertNoUserError(tester);
    }
}
