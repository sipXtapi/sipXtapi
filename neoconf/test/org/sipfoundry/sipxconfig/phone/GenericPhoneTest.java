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

/**
 * Comments
 */
public class GenericPhoneTest extends TestCase {
    
    public void testFactoryCreation() {
        PhoneTestHelper helper = PhoneTestHelper.createHelper();
        PhoneContext phoneContext = helper.getPhoneContext();
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        Phone phone = phoneContext.getPhone(endpoint);
        assertNotNull(phone);
    }
}
