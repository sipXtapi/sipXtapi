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

import org.sipfoundry.sipxconfig.device.ProfileContext;

public class GrandstreamProfileContext extends ProfileContext {
    private GrandstreamProfileWriter m_writer;

    public GrandstreamProfileContext(GrandstreamPhone device, boolean isTextFormatEnabled) {
        super(device, null);
        if (isTextFormatEnabled) {
            m_writer = new GrandstreamProfileWriter(device);
        } else {
            m_writer = new GrandstreamBinaryProfileWriter(device);
        }
    }

    public GrandstreamProfileWriter getWriter() {
        return m_writer;
    }
}
