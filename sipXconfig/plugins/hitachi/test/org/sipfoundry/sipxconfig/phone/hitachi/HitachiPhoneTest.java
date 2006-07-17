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
package org.sipfoundry.sipxconfig.phone.hitachi;

import junit.framework.TestCase;

public class HitachiPhoneTest extends TestCase {
    public void testFactoryRegistered() {
        // FIXME: TestHelper is not accesible here - need to find a way of loading application
        // context
        // PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
        // PhoneContext.CONTEXT_BEAN_NAME);
        // assertNotNull(pc.newPhone(HitachiModel.MODEL_3000));
        // assertNotNull(pc.newPhone(HitachiModel.MODEL_5000));
    }

    public void testGetFileName() throws Exception {
        HitachiPhone phone = new HitachiPhone();
        phone.setSerialNumber("001122334455");
        phone.setTftpRoot("abc");
        assertEquals("abc/334455user.ini", phone.getPhoneFilename());
    }
}
