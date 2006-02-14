/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.services.DataSqueezer;
import org.apache.tapestry.util.io.SqueezeAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public class ServerStatusSqueezeAdapter implements SqueezeAdaptor {
    private static final String PREFIX = "D"; // Arbitrary

    public String getPrefix() {
        return PREFIX;
    }

    public Class getDataClass() {
        return ServiceStatus.class;
    }

    public String squeeze(DataSqueezer squeezer, Object data) {
        ServiceStatus status = (ServiceStatus) data;
        return status.getServiceName();
    }

    public Object unsqueeze(DataSqueezer squeezer, String string) {
        Process process = Process.getEnum(string);
        return new ServiceStatus(process, Status.UNKNOWN);
    }
}
