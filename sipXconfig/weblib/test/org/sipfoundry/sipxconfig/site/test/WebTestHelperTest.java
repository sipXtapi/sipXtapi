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
package org.sipfoundry.sipxconfig.site.test;

import junit.framework.TestCase;

public class WebTestHelperTest extends TestCase {
    
    public void testApplicationLoad() {
        WebTestHelper helper = WebTestHelper.getInstance();
        assertEquals("weblib", helper.getProject());
    }
}
