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
import org.sipfoundry.sipxconfig.site.gateway.GatewaysTestUi;

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
            "Emergency", "false", "Emergency", "Emergency dialing plan"
        }, {
            "International", "false", "International", "International dialing"
        }, {
            "AutoAttendant", "true", "Attendant", "Default autoattendant dialing plan"
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
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        SiteTestHelper.setScriptingEnabled(true);
        clickLink("FlexibleDialPlan");
    }

    public void testDisplayAndClean() {
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
        // remove all
        for (int i = 0; i < DEFAULTS.length; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("dialplan:delete");
        // should be empty now
        WebTable rulesTable = getDialog().getWebTableBySummaryOrId("dialplan:list");
        assertEquals(1, rulesTable.getRowCount());
        // test revert
        clickButton("dialplan:revert");
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);
    }

    public void testViewRules() throws Exception {
        for (int i = 0; i < DEFAULTS.length; i++) {
            String name = DEFAULTS[i][0];
            clickLinkWithText(name);
            assertFormElementEquals("name", name);
            assertElementPresent("item:name");
            assertElementPresent("item:enabled");
            assertElementPresent("item:description");
            // all rules except "internal" have gateways panel
            if (!name.startsWith("Internal") && !name.startsWith("AutoAttendant")) {
                checkGateways();
            }
            setFormElement("name", "");
            clickButton("form:ok");
            // if validation kicks in we are on the same page
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertUserError(tester);
            setFormElement("name", name + "changed");
            clickButton("form:ok");
            // a link corresponding to new name should be in now
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);
            assertLinkPresentWithText(name + "changed");
        }
    }

    public void testCustomRuleAdd() throws Exception {
        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            selectOption("ruleTypeSelection", "Custom");

            SiteTestHelper.assertNoException(tester);

            assertLinkNotPresent("pattern:delete");
            assertLinkPresent("pattern:add");

            setFormElement("name", row[0]);
            setFormElement("description", row[2]);
            // dial pattern prefix
            setFormElement(SiteTestHelper.getIndexedId("prefix", 0), "333");
            // call pattern prefix
            setFormElement(SiteTestHelper.getIndexedId("prefix", 1), "444");

            checkAddDeletePattern();

            checkGateways();

            clickButton("form:ok");
            assertTextInTable("dialplan:list", row[2]);
            assertLinkPresentWithText(row[0]);
        }
    }

    public void testInternationalRuleAdd() {
        for (int i = 0; i < 4; i++) {
            String name = "international" + i;
            String description = "international description" + i;
            // 2 means custom...
            selectOption("ruleTypeSelection", "International");

            setFormElement("name", name);
            setFormElement("description", description);
            // dial pattern prefix
            setFormElement("internationalPrefix", "100" + i);

            clickButton("form:ok");

            assertTextInTable("dialplan:list", description);
            assertLinkPresentWithText(name);
        }
    }

    public void testAttendantRuleAdd() throws Exception {
        for (int i = 0; i < NAMES.length; i++) {
            String[] row = NAMES[i];
            selectOption("ruleTypeSelection", "Attendant");

            setFormElement("name", row[0]);
            setFormElement("description", row[2]);
            // dial pattern prefix
            setFormElement("extension", "33344" + i);
            setFormElement("autoAttendantAliases", "");

            clickButton("form:ok");
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);
            
            assertTextInTable("dialplan:list", row[2]);
            assertLinkPresentWithText(row[0]);
        }
    }

    public void testMove() {
        clickButton("dialplan:revert");
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("dialplan:move:up");
        // no changes
        SiteTestHelper.assertNoException(getTester());
        assertTableRowsEqual("dialplan:list", 1, DEFAULTS);

        // move first row down
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("dialplan:move:down");
        SiteTestHelper.assertNoException(getTester());
        WebTable rulesTable = getTester().getDialog().getWebTableBySummaryOrId("dialplan:list");
        assertEquals(DEFAULTS[0][0], rulesTable.getCellAsText(2, 1));
        assertEquals(DEFAULTS[1][0], rulesTable.getCellAsText(1, 1));
        assertEquals(DEFAULTS[2][0], rulesTable.getCellAsText(3, 1));
    }

    private void checkAddDeletePattern() throws Exception {
        // no delete link
        assertLinkNotPresent("pattern:delete");
        // add 2 more
        clickLink("pattern:add");
        SiteTestHelper.assertNoException(tester);
        clickLink("pattern:add");

        // delete 2

        clickLink("pattern:delete");
        clickLink("pattern:delete");
        // no delete link again
        assertLinkNotPresent("pattern:delete");
    }

    private void checkGateways() throws Exception {
        assertLinkPresent("gateway:add");
        assertLinkPresent("gateway:select");

        assertButtonPresent("gateway:remove");
        assertButtonPresent("gateway:moveUp");
        assertButtonPresent("gateway:moveDown");

        assertTablePresent("list:gateway");

        // add gateways

        final int gatewayCount = 4;
        String[][] gateways = new String[gatewayCount][];

        for (int i = 0; i < gatewayCount; i++) {
            clickLink("gateway:add");
            SiteTestHelper.assertNoException(tester);
            SiteTestHelper.assertNoUserError(tester);

            // Give the new gateway a name that is extremely unlikely to collide
            // with any existing gateway names
            String gatewayName = "gateway" + i + Long.toString(System.currentTimeMillis());

            gateways[i] = GatewaysTestUi.addGateway(tester, gatewayName);
            SiteTestHelper.assertNoException(tester);
        }

        assertEquals(gatewayCount + 1, SiteTestHelper.getRowCount(tester, "list:gateway"));
        assertTableRowsEqual("list:gateway", 1, gateways);

        // test moving up/down
        SiteTestHelper.selectRow(tester, 0, true);
        clickButton("gateway:moveUp");
        // no changes expected - cannot move up
        SiteTestHelper.assertNoException(getTester());
        assertTableRowsEqual("list:gateway", 1, gateways);

        // move down one row - no other changes expected
        clickButton("gateway:moveDown");
        WebTable gatewayTable = getTester().getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(gateways[0][0], gatewayTable.getCellAsText(2, 1));
        assertEquals(gateways[1][0], gatewayTable.getCellAsText(1, 1));
        assertEquals(gateways[2][0], gatewayTable.getCellAsText(3, 1));

        // click the gateway link - and then click cancel
        clickLinkWithText(gateways[0][0]);
        SiteTestHelper.assertNoException(tester);
        clickButton("form:cancel");
        SiteTestHelper.assertNoException(tester);

        // test removal
        for (int i = 0; i < gatewayCount; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("gateway:remove");
        // only header in the table
        assertEquals(1, SiteTestHelper.getRowCount(tester, "list:gateway"));

        // test adding existing gateways
        clickLink("gateway:select");
        // one more time check if gateway edit is working
        clickLinkWithText(gateways[0][0]);
        SiteTestHelper.assertNoException(tester);
        clickButton("form:cancel");

        SiteTestHelper.assertNoException(tester);
        for (int i = 0; i < gatewayCount; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("select:gateway:save");
        assertEquals(gatewayCount + 1, SiteTestHelper.getRowCount(tester, "list:gateway"));
    }
}
