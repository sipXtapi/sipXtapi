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
public class LongDistanceRule extends DialingRule {
    private String m_pstnPrefix;
    private String m_longDistancePrefix;
    private String m_areaCodes;
    private int m_externalLen;

    public LongDistanceRule() {
        List perms = Collections.singletonList(Permission.LONG_DISTANCE_DIALING);
        setPermissions(perms);
    }

    public String[] getPatterns() {
        DialPattern patternFull = new DialPattern(m_pstnPrefix + m_longDistancePrefix,
                m_externalLen);
        DialPattern patternNormal = new DialPattern(m_longDistancePrefix, m_externalLen);
        DialPattern patternShort = new DialPattern("", m_externalLen);

        ArrayList patterns = new ArrayList();
        patterns.add(patternFull.calculatePattern());
        patterns.add(patternNormal.calculatePattern());
        patterns.add(patternShort.calculatePattern());
        return (String[]) patterns.toArray(new String[patterns.size()]);
    }

    public Transform[] getTransforms() {
        CallPattern patternNormal = new CallPattern(m_longDistancePrefix,
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
        return Type.LONG_DISTANCE;
    }

    public String getAreaCodes() {
        return m_areaCodes;
    }

    public void setAreaCodes(String areaCodes) {
        m_areaCodes = areaCodes;
    }

    public int getExternalLen() {
        return m_externalLen;
    }

    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
    }

    public String getLongDistancePrefix() {
        return m_longDistancePrefix;
    }

    public void setLongDistancePrefix(String longDistancePrefix) {
        m_longDistancePrefix = longDistancePrefix;
    }

    public String getPstnPrefix() {
        return m_pstnPrefix;
    }

    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
    }
}
