/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class CopyFromCacheInterceptorTest extends TestCase {
    
    public void testCache() {
        ModelFilesContext mfc = (ModelFilesContext) TestHelper.getApplicationContext().getBean("modelFilesContext");
        Setting userSettings1 = mfc.loadModelFile("commserver/user-settings.xml");
        Setting userSettings2 = mfc.loadModelFile("commserver/user-settings.xml");
        // 2nd should be a copy. if this fails, cacheoncopy is failing
        assertNotSame(userSettings1, userSettings2);                
    }

}
