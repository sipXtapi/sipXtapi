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
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.enum.Enum;

/**
 * CallPattern
 */
public class CallPattern {
    private Digits m_digits;
    private String m_prefix;

    public Digits getDigits() {
        return m_digits;
    }
    
    public void setDigits(Digits digits) {
        m_digits = digits;
    }
    
    public String getPrefix() {
        return m_prefix;
    }
    
    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }
    
    public String calculatePattern() {
        String digits = "{" + m_digits.getName() + "}";
        if( m_digits.equals(Digits.NO_DIGITS))
        {
            digits = "";
        }
        return m_prefix + digits;
    }
    
    public static class Digits extends Enum {
        public static final Digits NO_DIGITS = new Digits("nodigits");
        public static final Digits VARIABLE_DIGITS = new Digits("vdigits");
        public static final Digits FIXED_DIGITS = new Digits("digits");
        
        public Digits(String name) {
            super(name);
        }        
    }
}
