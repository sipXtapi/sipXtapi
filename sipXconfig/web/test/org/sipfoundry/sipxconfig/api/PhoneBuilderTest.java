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
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

public class PhoneBuilderTest extends TestCase {

    public void testToApi() {
        PhoneBuilder builder = new PhoneBuilder();
        org.sipfoundry.sipxconfig.phone.Phone myPhone = new org.sipfoundry.sipxconfig.phone.Phone();
        Phone apiPhone = new Phone();
        ApiBeanUtil.toApiObject(builder, apiPhone, myPhone);
        assertEquals(apiPhone.getModelId(), myPhone.getModelId());
    }
}
