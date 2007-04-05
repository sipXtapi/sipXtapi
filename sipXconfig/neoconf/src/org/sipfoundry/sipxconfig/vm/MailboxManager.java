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
    
    public List<Voicemail> getVoicemail(Mailbox mailbox, String folder);
    
    public String getMailstoreDirectory();

    public Mailbox getMailbox(String userId);
    
    public void saveMailboxPreferences(Mailbox mailbox, MailboxPreferences preferences);
    
    public MailboxPreferences loadMailboxPreferences(Mailbox mailbox);

    public void saveDistributionLists(Mailbox mailbox, DistributionList[] lists);
    
    public DistributionList[] loadDistributionLists(Mailbox mailbox);

    public void markRead(Mailbox mailbox, Voicemail voicemail);
    
    public void move(Mailbox mailbox, Voicemail voicemail, String destinationFolderId);

    public void delete(Mailbox mailbox, Voicemail voicemail);
}
