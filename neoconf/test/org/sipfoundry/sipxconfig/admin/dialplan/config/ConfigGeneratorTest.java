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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.File;
import java.io.FileInputStream;
import java.io.StringWriter;
import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;

/**
 * ConfigGeneratorTest
 */
public class ConfigGeneratorTest extends XMLTestCase {
    public ConfigGeneratorTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetFileContent() throws Exception {
        MockControl controlPlan = MockControl.createStrictControl(DialPlanManager.class);
        DialPlanManager empty = (DialPlanManager) controlPlan.getMock();
        empty.getGenerationRules();
        controlPlan.setReturnValue(Collections.EMPTY_LIST);
        controlPlan.replay();        
        
        EmergencyRouting er = new EmergencyRouting();
        
        ConfigGenerator generator = new ConfigGenerator();        
        generator.generate(er);
        generator.generate(empty);
        checkConfigFileGeneration(generator, new AuthRules(), ConfigFileType.AUTH_RULES);
        checkConfigFileGeneration(generator, new MappingRules(), ConfigFileType.MAPPING_RULES);
        checkConfigFileGeneration(generator, new FallbackRules(), ConfigFileType.FALLBACK_RULES);
        controlPlan.verify();
    }

    /**
     * Execute test for a single configuration type. Tries to generate it
     * directly and generate pretty formatted text through generator.
     */
    private void checkConfigFileGeneration(ConfigGenerator generator, XmlFile configFile,
            ConfigFileType type) throws Exception {
        Document document = configFile.getDocument();
        StringWriter writer = new StringWriter();
        document.write(writer);
        String xml = generator.getFileContent(type);
        assertXMLEqual("Comparing: " + type, writer.getBuffer().toString(), xml);
    }
    
    public void testActivate() throws Exception {
        MockControl controlPlan = MockControl.createStrictControl(DialPlanManager.class);
        DialPlanManager empty = (DialPlanManager) controlPlan.getMock();
        empty.getGenerationRules();
        controlPlan.setReturnValue(Collections.EMPTY_LIST);
        controlPlan.replay();        
        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(empty);
        File x = File.createTempFile("test", "in");
        String tmpDir = x.getParent();
        x.deleteOnExit();
        generator.activate(tmpDir);
        
        File[] files = new File[3];
        files[0] = new File(tmpDir, "mappingrules.xml.in");
        files[1] = new File(tmpDir, "fallbackrules.xml.in");
        files[2] = new File(tmpDir, "authrules.xml.in");
        
        for (int i = 0; i < files.length; i++) {
            File file = files[i];
            assertTrue( file.canRead() );
            FileInputStream stream = new FileInputStream(file);
            assertEquals( '<', stream.read() );
            stream.close();
            file.deleteOnExit();
        }
        controlPlan.verify();
    }
}
