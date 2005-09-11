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

import java.text.MessageFormat;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class UserForm extends BaseComponent {

    public abstract CoreContext getCoreContext();
    public abstract ExtensionPoolContext getExtensionPoolContext();
    
    public abstract User getUser();
    public abstract void setUser(User user);
    
    public abstract String getPin();
    public abstract void setPin(String pin);
    
    public abstract String getAliasesString();
    public abstract void setAliasesString(String aliasesString);
    
    // Update the User object if input data is valid
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        
        if (!cycle.isRewinding()) {
            // Automatically assign a numeric extension if appropriate
            assignExtension();
            
            // Init the aliases string before rendering, if necessary
            if (StringUtils.isEmpty(getAliasesString())) {
                setAliasesString(getUser().getAliasesString());
            }
        }
        
        super.renderComponent(writer, cycle);
        
        if (cycle.isRewinding()) {
            // Don't take any actions if the page is not valid
            if (!TapestryUtils.isValid((AbstractPage) getPage())) {
                return;
            }

            // Set the user aliases from the aliases string
            getUser().setAliasesString(getAliasesString());
            
            // Make sure that the user ID and aliases don't collide with any other
            // user IDs or aliases.  Report an error if there is a collision.
            if (checkForUserIdOrAliasCollision()) {
                return;
            }
            
            // Update the user's PIN and aliases
            updatePin();
        }
    }
    
    // If the userName is empty and the user extension pool is enabled, then
    // try to fill in the userName with the next free extension from the pool.
    private void assignExtension() {
        if (!StringUtils.isEmpty(getUser().getUserName())) {
            return;     // there is already a username, don't overwrite it
        }
        
        // Get and use the next free extension
        ExtensionPoolContext epc = getExtensionPoolContext();
        Integer extension = epc.getNextFreeUserExtension();
        if (extension != null) {
            String extStr = extension.toString();
            getUser().setUserName(extStr);
        }
    }
    
    // Make sure that the user ID and aliases don't collide with any other
    // user IDs or aliases.  Report an error if there is a collision.
    private boolean checkForUserIdOrAliasCollision() {
        boolean result = false;
        String dup = getCoreContext().checkForDuplicateNameOrAlias(getUser());
        if (dup != null) {
            result = true;
            boolean internalCollision = false;
            
            // Check for a collision within the user itself, of the user ID with an alias,
            // so we can give more specific error feedback.  Since the aliases are filtered
            // for duplicates when assigned to the user, we don't have to worry about that
            // case.  Duplicate aliases are simply discarded.
            for (Iterator iter = getUser().getAliases().iterator(); iter.hasNext();) {
                String alias = (String) iter.next();
                if (getUser().getUserName().equals(alias)) {
                    recordError("message.userIdEqualsAlias", alias);
                    internalCollision = true;
                    break;
                }
            }
            // If it wasn't an internal collision, then the collision is with a different
            // user.  Record an appropriate error.
            if (!internalCollision) {
                recordError("message.duplicateUserIdOrAlias", dup);
            }
        }
        return result;
    }
    
    // Update the user's PIN.
    // Special case: don't set the PIN to be empty.
    // In some contexts (creating a new user) an empty PIN is an error, in
    // other contexts (editing an existing user) we just ignore it.
    // Leave it up to the containing component to decide whether an empty PIN
    // is an error or not.
    private void updatePin() {
        if (!StringUtils.isEmpty(getPin())) {
            CoreContext core = getCoreContext();
            getUser().setPin(getPin(), core.getAuthorizationRealm());
        }
    }

    private void recordError(String messageId, String arg) {
        IValidationDelegate delegate = TapestryUtils.getValidator((AbstractComponent) getPage());
        
        String message = MessageFormat.format(
                getMessage(messageId), new Object[] {arg});
        
        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }
    
}
