/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.IOException;
import java.io.OutputStream;

import org.sipfoundry.sipxconfig.device.AbstractProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileContext;

public class GrandstreamProfileGenerator extends AbstractProfileGenerator {

    protected void generateProfile(ProfileContext context, OutputStream out) throws IOException {
        GrandstreamProfileContext gpc = (GrandstreamProfileContext) context;
        GrandstreamProfileWriter writer = gpc.getWriter();
        writer.write(out);
    }
}
