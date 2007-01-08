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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * InternalRule
 */
public class MohRule extends DialingRule {
    private String m_user = "~~mh~";
    private String m_url = "<sip:moh@${ORBIT_SERVER_SIP_SRV_OR_HOSTPORT}>";

    public MohRule() {
        setEnabled(true);
        setName("Music on Hold");
        setDescription("Forward music on hold calls to Park Server");
    }

    public String[] getPatterns() {
        return new String[] {
            m_user
        };
    }

    public Transform[] getTransforms() {
        UrlTransform transform = new UrlTransform();
        transform.setUrl(m_url);
        return new Transform[] {
            transform
        };
    }

    public DialingRuleType getType() {
        return null;
    }

    public boolean isInternal() {
        return true;
    }
}
