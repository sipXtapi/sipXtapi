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
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEndRenderListener;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.ExtensionPoolContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class UserForm extends BaseComponent implements PageEndRenderListener {
    // Display this dummy PIN value (masked) to indicate that a PIN exists.
    // We can't use a real PIN.  We don't know the real PIN and if we did,
    // we shouldn't show it.
    // Pick an obscure PIN to avoid colliding with real user PINs.  (I tried using a
    // non-printable PIN "\1\1\1\1\1\1\1\1" but Tapestry silently discards the string!)
    private static final String DUMMY_PIN = "`p1n6P0\361g";
    
    public abstract CoreContext getCoreContext();
    public abstract SettingDao getSettingDao();
    public abstract ExtensionPoolContext getExtensionPoolContext();
    
    public abstract User getUser();
    public abstract void setUser(User user);
    
    public abstract String getPin();
    public abstract void setPin(String pin);
    
    public abstract String getAliasesString();
    public abstract void setAliasesString(String aliasesString);
    
    public abstract String getGroupsString();    
    public abstract void setGroupsString(String groups);
    public abstract Collection getGroupsCandidates();
    public abstract void setGroupCandidates(Collection groupsList);
    
    public void buildGroupCandidates(String groupsString) {
        List allGroups = getCoreContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);        
    }
    
    // Update the User object if input data is valid
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        
        if (!TapestryUtils.isRewinding(cycle, this)) {
            User user = getUser();        
            // Automatically assign a numeric extension if appropriate
            assignExtension();
            
            // Init the aliases string before rendering, if necessary
            if (StringUtils.isEmpty(getAliasesString())) {
                setAliasesString(user.getAliasesString());
            }

            initializePin();

            if (getGroupsString() == null) {
                setGroupsString(user.getGroupsNames());
            }
        }
        
        super.renderComponent(writer, cycle);
        
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            User user = getUser();        

            // Set the user aliases from the aliases string
            user.setAliasesString(getAliasesString());
            
            // Make sure that the user ID and aliases don't collide with any other
            // user IDs or aliases.  Report an error if there is a collision.
            if (checkForUserIdOrAliasCollision()) {
                return;
            }
            
            // Update the user's PIN and aliases
            updatePin();

            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, groupsString);
                user.setGroupsAsList(groups);
            }
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
        
    // For an existing user with a non-empty PIN, init the displayed PIN
    // to be the dummy PIN to make it clear that the PIN is not empty.
    private void initializePin() {
        if (!getUser().isNew() && getPin() == null) {
            setPin(DUMMY_PIN);
            
            // Reset the confirm PIN field as well.  Ugly to reach into the component
            // like this, but I haven't figured out a better way.
            PropertyUtils.write(getComponent("pin"), "confirmPassword", DUMMY_PIN);
        }
    }
    
    // Update the user's PIN
    private void updatePin() {
        if (!(getPin() == null) && !getPin().equals(DUMMY_PIN)) {
            CoreContext core = getCoreContext();
            getUser().setPin(getPin(), core.getAuthorizationRealm());
            
            // Having updated the user, scrub the PIN field for security
            setPin(null);
        }
    }

    private void recordError(String messageId, String arg) {
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
        
        String message = MessageFormat.format(
                getMessages().getMessage(messageId), new Object[] {arg});
        
        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }      
}
