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
    private ConditionalSet m_root;
    private Set m_definitions;
    

    protected void setUp() throws Exception {
        XmlModelBuilder builder = new XmlModelBuilder(TestHelper.getSettingModelContextRoot());
        InputStream in = getClass().getResourceAsStream("conditionals.xml");
        m_root = (ConditionalSet) builder.buildModel(in, null);
        m_definitions = new HashSet();
    }
    
    public void testIf() throws Exception {
        m_definitions.add("hairy");        
        Setting actual = m_root.evaluate(m_definitions);
        assertNotNull(actual.getSetting("man/shave"));
        
        m_definitions.add("neandrathal");
        Setting actual2 = m_root.evaluate(m_definitions);
        assertNull(actual2.getSetting("man/shave"));
        
        m_definitions.add("vegitarian");
        Setting actual3 = m_root.evaluate(m_definitions);
        assertNull(actual3.getSetting("human/eat/hamburger"));
        assertNotNull(actual3.getSetting("human/eat/vegiburger"));
    }
    
    public void testGroupIf() throws Exception {
        Setting actual = m_root.evaluate(m_definitions);
        assertNull(actual.getSetting("alien"));        
        assertNotNull(actual.getSetting("et"));
        
        m_definitions.add("borg");
        Setting actual2 = m_root.evaluate(m_definitions);
        assertNotNull(actual2.getSetting("alien"));
    }
}
