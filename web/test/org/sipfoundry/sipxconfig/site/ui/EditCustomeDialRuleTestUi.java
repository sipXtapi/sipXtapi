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

import net.sourceforge.jwebunit.WebTestCase;

/**
 * EditCustomeDialRuleTestUi
 */
public class EditCustomeDialRuleTestUi extends WebTestCase {    
    public void setUp() {
        getTestContext().setBaseUrl("http://localhost:8080/sipxconfig");
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
}
