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


import org.apache.tapestry.IRequestCycle;

import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;

/**
 * EditCustomeDialRule
 */
public abstract class EditCustomDialRule extends EditDialRule {
    public static final String PAGE = "EditCustomDialRule";

    protected DialingRule createNewRule() {
        return new CustomDialingRule();
    }

    /**
     * Creates the pages instance for current request cycle
     * 
     * @param cycle
     * @return page
     */
    public static EditCustomDialRule getPage(IRequestCycle cycle) {
        EditCustomDialRule page = (EditCustomDialRule) cycle.getPage(PAGE);
        return page;
    }
}
