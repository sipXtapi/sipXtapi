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

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.YesNo;

public class MailboxPreferencesWriter extends XmlWriterImpl<MailboxPreferences> {
    public MailboxPreferencesWriter() {
        setTemplate("mailbox/mailboxprefs.vm");
    }
    
    @Override
    protected void addContext(VelocityContext context, MailboxPreferences object) {
        context.put("preferences", object);
        context.put("yesNo", new YesNo());
    }
}
