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
package org.sipfoundry.sipxconfig.site.speeddial;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.speeddial.Button;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ButtonEditor extends BaseComponent {
    @Parameter(required = true)
    public abstract Button getButton();

    @Parameter(required = true)
    public abstract int getIndex();

    @Parameter(required = true)
    public abstract int getRemoveIndex();
}
