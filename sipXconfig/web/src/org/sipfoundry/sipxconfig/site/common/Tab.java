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
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;

@ComponentClass(allowBody = true, allowInformalParameters = false)
public abstract class Tab extends BaseComponent {

    @Parameter(required = true)
    public abstract boolean getIsActive();
    
    public String getCurrentStyle() {
        return getIsActive() ? "tab active" : "tab";
    }
}
