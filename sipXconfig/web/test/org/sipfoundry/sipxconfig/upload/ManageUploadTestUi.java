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
package org.sipfoundry.sipxconfig.upload;

import java.io.File;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

import net.sourceforge.jwebunit.WebTestCase;

public class ManageUploadTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageUploadTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink("link:resetUpload");
    }

    public void testDisplay() {
        clickLink("link:upload");
        SiteTestHelper.assertNoException(tester);
    }

    public void testDelete() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");
        int tableCount = SiteTestHelper.getRowCount(tester, "upload:list");        
        assertEquals(2, tableCount);
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:delete");
        int nextTableCount = SiteTestHelper.getRowCount(tester, "upload:list");        
        assertEquals(1, nextTableCount);
    }
    
    public void testActivation() throws Exception {
        seedUpload();
        SiteTestHelper.home(tester);
        clickLink("link:upload");

        // activate
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:activate");
        String[][] expectedData = {
                // [] Name Active Description
                {
                    "manage uploads seed", "true", "Unmanaged TFTP",  ""
                }
            };
        assertTableRowsEqual("upload:list", 1, expectedData);

        // inactivate
        SiteTestHelper.enableCheckbox(tester, "checkbox", 0, true);
        clickButton("upload:inactivate");
        expectedData[0][1] = "false";
        assertTableRowsEqual("upload:list", 1, expectedData);        
    }
    
    private void seedUpload() throws Exception {
        clickLink("link:newUpload");
        setFormElement("name", "manage uploads seed");
        clickButton("form:apply");        
        File f = File.createTempFile("manage-upload", ".dat");
        assertTrue(getDialog().getForm().hasParameterNamed("promptUpload"));
        getDialog().getForm().setParameter("promptUpload", f);
        clickButton("form:ok");
    }
}
