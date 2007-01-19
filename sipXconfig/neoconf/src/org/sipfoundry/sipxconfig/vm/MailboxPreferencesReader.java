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

import org.dom4j.Document;
import org.dom4j.Node;

public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {

    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();
        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.ActiveGreeting greeting = MailboxPreferences.ActiveGreeting.valueOfById(greetingId); 
        prefs.setActiveGreeting(greeting);
        prefs.setEmailAddress(root.valueOf("notification/contact/text()"));
        String sAttachVm = root.valueOf("notification/contact/@attachments");
        prefs.setAttachVoicemailToEmail("yes".equals(sAttachVm));
        return prefs;
    }
}
