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

import org.sipfoundry.sipxconfig.admin.dialplan.BeanWithId;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.phone.User;

/**
 * CallSequence
 */
public class CallSequence extends BeanWithId {
    private List m_calls = new ArrayList();
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
        m_calls = calls;
        m_user = user;
        m_withVoicemail = withVoicemail;
    }

    public Ring insertRing() {
        Ring ring = new Ring();
        ring.setCallSequence(this);
        m_calls.add(ring);
        ring.setPosition(m_calls.size() - 1);
        return ring;
    }

    public void removeRing(Integer ringId) {
        int index = m_calls.indexOf(new BeanWithId(ringId));
        m_calls.remove(index);
        for (int i = index; i < m_calls.size(); i++) {
            Ring ring = (Ring) m_calls.get(i);
            ring.setPosition(i);
        }
    }

    public boolean moveRingUp(Ring ring) {
        int i = m_calls.indexOf(ring);
        if (i <= 0) {
            return false;
        }
        m_calls.remove(i);
        m_calls.add(i - 1, ring);
        ring.setPosition(i - 1);
        return true;
    }

    public boolean moveRingDown(Ring ring) {
        int i = m_calls.indexOf(ring);
        if (i < 0 || i >= m_calls.size()) {
            return false;
        }
        m_calls.remove(i);
        m_calls.add(i + 1, ring);
        ring.setPosition(i + 1);
        return true;
    }

    public List generateAliases() {
        List aliases = new ArrayList(m_calls.size());
        ForkQueueValue q = new ForkQueueValue(m_calls.size());
        String domain = m_user.getOrganization().getDnsDomain();
        String identity = m_user.getDisplayId() + "@" + domain;
        for (Iterator i = m_calls.iterator(); i.hasNext();) {
            Ring r = (Ring) i.next();
            String contact = r.calculateContact(domain, q);
            AliasMapping alias = new AliasMapping(identity, contact);
            aliases.add(alias);
        }
        return aliases;
    }

    public synchronized List getCalls() {
        return m_calls;
    }

    public synchronized void setCalls(List calls) {
        m_calls = calls;
    }

    public synchronized User getUser() {
        return m_user;
    }

    public synchronized void setUser(User user) {
        m_user = user;
    }

    public synchronized boolean isWithVoicemail() {
        return m_withVoicemail;
    }

    public synchronized void setWithVoicemail(boolean withVoicemail) {
        m_withVoicemail = withVoicemail;
    }
}
