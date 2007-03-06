/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.html.BasePage;

public abstract class Home extends BasePage {
    public static final String PAGE = "Home";

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();
}
