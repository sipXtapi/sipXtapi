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
package org.sipfoundry.sipxconfig.setting;

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class ModelFilesContextTest extends TestCase {
    
    public void testCache() {
        ModelFilesContext mfc = (ModelFilesContext) TestHelper.getApplicationContext().getBean("modelFilesContext");
        Setting userSettings1 = mfc.loadModelFile("user-settings.xml");
        Setting userSettings2 = mfc.loadModelFile("user-settings.xml");
        assertSame(userSettings1, userSettings2);                
    }

}
