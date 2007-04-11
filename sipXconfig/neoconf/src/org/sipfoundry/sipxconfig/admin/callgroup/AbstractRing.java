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

import java.text.MessageFormat;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.EnumUserType;

public abstract class AbstractRing extends BeanWithId implements DataCollectionItem {
    public static final String TYPE_PROP = "type";

    private static final int DEFAULT_EXPIRATION = 30;
    private static final String FORMAT = "<sip:{0}{1}{4}?expires={2}>;{3}";
    private static final String IGNORE_VOICEMAIL_FIELD_PARAM = ";sipx-noroute=Voicemail";

    private int m_expiration = DEFAULT_EXPIRATION;
    private Type m_type = Type.DELAYED;
    private int m_position;
    private boolean m_enabled = true;

    public int getExpiration() {
        return m_expiration;
    }

    public void setExpiration(int expiration) {
        m_expiration = expiration;
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }

    public Type getType() {
        return m_type;
    }

    public void setType(Type type) {
        m_type = type;
    }


    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }


    public static class Type extends Enum {
        public static final Type DELAYED = new Type("If no response");
        public static final Type IMMEDIATE = new Type("At the same time");

        public Type(String name) {
            super(name);
        }

        public static Type getEnum(String type) {
            return (Type) getEnum(Type.class, type);
        }
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(Type.class);
        }
    }

    /**
     * Retrieves the user part of the contact used to calculate contact
     * 
     * @return String or object implementing toString method
     */
    protected abstract Object getUserPart();

    static boolean isAor(String userPart) {
        return userPart.indexOf('@') >= 0;
    }

    /**
     * Calculates contact for line or alias. See FORMAT field.
     * 
     * @param domain contact domain
     * @param q contact q value
     * @return String representing the contact
     */
    public final String calculateContact(String domain, ForkQueueValue q,
            boolean appendIgnoreVoicemail) {

        // XCF-963 Allow forwarding to user supplied AORs
        String domainPart;
        String userPart = getUserPart().toString();
        if (isAor(userPart)) {
            domainPart = StringUtils.EMPTY;
        } else {
            domainPart = '@' + domain;
        }

        MessageFormat format = new MessageFormat(FORMAT);
        String urlParams = appendIgnoreVoicemail ? IGNORE_VOICEMAIL_FIELD_PARAM
                : StringUtils.EMPTY;
        Object[] params = new Object[] {
            userPart, domainPart, new Integer(m_expiration), q.getValue(m_type), urlParams
        };
        return format.format(params);
    }
}