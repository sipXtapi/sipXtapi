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

import java.text.MessageFormat;

import org.apache.commons.lang.enum.Enum;

import org.sipfoundry.sipxconfig.admin.dialplan.BeanWithId;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;

/**
 * Ring - represents one stage in a call forwaring sequence
 */
public class Ring extends BeanWithId {
    private static final String FORMAT = "<sip:{0}@{1}?expires={2}>;;{3}";

    private String m_number;
    private int m_expiration;
    private Type m_type = Type.DELAYED;

    /**
     * Default "bean" constructor
     */
    public Ring() {
        // empty
    }

    /**
     * @param number phone number or SIP url to which call is to be transfered
     * @param expiration number of seconds that call will ring
     * @param type if the call should wait for the previous call failure or start ringing at the
     *        same time
     */
    Ring(String number, int expiration, Type type) {
        m_number = number;
        m_expiration = expiration;
        m_type = type;
    }

    public static class Type extends Enum {
        public static final Type DELAYED = new Type("If no response");
        public static final Type IMMEDIATE = new Type("At the same time");

        public Type(String name) {
            super(name);
        }
    }

    public String calculateContact(String domain, ForkQueueValue q) {
        MessageFormat format = new MessageFormat(FORMAT);
        Object[] params = new Object[] {
            m_number, domain, new Integer(m_expiration), q.getValue(m_type)
        };
        return format.format(params);
    }

    // getters on setters
    public synchronized int getExpiration() {
        return m_expiration;
    }

    public synchronized void setExpiration(int expiration) {
        m_expiration = expiration;
    }

    public synchronized String getNumber() {
        return m_number;
    }

    public synchronized void setNumber(String number) {
        m_number = number;
    }

    public synchronized Type getType() {
        return m_type;
    }

    public synchronized void setType(Type type) {
        m_type = type;
    }
}
