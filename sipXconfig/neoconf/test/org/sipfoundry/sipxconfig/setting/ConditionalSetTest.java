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

import java.io.InputStream;
import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class ConditionalSetTest extends TestCase {
    private ModelBuilder m_builder;

    protected void setUp() throws Exception {
        m_builder = new XmlModelBuilder(TestHelper.getSettingModelContextRoot());
    }
    
    public void testIf() throws Exception {
        InputStream in = getClass().getResourceAsStream("conditionals.xml");
        SettingSet root = m_builder.buildModel(in, null);
        assertTrue(root instanceof ConditionalSet);
        ConditionalSet conditional = (ConditionalSet) root;
        
        Set defines = new HashSet();
        defines.add("american");
        
        Setting actual = conditional.evaluate(defines);
        assertNotNull(actual.getSetting("woman/shave"));
        
        defines.add("european");
        Setting actual2 = conditional.evaluate(defines);
        assertNull(actual2.getSetting("woman/shave"));
        
        defines.add("vegitarian");
        Setting actual3 = conditional.evaluate(defines);
        assertNull(actual3.getSetting("human/eat/hamburger"));
        assertNotNull(actual3.getSetting("human/eat/vegiburger"));
    }
}
