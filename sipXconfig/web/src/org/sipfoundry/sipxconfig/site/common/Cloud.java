/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Block;

@ComponentClass(allowBody = true, allowInformalParameters = false)
public abstract class Cloud extends BaseComponent {
    @Parameter(required = true)
    public abstract Block getHeader();

    @Parameter(required = true)
    public abstract Block getFooter();
}
