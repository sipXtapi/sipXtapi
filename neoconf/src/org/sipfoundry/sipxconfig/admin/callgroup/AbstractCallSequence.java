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
import java.util.List;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * CallSequence
 */
public class AbstractCallSequence extends BeanWithId {
    private List m_calls = new ArrayList();

    public AbstractCallSequence() {
        // empty default constructor
    }

    protected void insertRing(AbstractRing ring) {
        m_calls.add(ring);
        ring.setPosition(m_calls.size() - 1);
    }

    public void removeRing(AbstractRing ringToRemove) {
        int index = m_calls.indexOf(ringToRemove);
        m_calls.remove(index);
        for (int i = index; i < m_calls.size(); i++) {
            AbstractRing ring = (AbstractRing) m_calls.get(i);
            ring.setPosition(i);
        }
    }

    public boolean moveRingUp(AbstractRing ring) {
        int i = m_calls.indexOf(ring);
        if (i <= 0) {
            return false;
        }
        m_calls.remove(i);
        m_calls.add(i - 1, ring);
        ring.setPosition(i - 1);
        return true;
    }

    public boolean moveRingDown(AbstractRing ring) {
        int i = m_calls.indexOf(ring);
        if (i < 0 || i >= m_calls.size()) {
            return false;
        }
        m_calls.remove(i);
        m_calls.add(i + 1, ring);
        ring.setPosition(i + 1);
        return true;
    }

    public List getCalls() {
        return m_calls;
    }

    public void setCalls(List calls) {
        m_calls = calls;
    }
}
