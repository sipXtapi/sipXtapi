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

import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;

public class PhoneModelTest extends TestCase {
    
    public void testRegister() {
       PhoneModel m = Phone.MODEL;
       assertSame(m, PhoneModel.getModel(m.getBeanId(), m.getModelId()));
    }
    
    public void testRegisterSubclass() {
       PhoneModel m = CiscoModel.MODEL_7905;
       assertSame(m, PhoneModel.getModel(m.getBeanId(), m.getModelId()));
    }

    public void testNotRegistered() {
       PhoneModel unreg = new PhoneModel("beanId", "Label");
       unreg.setRegistered(false);
       try {
           PhoneModel.getModel(unreg.getBeanId(), unreg.getModelId());
           fail();
       } catch (IllegalArgumentException expected) {
           assertTrue(true);
       }
    }
}
