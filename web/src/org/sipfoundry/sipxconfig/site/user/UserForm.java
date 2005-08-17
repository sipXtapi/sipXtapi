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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public abstract class UserForm extends BaseComponent {

    public abstract CoreContext getCoreContext();
    
    public abstract User getUser();
    public abstract void setUser(User user);
    
    public abstract String getPin();
    public abstract void setPin(String pin);
    
    // Update the User object with the PIN that was entered by the user
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        
        if (cycle.isRewinding()) {
            CoreContext core = getCoreContext();
            getUser().setPin(getPin(), core.getAuthorizationRealm());
        }
    }
    
}
