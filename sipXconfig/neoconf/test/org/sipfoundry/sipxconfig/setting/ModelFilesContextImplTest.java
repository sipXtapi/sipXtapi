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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;

import junit.framework.TestCase;

import org.easymock.MockControl;

public class ModelFilesContextImplTest extends TestCase {
    private static String DIR = "tmp";
    private ModelFilesContextImpl m_impl;

    public void setUp() {
        m_impl = new ModelFilesContextImpl();
        m_impl.setConfigDirectory(DIR);
    }

    public void testGetFile() {
        File modelFile = m_impl.getModelFile("manu", "basename");
        assertEquals("basename", modelFile.getName());
        assertEquals(getFile("manu", "basename"), modelFile);
    }

    public void testLoadModelFile() {
        SettingSet setting = new SettingSet();
        setting.setValue("555");
        setting.setName("bongo");

        MockControl control = MockControl.createControl(ModelBuilder.class);
        ModelBuilder builder = (ModelBuilder) control.getMock();
        builder.buildModel(getFile("aa", "bb"));
        control.setReturnValue(setting);
        control.replay();

        m_impl.setModelBuilder(builder);
        Setting loadedSetting = m_impl.loadModelFile("aa", "bb");

        assertEquals("555", loadedSetting.getValue());
        assertEquals("bongo", loadedSetting.getName());

        control.verify();
    }

    private static File getFile(String manu, String base) {
        return new File(DIR + File.separator + manu + File.separator + base);
    }
}
