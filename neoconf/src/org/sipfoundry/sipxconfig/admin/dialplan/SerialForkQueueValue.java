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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.text.NumberFormat;

class SerialForkQueueValue {
    private static final float MAX = 1.0f;
    private static final float MIN = 0.80f;
    private static final int MAX_FRAC = 3;
    private static final int MIN_FRAC = 1;
    private static final NumberFormat Q_FORMAT = NumberFormat.getInstance();
    static {
        Q_FORMAT.setMaximumFractionDigits(MAX_FRAC);
        Q_FORMAT.setMinimumFractionDigits(MIN_FRAC);
    }

    private float m_nextValue = MAX;
    private float m_step;

    public SerialForkQueueValue(int sequenceCont) {
        m_step = (MAX - MIN) / (sequenceCont + 1);
    }

    public String getNextValue() {
        String result = "Q=" + Q_FORMAT.format(m_nextValue);
        m_nextValue = m_nextValue - m_step;
        return result;
    }
}