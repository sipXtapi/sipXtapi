/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.sds.sip;

import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class SipMultiValuedHeader {
    private String m_primaryValue;
    private Properties m_nvPairs;

    // Contructor used during restoration from the database
    public SipMultiValuedHeader( String primaryValue, Properties nvPairs ) {
        m_primaryValue = primaryValue;
        m_nvPairs = nvPairs;
    }

   /**
     * Constructor that parses the multiPartSipHeader into a
     * primaryValue and a set of names value pairs associated
     * with this primary value
     *
     * @param multiPartSipHeader the string in the form of
     * primaryvalue;n1=v1;n2=v2...nn=vn
     */
    public SipMultiValuedHeader( String multiPartSipHeader ) {
        StringTokenizer toker = new StringTokenizer(multiPartSipHeader, ";");

        if ( toker.hasMoreElements() ) {
            // primary value of this MultiPart header, this should be constant
            // throughoput a session
            m_primaryValue = toker.nextToken();
            if (m_primaryValue != null) {
                // Get all of the other params
                while (toker.hasMoreElements()) {
                    // Ensure that the properties object is
                    // allocated only when required
                    if ( m_nvPairs == null )
                        m_nvPairs = new Properties();

                    String strParam = toker.nextToken();
                    if (strParam != null) {
                        strParam = strParam.trim();
                        int iEqualsIndex = strParam.indexOf('=');
                        if (iEqualsIndex != -1) {
                            String strName = strParam.substring(0, iEqualsIndex);
                            String strValue = "";
                            if (iEqualsIndex < strParam.length()) {
                                strValue = strParam.substring(iEqualsIndex+1);
                            }
                            m_nvPairs.setProperty(strName, strValue);
                        }
                    }
                }
            }
        }
    }

    /**
     * Getter for the multivalued attributes
     */
    public Properties getNvPairs() {
        return m_nvPairs;
    }

    /**
     * getter for an attribute field
     */
    public String getProperty (String propertyName) {
        return m_nvPairs.getProperty( propertyName );
    }

    /**
     * Event, To, From or Call-ID value
     */
    public String getPrimaryValue() {
        return m_primaryValue;
    }

    /**
     * Stored in a hashtable so this will be called when we
     * test for equality between 2 Contexts
     */
    public boolean equals(Object arg) {
        if ( arg instanceof SipMultiValuedHeader ) {
            if (arg == this) {
                return true;
            } else {
                // Compare the primary values first
                SipMultiValuedHeader rhs = (SipMultiValuedHeader) arg;
                if (m_primaryValue.equals(rhs.getPrimaryValue())) {
                    // Now compare the name value pairs
                    if ( (m_nvPairs != null) && ( rhs.getNvPairs() != null )) {
                        return m_nvPairs.equals( rhs.getNvPairs() );
                    } else if ((m_nvPairs == null) && ( rhs.getNvPairs() == null )) {
                        // both sides with null Name value pairs
                        return true;
                    }
                }
            }
        }
        return false;
    }

    /**
     *  @JC Method that attempts to reconstruct the original Multivalued Header
     *  this should be the primaryValue with ';' separating all the other name
     *  value pairs
     */
    public String toString() {
        StringBuffer sb = new StringBuffer(m_primaryValue);
        if ( m_nvPairs != null ) {
            Enumeration enum = m_nvPairs.propertyNames();
            while ( enum.hasMoreElements() ) {
                String name = (String)enum.nextElement();
                sb.append(";" + name + "=" + m_nvPairs.getProperty (name) );
            }
        }
        return sb.toString();
    }
}
