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

import com.meterware.httpunit.HttpUnitOptions;

import net.sourceforge.jwebunit.WebTestCase;

/**
 * EditCustomeDialRuleTestUi
 */
public class EditCustomeDialRuleTestUi extends WebTestCase {    
    public void setUp() {
        HttpUnitOptions.setExceptionsThrownOnScriptError(false);
        
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        beginAt("/");
        clickLink("EditCustomDialRule");
    }
    
    public void testDisplay() throws Exception {
        setFormElementWithLabel("Name", "NewName");
        setFormElementWithLabel("Description", "NewDescription");
        setFormElement("permissions","3");
        
        clickButton("rule:save");
        
        assertFormElementPresentWithLabel("Name");
    }
    
    public void testAddDeletePattern() throws Exception {
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
}
