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

/**
 * Could not handle resource.
 */
public class ResourceException extends RuntimeException {
    
    private int m_httpErrorCode;

    /**
     * @param httpErrorCode see HttpServletResponse error codes.
     * @param message
     */
    public ResourceException(int httpErrorCode, String message) {
        super(message);
        m_httpErrorCode = httpErrorCode; 
    }

    public int getHttpErrorCode() {
        return m_httpErrorCode;
    }
}
