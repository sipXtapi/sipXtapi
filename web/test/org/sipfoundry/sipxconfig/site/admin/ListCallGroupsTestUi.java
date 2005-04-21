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
import net.sourceforge.jwebunit.ExpectedRow;
import net.sourceforge.jwebunit.ExpectedTable;
import net.sourceforge.jwebunit.WebTestCase;
import net.sourceforge.jwebunit.WebTester;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListCallGroupsTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListCallGroupsTestUi.class);
    }

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetCallGroupContext");
        clickLink("ListCallGroups");
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("callgroups:form");
        assertLinkPresent("callgroups:add");
        // assertTableEquals("callgroups:list", createExpectedTable());
        assertEquals(1, SiteTestHelper.getRowCount(tester, "callgroups:list"));
        assertButtonPresent("callgroups:delete");
        assertButtonPresent("callgroups:duplicate");
        assertButtonPresent("callgroups:activate");
    }

    public void testAdd() {
        final int count = 5;
        ExpectedTable expected = new ExpectedTable();
        for (int i = 0; i < count; i++) {
            String name = "call group" + i;
            String extension = Integer.toString(400 + i);
            String description = "Description" + i;
            addCallGroup(getTester(), name, extension, description);
            ExpectedRow row = new ExpectedRow(new Object[] {
                name, "false", extension, description
            });
            expected.appendRow(row);
        }
        // we should have 2 gateway now
        assertEquals(count + 1, SiteTestHelper.getRowCount(tester, "callgroups:list"));
        assertTableRowsEqual("callgroups:list", 1, expected);
    }

    public void testEdit() {
        final String name = "call group xxx";
        final String extension = "404";
        addCallGroup(getTester(), name, extension, "");

        // click on name - it should take us to the edit page
        clickLinkWithText(name);

        assertFormElementEquals("callGroupName", name);
        assertFormElementEquals("callGroupExtension", extension);
    }

    public void testDelete() {
        final int[] toBeRemoved = {
            2, 4
        };
        final int count = 5;
        ExpectedTable expected = new ExpectedTable();

        for (int i = 0; i < count; i++) {
            String name = "call group" + i;
            String extension = Integer.toString(400 + i);
            String description = "Description" + i;
            addCallGroup(getTester(), name, extension, description);
            if (!ArrayUtils.contains(toBeRemoved, i)) {
                ExpectedRow row = new ExpectedRow(new Object[] {
                    name, "false", extension, description
                });
                expected.appendRow(row);
            }
        }
        // remove 2nd and 4th
        for (int i = 0; i < toBeRemoved.length; i++) {
            SiteTestHelper.checkCheckbox(getTester(), "selectedRow", toBeRemoved[i]);
        }

        clickButton("callgroups:delete");

        assertEquals(count + 1 - toBeRemoved.length, SiteTestHelper.getRowCount(tester,
                "callgroups:list"));
        assertTableRowsEqual("callgroups:list", 1, expected);
    }

    public void testDisplayEditCallGroup() {
        tester.clickLink("callgroups:add");
        SiteTestHelper.assertNoException(getTester());
        assertFormPresent("callgroup:form");
        assertButtonPresent("callgroup:save");
        assertButtonPresent("cancel");
    }

public static void addCallGroup(WebTester tester, String name, String extension, String description) {
        tester.clickLink("callgroups:add");
        tester.setFormElement("callGroupName", name);
        tester.setFormElement("callGroupExtension", extension);
        tester.setFormElement("callGroupDescription", description);
        tester.clickButton("callgroup:save");
    }
}
