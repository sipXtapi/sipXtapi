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
package org.sipfoundry.sipxconfig.vm;

import java.util.List;

public interface MailboxManager {
    
    public boolean isEnabled();
    
    public List<String> getFolderIds(String userid);

    public List<Voicemail> getVoicemail(String userid, String folder);
    
    public String getMailstoreDirectory();

    public Mailbox getMailbox(String userId);
    
    public void saveMailboxPreferences(Mailbox mailbox, MailboxPreferences preferences);
    
    public MailboxPreferences loadMailboxPreferences(Mailbox mailbox);

    public void saveDistributionLists(Mailbox mailbox, DistributionList[] lists);
    
    public DistributionList[] loadDistributionLists(Mailbox mailbox);

}
