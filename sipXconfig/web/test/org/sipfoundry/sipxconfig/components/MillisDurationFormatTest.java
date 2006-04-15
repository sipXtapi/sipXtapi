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

import org.apache.commons.lang.time.DateUtils;

public class MillisDurationFormatTest extends TestCase {
    private MillisDurationFormat m_format;

    protected void setUp() throws Exception {
        m_format = new MillisDurationFormat();
    }

    public void testZeroFormat() {
        assertEquals("", m_format.format(new Long(0)));                
        assertEquals("", m_format.format(new Long(1)));
        assertEquals("", m_format.format(new Long(999)));

        m_format.setShowZero(true);
        
        assertEquals("0 seconds", m_format.format(new Long(0)));                
        assertEquals("0 seconds", m_format.format(new Long(1)));
        assertEquals("0 seconds", m_format.format(new Long(999)));        
    }
    
    public void testFormat() {
        assertEquals("1 second", m_format.format(new Long(1000)));

        assertEquals("5 seconds", m_format.format(new Long(5000)));
        assertEquals("5 seconds", m_format.format(new Long(5999)));

        assertEquals("1 minute, 5 seconds", m_format.format(new Long(60000 + 5000)));

        assertEquals("3 hours, 0 minutes, 5 seconds", m_format.format(new Long(
                3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("5 days, 0 hours, 1 minute, 7 seconds", m_format.format(new Long(5
                * DateUtils.MILLIS_PER_DAY + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatSeconds() {
        m_format.setMaxField(MillisDurationFormat.SECONDS);
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("", m_format.format(new Long(1)));
        assertEquals("", m_format.format(new Long(999)));
        assertEquals("1 second", m_format.format(new Long(1000)));

        assertEquals("5 seconds", m_format.format(new Long(5000)));
        assertEquals("5 seconds", m_format.format(new Long(5999)));

        assertEquals("65 seconds", m_format.format(new Long(60000 + 5000)));

        assertEquals("10,805 seconds", m_format.format(new Long(
                3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("432,067 seconds", m_format.format(new Long(5 * DateUtils.MILLIS_PER_DAY
                + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatNoLabels() {
        m_format.setMaxField(MillisDurationFormat.SECONDS);
        m_format.setUseLabels(false);
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("1", m_format.format(new Long(1000)));

        assertEquals("5", m_format.format(new Long(5000)));
        assertEquals("5", m_format.format(new Long(5999)));

        assertEquals("65", m_format.format(new Long(60000 + 5000)));

        assertEquals("10,805", m_format.format(new Long(3 * DateUtils.MILLIS_PER_HOUR + 5050)));
        assertEquals("432,067", m_format.format(new Long(5 * DateUtils.MILLIS_PER_DAY
                + DateUtils.MILLIS_PER_MINUTE + 7070)));
    }

    public void testFormatSeparator() {
        m_format.setMaxField(MillisDurationFormat.HOURS);
        m_format.setUseLabels(false);
        m_format.setSeparator(":");
        assertEquals("", m_format.format(new Long(0)));
        assertEquals("3:0:5", m_format.format(new Long(3 * DateUtils.MILLIS_PER_HOUR + 5050)));
    }
}
