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

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

/**
 * DialPlanEditTestUi
 */
public class DialPlanEditTestUi extends WebTestCase {
    private static final String NEW_DESCRIPTION = "New kuku description";
    private static final String[][] NAMES = { 
        { "kukuName", "false", "kuku description" },
        { "bongoName", "false", "bongoDescription" }, 
    };

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        beginAt("/");
        clickLink("resetDialPlans");
    }

    public void testAddEditPlan() {
        clickLink("FlexibleDialPlan");

        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            addDialPlan(row);
        }

        assertTableRowsEqual("dialplan:list", 1, NAMES);

        // edit
        clickLinkWithText(NAMES[0][0]);
        setFormElement("description", NEW_DESCRIPTION);
        clickButton("rule:save");
        assertTextInTable("dialplan:list", NEW_DESCRIPTION);
        assertTextInTable("dialplan:list", NAMES[1][2]);

        // check validation
        clickLinkWithText(NAMES[0][0]);
        setFormElement("name", "");
        clickButton("rule:save");
        // if validation kicks in we are on the same page
        clickButton("cancel");

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
        setFormElement("name", row[0]);
        setFormElement("description", row[2]);
        setFormElement("prefix", "33");
        setFormElement("prefix$0", "33");
        clickButton("rule:save");
    }
}
