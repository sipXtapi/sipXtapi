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
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * LongDistanceRule
 */
public class InternationalRule extends DialingRule {
    private String m_internationalPrefix;

    public InternationalRule() {
        List perms = Collections.singletonList(Permission.INTERNATIONAL_DIALING);
        setPermissions(perms);
    }

    public String[] getPatterns() {
        String pattern = m_internationalPrefix + "x.";
        return new String[] {
            pattern
        };
    }

    public Transform[] getTransforms() {
        CallPattern patternNormal = new CallPattern(m_internationalPrefix,
                CallDigits.VARIABLE_DIGITS);
        String user = patternNormal.calculatePattern();
        List gateways = getGateways();
        List transforms = new ArrayList(gateways.size());
        SerialForkQueueValue q = new SerialForkQueueValue(gateways.size());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            FullTransform transform = new FullTransform();
            transform.setUser(user);
            transform.setHost(gateway.getAddress());
            String[] fieldParams = new String[] {
                q.getNextValue()
            };
            transform.setFieldParams(fieldParams);
            transforms.add(transform);
        }
        return (Transform[]) transforms.toArray(new Transform[transforms.size()]);
    }

    public Type getType() {
        return Type.INTERNATIONAL;
    }

    public String getInternationalPrefix() {
        return m_internationalPrefix;
    }

    public void setInternationalPrefix(String internationalPrefix) {
        m_internationalPrefix = internationalPrefix;
    }
}
