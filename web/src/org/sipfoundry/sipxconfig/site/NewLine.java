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
 * Create a new line
 */
public abstract class NewLine extends BasePage {
    
    public static final String PAGE = "NewLine";
    
    public abstract void setPhone(Phone phone);

    public abstract Phone getPhone();
    
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public abstract void setReturnPage(String returnPage);
    
    public abstract String getReturnPage();
}
