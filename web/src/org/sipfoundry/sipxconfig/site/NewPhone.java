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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;


/**
 * Comments
 */
public abstract class NewPhone extends AbstractPhonePage implements PageRenderListener {
    
    public static final String PAGE = "NewPhone"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);

}
