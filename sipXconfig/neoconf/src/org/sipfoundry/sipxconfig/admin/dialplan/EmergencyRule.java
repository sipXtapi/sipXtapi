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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * LongDistanceRule
 */
public class EmergencyRule extends DialingRule {
    private static final String SOS = "sos";

    private String m_emergencyNumber;
    private String m_optionalPrefix;
    private boolean m_useMediaServer;

    public String[] getPatterns() {
        ArrayList patterns = new ArrayList();
        patterns.add(SOS);
        patterns.add(m_emergencyNumber);
        if (null != m_optionalPrefix && 0 < m_optionalPrefix.length()) {
            patterns.add(m_optionalPrefix + m_emergencyNumber);
        }
        return (String[]) patterns.toArray(new String[patterns.size()]);
    }

    public Transform[] getStandardTransforms() {
        List gateways = getGateways();
        List transforms = new ArrayList(gateways.size());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            String url = "sip:" + m_emergencyNumber + "@" + gateway.getAddress();
            UrlTransform transform = new UrlTransform();
            transform.setUrl(url);
            transforms.add(transform);
        }
        return (Transform[]) transforms.toArray(new Transform[transforms.size()]);
    }

    public Transform[] getMediaServerTransforms() {
        UrlTransform transform = new UrlTransform();
        String url = MappingRule.buildUrl(CallDigits.FIXED_DIGITS, SOS, null);
        transform.setUrl(url);
        return new Transform[] {
            transform
        };
    }

    public Transform[] getTransforms() {
        return m_useMediaServer ? getMediaServerTransforms() : getStandardTransforms();
    }

    public DialingRuleType getType() {
        return DialingRuleType.EMERGENCY;
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    public void appendToGenerationRules(List rules) {
        if (!isEnabled()) {
            return;
        }
        if (!m_useMediaServer) {
            super.appendToGenerationRules(rules);
            return;
        }
        try {
            DialingRule rule = (DialingRule) clone();
            rule.setGateways(Collections.EMPTY_LIST);
            rule.setDescription(getDescription());
            rules.add(rule);
        } catch (CloneNotSupportedException e) {
            // should never happen
            throw new RuntimeException(e);
        }
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

    public boolean getUseMediaServer() {
        return m_useMediaServer;
    }

    public void setUseMediaServer(boolean useMediaServer) {
        m_useMediaServer = useMediaServer;
    }
}
