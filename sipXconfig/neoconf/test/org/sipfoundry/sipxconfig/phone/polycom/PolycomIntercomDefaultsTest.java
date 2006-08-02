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
package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;

public class PolycomIntercomDefaultsTest extends TestCase {
    private PolycomIntercomDefaultsDummy m_noItercomDefaults;

    protected void setUp() throws Exception {
        m_noItercomDefaults = new PolycomIntercomDefaultsDummy(null);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetAlertInfoValue() {
        assertNull(m_noItercomDefaults.getAlertInfoValue());
        Intercom intercom = new Intercom();
        intercom.setCode("abc");
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        assertEquals("abc", defaults.getAlertInfoValue());
    }

    public void testGetAlertInfoClass() {
        assertEquals(PolycomIntercomDefaults.DEFAULT_RING_CLASS, m_noItercomDefaults
                .getAlertInfoClass());
        Intercom intercom = new Intercom();
        intercom.setTimeout(0);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        assertEquals(PolycomIntercomDefaults.AUTO_ANSWER_RING_CLASS, defaults.getAlertInfoClass());
        intercom.setTimeout(1000);
        assertEquals(PolycomIntercomDefaults.RING_ANSWER_RING_CLASS, defaults.getAlertInfoClass());
    }

    public void testGetRingAnswerTimeout() {
        assertEquals(2000, m_noItercomDefaults.getRingAnswerTimeout());
        Intercom intercom = new Intercom();
        intercom.setTimeout(0);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        assertEquals(2000, defaults.getRingAnswerTimeout());
        intercom.setTimeout(1000);
        assertEquals(1000, defaults.getRingAnswerTimeout());
    }

    public static class PolycomIntercomDefaultsDummy extends PolycomIntercomDefaults {
        private Intercom m_itercom;

        public PolycomIntercomDefaultsDummy(Intercom intercom) {
            super(null);
            m_itercom = intercom;
        }

        @Override
        protected Intercom getIntercom() {
            return m_itercom;
        }
    }
}
