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
 
package com.pingtel.pds.pgs.jsptags.util;

import javax.servlet.jsp.JspException;

public abstract class BodyReaderTag 
    extends ExBodyTagSupport {

    public int doAfterBody() 
        throws JspException 
    {
        processBody(bodyContent.getString());
        return SKIP_BODY;
    }
    
    protected abstract void processBody(String content)
        throws JspException;
}
