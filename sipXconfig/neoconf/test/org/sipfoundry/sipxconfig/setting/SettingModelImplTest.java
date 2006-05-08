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

import org.easymock.MockControl;

public class SettingModelImplTest extends TestCase {
    
    public void testAddHandler() {
        SettingValue2 pin = new SettingValueImpl("pin");
        MockControl handlerCtrl = MockControl.createControl(SettingValueHandler.class);
        SettingValueHandler handler = (SettingValueHandler) handlerCtrl.getMock();
        handlerCtrl.expectAndReturn(handler.getSettingValue("/tree/oak"), pin);
        handlerCtrl.expectAndReturn(handler.getSettingValue("/tree/pine"), null);
        handlerCtrl.replay();

        SettingModel2 model = new SettingModelImpl();
        model.addSettingValueHandler(handler);
        assertEquals("pin", model.getSettingValue("/tree/oak"));
        assertNull(model.getSettingValue("/tree/pine"));

        handlerCtrl.verify();
    }

}
