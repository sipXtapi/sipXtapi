/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

public class NewEnumPropertySelectionModelTest extends TestCase {
    
    private NewEnumPropertySelectionModel<Numbers> m_model;

    enum Numbers {
        ZERO, ONE, TWO, THREE;
    }

    protected void setUp() throws Exception {
        m_model = new NewEnumPropertySelectionModel<Numbers>(Numbers.class);        
    }

    public void testGetOptionCount() {
        assertEquals(4, m_model.getOptionCount());
    }

    public void testGetOption() {
        assertEquals(Numbers.TWO, m_model.getOption(2));
    }

    public void testGetLabel() {
        assertEquals("THREE", m_model.getLabel(3));
    }

    public void testGetValue() {
        assertEquals("0", m_model.getValue(0));
    }

    public void testTranslateValue() {
        assertEquals(Numbers.TWO, m_model.translateValue("2"));
    }
}
