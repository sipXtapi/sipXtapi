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
package org.sipfoundry.sipxconfig.components;

import java.util.Collection;

/**
 * Wraps a collection of items where "selecting" items actualy adds items
 * to collection, and "unselecting" items removes items. Handy for multi-
 * select UIs. 
 * 
 * Items are assumed to be unique even if collection is not
 * a Set, but rather a List
 */
public class SelectionCollection {
    
    private Collection m_items;
    
    /**
     * Required, typically called in Tapestry's "bean" tag.   
     * 
     * Example:
     *  set-property name="collection" expression="bag.marbles"
     */
    public void setCollection(Collection items) {
        m_items = items;
    }
    
    /**
     * Example:
     *    beans.myMarbles.selected[currentMarble]
     */
    public boolean getSelected(Object id) {
        return m_items.contains(id);
    }
    
    public void setSelected(Object id, boolean selected) {
        if (selected) {
            if (!m_items.contains(id)) {
                m_items.add(id);
            }
        } else {
            m_items.remove(id);
        }
    }
}
