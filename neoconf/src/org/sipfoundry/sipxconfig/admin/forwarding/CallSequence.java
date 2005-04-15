/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.callgroup.AbstractCallSequence;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.common.User;

/**
 * CallSequence
 */
public class CallSequence extends AbstractCallSequence {
    private User m_user;
    private boolean m_withVoicemail;

    public CallSequence() {
        // empty default constructor
    }

    /**
     * @param calls list of ring objects
     * @param user original phone number
     * @param withVoicemail
     */
    CallSequence(List calls, User user, boolean withVoicemail) {
        m_user = user;
        m_withVoicemail = withVoicemail;
        setCalls(calls);
    }

    public Ring insertRing() {
        Ring ring = new Ring();
        ring.setCallSequence(this);
        insertRing(ring);
        return ring;
    }

    public List generateAliases() {        
        List calls = getCalls();
        List aliases = new ArrayList(calls.size());
        ForkQueueValue q = new ForkQueueValue(calls.size());
        String domain = m_user.getOrganization().getDnsDomain();
        String identity = m_user.getDisplayId() + "@" + domain;
        for (Iterator i = calls.iterator(); i.hasNext();) {
            Ring r = (Ring) i.next();
            String contact = r.calculateContact(domain, q);
            AliasMapping alias = new AliasMapping(identity, contact);
            aliases.add(alias);
        }
        return aliases;
    }
    
    public List generateAuthExceptions() {
        List calls = getCalls();
        List authExceptions = new ArrayList();
        for (Iterator i = calls.iterator(); i.hasNext();) {
            Ring r = (Ring) i.next();
            authExceptions.add(r.getNumber());
        }
        return authExceptions;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public boolean isWithVoicemail() {
        return m_withVoicemail;
    }

    public void setWithVoicemail(boolean withVoicemail) {
        m_withVoicemail = withVoicemail;
    }
}
