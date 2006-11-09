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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class ModelFilesContextImplTest extends TestCase {
    private static String DIR = "tmp";
    private ModelFilesContextImpl m_impl;

    public void setUp() {
        m_impl = new ModelFilesContextImpl();
        m_impl.setConfigDirectory(DIR);
    }

    public void testGetFile() {
        File modelFile = m_impl.getModelFile("basename", "manu");
        assertEquals("basename", modelFile.getName());
        assertEquals(getFile("manu", "basename"), modelFile);
    }
    
    public void testLoadModelFile() {
        SettingSet setting = new SettingSet();
        setting.setName("bongo");

        IMocksControl control = EasyMock.createControl();
        ModelBuilder builder = control.createMock(ModelBuilder.class);
        builder.buildModel(getFile("aa", "bb"));
        control.andReturn(setting);
        control.replay();

        m_impl.setModelBuilder(builder);
        Setting loadedSetting = m_impl.loadModelFile("bb", "aa");

        assertEquals("bongo", loadedSetting.getName());

        control.verify();
    }
    
    private static File getFile(String manu, String base) {
        return new File(DIR + File.separator + manu + File.separator + base);
    }
}
