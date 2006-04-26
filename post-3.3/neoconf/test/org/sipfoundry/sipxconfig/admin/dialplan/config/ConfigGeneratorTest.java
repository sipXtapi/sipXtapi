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

import java.io.StringWriter;
import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;

/**
 * ConfigGeneratorTest
 */
public class ConfigGeneratorTest extends XMLTestCase {
    public ConfigGeneratorTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetFileContent() throws Exception {
        MockControl planCtrl = MockControl.createStrictControl(DialPlanContext.class);
        DialPlanContext empty = (DialPlanContext) planCtrl.getMock();
        empty.getGenerationRules();
        planCtrl.setReturnValue(Collections.EMPTY_LIST);
        empty.getAttendantRules();
        planCtrl.setReturnValue(Collections.EMPTY_LIST);
        planCtrl.replay();

        EmergencyRouting er = new EmergencyRouting();

        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(empty, er);

        AuthRules authRules = new AuthRules();
        authRules.begin();
        checkConfigFileGeneration(generator, authRules, ConfigFileType.AUTH_RULES);
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        checkConfigFileGeneration(generator, mappingRules, ConfigFileType.MAPPING_RULES);
        FallbackRules fallbackRules = new FallbackRules();
        fallbackRules.begin();
        checkConfigFileGeneration(generator, fallbackRules, ConfigFileType.FALLBACK_RULES);
        planCtrl.verify();
    }

    /**
     * Execute test for a single configuration type. Tries to generate it directly and generate
     * pretty formatted text through generator.
     */
    private void checkConfigFileGeneration(ConfigGenerator generator, XmlFile configFile,
            ConfigFileType type) throws Exception {
        Document document = configFile.getDocument();
        StringWriter writer = new StringWriter();
        document.write(writer);
        String xml = generator.getFileContent(type);
        assertXMLEqual("Comparing: " + type, writer.getBuffer().toString(), xml);
    }
}
