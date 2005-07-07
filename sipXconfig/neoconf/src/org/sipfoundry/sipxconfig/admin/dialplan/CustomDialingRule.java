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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * CustomDialingRule
 */
public class CustomDialingRule extends DialingRule {
    private List m_dialPatterns = new ArrayList();
    private CallPattern m_callPattern = new CallPattern();
    private List m_permissions = new ArrayList();

    public CustomDialingRule() {
        m_dialPatterns.add(new DialPattern());
    }

    protected Object clone() throws CloneNotSupportedException {
        CustomDialingRule clone = (CustomDialingRule) super.clone();
        clone.m_permissions = new ArrayList(m_permissions);
        clone.m_dialPatterns = new ArrayList(m_dialPatterns);
        return clone;
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
        Transform[] transforms;
        if (gateways.isEmpty()) {
            FullTransform transform = new FullTransform();
            transform.setUser(calculatePattern);
            transforms = new Transform[] {
                transform
            };
        } else {
            transforms = new Transform[gateways.size()];
            ForkQueueValue q = new ForkQueueValue(gateways.size());
            for (int i = 0; i < transforms.length; i++) {
                Gateway g = (Gateway) gateways.get(i);
                FullTransform transform = new FullTransform();
                transform.setHost(g.getAddress());
                transform.setUser(calculatePattern);
                String[] fieldParams = new String[] {
                    q.getSerial()
                };
                transform.setFieldParams(fieldParams);
                transforms[i] = transform;
            }
        }
        return transforms;
    }

    public DialingRuleType getType() {
        return DialingRuleType.CUSTOM;
    }

    public List getPermissions() {
        return m_permissions;
    }

    public void setPermissions(List permissions) {
        m_permissions = permissions;
    }

    /**
     * External rule if there are gateways. Internal if no gateways
     */
    public boolean isInternal() {
        return getGateways().isEmpty();
    }

    public String[] getTransformedPatterns() {
        List dialPatterns = getDialPatterns();
        Set transformedPatterns = new LinkedHashSet();
        for (Iterator i = dialPatterns.iterator(); i.hasNext();) {
            DialPattern dp = (DialPattern) i.next();
            DialPattern tdp = m_callPattern.transform(dp);
            transformedPatterns.add(tdp.calculatePattern());
        }
        return (String[]) transformedPatterns.toArray(new String[transformedPatterns.size()]);
    }
}
