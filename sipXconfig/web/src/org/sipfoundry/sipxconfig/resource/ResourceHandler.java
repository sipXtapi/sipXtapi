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
package org.sipfoundry.sipxconfig.resource;

import java.io.InputStream;

public interface ResourceHandler {
    
    public InputStream deleteResource(ResourceContext context, InputStream in);

    public InputStream postResource(ResourceContext context, InputStream in);

    public InputStream putResource(ResourceContext context, InputStream in);

    public InputStream getResource(ResourceContext context, InputStream in);

    public InputStream onResource(ResourceContext context, String method, InputStream in);
}
