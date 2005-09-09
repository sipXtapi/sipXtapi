/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.NamedObject;


/**
 * User labeled storage of settings.    
 * 
 * @author dhubler
 *
 */
public class Group extends ValueStorage implements Comparable, DataCollectionItem, NamedObject {

    private String m_name;
    
    private String m_description;

    private String m_resource;
    
    private Integer m_weight;

    public String getName() {
        return m_name;
    }

    public void setName(String label) {
        m_name = label;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getResource() {
        return m_resource;
    }

    public void setResource(String resource) {
        m_resource = resource;
    }

    /**
     * When setting values conflict, the setting with the highest weight wins.
     * 
     * @return setting weight
     */
    public Integer getWeight() {
        return m_weight;
    }

    public void setWeight(Integer weight) {
        m_weight = weight;
    }

    public int compareTo(Object arg0) {
        Group b = (Group) arg0;
        Integer w = m_weight != null ? m_weight : new Integer(-1);
        
        return w.compareTo(b.getWeight());
    }

    /**
     * byproduct of DataCollectionItem interface, returns weight - 1
     */
    public int getPosition() {
        int w = (m_weight != null ? m_weight.intValue() : -1);
        return w - 1;
    }

    /**
     * byproduct of DataCollectionItem interface, sets weight to position + 1
     */
    public void setPosition(int position) {
        m_weight = new Integer(position + 1);
    }
}
