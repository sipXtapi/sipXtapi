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

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;

public abstract class GatewaysPanel extends BaseComponent {

    public abstract Collection getGatewaysToRemove();

    public abstract DialingRule getRule();

    public boolean onFormSubmit() {
        DialingRule rule = getRule();
        boolean ruleChanged = false;
        Collection selectedGateways = getGatewaysToRemove();
        if (null != selectedGateways) {
            rule.removeGateways(selectedGateways);
            ruleChanged = true;
        }
        return ruleChanged;
    }
}
