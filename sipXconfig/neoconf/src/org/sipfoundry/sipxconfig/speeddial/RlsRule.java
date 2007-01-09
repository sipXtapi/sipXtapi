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
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalForwardRule;

public class RlsRule extends InternalForwardRule {
    public RlsRule() {
        super(new DialPattern("~~rl~", DialPattern.VARIABLE_DIGITS),
                "<sip:{digits}@${RLS_ADDR}:${RLS_SIP_PORT}>");
        setName("RLS");
        setDescription("Forward resource list subscriptions to RLS");
    }
}
