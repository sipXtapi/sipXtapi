/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import com.meterware.httpunit.WebTable;


public class EditPhoneLinesTestUi extends WebTestCase {

    private PhoneTestHelper m_helper;

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(EditPhoneLinesTestUi.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        m_helper = new PhoneTestHelper(tester);
        m_helper.reset();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        dumpResponse(System.err);
    }

    public void testMoveLine() {
        m_helper.seedLine(3);
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");        
        tester.checkCheckbox("selectedRow");
        clickButton("line:moveDown");
        // can't assert rows moved because only one user        
        clickButton("line:moveUp");
        SiteTestHelper.assertNoException(tester);
    }    

    public void testDeleteLine() {
        m_helper.seedLine(1);        
        clickLink("ManagePhones");        
        clickLinkWithText(m_helper.endpoint[0].getSerialNumber());
        clickLinkWithText("Lines");
        
        // checking seed shouldn't be nec. but helpful
        String[][] preDelete = new String[][] { { m_helper.user[0].getDisplayId() } };
        assertTableRowsEqual("line:list", 1, preDelete);
        
        tester.checkCheckbox("selectedRow");
        clickButton("line:delete");
        WebTable lineTable = getDialog().getWebTableBySummaryOrId("line:list");
        assertEquals(1, lineTable.getRowCount());
    }
    
}
