/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user.EditPinComponent;
import org.sipfoundry.sipxconfig.site.user.UserForm;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;


public abstract class EditMyInformation extends UserBasePage implements EditPinComponent {
    
    public abstract String getPin();
    
    public abstract MailboxPreferences getMailboxPreferences();
    public abstract void setMailboxPreferences(MailboxPreferences preferences);
    
    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();
    
    public void save() {
        if (!getValidator().getHasErrors()) {
            getCoreContext().saveUser(getUser());
            
            MailboxManager mailMgr = getMailboxManager();
            if (mailMgr.isEnabled()) {
                Mailbox mailbox = mailMgr.getMailbox(getUser().getUserName());
                mailMgr.saveMailboxPreferences(mailbox, getMailboxPreferences());
            }
        }
    }
    
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        if (getPin() == null) {
            UserForm.initializePin(getComponent("pin"), this, getUser());
        } else if (TapestryUtils.isValid(this)) {
            UserForm.updatePin(this, getUser(), getCoreContext().getAuthorizationRealm());
        }
        
        MailboxManager mailMgr = getMailboxManager();
        if (getMailboxPreferences() == null && mailMgr.isEnabled()) {
            Mailbox mailbox = mailMgr.getMailbox(getUser().getUserName());
            setMailboxPreferences(mailMgr.loadMailboxPreferences(mailbox));            
        }
    }
}
