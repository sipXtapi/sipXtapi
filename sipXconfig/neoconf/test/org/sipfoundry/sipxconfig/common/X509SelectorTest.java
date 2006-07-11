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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class X509SelectorTest extends TestCase {
    
    public void testAvailableAlgorithm() {
        String alg = new X509Selector().getAvailableAlgorithm();
        // only works w/2 VM vendors. If others are known to work, add here
        assertTrue(alg.equals("SunX509") || alg.equals("IbmX509"));
    }
    
    public void testUnavailableAlgorithm() {
        assertNull(new X509Selector().getAvailableAlgorithm(new String[] { "bogus" }));
    }

}
