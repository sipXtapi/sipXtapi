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
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;

/**
 * CallSequence
 */
public class AbstractCallSequence extends BeanWithId {
    private List m_calls = new ArrayList();

    public AbstractCallSequence() {
        // empty default constructor
    }

    /**
     * Deep clone. Hiberate friendly.
     */
    protected Object clone() throws CloneNotSupportedException {
        AbstractCallSequence clone = (AbstractCallSequence) super.clone();
        clone.m_calls = new ArrayList(m_calls.size());
        DataCollectionUtil.duplicate(m_calls, clone.m_calls);
        return clone;
    }

    protected void insertRing(AbstractRing ring) {
        m_calls.add(ring);
        DataCollectionUtil.updatePositions(m_calls);
    }

    public void removeRings(Collection ids) {
        DataCollectionUtil.removeByPrimaryKey(m_calls, ids.toArray());
    }

    public void moveRings(Collection ids, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_calls, ids.toArray(), step);
    }

    public void removeRing(AbstractRing ringToRemove) {
        Object[] keys = new Object[] {
            ringToRemove.getId()
        };
        DataCollectionUtil.removeByPrimaryKey(m_calls, keys);
    }

    protected void clearRings() {
        m_calls.clear();
    }

    public boolean moveRingUp(AbstractRing ring) {
        Object[] keys = new Object[] {
            ring.getId()
        };
        DataCollectionUtil.moveByPrimaryKey(m_calls, keys, -1);
        return true;
    }

    public boolean moveRingDown(AbstractRing ring) {
        Object[] keys = new Object[] {
            ring.getId()
        };
        DataCollectionUtil.moveByPrimaryKey(m_calls, keys, 1);
        return true;
    }

    public List getCalls() {
        return m_calls;
    }

    public void setCalls(List calls) {
        m_calls = calls;
    }

    /**
     * Generate aliases from the calling list. All aliases have the following form: identity ->
     * ring_contact
     * 
     * @param identity
     * @param domain used to calculate proper URI for ring contact
     * @return list of AliasMapping objects
     */
    protected List generateAliases(String identity, String domain) {
        List calls = getCalls();
        List aliases = new ArrayList(calls.size());
        ForkQueueValue q = new ForkQueueValue(calls.size());
        for (Iterator i = calls.iterator(); i.hasNext();) {
            AbstractRing r = (AbstractRing) i.next();
            if (StringUtils.isEmpty(r.getUserPart().toString())) {
                continue;
            }
            String contact = r.calculateContact(domain, q, true);
            AliasMapping alias = new AliasMapping(identity, contact);
            aliases.add(alias);
        }
        return aliases;
    }

    /**
     * Clear all calls for this sequence
     */
    public void clear() {
        m_calls.clear();
    }
}
