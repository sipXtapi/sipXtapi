/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Set;

/**
 * look for expression in the set
 */
public class SimpleDefinitionsEvaluator implements SettingExpressionEvaluator {
    private Set m_defines;

    public SimpleDefinitionsEvaluator(Set defines) {
        m_defines = defines;
    }

    public boolean isExpressionTrue(String expression, Setting setting_) {
        return m_defines.contains(expression);
    }
}
