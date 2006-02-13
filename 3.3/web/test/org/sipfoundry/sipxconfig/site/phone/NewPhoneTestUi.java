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

public class NewPhoneTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(NewPhoneTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testAddPhone() {
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        selectOption("phoneModel", "Polycom SoundPoint IP 500/501");
        clickButton("phone:ok");
        String[][] table = new String[][] {
            {
                "000000000000", "", "Polycom SoundPoint IP 500/501"
            },
        };
        assertTextInTable("newPhoneTable", table[0]);
    }

    public void testSaveAndStay() {
        clickLink("NewPhone");
        setFormElement("serialNumber", "000000000000");
        selectOption("phoneModel", "Polycom SoundPoint IP 500/501");
        checkCheckbox("stay");
        clickButton("phone:ok");
        assertCheckboxSelected("stay");
        // should clear the form
        assertEquals("", getDialog().getFormParameterValue("serialNumber"));

        clickButton("phone:cancel");
        String[][] table = new String[][] {
            {
                "000000000000", "", "Polycom SoundPoint IP 500/501"
            },
        };
        assertTextInTable("phone:list", table[0]);
    }

    public void testInvalidSerialNumber() {
        clickLink("NewPhone");

        // no digits
        clickButton("phone:ok");
        assertElementPresent("user:error");

        // wrong chars and wrong number
        setFormElement("serialNumber", "x");
        clickButton("phone:ok");
        assertElementPresent("user:error");

        // 12 digits, but not valid chars
        setFormElement("serialNumber", "123456789abx");
        clickButton("phone:ok");
        assertElementPresent("user:error");

        // 16 correct digits
        setFormElement("serialNumber", "123456789abcdef");
        clickButton("phone:ok");
        assertElementPresent("user:error");

        // finally got it right
        setFormElement("serialNumber", "123456789abc");
        clickButton("phone:ok");
        assertElementNotPresent("user:error");
    }
}
