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
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * LongDistanceRule
 */
public class EmergencyRule extends DialingRule {
    private String m_emergencyNumber;
    private String m_optionalPrefix;

    public String[] getPatterns() {
        ArrayList patterns = new ArrayList();
        patterns.add("sos");
        patterns.add(m_emergencyNumber);
        if (null != m_optionalPrefix && 0 < m_optionalPrefix.length()) {
            patterns.add(m_optionalPrefix + m_emergencyNumber);
        }
        return (String[]) patterns.toArray(new String[patterns.size()]);
    }

    public Transform[] getTransforms() {
        List gateways = getGateways();
        List transforms = new ArrayList(gateways.size());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            String url = "sip:911@" + gateway.getAddress();
            UrlTransform transform = new UrlTransform();
            transform.setUrl(url);
            transforms.add(transform);
        }
        return (Transform[]) transforms.toArray(new Transform[transforms.size()]);
    }

    public Type getType() {
        return Type.EMERGENCY;
    }

    public String getEmergencyNumber() {
        return m_emergencyNumber;
    }

    public void setEmergencyNumber(String emergencyNumber) {
        m_emergencyNumber = emergencyNumber;
    }

    public String getOptionalPrefix() {
        return m_optionalPrefix;
    }

    public void setOptionalPrefix(String optionalPrefix) {
        m_optionalPrefix = optionalPrefix;
    }
}
