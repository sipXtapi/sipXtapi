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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * @author dhubler
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class SipConfiguration  extends ConfigurationTemplate {
    
    public SipConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }
    
    public void addContext(VelocityContext context) {
        context.put("sip", this);
    }
}
