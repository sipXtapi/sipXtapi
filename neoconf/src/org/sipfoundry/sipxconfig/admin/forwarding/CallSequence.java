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

/**
 * CallSequence
 */
public class CallSequence extends BeanWithId {
    private List m_calls = new ArrayList();
    private String m_user;
    private String m_domain;
    private boolean m_withVoicemail;

    public CallSequence() {
        // empty default constructor
    }

    /**
     * @param calls list of ring objects
     * @param user original phone number
     * @param withVoicemail
     */
    CallSequence(List calls, String user, boolean withVoicemail) {
        m_calls = calls;
        m_user = user;
        m_withVoicemail = withVoicemail;
    }

    public void addRing(Ring ring) {
        m_calls.add(ring);
    }

    public void removeRing(Integer ringId) {
        m_calls.remove(new BeanWithId(ringId));
    }

    private Ring getRingOrg(Integer ringId) {
        int i = m_calls.indexOf(new BeanWithId(ringId));
        if (i < 0) {
            return null;
        }
        return (Ring) m_calls.get(i);
    }

    public Ring getRing(Integer ringId) {
        return (Ring) getRingOrg(ringId).detach();
    }

    public void updateRing(Integer ringId, Ring ring) {
        Ring ringOrg = getRingOrg(ringId);
        ringOrg.update(ring);
    }

    public List generateAliases() {
        List aliases = new ArrayList(m_calls.size());
        ForkQueueValue q = new ForkQueueValue(m_calls.size());
        String identity = m_user + "@" + m_domain;
        for (Iterator i = m_calls.iterator(); i.hasNext();) {
            Ring r = (Ring) i.next();
            String contact = r.calculateContact(m_domain, q);
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

    public synchronized String getUser() {
        return m_user;
    }

    public synchronized void setUser(String user) {
        m_user = user;
    }

    public synchronized boolean isWithVoicemail() {
        return m_withVoicemail;
    }

    public synchronized void setWithVoicemail(boolean withVoicemail) {
        m_withVoicemail = withVoicemail;
    }

    public synchronized String getDomain() {
        return m_domain;
    }

    public synchronized void setDomain(String domain) {
        m_domain = domain;
    }
}
