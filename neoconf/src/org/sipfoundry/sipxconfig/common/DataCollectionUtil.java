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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;


public final class DataCollectionUtil {
    
    private DataCollectionUtil() {    
    }

    /**
     * Removes items from collections by their primary key and updates the 
     * positions on the items
     * 
     * @param c items must implement DataCollectionItem
     */
    public static Collection removeByPrimaryKey(Collection c, Object[] primaryKeys) {

        Collection removed = findByPrimaryKey(c, primaryKeys);
        Iterator remove = removed.iterator();
        while (remove.hasNext()) {
            c.remove(remove.next());
        }
        
        updatePositions(c);
                
        return removed;
    }
    
    /**
     * Fixes position fields after a list manipluation such.
     */
    public static void updatePositions(Collection c) {
        // update positions
        Iterator update = c.iterator();
        for (int i = 0; update.hasNext(); i++) {
            DataCollectionItem item = (DataCollectionItem) update.next();
            item.setPosition(i);
        }                
    }
    
    /**
     * Return list of items by their primary key.  This may not be efficient for
     * large collections.
     */
    public static Collection findByPrimaryKey(Collection c, Object[] primaryKeys) {
        List list = new ArrayList();
        
        // mark items to be deleted
        Iterator remove = c.iterator();
        while (remove.hasNext()) {
            DataCollectionItem item = (DataCollectionItem) remove.next();
            for (int j = 0; j < primaryKeys.length; j++) {
                if (item.getPrimaryKey().equals(primaryKeys[j])) {
                    list.add(item);
                }
            }
        }    
        
        return list;
    }
    
    /**
     * Moves items from collections by their primary key and updates the 
     * positions on the items.  Items that are attempted to move out of list
     * bounds are quietly moved to beginning or end of list.  
     * 
     * @param step how many slots to move items, positive or negative
     * @param c items must implement DataCollectionItem
     */
    public static Collection moveByPrimaryKey(Collection c, Object[] primaryKeys, int step) {
        List clist = new ArrayList(c);
        Collection move = findByPrimaryKey(clist, primaryKeys);
        
        Iterator imove = move.iterator();
        for (int i = 0; imove.hasNext(); i++) {
            DataCollectionItem item = (DataCollectionItem) imove.next();
            int newPosition = Math.max(i, Math.min(item.getPosition() + step, c.size() - 1));            
            clist.remove(item);
            clist.add(newPosition, item);
        }
        
        updatePositions(clist);

        // copy into original list
        c.clear();
        c.addAll(clist);
                
        return move;        
    }
}
