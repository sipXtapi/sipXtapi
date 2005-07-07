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

import javax.servlet.http.HttpServletResponse;

/**
 * If you don't support this method
 */
public class MethodNotAllowedException extends ResourceException {    
    public MethodNotAllowedException(String method) {
        super(HttpServletResponse.SC_METHOD_NOT_ALLOWED, "Method not implemented:" + method); 
    }
}
