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
import java.util.List;

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
}
