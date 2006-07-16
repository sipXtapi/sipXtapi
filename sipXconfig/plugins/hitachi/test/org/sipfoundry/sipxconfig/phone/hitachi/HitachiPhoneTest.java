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

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class HitachiPhoneTest extends TestCase {
    public void testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(
             PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(HitachiModel.MODEL_3000));
        assertNotNull(pc.newPhone(HitachiModel.MODEL_5000));
    }    

}
