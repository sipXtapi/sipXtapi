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

import org.sipfoundry.sipxconfig.admin.forwarding.Ring;

public class ForkQueueValue {
    private static final float MAX = 1.0f;
    private static final float MIN = 0.80f;
    private static final int MAX_FRAC = 3;
    private static final int MIN_FRAC = 1;
    private static final NumberFormat Q_FORMAT = NumberFormat.getInstance();
    static {
        Q_FORMAT.setMaximumFractionDigits(MAX_FRAC);
        Q_FORMAT.setMinimumFractionDigits(MIN_FRAC);
    }

    private float m_step;
    private float m_value = MAX;

    public ForkQueueValue(int sequenceCont) {
        m_step = (MAX - MIN) / (sequenceCont + 1);
    }

    /**
     * Use for serial forking
     */
    public String getSerial() {
        float next = m_value - m_step;
        if (next <= MIN) {
            throw new IllegalStateException();
        }
        m_value = next;
        return getParallel();
    }

    /**
     * Use for parallel forking.
     * 
     * It is safe to call this function even if getNextValue has not been called first.
     */
    public String getParallel() {
        return "q=" + Q_FORMAT.format(m_value);
    }

    public String getValue(Ring.Type type) {
        if (Ring.Type.IMMEDIATE.equals(type)) {
            return getParallel();
        }
        return getSerial();
    }
}