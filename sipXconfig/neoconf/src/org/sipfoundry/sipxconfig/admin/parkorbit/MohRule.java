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
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.admin.dialplan.InternalForwardRule;

public class MohRule extends InternalForwardRule {

    public MohRule() {
        super("~~mh~", "<sip:moh@${ORBIT_SERVER_SIP_SRV_OR_HOSTPORT}>");
        setName("Music on Hold");
        setDescription("Forward music on hold calls to Park Server");
    }
}
