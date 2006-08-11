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
package org.sipfoundry.sipxconfig.permission;

import java.io.Serializable;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;

import org.apache.commons.lang.StringUtils;
import org.hibernate.Hibernate;
import org.hibernate.usertype.UserType;
import org.hibernate.util.EqualsHelper;

public class PermissionUserType implements UserType {

    public Object assemble(Serializable cached, Object owner) {
        return null;
    }

    public Object deepCopy(Object value) {
        return value;
    }

    public Serializable disassemble(Object value) {
        return null;
    }

    public boolean equals(Object x, Object y) {
        return EqualsHelper.equals(x, y);
    }

    public int hashCode(Object x) {
        return x.hashCode();
    }

    public boolean isMutable() {
        return false;
    }

    public Object nullSafeGet(ResultSet rs, String[] names, Object owner) throws SQLException {
        String name = (String) Hibernate.STRING.nullSafeGet(rs, names[0]);
        if (StringUtils.isEmpty(name)) {
            return null;
        }
        return Permission.Type.CALL.create(name);
    }

    public void nullSafeSet(PreparedStatement st, Object value, int index) throws SQLException {
        // make sure the received value is of the right type
        if ((value != null) && !(value instanceof Permission)) {
            throw new IllegalArgumentException("Received value is not a Permission but ["
                    + value.getClass() + "]");
        }

        if (value == null) {
            st.setNull(index, Types.VARCHAR);
            return;
        }

        Permission p = (Permission) value;
        st.setString(index, p.getName());
    }

    public Object replace(Object original, Object target, Object owner) {
        return original;
    }

    public Class returnedClass() {
        return Permission.class;
    }

    public int[] sqlTypes() {
        return new int[] {
            Types.VARCHAR
        };
    }
}
