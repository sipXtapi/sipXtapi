/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/PhoneNumberScrubber.java#2 $
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

/**
 * Utility class that provides an implementation for scrubbing the various
 * decorators from phone numbers.  This includes all of the non-dialable
 * digits ('(', ')', '-', etc).
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PhoneNumberScrubber
{

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * This is utility class and should never be instantiated.  Thus, the
     * constructor has been made private.
     */
    private PhoneNumberScrubber()
    {
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Removes all of the decorations that are commonly inserted into phone
     * numbers.  This includes all of the whitespace, dashes, plus signs,
     * periods, etc.  This routinue actually removes ALL non-dialable digits
     * and adds logic for removing extentions.
     * <br>
     * All text preceeding the first instance of 'x' is considered an extension
     * and disgarded with one exception: all 'x's are ignored until a some
     * dialing digits are collected.  This allows a string such as "x123" or
     * "ext123" to resolve to "123".
     *
     * @param strSource The source string that will be scrubbed of non-dialable
     *        digits.
     *
     * @return The scrubbed version of strSource where all of the non-dialable
     *         digits have been removed.
     */
    public static String scrub(String strSource)
    {
        StringBuffer results = new StringBuffer() ;

        if (strSource != null)
        {
            int iLength = strSource.length() ;
            for (int i=0; i<iLength; i++)
            {
                char ch = strSource.charAt(i) ;

                // If a number, #, or *, append to results
                if (Character.isDigit(ch) || (ch == '*') || (ch == '#') || 
                    (ch == PhoneNumberParser.CHAR_WAIT) || 
                    (ch == PhoneNumberParser.CHAR_PAUSE))
                {
                    results.append(ch) ;
                }
                // If an 'X' or 'x', this could be the start of an extention
                // in which case, we should kick out and drop digits.  However,
                // if we receive an 'x' without any OTHER digits first, simply
                // disgard it and continue.
                else if ((ch == 'x') || (ch == 'X'))
                {
                    if (results.length() > 0)
                        break ;
                }
            }
        }
        return results.toString() ;
    }


    /**
     * Determines if the specified string appears to be a phone number. A
     * string is considered a phone number if it matches any of the following
     * constraints:
     *
     * <ul>
     *   <li> +# - Begins with a '+', followed immediately by a dialable digit,
     *             ignoring whitespace</li>
     *   <li> #  - Begins with a dialing digits, ignoring whitespace</li>
     *   <li> (# - Begins with an '(', followed immediately by a dialing digit,
     *             ignoring whitespace</li>
     *   <li> x# - Beings with an 'x', followed immediately by a dialing digit,
     *             ignoring whitespace</li>
     *  </ul>
     *
     * A 'Dialing Digit' in anything within ('0'..'9','#','*', 'w', 'p')
     */
    public static boolean looksLikePhoneNumber(String strSource)
    {
        boolean bPhoneNumber = false ;

        if (strSource != null)
        {
            // To ease the process, collapse all of the whitespace from the
            // source string.
            String strCleanedSource = TextUtils.removeAllWhitespace(strSource) ;
            int iLength = strCleanedSource.length() ;
            if (iLength >= 1) {
                char chFirst = strCleanedSource.charAt(0) ;

                // Look for items which require a second look
                if ((chFirst == '+') || (chFirst == '(') || (chFirst == 'x')
                        || (chFirst == 'X'))
                {
                    // Get the second digit, and see if it is dialable
                    if (iLength >= 2)
                    {
                        char chSecond = strCleanedSource.charAt(1) ;

                        if (TextUtils.isDialableDigit(chSecond))
                            bPhoneNumber = true ;
                    }
                }
                // Otherwise, if the first digit is dialable, assume it is a
                // phone number
                else if (TextUtils.isDialableDigit(chFirst))
                {
                    bPhoneNumber = true ;
                }
            }
        }

        return bPhoneNumber ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

}
