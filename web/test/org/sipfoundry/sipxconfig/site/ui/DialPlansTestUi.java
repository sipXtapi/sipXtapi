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

import java.io.IOException;

import junit.framework.TestCase;

import org.xml.sax.SAXException;

import com.meterware.httpunit.SubmitButton;
import com.meterware.httpunit.WebForm;
import com.meterware.httpunit.WebLink;
import com.meterware.httpunit.WebResponse;
import com.meterware.httpunit.WebTable;

/**
 * GatewaysTestUi
 */
public class DialPlansTestUi extends TestCase {
    private WebResponse m_home;

    protected void setUp() throws Exception {
        m_home = TestUiHelper.getHomePage();
        // reset page
        m_home = TestUiHelper.resetDialPlans(m_home);
    }    
    
    public void testAddDialPlans() throws Exception {
        WebLink link = m_home.getLinkWith("List Plans");
        WebResponse listDialPlans = link.click();

        WebTable gatewaysTable = listDialPlans.getTableWithID("dialplan:list");
        int lastColumn = gatewaysTable.getColumnCount() - 1;
        assertEquals(2, lastColumn);

        WebLink addLink = listDialPlans.getLinkWithID("dialplan:add");
        WebResponse addPage = addLink.click();

        addPage = addDialPlan(addPage, null);
        // if validation works we are still on the same page
        assertEquals("Dial Plan Configuration", addPage.getTitle());

        listDialPlans = addDialPlan(addPage, "bongo");
        gatewaysTable = listDialPlans.getTableWithID("dialplan:list");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", gatewaysTable.getCellAsText(1, lastColumn));

        addLink = listDialPlans.getLinkWithID("dialplan:add");
        addPage = addLink.click();

        listDialPlans = addDialPlan(addPage, "kuku");
        gatewaysTable = listDialPlans.getTableWithID("dialplan:list");
        // we should have 2 gateway now
        assertEquals(3, gatewaysTable.getRowCount());
        assertEquals("kukuDescription", gatewaysTable.getCellAsText(2, lastColumn));
    }

    /**
     * Fills and submits edit gateway form
     * 
     * @param page edit gateway page
     * @param name response after clicking submit button
     * @return
     */
    private WebResponse addDialPlan(WebResponse page, String name) throws SAXException,
            IOException {
        WebForm formGateway = page.getFormWithID("dialplan");
        if (null != name) {
            formGateway.setParameter("dialPlanName", name + "Name");
            formGateway.setParameter("dialPlanDescription", name + "Description");
        }
        SubmitButton buttonSave = formGateway.getSubmitButtonWithID("dialplan:save");
        return formGateway.submit(buttonSave);
    }
}
