/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/Comparable.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

/**
 * Comparable.java
 *
 *
 * Created: Thu Oct 11 17:51:07 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */
package org.sipfoundry.util;

public interface Comparable {

    /** @return  a negative integer, zero, or a positive integer as this object
     *	 is less than, equal to, or greater than the specified object.
     */
    public int compareTo(Object o);

}// Comparable
