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

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;

public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {
    
    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();
        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.ActiveGreeting greeting = MailboxPreferences.ActiveGreeting.valueOfById(greetingId); 
        prefs.setActiveGreeting(greeting);
        List<Element> contacts = root.selectNodes("notification/contact");
        prefs.setEmailAddress(getEmailAddress(0, contacts));
        prefs.setAlternateEmailAddress(getEmailAddress(1, contacts));
        prefs.setAttachVoicemailToEmail(getAttachVoicemail(0, contacts));
        prefs.setAttachVoicemailToAlternateEmail(getAttachVoicemail(1, contacts));
        return prefs;
    }
    
    private String getEmailAddress(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return null;
        }
        return contacts.get(index).getText();
    }
    
    private boolean getAttachVoicemail(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return false;
        }
        String sAttachVm = contacts.get(index).attributeValue("attachments");
        return "yes".equals(sAttachVm);
    }
}
