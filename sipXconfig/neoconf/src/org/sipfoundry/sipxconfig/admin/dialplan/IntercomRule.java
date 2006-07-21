/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;

public class IntercomRule extends DialingRule {

    private DialPattern m_dialPattern;
    private FullTransform m_transform;

    public IntercomRule(String prefix, String code) {
        m_dialPattern = new DialPattern(prefix, DialPattern.VARIABLE_DIGITS);
        
        m_transform = new FullTransform();
        CallPattern callPattern = new CallPattern(prefix, CallDigits.VARIABLE_DIGITS);
        String user = callPattern.calculatePattern();        
        m_transform.setUser(user);
        String headerParam = String.format("Alert-info=%s", code);
        m_transform.setHeaderParams(new String[] {headerParam});
    }
    
    public IntercomRule(Intercom intercom) {
        this(intercom.getPrefix(), intercom.getCode());
    }
    
    @Override
    public String[] getPatterns() {
        return new String[] {
                m_dialPattern.calculatePattern()
        };
    }

    @Override
    public Transform[] getTransforms() {
        return new Transform[] {
            m_transform
        };
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.INTERCOM;
    }

    public boolean isInternal() {
        return true;
    }

}
