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
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Comments
 */
public abstract class AbstractPhonePage extends BasePage {

    public abstract PhoneContext getPhoneContext();

    public abstract void setPhoneContext(PhoneContext phoneContext);
}
