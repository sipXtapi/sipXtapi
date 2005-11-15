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
package org.sipfoundry.sipxconfig.site.conference;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListBridgesTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListBridgesTestUi.class);
    }

    public ListBridgesTestUi() {
        super("ListBridges", "resetConferenceBridgeContext", "bridge");
        setHasDuplicate(false);
    }

    protected String[] getParamNames() {
        return new String[] {
            "name", "description"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "bridge" + i, "Description" + i
        };
    }

    protected void clickAddLink() throws Exception {
        SiteTestHelper.clickSubmitLink(tester, "addLink");
    }

    protected Object[] getExpectedTableRow(String[] paramValues) {
        Object[] expected = new Object[3];
        expected[0] = paramValues[0];
        expected[1] = "false";
        expected[2] = paramValues[1];
        return expected;
    }
}
