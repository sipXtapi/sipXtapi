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
package org.sipfoundry.sipxconfig.speeddial;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

public class Button {
    private String m_label;
    private String m_number;

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getNumber() {
        return m_number;
    }

    public void setNumber(String number) {
        m_number = number;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_label).append(m_number).toHashCode();
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Button)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Button rhs = (Button) obj;
        return new EqualsBuilder().append(m_label, rhs.m_label).append(m_number, rhs.m_number)
                .isEquals();
    }
}
