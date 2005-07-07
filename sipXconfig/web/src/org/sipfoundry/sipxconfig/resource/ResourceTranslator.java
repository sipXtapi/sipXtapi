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

public interface ResourceTranslator {
    
    public Object read(InputStream in);

    public void merge(InputStream from, Object into);

    public InputStream write(Object object);
}
