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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * LongDistanceRule
 */
public class LocalRule extends DialingRule {
    private String m_pstnPrefix;
    private int m_externalLen;

    public LocalRule() {
        // empty
    }

    public List getPermissions() {
        List perms = Collections.singletonList(Permission.LOCAL_DIALING);
        return perms;
    }

    public String[] getPatterns() {
        DialPattern patternFull = new DialPattern(m_pstnPrefix, m_externalLen);
        DialPattern patternShort = new DialPattern(StringUtils.EMPTY, m_externalLen);

        return new String[] {
            patternFull.calculatePattern(), patternShort.calculatePattern()
        };
    }

    public String[] getTransformedPatterns() {
        DialPattern patternShort = new DialPattern(StringUtils.EMPTY, m_externalLen);
        return new String[] {
            patternShort.calculatePattern()
        };
    }

    public Transform[] getTransforms() {
        CallPattern patternNormal = new CallPattern(StringUtils.EMPTY, CallDigits.VARIABLE_DIGITS);
        String user = patternNormal.calculatePattern();
        List gateways = getGateways();
        List transforms = new ArrayList(gateways.size());
        ForkQueueValue q = new ForkQueueValue(gateways.size());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            FullTransform transform = new FullTransform();
            transform.setUser(user);
            transform.setHost(gateway.getAddress());
            String[] fieldParams = new String[] {
                q.getSerial()
            };
            transform.setFieldParams(fieldParams);
            transforms.add(transform);
        }
        return (Transform[]) transforms.toArray(new Transform[transforms.size()]);
    }

    public DialingRuleType getType() {
        return DialingRuleType.LOCAL;
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    public int getExternalLen() {
        return m_externalLen;
    }

    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
    }

    public String getPstnPrefix() {
        return m_pstnPrefix;
    }

    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
    }
}
