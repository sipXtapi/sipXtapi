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
package com.pingtel.commserver.utility;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.net.URL;
import java.util.Properties;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;

public class GlobalConfigUIHelperTest extends XMLTestCase {
    public GlobalConfigUIHelperTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerateXML() throws Exception {
        URL url = getClass().getResource("config.defs");
        GlobalConfigUIHelper helper = new GlobalConfigUIHelper(url.getPath());
        String xmlOut = helper.generateXML();
        Reader controlXml = new InputStreamReader(getClass().getResourceAsStream(
                "config.defs.xml"));
        assertXMLEqual(controlXml, new StringReader(xmlOut));
    }

    public void testWriteToConfigFile() throws Exception {
        File file = File.createTempFile("test", null);
        file.deleteOnExit();
        String path = file.getAbsolutePath();
        GlobalConfigUIHelper helper = new GlobalConfigUIHelper(path);

        Properties controlProps = new Properties();
        controlProps.setProperty("bongo", "bongovalue");
        controlProps.setProperty("kuku", "kukuvalue");
        controlProps.setProperty("definitionlist", "bongo;kuku");

        helper.writeToConfigFile(controlProps);

        controlProps.remove("definitionlist");

        Properties testProps = new Properties();
        testProps.load(new FileInputStream(path));

        assertEquals(controlProps, testProps);
    }

    public void testReplaceAndWrite() throws Exception {
        String testPath = createTestFile();
        GlobalConfigUIHelper testHelper = new GlobalConfigUIHelper(testPath);

        Properties mods = new Properties();
        mods.setProperty(GlobalConfigUIHelper.SIPXCHANGE_DOMAIN_NAME, "new.domain.name");
        mods.setProperty(GlobalConfigUIHelper.SIPXCHANGE_REALM, "new.realm.name");
        
        testHelper.replaceAndWrite(mods);
        
        // get Properties and check the difference
        Properties control = new Properties();
        control.load(new FileInputStream(getControlPath()));
        Properties test = new Properties();
        test.load(new FileInputStream(testPath));
        
        // make sure we managed to set it correctly
        assertEquals("new.domain.name",test.getProperty(GlobalConfigUIHelper.SIPXCHANGE_DOMAIN_NAME));
        assertEquals("new.realm.name",test.getProperty(GlobalConfigUIHelper.SIPXCHANGE_REALM));
        
        // the only difference should be those 2 properties
        test.remove(GlobalConfigUIHelper.SIPXCHANGE_DOMAIN_NAME);
        test.remove(GlobalConfigUIHelper.SIPXCHANGE_REALM);
        
        control.remove(GlobalConfigUIHelper.SIPXCHANGE_DOMAIN_NAME);
        control.remove(GlobalConfigUIHelper.SIPXCHANGE_REALM);
        
        assertEquals(control,test);
    }

    private String getControlPath() {
        URL url = getClass().getResource("config.defs");
        String controlPath = url.getPath();
        return controlPath;
    }

    /**
     * Creates a new temporary reference test file.
     * 
     * @return
     */
    private String createTestFile() throws Exception {
        File file = File.createTempFile("test", null);
        String path = file.getAbsolutePath();

        FileWriter writer = new FileWriter(path);
        FileReader reader = new FileReader(getControlPath());
        
        char charBuff[] = new char[200];
        int len = 0;
        while( (len = reader.read(charBuff, 0, 200)) > 0 )
        {
            writer.write(charBuff, 0, len);
        }        
        
        reader.close();
        writer.close();
        return path;
    }
}
