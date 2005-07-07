/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/Comparer.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util;


/**
 * This interface defines an external comparer that can be used with the
 * QuickSort algorithm.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface Comparer
{
    /**
     * Compare the two supplied objects for equality.  The method should return
     * <pre>
     *    < 0 : if o1 < o2
     *    = 0 : if o1 == o2
     *    > 0 : if o1 > 02
     *</pre>
     *
     * @param o1 the first object to compare with
     * @param o2 the second object to compare with
     */
    public int compare(Object o1, Object o2);
}
