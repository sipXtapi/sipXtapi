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
import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * CustomDialingRule
 */
public class CustomDialingRule extends DialingRule {
    private List m_dialPatterns = new ArrayList();
    private CallPattern m_callPattern = new CallPattern();

    public CustomDialingRule() {
        m_dialPatterns.add(new DialPattern());
    }

    public List getDialPatterns() {
        return m_dialPatterns;
    }

    public void setDialPatterns(List dialPaterns) {
        m_dialPatterns = dialPaterns;
    }

    public CallPattern getCallPattern() {
        return m_callPattern;
    }

    public void setCallPattern(CallPattern callPattern) {
        m_callPattern = callPattern;
    }

    public String[] getPatterns() {
        String[] patterns = new String[m_dialPatterns.size()];
        for (int i = 0; i < patterns.length; i++) {
            DialPattern p = (DialPattern) m_dialPatterns.get(i);
            patterns[i] = p.calculatePattern();
        }
        return patterns;
    }

    public Transform[] getTransforms() {
        final String calculatePattern = getCallPattern().calculatePattern();
        List gateways = getGateways();
        Transform[] transforms = new Transform[gateways.size()];
        SerialForkQueueValue q = new SerialForkQueueValue(gateways.size());
        for (int i = 0; i < transforms.length; i++) {
            Gateway g = (Gateway) gateways.get(i);
            FullTransform transform = new FullTransform();
            transform.setHost(g.getAddress());
            transform.setUser(calculatePattern);
            String[] fieldParams = new String[] {q.getNextValue()};
            transform.setFieldParams(fieldParams);
            transforms[i] = transform;
        }
        return transforms;
    }

    static class SerialForkQueueValue {
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

        SerialForkQueueValue(int sequenceCont) {
            m_step = (MAX - MIN) / (sequenceCont + 1);
        }

        String getNextValue() {
            String result = "Q=" + Q_FORMAT.format(m_nextValue);
            m_nextValue = m_nextValue - m_step;
            return result;
        }
    }

    public Type getType() {
        return Type.CUSTOM;
    }
}
