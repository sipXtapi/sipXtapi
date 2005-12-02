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
package org.sipfoundry.sipxconfig.site.gateway;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;

public abstract class GatewaysPanel extends BaseComponent {

    public abstract Collection getRowsToDelete();

    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();
    
    public abstract IActionListener getAction();
    
    public abstract DialingRule getRule();

    public boolean onFormSubmit() {
        DialingRule rule = getRule();
        Collection selectedGateways = getRowsToDelete();
        if (null != selectedGateways) {
            rule.removeGateways(selectedGateways);
            return true;
        }
        selectedGateways = getRowsToMoveDown();
        if (null != selectedGateways) {
            rule.moveGateways(selectedGateways, 1);
            return true;
        }
        selectedGateways = getRowsToMoveUp();
        if (null != selectedGateways) {
            rule.moveGateways(selectedGateways, -1);
            return true;
        }
        return false;
    }
}
