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

import java.io.InputStream;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;

public class SettingModelImplTest extends TestCase {
    
    public void testAddHandler() throws Exception {
        SettingValue2 ten = new SettingValueImpl("10");
        InputStream in = getClass().getResourceAsStream("birds.xml");
        Setting birds = TestHelper.loadSettings(in);                
        Setting peewee = birds.getSetting("flycatcher/peewee");
        Setting canyonTowhee = birds.getSetting("towhee/canyon");
        
        MockControl handlerCtrl = MockControl.createControl(SettingValueHandler.class);
        SettingValueHandler handler = (SettingValueHandler) handlerCtrl.getMock();
        handlerCtrl.expectAndReturn(handler.getSettingValue(peewee), ten);
        handlerCtrl.expectAndReturn(handler.getSettingValue(canyonTowhee), null);
        handlerCtrl.replay();

        SettingModelImpl model = new SettingModelImpl();
        model.setSettings(birds);
        
        model.addSettingValueHandler(handler);
        assertEquals("10", model.getSettingValue(peewee, null));
        assertNull(model.getSettingValue(canyonTowhee, null));

        handlerCtrl.verify();
    }

}
