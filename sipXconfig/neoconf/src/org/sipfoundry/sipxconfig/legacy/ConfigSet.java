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
package org.sipfoundry.sipxconfig.legacy;


/**
 * Represents legacy ProfileGenerator config sets
 * Config set is xlob (Xml Large OBject). Useful information is hidden in undocumented XML entries.
 * All XML parsing functions are defined here
 */
public class ConfigSet {
    private Integer m_id;
    private String m_content;
    public String getContent() {
        return m_content;
    }    

    public void setContent(String content) {
        this.m_content = content;
    }
    
    public Integer getId() {
        return m_id;
    }
    
    public void setId(Integer id) {
        this.m_id = id;
    }    
}
