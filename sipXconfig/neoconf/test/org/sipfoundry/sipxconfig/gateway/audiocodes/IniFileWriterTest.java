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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.setting.ModelBuilder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

public class IniFileWriterTest extends TestCase {

    private Setting m_setting;

    protected void setUp() throws Exception {
        ModelBuilder builder = new XmlModelBuilder(TestHelper.getSysDirProperties()
                .getProperty("sysdir.etc"));
        InputStream stream = getClass().getResourceAsStream("IniFileWriter.test.xml");
        m_setting = builder.buildModel(stream, null).copy();
        stream.close();
    }

    /*
     * Test method for
     * 'org.sipfoundry.sipxconfig.gateway.audiocodes.IniFileWriter.IniFileWriter(Writer)'
     */
    public void testIniFileWriter() throws Exception {
        StringWriter writer = new StringWriter();
        IniFileWriter iniWriter = new IniFileWriter(writer);
        m_setting.acceptVisitor(iniWriter);

        BufferedReader expected = new BufferedReader(new InputStreamReader(getClass()
                .getResourceAsStream("IniFileWriter.test.ini")));

        BufferedReader result = new BufferedReader(new StringReader(writer.toString()));

        while (expected.ready()) {
            assertEquals(expected.readLine(), result.readLine());
        }

        expected.close();
        result.close();
    }

    public void testVisitSetting() {
        StringWriter writer = new StringWriter();
        IniFileWriter iniWriter = new IniFileWriter(writer);

        SettingImpl setting = new SettingImpl("abc");
        setting.setValue("xyz");

        setting.acceptVisitor(iniWriter);
    }

    public void testVisitSettingSet() {
        StringWriter writer = new StringWriter();
        IniFileWriter iniWriter = new IniFileWriter(writer);

        SettingSet first = new SettingSet("first");
        SettingSet second = new SettingSet("second");

        first.acceptVisitor(iniWriter); // without new line
        iniWriter.setGroupNameSuffix(" XXX");
        second.acceptVisitor(iniWriter); // with new line
        String actual = TestUtil.cleanEndOfLines("[first]\n\n\n[second XXX]\n\n");        
        assertEquals(actual, writer.toString());
    }
}
