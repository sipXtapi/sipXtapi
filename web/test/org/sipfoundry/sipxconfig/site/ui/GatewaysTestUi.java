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

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

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
        beginAt("/");
        clickLink("resetDialPlans");
    }

    public void testAddGateways() throws Exception {
        clickLink("ListGateways");

        assertTablePresent("list:gateway");
        WebTable gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        int lastColumn = gatewaysTable.getColumnCount() - 1;
        assertEquals(3, lastColumn);

        clickLink("addGateway");

        addGateway(null);
        // if validation works we are still on the same page
        assertTableNotPresent("list:gateway");

        addGateway("bongo");
        assertTablePresent("list:gateway");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", gatewaysTable.getCellAsText(1, lastColumn));

        clickLink("addGateway");
        addGateway("kuku");

        gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        // we should have 2 gateway now
        assertEquals(3, gatewaysTable.getRowCount());
        assertEquals("kukuDescription", gatewaysTable.getCellAsText(2, lastColumn));
    }

    public void testDeleteGateways() throws Exception {
        clickLink("ListGateways");

        for (int i = 0; i < 10; i++) {
            clickLink("addGateway");
            addGateway("gateway" + i);
        }

        assertTablePresent("list:gateway");
        WebTable gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(11, gatewaysTable.getRowCount());

        checkCheckbox("selectedRow");
        checkCheckbox("selectedRow$0");
        clickButton("list:gateway:delete");

        assertTablePresent("list:gateway");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(9, gatewaysTable.getRowCount());

        checkCheckbox("selectedRow");
        for (int i = 0; i < 7; i++) {
            checkCheckbox("selectedRow$" + i);
        }
        clickButton("list:gateway:delete");

        assertTablePresent("list:gateway");
        gatewaysTable = getDialog().getWebTableBySummaryOrId("list:gateway");
        assertEquals(1, gatewaysTable.getRowCount());
    }

    /**
     * Fills and submits edit gateway form
     * 
     * @param name response after clicking submit button
     */
    private void addGateway(String name) {
        if (null != name) {
            setFormElement("gatewayName", name + "Name");
            setFormElement("gatewayAddress", name + "Address");
            setFormElement("gatewayDescription", name + "Description");
        }
        clickButton("gateway:save");
    }
}
