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
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.Arrays;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;
import net.sourceforge.jwebunit.WebTester;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;

/**
 * GatewaysTestUi
 */
public class GatewaysTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(GatewaysTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
    }

    public void testAddGateways() throws Exception {
        clickLink("ListGateways");

        assertTablePresent("table");
        WebTable gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        int lastColumn = gatewaysTable.getColumnCount() - 1;
        assertEquals(3, lastColumn);

        clickLink("addGateway");

        addGateway(null);
        // if validation works we are still on the same page
        assertTableNotPresent("table");

        addGateway("bongo");
        assertTablePresent("table");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", gatewaysTable.getCellAsText(1, lastColumn));

        clickLink("addGateway");
        addGateway("kuku");

        gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        // we should have 2 gateway now
        assertEquals(3, gatewaysTable.getRowCount());
        assertEquals("kukuDescription", gatewaysTable.getCellAsText(2, lastColumn));
    }

    public void testDeleteGateways() throws Exception {
        addTestGateways(getTester(), 10);

        assertTablePresent("table");
        WebTable gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        assertEquals(11, gatewaysTable.getRowCount());

        SiteTestHelper.selectRow(tester, 0, true);
        SiteTestHelper.selectRow(tester, 1, true);
        clickButton("list:gateway:delete");

        assertTablePresent("table");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        assertEquals(9, gatewaysTable.getRowCount());

        for (int i = 0; i < 8; i++) {
            SiteTestHelper.selectRow(tester, i, true);
        }
        clickButton("list:gateway:delete");

        assertTablePresent("table");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("table");
        assertEquals(1, gatewaysTable.getRowCount());
    }

    public void testValidateDescription() {
        clickLink("ListGateways");
        clickLink("addGateway");
        addGateway("bongo");
        clickLinkWithText("bongo");
        int limit = 255; // postgres database field size
        char[] descriptionToLong = new char[limit + 1];
        Arrays.fill(descriptionToLong, 'x');
        char[] descriptionOk = new char[limit];
        Arrays.fill(descriptionOk, 'x');
        setFormElement("gatewayDescription", new String(descriptionToLong));
        tester.clickButton("form:ok");
        // there should be an error now
        assertTextPresent("Enter at most");
        setFormElement("gatewayDescription", new String(descriptionOk));
        tester.clickButton("form:ok");
        SiteTestHelper.assertNoException(tester);
        // we should not get error this time
        assertTablePresent("table");
    }

    /**
     * Fills and submits edit gateway form
     * 
     * @param name response after clicking submit button
     */
    private void addGateway(String name) {
        addGateway(getTester(), name);
    }

    /**
     * Static version to be called from other tests
     * 
     * @param name response after clicking submit button
     */
    public static String[] addGateway(WebTester tester, String name) {
        String[] row = new String[] {
            name + "Name", name + "Address", name + "Description"
        };
        if (null != name) {
            tester.setFormElement("gatewayName", row[0]);
            tester.setFormElement("gatewayAddress", row[1]);
            tester.setFormElement("gatewayDescription", row[2]);
        }
        tester.clickButton("form:ok");
        return row;
    }

    /**
     * Adds number of test gateways to test
     * 
     * @param counter number of gateways to add - names gateway0..gateway'count-1'
     */
    public static void addTestGateways(WebTester tester, int counter) {
        tester.clickLink("ListGateways");

        for (int i = 0; i < counter; i++) {
            tester.clickLink("addGateway");

            // Give the new gateway a name that is extremely unlikely to collide
            // with any existing gateway names
            String gatewayName = "gateway" + i + Long.toString(System.currentTimeMillis());

            addGateway(tester, gatewayName);
        }
    }
}
