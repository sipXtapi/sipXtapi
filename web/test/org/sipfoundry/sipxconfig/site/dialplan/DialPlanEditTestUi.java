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
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;

/**
 * DialPlanEditTestUi
 */
public class DialPlanEditTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(DialPlanEditTestUi.class);
    }

    private static final String[][] NAMES = {
        {
            "kukuName", "false", "kuku description"
        }, {
            "bongoName", "false", "bongoDescription"
        },
    };

    private static final String[][] DEFAULTS = {
        {
            "Emergency", "true", "Emergency", "Emergency dialing plan"
        }, {
            "International", "false", "International", "International dialing"
        }, {
            "Internal", "true", "Internal", "Default internal dialing plan"
        }, {
            "Local", "false", "Local", "Local dialing"
        }, {
            "Long Distance", "false", "Long Distance", "Long distance dialing plan"
        }, {
            "Restricted", "false", "Long Distance", "Restricted dialing"
        }, {
            "Toll free", "false", "Long Distance", "Toll free dialing"
        }
    };

    public void setUp() {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        beginAt("/");
        clickLink("resetDialPlans");
        clickLink("FlexibleDialPlan");
    }

    public void testDisplayAndClean() {
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
        // remove all
        for (int i = 0; i < 7; i++) {
            checkCheckbox("selectedRow", i);
        }
        clickButton("dialplan:delete");
        // should be empty now
        WebTable rulesTable = getDialog().getWebTableBySummaryOrId("dialplan:list");
        assertEquals(1, rulesTable.getRowCount());
        // test revert
        clickButton("dialplan:revert");
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
    }

    public void testViewRules() {
        for (int i = 0; i < DEFAULTS.length; i++) {
            String name = DEFAULTS[i][0];
            clickLinkWithText(name);
            assertFormElementEquals("name", name);

            setFormElement("name", "");
            clickButton("rule:save");
            // if validation kicks in we are on the same page
            assertTextPresent("You must enter a value for Name.");
            setFormElement("name", name + "changed");
            clickButton("rule:save");
            // a link corresponding to new name should be in now
            assertLinkPresentWithText(name + "changed");
        }
    }

    public void testCustomRuleAdd() {
        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            clickLink("dialplan:add");
            // 2 means custom...
            selectOption("ruleTypeSelection", "Custom");
            clickButton("selectRule:next");

            assertLinkNotPresent("pattern:delete");
            assertLinkPresent("pattern:add");

            setFormElement("name", row[0]);
            setFormElement("description", row[2]);
            // dial patter prefix
            setFormElement("prefix", "333");
            // call pattern prefix
            setFormElement("prefix$0", "444");

            clickButton("rule:save");

            assertTextInTable("dialplan:list", row[2]);
            assertLinkPresentWithText(row[0]);
        }
    }

    
    public void testInternationalRuleAdd() {
        for (int i = 0; i < 12; i++) {
            clickLink("dialplan:add");
            String name = "international" + i;
            String description = "international description" + i;
            // 2 means custom...
            selectOption("ruleTypeSelection", "International");
            clickButton("selectRule:next");

            setFormElement("name", name);
            setFormElement("description", description);
            // dial patter prefix
            setFormElement("internationalPrefix", "100" + i);

            clickButton("rule:save");

            assertTextInTable("dialplan:list", description);
            assertLinkPresentWithText(name);
        }
    }
    
    // TODO: add this for custom rule test - it relies on Java script at the moment
    private void checkAddDeletePattern() {
        // no delete link
        assertLinkNotPresent("pattern:delete");
        // add 2 more
        clickLink("pattern:add");
        clickLink("pattern:add");

        // delete 2

        clickLink("pattern:delete");
        clickLink("pattern:delete");
        // no delete link again
        assertLinkNotPresent("pattern:delete");
    }

    /**
     * Translates between Tapestry index and normal index
     * 
     * @param id
     * @param index
     */
    private void checkCheckbox(String id, int index) {
        String suffix = "";
        if (index > 0) {
            suffix = "$" + (index - 1);
        }
        checkCheckbox(id + suffix);
    }
}
