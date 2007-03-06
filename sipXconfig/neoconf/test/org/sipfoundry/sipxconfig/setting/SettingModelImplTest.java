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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingTest.BirdWithSettings;

public class SettingModelImplTest extends TestCase {

    public void testAddHandler() throws Exception {
        SettingValue ten = new SettingValueImpl("10");
        InputStream in = getClass().getResourceAsStream("birds.xml");
        Setting birds = TestHelper.loadSettings(in);
        Setting peewee = birds.getSetting("flycatcher/peewee");
        Setting canyonTowhee = birds.getSetting("towhee/canyon");

        IMocksControl handlerCtrl = EasyMock.createControl();
        SettingValueHandler handler = handlerCtrl.createMock(SettingValueHandler.class);
        handler.getSettingValue(peewee);
        handlerCtrl.andReturn(ten);
        handler.getSettingValue(canyonTowhee);
        handlerCtrl.andReturn(null);
        handlerCtrl.replay();

        BeanWithSettings bean = new BirdWithSettings();
        BeanWithSettingsModel model = bean.getSettingModel2();
        bean.setSettings(birds);

        bean.addDefaultSettingHandler(handler);
        assertEquals(ten, model.getSettingValue(peewee));
        assertNull(model.getSettingValue(canyonTowhee));

        handlerCtrl.verify();
    }

}
