package org.sipfoundry.util;

import java.util.*;

/**
 * Hashtable that preserves order
 */
public class OrderedHashtable extends Hashtable
{
    Vector m_orderedKeys;

    public OrderedHashtable()
    {
        m_orderedKeys = new Vector();
    }

    public void clear()
    {
        super.clear();
        m_orderedKeys.removeAllElements();
    }

    public Object remove(Object key)
    {
        Object o = super.remove(key);
        if (o != null)
        {
            // o is not null when there was already an item so 
            // replace it.
            m_orderedKeys.removeElement(key);
        }

        return o;
    }

    public synchronized Object put(Object key, Object value)
    {
        Object o = super.put(key, value);

        // update ordered list, put on end if new item
        int index = -1;
        if (o != null)
        {
            // o is not null when there was already an item so 
            // replace it.
            index = m_orderedKeys.indexOf(key);
        }

        if (index >= 0)
        {
            m_orderedKeys.setElementAt(key, index);
        }
        else
        {
            m_orderedKeys.addElement(key);
        }
        
        return o;
    }

    /**
     * keys as they were added to hashtable
     */
    public Enumeration getOrderedKeys()
    {
        return m_orderedKeys.elements();
    }
}
