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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Phone;


/**
 * Comments
 */
public abstract class EditPhoneLines extends BasePage {

    public static final String PAGE = "EditPhoneLines";

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
}
