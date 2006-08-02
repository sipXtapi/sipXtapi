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

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public class ServerStatusSqueezeAdapter implements IPrimaryKeyConverter {

    public Object getPrimaryKey(Object value) {
        ServiceStatus status = (ServiceStatus) value;
        return status.getServiceName();
    }

    public Object getValue(Object primaryKey) {
        Process process = Process.getEnum((String) primaryKey);
        return new ServiceStatus(process, Status.UNKNOWN);
    }
}
