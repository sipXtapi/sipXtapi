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
package org.sipfoundry.sipxconfig.site.dialplan;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LongDistanceRule;

/**
 * EditInternalDialRule
 */
public abstract class EditLongDistanceDialRule extends EditDialRule {
    public static final String PAGE = "EditLongDistanceDialRule";

    protected DialingRule createNewRule() {
        return new LongDistanceRule();
    }
}
