/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.common;

import javax.servlet.jsp.JspException;

public class RedirectServletException extends JspException {

    private String m_source;

    public RedirectServletException ( String message, String sourceURL, String parameters ) {
        super (message );

        sourceURL = sourceURL + "?";
        if ( parameters != null ) {
            sourceURL = sourceURL + parameters + "&";
        }

        sourceURL = sourceURL + "error_message=" + XMLSupport.encodeUtf8(message);

        m_source = sourceURL;
    }

    public String getSourceURL () {
        return m_source;
    }
}
