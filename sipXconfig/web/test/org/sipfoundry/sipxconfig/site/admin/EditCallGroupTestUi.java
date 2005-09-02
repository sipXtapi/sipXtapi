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
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class EditCallGroupTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditCallGroupTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.seedUser(getTester());
        SiteTestHelper.home(getTester());
        // clickLink("resetCallGroupContext");
        clickLink("NewCallGroup");
        setFormData();
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("callgroup:form");
        assertElementPresent("item:name");
        assertElementPresent("item:extension");
        assertElementPresent("item:description");
        assertElementPresent("item:enabled");

        assertLinkPresent("callgroup:addRing");
        // assertTableEquals("callgroups:list", createExpectedTable());
        assertEquals(1, SiteTestHelper.getRowCount(tester, "userring:list"));
        assertButtonPresent("userring:delete");
        assertButtonPresent("userring:moveUp");
        assertButtonPresent("userring:moveDown");
    }

    public void testAddDeleteRing() throws Exception {
        addUser();

        assertEquals(2, SiteTestHelper.getRowCount(tester, "userring:list"));

        checkCheckbox("selectedRow");
        clickButton("userring:delete");

        assertEquals(1, SiteTestHelper.getRowCount(tester, "userring:list"));
    }

    public void testMoveRing() throws Exception {
        for (int i = 0; i < 3; i++) {
            addUser();
        }

        checkCheckbox("selectedRow");
        clickButton("userring:moveDown");
        assertCheckboxNotSelected("selectedRow");
        assertCheckboxSelected("selectedRow$0");
        clickButton("userring:moveUp");
        assertCheckboxSelected("selectedRow");
        assertCheckboxNotSelected("selectedRow$0");

        for (int i = 0; i < 3; i++) {
            SiteTestHelper.checkCheckbox(getTester(), "selectedRow", i);
        }
    }

    private void addUser() throws Exception {
        SiteTestHelper.clickSubmitLink(getTester(), "addRow");

        clickButton("user:search");
        checkCheckbox("selectedRow");
        clickButton("user:select");
    }

    private void setFormData() {
        setFormElement("name", "testName");
        setFormElement("extension", "123");
    }

    /** Make sure that one cannot enable 2 huntgroups with the same name */
    public void testEnableDuplicate() throws Exception {
        checkCheckbox("enabled");
        setFormElement("name", "xxxx");
        setFormElement("extension", "123");
        clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        clickLink("callgroups:add");
        checkCheckbox("enabled");
        setFormElement("name", "xxxx");
        setFormElement("extension", "123");
        clickButton("form:ok");
        // this time we expect page will complain
        SiteTestHelper.assertUserError(tester);
    }
}
