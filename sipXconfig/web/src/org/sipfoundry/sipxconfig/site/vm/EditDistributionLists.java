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
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Lifecycle;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.RowInfo;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.DistributionList;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public abstract class EditDistributionLists extends UserBasePage {   
    public static final String PAGE = "vm/EditDistributionLists";
    
    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();
    
    public abstract DistributionList[] getDistributionLists();
    public abstract void setDistributionLists(DistributionList[] lists);
    
    public abstract DistributionList getDistributionList();
    
    @Bean()
    public RowInfo getRowInfo() {
        return RowInfo.UNSELECTABLE;
    }
    
    @Bean(lifecycle = Lifecycle.PAGE)
    public abstract EvenOdd getRowClass();
    
    public String getExtensionsString() {
        return TapestryUtils.joinBySpace(getDistributionList().getExtensions());
    }
    
    public void save() {
        if (!getValidator().getHasErrors()) {
            Mailbox mailbox = getMailboxManager().getMailbox(getUser().getUserName()); 
            getMailboxManager().saveDistributionLists(mailbox, getDistributionLists());
        }        
    }
    
    public void setExtensionsString(String extensions) {
        getDistributionList().setExtensions(TapestryUtils.splitBySpace(extensions));
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        
        DistributionList[] lists = getDistributionLists();
        if (lists == null) {
            Mailbox mailbox = getMailboxManager().getMailbox(getUser().getUserName());
            lists = getMailboxManager().loadDistributionLists(mailbox);
            setDistributionLists(lists);            
        }
    }    
}
