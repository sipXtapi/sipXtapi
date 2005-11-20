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
import java.io.FilenameFilter;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.TestUtil;

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
    
    public void testGetExtension() {
        assertEquals("xml", ModelFilesContextImpl.getExtension("a.xml"));
    }
    
    public void testChompExtension() {
        assertEquals("a", ModelFilesContextImpl.chompExtension("a.xml"));
    }

    public void testGetCandidates() {
        String[] details = new String[] {
                "model", "version"
        };
        String[] actual = ModelFilesContextImpl.getCandidates("basename.xml", details);
        assertEquals("basename.xml", actual[0]);
        assertEquals("basename_model.xml", actual[1]);
        assertEquals("basename_model_version.xml", actual[2]);
    }
    
    public void testGlobToPattern() {
        assertEquals(".*", ModelFilesContextImpl.globToPattern("xxx"));
        assertEquals("a_.*_b", ModelFilesContextImpl.globToPattern("a_xxx_b"));
        assertEquals("a_.*_1[.]5", ModelFilesContextImpl.globToPattern("a_xxx_1.5"));
        assertEquals("a_.*x_1[.]5", ModelFilesContextImpl.globToPattern("a_xxxx_1.5"));
    }
    
    public void testBasenameFilterAccept() {
        String[] details = new String[] {
                "model", "version"
        };
        String[] candidates = ModelFilesContextImpl.getCandidates("basename.xml", details);        
        File dir = new File(".");
        FilenameFilter filter = new ModelFilesContextImpl.BasenameFilter(candidates);

        assertTrue(filter.accept(dir, "basename.xml"));
        assertTrue(filter.accept(dir, "basename_modelxxx.xml"));
        assertTrue(filter.accept(dir, "basename_xxx_version.xml"));       
        assertTrue(filter.accept(dir, "basename_model.xml"));       
        assertTrue(filter.accept(dir, "basename_model_xxx.xml"));       
        assertTrue(filter.accept(dir, "basename_model_version.xml"));       

        assertFalse(filter.accept(dir, "xbasename.xml"));
        assertFalse(filter.accept(dir, "basenamex.xml"));
        assertFalse(filter.accept(dir, "basename_nomatch.xml"));
        assertFalse(filter.accept(dir, "basename_version.xml"));       
        assertFalse(filter.accept(dir, "basename_xxxx_version.xml"));       
    }
    
    public void testCompareDetailLevel() {
        assertTrue(ModelFilesContextImpl.compareDetailLevel("a", "b") == 0);
        assertTrue(ModelFilesContextImpl.compareDetailLevel("a", "a_b") == -1);
        assertTrue(ModelFilesContextImpl.compareDetailLevel("a_b", "a") == 1);
        assertTrue(ModelFilesContextImpl.compareDetailLevel("a_b", "a_b") == 0);
        assertTrue(ModelFilesContextImpl.compareDetailLevel("abc_def", "ghi_jkl") == 0);
        assertTrue(ModelFilesContextImpl.compareDetailLevel("a_b", "xyz_pdq") == 0);
    }
    
    public void testEncode() {
        assertEquals("0", ModelFilesContextImpl.encode("a"));
        assertEquals("01", ModelFilesContextImpl.encode("axxx"));
        assertEquals("010", ModelFilesContextImpl.encode("axxxx"));
        assertEquals("000001", ModelFilesContextImpl.encode("7900_xxx"));
        assertEquals("100000", ModelFilesContextImpl.encode("xxx_1.60"));        
    }
    
    public void testReversePad() {
        assertEquals("cba111", ModelFilesContextImpl.reversePad("abc", 6));
    }
    
    public void testCompareSpecificity() {
        assertEquals("0", "a".replaceAll("[^x]", "0"));
        assertEquals("01", "axxx".replaceAll("[^x]", "0").replaceAll("(xxx)", "1"));
        assertEquals("010", "axxxx".replaceAll("[^x]", "0").replaceAll("(xxx)", "1").replaceAll("x", "0"));
        
        assertTrue(ModelFilesContextImpl.compareSpecificity("a", "b") == 0);        
        assertTrue(ModelFilesContextImpl.compareSpecificity("xxx", "b") < 0);        
        assertTrue(ModelFilesContextImpl.compareSpecificity("b", "xxx") > 0);   
        assertTrue(ModelFilesContextImpl.compareSpecificity("7900_xxx", "xxx_1.60") < 0);
        assertTrue(ModelFilesContextImpl.compareSpecificity("79xxx_xxx", "xxx_xxx") > 0);
    }

    public void testLoadModelFile() {
        SettingSet setting = new SettingSet();
        setting.setValue("555");
        setting.setName("bongo");

        MockControl control = MockControl.createControl(ModelBuilder.class);
        ModelBuilder builder = (ModelBuilder) control.getMock();
        builder.buildModel(getFile("aa", "bb"), null);
        control.setReturnValue(setting);
        control.replay();

        m_impl.setModelBuilder(builder);
        Setting loadedSetting = m_impl.loadModelFile("bb", "aa");

        assertEquals("555", loadedSetting.getValue());
        assertEquals("bongo", loadedSetting.getName());

        control.verify();
    }
    
    public void testLoadModelFileWithDetailsFiles() {
        SettingSet setting = new SettingSet();
        SettingSet setting2 = new SettingSet();
        SettingSet setting3 = new SettingSet();

        File dir = new File(TestUtil.getTestSourceDirectory(this.getClass()));
        MockControl control = MockControl.createControl(ModelBuilder.class);
        ModelBuilder builder = (ModelBuilder) control.getMock();
        control.expectAndReturn(builder.buildModel(new File(dir, "basename.xml"), null), setting);
        control.expectAndReturn(builder.buildModel(new File(dir, "basename_model.xml"), setting), setting2);
        control.expectAndReturn(builder.buildModel(new File(dir, "basename_model_version.xml"), setting2), setting3);
        control.replay();
        
        ModelFilesContextImpl impl = new ModelFilesContextImpl();
        impl.setConfigDirectory(dir.getAbsolutePath());
        impl.setModelBuilder(builder);
        String[] details = new String[] {
                "model", "version"
        };
        Setting loadedSetting = impl.loadModelFile("basename.xml", null, details);
        assertSame(setting3, loadedSetting);
        
        control.verify();
    }

    private static File getFile(String manu, String base) {
        return new File(DIR + File.separator + manu + File.separator + base);
    }
}
