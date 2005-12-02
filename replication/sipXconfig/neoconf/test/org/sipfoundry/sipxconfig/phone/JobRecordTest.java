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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;


public class JobRecordTest extends TestCase {
    
    // probably good candidate for removal - nothing to test really...
    // I am keeping it in case JobRecord takes on new responsibilities
    public void testBasic() {
        JobRecord j = new JobRecord();
        Phone[] phones = new Phone[1];
        j.setPhones(phones);
        assertSame(phones, j.getPhones());
    }
    
}
