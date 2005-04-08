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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

public class ExtraOptionModelDecoratorTest extends TestCase {

    private ExtraOptionModelDecorator m_model;

    protected void setUp() throws Exception {
        LenSelectionModel model = new LenSelectionModel();
        model.setMin(3);
        model.setMax(6);
        m_model = new ExtraOptionModelDecorator();
        m_model.setModel(model);
        m_model.setExtraOption("A new option");

    }

    public void testGetOptionCount() {
        assertEquals(5, m_model.getOptionCount());
    }

    public void testGetOption() {
        assertEquals(new Integer(-1), m_model.getOption(0));
        assertEquals(new Integer(4), m_model.getOption(2));
        assertEquals(new Integer(5), m_model.getOption(3));
    }

    public void testGetLabel() {
        assertEquals("A new option", m_model.getLabel(0));
        assertEquals("4 digits", m_model.getLabel(2));
        assertEquals("5 digits", m_model.getLabel(3));
    }

    public void testGetValue() {
        assertEquals("-1", m_model.getValue(0));
        assertEquals("3", m_model.getValue(1));
        assertEquals("5", m_model.getValue(3));
    }

    public void testTranslateValue() {
        String value0 = m_model.getValue(0);
        assertEquals(new Integer(-1), m_model.translateValue(value0));
        String value = m_model.getValue(3);
        assertEquals(new Integer(5), m_model.translateValue(value));
    }
}
