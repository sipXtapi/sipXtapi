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

import org.apache.commons.lang.ArrayUtils;

public final class DataCollectionUtil {

    private DataCollectionUtil() {
        // prevent creation
    }

    /**
     * Removes items from collections by their primary key and updates the positions on the items
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
     * Given a list of objects that know their primary keys, extract a collection of just primary
     * keys
     * 
     * @param c collection of PrimaryKeySource objects
     * @return collecion of primary keys
     */
    public static Collection extractPrimaryKeys(Collection c) {
        ArrayList list = new ArrayList(c.size());
        for (Iterator i = c.iterator(); i.hasNext();) {
            list.add(((PrimaryKeySource) i.next()).getPrimaryKey());
        }
        return list;
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
     * Return list of items by their primary key. This may not be efficient for large collections.
     */
    public static Collection findByPrimaryKey(Collection c, Object[] primaryKeys) {
        List list = new ArrayList();

        // mark items to be deleted
        Iterator remove = c.iterator();
        while (remove.hasNext()) {
            PrimaryKeySource item = (PrimaryKeySource) remove.next();
            for (int j = 0; j < primaryKeys.length; j++) {
                if (item.getPrimaryKey().equals(primaryKeys[j])) {
                    list.add(item);
                }
            }
        }

        return list;
    }

    /**
     * Version of moveByPrimary keys that only works on collections of objects implementing
     * DataCollectionItem interface
     */
    public static void moveByPrimaryKey(List c, Object[] primaryKeys, int step) {
        moveByPrimaryKey(c, primaryKeys, step, true);
    }

    /**
     * Moves items from collections by their primary key and updates the positions on the items.
     * Items that are attempted to move out of list bounds are quietly moved to beginning or end
     * of list.
     * 
     * @param step how many slots to move items, positive or negative
     * @param c items must implement DataCollectionItem
     */
    public static void moveByPrimaryKey(List c, Object[] primaryKeys, int step,
            boolean updatePositions) {
        if (step < 0) {
            // move down
            int minPosition = 0;
            for (int i = 0; i < c.size(); i++) {
                PrimaryKeySource item = (PrimaryKeySource) c.get(i);
                boolean toBeMoved = ArrayUtils.contains(primaryKeys, item.getPrimaryKey());
                if (toBeMoved) {
                    int newPosition = Math.max(minPosition, i + step);
                    c.remove(i);
                    c.add(newPosition, item);
                    minPosition = newPosition + 1;
                }
            }
        } else if (step > 0) {
            // move up
            int maxPosition = c.size() - 1;
            for (int i = c.size() - 1; i >= 0; i--) {
                PrimaryKeySource item = (PrimaryKeySource) c.get(i);
                boolean toBeMoved = ArrayUtils.contains(primaryKeys, item.getPrimaryKey());
                if (toBeMoved) {
                    int newPosition = Math.min(i + step, maxPosition);
                    c.remove(i);
                    c.add(newPosition, item);
                    maxPosition = newPosition - 1;
                }
            }
        }

        if (updatePositions) {
            updatePositions(c);
        }
    }
}
