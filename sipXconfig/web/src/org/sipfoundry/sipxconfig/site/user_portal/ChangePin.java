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
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.site.Visit;

/**
 * ChangePin
 */

public abstract class ChangePin extends BasePage {

    /**
     * Properties
     */
    
    public abstract CoreContext getCoreContext();
    public abstract LoginContext getLoginContext();

    public abstract String getCurrentPin();
    public abstract void setCurrentPin(String currentPin);

    public abstract String getNewPin();
    public abstract void setNewPin(String newPin);

    /**
     * Listeners
     */  
    
    public void changePin(IRequestCycle cycle_) {
        // Proceed only if Tapestry validation succeeded
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        // Get the userId.  Note that the Border component of the page ensures
        // that the user is logged in and therefore that userId is non-null.
        Visit visit = (Visit) getPage().getVisit();
        Integer userId = visit.getUserId();

        // Validate the current PIN.
        // Note that the ConfirmPassword component ensures that the new PIN and
        // confirm new PIN fields match, so we don't have to worry about that here.
        CoreContext coreContext = getCoreContext();
        User user = coreContext.loadUser(userId);
        LoginContext loginContext = getLoginContext();
        user = loginContext.checkCredentials(user.getUserName(), getCurrentPin());
        if (user == null) {
            IValidationDelegate delegate = TapestryUtils.getValidator(this);
            delegate.record(getMessage("message.badCurrentPin"), ValidationConstraint.CONSISTENCY);
            return;
        }

        // Change the PIN
        user.setPin(getNewPin(), coreContext.getAuthorizationRealm());
        coreContext.saveUser(user);
        
        // TODO: Provide a success message confirming the PIN change,
        // when the mechanism for success messages becomes available.
    }
    
}
