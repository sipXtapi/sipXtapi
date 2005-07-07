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

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;

import net.sf.hibernate.Hibernate;
import net.sf.hibernate.HibernateException;
import net.sf.hibernate.UserType;

import org.apache.commons.lang.enum.Enum;
import org.apache.commons.lang.enum.EnumUtils;

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
     * @see net.sf.hibernate.UserType#sqlTypes()
     */
    public int[] sqlTypes() {
        return new int[] {
            Types.VARCHAR
        };
    }

    /**
     * @see net.sf.hibernate.UserType#returnedClass()
     */
    public Class returnedClass() {
        return m_enumClass;
    }

    /**
     * @see net.sf.hibernate.UserType#equals(java.lang.Object, java.lang.Object)
     */
    public boolean equals(Object x, Object y) throws HibernateException {
        if (x == y) {
            return true;
        }

        if (x == null || y == null) {
            return false;
        }

        return Hibernate.STRING.equals(x, y);
    }

    /**
     * @see net.sf.hibernate.UserType#nullSafeGet(java.sql.ResultSet, java.lang.String[],
     *      java.lang.Object)
     */
    public Object nullSafeGet(ResultSet rs, String[] names, Object owner_)
        throws HibernateException, SQLException {
        String enumCode = (String) Hibernate.STRING.nullSafeGet(rs, names[0]);

        return EnumUtils.getEnum(m_enumClass, enumCode);
    }

    /**
     * @see net.sf.hibernate.UserType#nullSafeSet(java.sql.PreparedStatement, java.lang.Object,
     *      int)
     */
    public void nullSafeSet(PreparedStatement st, Object value, int index) throws SQLException {
        // make sure the received value is of the right type
        if ((value != null) && !returnedClass().isAssignableFrom(value.getClass())) {
            throw new IllegalArgumentException("Received value is not a ["
                    + returnedClass().getName() + "] but [" + value.getClass() + "]");
        }

        Enum enum = (Enum) value;
        String enumCode = enum.getName();
        st.setString(index, enumCode);
    }

    /**
     * @see net.sf.hibernate.UserType#deepCopy(java.lang.Object)
     */
    public Object deepCopy(Object value) {
        return value;
    }

    /**
     * @see net.sf.hibernate.UserType#isMutable()
     */
    public boolean isMutable() {
        return false;
    }
}
