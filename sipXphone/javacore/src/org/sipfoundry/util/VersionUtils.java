/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/VersionUtils.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.util ;

import java.util.StringTokenizer ;

/**
 * This is a utility class housing static helper routines for version
 * comparing, displaying, etc.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class VersionUtils
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private VersionUtils()
    {

    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Build a version string that combines the base ("x.x.x") and the build
     * number into a "x.x.x.x" string suitable for presentation.  If the
     * iBuildNumber is less than zero, strBase is returned.
     *
     * @param strBase the base version in the format "x.X.x"
     * @param iBuildNumber the build number
     *
     * @return a combined version in the format "x.x.x.x" or strBase if the
     *         build number is less than zero.
     */
    public static String buildVersionString(String strBase, int iBuildNumber)
    {
        String strVersion = "" ;

        if (strBase != null)
            strVersion = strBase ;

        if (iBuildNumber >= 0)
            strVersion = strVersion + "." + Integer.toString(iBuildNumber) ;

        return strVersion ;
    }


    /**
     * Compares two version strings for equality.
     *
     * @return < 0 if strVersion1 <  strVersion2,
     *         > 0 if strVersion1 >  strVersion2,
     *         = 0 if strVersion2 == strVersion2
     */
    public static int compareVersions(String strVersion1, String strVersion2)
    {
        // Check Nulls
        if ((strVersion1 == null) && (strVersion2 == null))
            return 0 ;
        else if (strVersion1 == null)
            return -1 ;
        else if (strVersion2 == null)
            return 1 ;

        // Check real data
        StringTokenizer ver1Tokenizer = new StringTokenizer(strVersion1, ".");
        StringTokenizer ver2Tokenizer = new StringTokenizer(strVersion2, ".");

        try {
            while (ver1Tokenizer.hasMoreTokens() && ver2Tokenizer.hasMoreTokens())
            {
                int iVer1 = 0 ;
                int iVer2 = 0 ;

                try
                {
                    iVer1 = Integer.parseInt((String) ver1Tokenizer.nextToken());
                }
                catch (NumberFormatException nfe) { }

                try
                {
                    iVer2 = Integer.parseInt((String) ver2Tokenizer.nextToken());
                }
                catch (NumberFormatException nfe) { }

                if (iVer1 < iVer2)
                {
                    return -1;
                }
                else if (iVer1 > iVer2)
                {
                    return 1 ;
                }
            }

            // At this point, both tokenizers have been exhausted or one still
            // has some data.  In which, we should favor the one with more
            // data.
            if (ver1Tokenizer.hasMoreTokens())
                return 1 ;
            else if (ver2Tokenizer.hasMoreTokens())
                return -1 ;
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }

        return 0 ;
    }


    /**
     * Compares two version strings for equality upto some max depth.  The depth
     * indicates the max number of version numbers to compare.  For example, 1.2.3.5
     * and 1.2.3.6 would match if the depth was set to 3 (1.2.3.* vs 1.2.3.*)
     *
     * @return < 0 if strVersion1 <  strVersion2,
     *         > 0 if strVersion1 >  strVersion2,
     *         = 0 if strVersion2 == strVersion2
     */
    public static int compareVersions(String strVersion1, String strVersion2, int iMaxDepth)
    {
        // Check Nulls
        if ((strVersion1 == null) && (strVersion2 == null))
            return 0 ;
        else if (strVersion1 == null)
            return -1 ;
        else if (strVersion2 == null)
            return 1 ;

        // Check real data
        StringTokenizer ver1Tokenizer = new StringTokenizer(strVersion1, ".");
        StringTokenizer ver2Tokenizer = new StringTokenizer(strVersion2, ".");

        try {
            while (ver1Tokenizer.hasMoreTokens() && ver2Tokenizer.hasMoreTokens() &&
                    (iMaxDepth > 0))
            {
                int iVer1 = 0 ;
                int iVer2 = 0 ;

                try
                {
                    iVer1 = Integer.parseInt((String) ver1Tokenizer.nextToken());
                }
                catch (NumberFormatException nfe) { }

                try
                {
                    iVer2 = Integer.parseInt((String) ver2Tokenizer.nextToken());
                }
                catch (NumberFormatException nfe) { }

                if (iVer1 < iVer2)
                {
                    return -1;
                }
                else if (iVer1 > iVer2)
                {
                    return 1 ;
                }

                iMaxDepth-- ;
            }

            // At this point, both tokenizers have been exhausted or one still
            // has some data.  In which, we should favor the one with more
            // data.
            if (iMaxDepth > 0)
            {
                if (ver1Tokenizer.hasMoreTokens())
                    return 1 ;
                else if (ver2Tokenizer.hasMoreTokens())
                    return -1 ;
            }
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }

        return 0 ;
    }
}
