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
package org.sipfoundry.sipxconfig;

import junit.framework.TestCase;

/**
 * Load the spring context, nothing else.  If gaurontee some other unittest
 * will cover this, this unittest may not be nec.
 */
public class SipxconfigApplicationContextTest extends TestCase {
    
    public void testLoadingApplicationContext() throws Exception {
        assertNotNull(TestHelper.getApplicationContext());
    }

}
