/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/DateUtils.java#2 $
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
 * DateUtils.java
 *
 * This class has utility methods for displaying or converting dates
 * to different formats.
 *
 * Created: Wed Oct 17 15:39:40 2001
 *
 * @version 1.0
 */

public class DateUtils {

    /**
    * Get the duration of the call in seconds formatted as a String.  This
    * works identically as getDuration, but added as a convenience.  The
    * format of the resulting string is: hh:mm:ss
    * @param duration duration in milliseconds
    * @return call duration in seconds formatted as a string
    */
    public static String getDurationAsString(long lDuration)
    {
        StringBuffer buffer = new StringBuffer() ;
        if( lDuration < 0)
            lDuration = 0;
        long lTotalSeconds = lDuration/1000 ;

        long lHours = (lTotalSeconds / (60*60)) ;
        lTotalSeconds -= (lHours * (60*60)) ;
        long lMinutes = (lTotalSeconds / 60) ;
        lTotalSeconds -= (lMinutes * 60) ;
        long lSeconds = lTotalSeconds ;

        buffer.append((lHours > 9) ? Long.toString(lHours) : "0"
                                                + Long.toString(lHours)) ;
        buffer.append( ":" );
        buffer.append((lMinutes > 9) ? Long.toString(lMinutes) : "0"
                                                + Long.toString(lMinutes)) ;
        buffer.append( ":" );
        buffer.append((lSeconds > 9) ? Long.toString(lSeconds) : "0"
                                                + Long.toString(lSeconds)) ;

        return buffer.toString() ;
    }



}// DateUtils
