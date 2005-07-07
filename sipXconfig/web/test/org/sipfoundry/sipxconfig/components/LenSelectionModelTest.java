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
package org.sipfoundry.sipxconfig.components;

import org.sipfoundry.sipxconfig.components.LenSelectionModel;

import junit.framework.TestCase;

/**
 * LenSelectionModelTest
 */
public class LenSelectionModelTest extends TestCase {

    private LenSelectionModel m_model;

    protected void setUp() throws Exception {
        super.setUp();
        m_model = new LenSelectionModel();
        m_model.setMin(3);
        m_model.setMax(6);
    }

    protected void tearDown() throws Exception {
        m_model = null;
        super.tearDown();
    }

    public void testGetOptionCount() {
        assertEquals(4, m_model.getOptionCount());
    }

    public void testGetOption() {
        assertEquals(new Integer(4), m_model.getOption(1));
        assertEquals(new Integer(5), m_model.getOption(2));
    }

    public void testGetLabel() {
        assertEquals("4 digits", m_model.getLabel(1));
        assertEquals("5 digits", m_model.getLabel(2));
    }

    public void testGetValue() {
        assertEquals("3", m_model.getValue(0));
        assertEquals("5", m_model.getValue(2));
    }

    public void testTranslateValue() {
        String value = m_model.getValue(2);
        assertEquals(new Integer(5), m_model.translateValue(value));
    }

    public void testInvalidState() {
        m_model.setMax(2);
        m_model.setMin(4);
        try {
            m_model.getOptionCount();
            fail("Should throw illegal state exception");
        } catch (IllegalStateException e) {
            // this was expected
        }
    }
}
