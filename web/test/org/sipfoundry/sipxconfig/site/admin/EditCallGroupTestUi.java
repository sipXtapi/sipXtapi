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
        SiteTestHelper.home(getTester());
        clickLink("resetCallGroupContext");
        clickLink("ListCallGroups");
        ListCallGroupsTestUi.addCallGroup(getTester(), "testGroup", "233", "test description");
        clickLinkWithText("testGroup");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("callgroup:form");
        assertElementPresent("callgroup:name");
        assertElementPresent("callgroup:extension");
        assertElementPresent("callgroup:description");
        assertElementPresent("callgroup:enabled");

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
        for( int i = 0; i < 3; i++) {
            addUser();
        }
        
        checkCheckbox("selectedRow");
        clickButton("userring:moveDown");
        assertCheckboxNotSelected("selectedRow");
        assertCheckboxSelected("selectedRow$0");
        clickButton("userring:moveUp");
        assertCheckboxSelected("selectedRow");
        assertCheckboxNotSelected("selectedRow$0");
        
        for( int i = 0; i < 3; i++) {
            SiteTestHelper.checkCheckbox(getTester(), "selectedRow", i);
        }
    }

    private void addUser() throws Exception {
        SiteTestHelper.clickSubmitLink(getTester(), "addRow");

        clickButton("user:search");
        checkCheckbox("selectedRow");
        clickButton("user:select");
    }
}
