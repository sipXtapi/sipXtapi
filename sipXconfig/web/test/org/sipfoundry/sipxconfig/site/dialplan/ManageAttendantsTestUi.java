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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.io.IOException;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManageAttendantsTestUi extends WebTestCase {
    
    private static final String SEED_DESCRIPTION = "ManageAttendantsTestUi seed description";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageAttendantsTestUi.class);
    }
   
    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());        
        SiteTestHelper.home(getTester());
        clickLink("resetDialPlans");
        clickLink("ManageAttendants");
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testAddAttendants() throws IOException {
        seedAttendants(3);
        String [][] expectedData = {
            //            Name             Ext     Description
            { "ManageAttendantsTestUi 0", "300", SEED_DESCRIPTION },
            { "ManageAttendantsTestUi 1", "301", SEED_DESCRIPTION },
            { "ManageAttendantsTestUi 2", "302", SEED_DESCRIPTION }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);
    }
    
    public void testEditAttendants() throws IOException {
        seedAttendants(2);
        clickLinkWithText("ManageAttendantsTestUi 1");
        assertFormPresent("attendant:form");
        setFormElement("name", "Name edited");
        clickButton("form:ok");
        String [][] expectedData = {
                //            Name             Ext     Description
                { "ManageAttendantsTestUi 0", "300", SEED_DESCRIPTION },
                { "Name edited", "301", SEED_DESCRIPTION }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);
    }
    
    public void testDeleteAttendants() throws IOException {
        seedAttendants(4);
        // delete 2nd and last for no brilliant reason 
        SiteTestHelper.enableCheckbox(tester, "selectedRow", 1, true);
        SiteTestHelper.enableCheckbox(tester, "selectedRow", 3, true);
        clickButton("list:attendant:delete");
        String [][] expectedData = {
                //            Name             Ext     Description
                { "ManageAttendantsTestUi 0", "300", SEED_DESCRIPTION },
                { "ManageAttendantsTestUi 2", "302", SEED_DESCRIPTION }
        };
        assertTableRowsEqual("list:attendant", 1, expectedData);        
    }
    
    private void seedAttendants(int count) throws IOException {
        String promptTestFile = EditAutoAttendantTestUi.seedPromptFile();
        for (int i = 0; i < count; i++) {
            clickLink("addAttendant");   
            setFormElement("name", "ManageAttendantsTestUi " + i);
            setFormElement("description", SEED_DESCRIPTION);
            setFormElement("extension", Integer.toString(300 + i));
            selectOption("prompt", promptTestFile);
            clickButton("form:ok");
            SiteTestHelper.assertNoException(tester);
        }
    }
}
