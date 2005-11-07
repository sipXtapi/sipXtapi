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
package org.sipfoundry.sipxconfig.common;

import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.collections.iterators.EmptyIterator;
import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;

public final class SipxCollectionUtils {

    /** Singleton class with static methods, don't allow anyone to instantiate */
    private SipxCollectionUtils() {
    }
    
    /** Return the Collection size.  Return 0 if the Collection is null. */
    public static int safeSize(Collection coll) {
        return coll != null ? coll.size() : 0;
    }
    
    public static boolean safeIsEmpty(Collection coll) {
        return safeSize(coll) == 0;
    }
    
    /** Return a Collection iterator.  Return an empty iterator if the Collection is null. */
    public static Iterator safeIterator(Collection coll) {
        return coll != null ? coll.iterator() : EmptyIterator.INSTANCE;
    }

    /**
     * Given a comma-delimited string of names, return the names as a string array. Trim
     * leading and trailing whitespace from each name.
     */
    public static String[] splitString(String delimitedString) {
        if (StringUtils.isBlank(delimitedString)) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        String delmittedClean = delimitedString.trim();
        String[] split = delmittedClean.split("\\s*,\\s*");
        return split;
    }
}
