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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

import junit.framework.TestCase;

/**
 * GatewaySelectionModelTest
 */
public class GatewaySelectionModelTest extends TestCase {
    static private final String[] NAMES = {
        "a", "b", "c"
    };
    private GatewaySelectionModel m_model;

    protected void setUp() throws Exception {
        List gateways = new ArrayList();
        for (int i = 0; i < NAMES.length; i++) {
            String name = NAMES[i];
            Gateway gateway = new Gateway();
            gateway.setUniqueId();
            gateway.setName(name);
            gateways.add(gateway);
        }
        m_model = new GatewaySelectionModel();
        m_model.setGateways(gateways);
    }

    public void testSetGateways() {
        try {
            m_model.setGateways(new ArrayList());
            fail("Should have thrown the exception");
            
        } catch (IllegalStateException e) {
            // this is expected
        }
    }

    public void testGetOptionCount() {
        assertEquals(NAMES.length, m_model.getOptionCount());
    }

    public void testGetOption() {
        Object option = m_model.getOption(2);
        assertTrue(option instanceof Gateway);
        Gateway g = (Gateway) option;
        assertEquals(NAMES[2], g.getName());
    }

    public void testGetLabel() {
        assertEquals(NAMES[1], m_model.getLabel(1));        
    }

    public void testGetValue() {
        for (int i = 0; i < NAMES.length; i++) {
            String value = m_model.getValue(i);
            Object object = m_model.translateValue(value);
            assertTrue(object instanceof Gateway);
            Gateway g = (Gateway) object;
            assertEquals(NAMES[i], g.getName());
        }
    }
}
