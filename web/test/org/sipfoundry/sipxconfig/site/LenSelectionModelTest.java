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
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

/**
 * LenSelectionModelTest
 */
public class LenSelectionModelTest extends TestCase {

    private LenSelectionModel m_model;

    protected void setUp() throws Exception {
        super.setUp();
        m_model = new LenSelectionModel();
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
        assertEquals("5 digits", m_model.getValue(2));
    }

    public void testTranslateValue() {
        assertEquals(new Integer(5), m_model.translateValue("5 digits"));
    }

}
