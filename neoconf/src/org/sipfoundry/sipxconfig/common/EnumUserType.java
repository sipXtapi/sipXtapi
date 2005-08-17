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
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;

import org.apache.commons.lang.enums.Enum;
import org.apache.commons.lang.enums.EnumUtils;
import org.hibernate.Hibernate;
import org.hibernate.usertype.UserType;

/**
 * EnumUserType Maps a commons-lang <code>Enum</code> to a Hibernate type. From the example on
 * http://www.hibernate.org/172.html
 */
public class EnumUserType implements UserType {
    private Class m_enumClass;

    public EnumUserType(Class enumClass) {
        m_enumClass = enumClass;
    }

    /**
     * @see org.hibernate.usertype.UserType#sqlTypes()
     */
    public int[] sqlTypes() {
        return new int[] {
            Types.VARCHAR
        };
    }

    /**
     * @see org.hibernate.usertype.UserType#returnedClass()
     */
    public Class returnedClass() {
        return m_enumClass;
    }

    /**
     * @see org.hibernate.usertype.UserType#equals(java.lang.Object, java.lang.Object)
     */
    public boolean equals(Object x, Object y)  {
        if (x == y) {
            return true;
        }

        if (x == null || y == null) {
            return false;
        }

        return Hibernate.STRING.isEqual(x, y);
    }

    /**
     * @see org.hibernate.usertype.UserType#nullSafeGet(java.sql.ResultSet, java.lang.String[],
     *      java.lang.Object)
     */
    public Object nullSafeGet(ResultSet rs, String[] names, Object owner_) throws SQLException {
        String enumCode = (String) Hibernate.STRING.nullSafeGet(rs, names[0]);

        return EnumUtils.getEnum(m_enumClass, enumCode);
    }

    /**
     * @see org.hibernate.usertype.UserType#nullSafeSet(java.sql.PreparedStatement, java.lang.Object,
     *      int)
     */
    public void nullSafeSet(PreparedStatement st, Object value, int index) throws SQLException {
        // make sure the received value is of the right type
        if ((value != null) && !returnedClass().isAssignableFrom(value.getClass())) {
            throw new IllegalArgumentException("Received value is not a ["
                    + returnedClass().getName() + "] but [" + value.getClass() + "]");
        }

        Enum enumeration = (Enum) value;
        String enumCode = enumeration.getName();
        st.setString(index, enumCode);
    }

    /**
     * @see org.hibernate.usertype.UserType#deepCopy(java.lang.Object)
     */
    public Object deepCopy(Object value) {
        return value;
    }

    /**
     * @see org.hibernate.usertype.UserType#isMutable()
     */
    public boolean isMutable() {
        return false;
    }

    public int hashCode(Object x) {
        return x.hashCode();
    }

    public Serializable disassemble(Object value_) {
        return null;
    }

    public Object assemble(Serializable cached_, Object owner_) {
        return null;
    }

    public Object replace(Object original, Object target_, Object owner_) {
        return original;
    }
}
