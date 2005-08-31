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

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

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
    
    // Update the User object with the PIN that was entered by the user
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        
        if (!cycle.isRewinding()) {
            // Automatically assign a numeric extension if appropriate
//            assignExtension();
            
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
            
            // Update the user's PIN and aliases
            updatePin();
            setAliasesFromString(getAliasesString());
        }
    }
/*    
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
*/
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

    /** 
     * Update user aliases from the comma-separated list in aliasesString.
     * Don't validate the format of each alias, that is handled separately.
     * But do make sure that we aren't adding any duplicate aliases, or an
     * alias that matches the user ID.
     */
    private void setAliasesFromString(String aliasesString) {        
        Set aliases = getUser().getAliases();
        aliases.clear();
        if (StringUtils.isEmpty(aliasesString)) {
            return;
        }

        // Tokenize the alias list and make sure there are no duplicates in
        // the list, or a collision with the user ID
        String[] aliasStrings = aliasesString.split(",");
        if (complainIfDuplicateAlias(aliasStrings)) {
            return;
        }
        
        // DO_NOW: check for collisions with the database, so that the aliases
        // we're adding won't duplicate existing aliases or user IDs.  Note:
        // the user we're modifying might already have some of these aliases,
        // that's OK since we're overwriting the aliases list.
        
        // Add each alias
        for (int i = 0; i < aliasStrings.length; i++) {
            String alias = aliasStrings[i];
            aliases.add(alias.trim());
        }
    }

    /**
     * Check for duplicate aliases and report an error if there are any.
     * If an alias matches the user ID, consider that a duplicate as well.
     * Return true if there were duplicates, false otherwise.
     */
    private boolean complainIfDuplicateAlias(String[] aliases) {
        if (aliases == null || aliases.length == 0) {
            return false;
        }
        final String dupAliasInEdit = "message.dupAliasInEdit";
        
        // store each alias in a map and look for collisions
        Map map = new HashMap(aliases.length);
        for (int i = 0; i < aliases.length; i++) {
            String alias = aliases[i];
            if (map.containsKey(alias)) {
                recordError(dupAliasInEdit);
                return true;
            }
            map.put(alias, null);
        }
        if (map.containsKey(getUser().getUserName())) {
            recordError(dupAliasInEdit);
            return true;
        }
        return false;
    }

    private void recordError(String messageId) {
        IValidationDelegate delegate = TapestryUtils.getValidator((AbstractComponent) getPage());
        delegate.record(getMessage(messageId), ValidationConstraint.CONSISTENCY);
    }
    
}
