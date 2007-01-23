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
package org.sipfoundry.sipxconfig.vm;

public class DistributionList implements Comparable {
    private int m_position = 1;
    private String[] m_extensions = new String[0];
    
    public String[] getExtensions() {
        return m_extensions;
    }
    public void setExtensions(String[] extensions) {
        m_extensions = extensions;
    }
    public int getPosition() {
        return m_position;
    }
    public void setPosition(int position) {
        m_position = position;
    }
    public int compareTo(Object o) {
        if (o == null || !(o instanceof DistributionList)) {
            return 1;
        }

        return getPosition() - ((DistributionList) o).getPosition();
    }
}
