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
package org.sipfoundry.sipxconfig.site.ui;

import net.sourceforge.jwebunit.WebTestCase;

/**
 * DialPlanEditTestUi
 */
public class DialPlanEditTestUi extends WebTestCase {
    private static final String NEW_DESCRIPTION = "New kuku description";
    private static final String[][] NAMES = { 
        { "kukuName", "Inactive", "kuku description" },
        { "bongoName", "Inactive", "bongoDescription" }, 
    };

    public void setUp() {
        getTestContext().setBaseUrl("http://localhost:8080/sipxconfig");
        beginAt("/");
        clickLink("resetDialPlans");
    }

    public void testAddEditPlan() {
        clickLink("ListDialPlans");

        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            addDialPlan(row);
        }

        assertTableRowsEqual("dialplan:list", 1, NAMES);

        // edit
        clickLinkWithText(NAMES[0][0]);
        setFormElement("dialPlanDescription", NEW_DESCRIPTION);
        clickButton("dialplan:save");
        assertTextInTable("dialplan:list", NEW_DESCRIPTION);
        assertTextInTable("dialplan:list", NAMES[1][2]);

        // check validation
        clickLinkWithText(NAMES[0][0]);
        setFormElement("dialPlanName", "");
        clickButton("dialplan:save");
        // if validation kicks in we are on the same page
        assertTitleEquals("Dial Plan Configuration");
        clickButton("dialplan:cancel");

        assertTextInTable("dialplan:list", NEW_DESCRIPTION);
        assertTextInTable("dialplan:list", NAMES[1][2]);

        // remove dial plan
        checkCheckbox("selectedRow");
        clickButton("dialplan:delete");

        assertTextNotInTable("dialplan:list", NEW_DESCRIPTION);
        assertTextInTable("dialplan:list", NAMES[1][2]);
    }

    /**
     * Fills and submits edit DialPlan form
     * 
     */
    private void addDialPlan(String[] row) {
        clickLink("dialplan:add");
        setFormElement("dialPlanName", row[0]);
        setFormElement("dialPlanDescription", row[2]);
        clickButton("dialplan:save");
    }
}
