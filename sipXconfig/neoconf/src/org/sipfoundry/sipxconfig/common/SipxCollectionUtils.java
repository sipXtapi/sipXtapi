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

public final class SipxCollectionUtils {

    /** Singleton class with static methods, don't allow anyone to instantiate */
    private SipxCollectionUtils() {
    }

    /** Return the Collection size. Return 0 if the Collection is null. */
    public static int safeSize(Collection coll) {
        return coll != null ? coll.size() : 0;
    }

    public static boolean safeIsEmpty(Collection coll) {
        return safeSize(coll) == 0;
    }

    /** Return a Collection iterator. Return an empty iterator if the Collection is null. */
    public static Iterator safeIterator(Collection coll) {
        return coll != null ? coll.iterator() : EmptyIterator.INSTANCE;
    }
}
