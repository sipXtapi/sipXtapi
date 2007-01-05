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

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListCallGroupsTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListCallGroupsTestUi.class);
    }

    public ListCallGroupsTestUi() {
        super("ListCallGroups", "resetCallGroupContext", "callgroups");
    }

    public void testDisplay() {
        super.testDisplay();
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "extension", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "call_group" + i, Integer.toString(400 + i), "Description" + i
        };
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        Object[] expected = new Object[4];
        expected[0] = paramValues[0];
        expected[1] = "Disabled";
        expected[2] = paramValues[1];
        expected[3] = paramValues[2];
        return expected;
    }

    public void testDisplayEditCallGroup() {
        tester.clickLink("callgroups:add");
        SiteTestHelper.assertNoException(getTester());
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
    }
}
