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
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Phone;


/**
 * Comments
 */
public abstract class LineSettings extends BasePage {
    
    public static final String PAGE = "LineSettings";

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);

    public abstract void setSetting(String setting);

    public abstract String getSetting();
}
