/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags.util;

import java.util.Comparator;

public class CaseInsensitiveComparator implements Comparator {

    public int compare ( Object o1, Object o2 ) {

        String s1 = (String) o1;
        String s2 = (String) o2;

        if ( s1.compareToIgnoreCase( s2 ) == 0 )
            return s1.compareTo( s2 );

        return s1.compareToIgnoreCase( s2 );
    }

}