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

/**
 * RestrictedRule
 */
public class RestrictedRule extends DialingRule {
    private String m_areaCodes;

    public String[] getPatterns() {
        // TODO Auto-generated method stub
        return null;
    }

    public Transform[] getTransforms() {
        // TODO Auto-generated method stub
        return null;
    }

    public Type getType() {
        return Type.RESTRICTED;
    }

    public String getAreaCodes() {
        return m_areaCodes;
    }

    public void setAreaCodes(String areaCodes) {
        m_areaCodes = areaCodes;
    }
}
