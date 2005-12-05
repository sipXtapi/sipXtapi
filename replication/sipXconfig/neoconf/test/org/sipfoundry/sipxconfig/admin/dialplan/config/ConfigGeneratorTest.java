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
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
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
        MockControl controlPlan = MockControl.createStrictControl(DialPlanContext.class);
        DialPlanContext empty = (DialPlanContext) controlPlan.getMock();
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

    public void testActivate() throws Exception {
        MockControl repCtrl = MockControl.createControl(SipxReplicationContext.class);
        SipxReplicationContext rep = (SipxReplicationContext) repCtrl.getMock();
        rep.replicate(new MappingRules());
        rep.replicate(new FallbackRules());
        rep.replicate(new AuthRules());
        repCtrl.replay();

        MockControl planCtrl = MockControl.createStrictControl(DialPlanContext.class);
        DialPlanContext empty = (DialPlanContext) planCtrl.getMock();
        empty.getGenerationRules();
        planCtrl.setReturnValue(Collections.EMPTY_LIST);
        planCtrl.replay();

        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(empty);
        generator.activate(rep);

        planCtrl.verify();
        repCtrl.verify();
    }
}
