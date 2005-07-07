/*
 * $Id$
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

import java.util.Vector ;
import java.awt.FontMetrics ;
import java.util.StringTokenizer;

/**
 * TextUtils is a collection of text utilities used by Pingtel Corporation.
 * This should not be exp1osed to outside developers as they are only quick
 * convenience methods and shouldn't stand alone.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class TextUtils
{

    /**
     * Searches for the last character (closest to end of string) delimiter
     * that exists in strSource.
     *
     * @param strSource source file to search
     * @param iPosition starting position
     * @param strDelimiter string of characters used as delimitation points
     *
     * @return index of character closest to the end of the string
     */
    public static int lastCharacter(String strSource, int iPosition, String strDelimiters)
    {
        int iRC = -1 ;
        int iDelimiters = strDelimiters.length() ;
        int iFoundPosition ;

        // Calculate position of last character for each char...
        for (int i=0; i<iDelimiters; i++) {
            iFoundPosition = strSource.lastIndexOf(strDelimiters.charAt(i), iPosition) ;
            if (iFoundPosition > iRC) {
                iRC = iFoundPosition ;
            }
        }
        return iRC ;
    }


    /**
     * Searches for the first character (closest to start of string) delimiter
     * that exists in strSource.
     *
     * @param strSource source string to search
     * @param iPosition starting position
     * @param strDelimiter string of characters used as delimitation points
     *
     * @return index of delimiter closest to the start of the string or -1 if
     *         not found
     */
    public static int firstCharacter(String strSource, int iPosition, String strDelimiters)
    {
        int     iRC = -1;
        if( strSource == null )
            return iRC;

        int     istrLen = strSource.length() ;
        int     iNumDelimiters = strDelimiters.length() ;

        // Calculate position of first character for each char...
        for (int i = iPosition; i < istrLen && iRC == -1;i++)
        {
            for (int j =0; j<iNumDelimiters;  j++)
            {
                if (strSource.charAt(i) == strDelimiters.charAt(j))
                {
                    iRC = i;
                    break;
                }
            }
        }
        return iRC ;
    }

    /**
     * Searches for the last delimiter within the width specified.
     *
     * @param strSource source string to search
     * @param strDelimiter string of characters used as delimitation points
     * @param fm FontMetric of font used to display text
     * @param iWidth width of screen area
     * @return index of the last delimiter within the width specified  or -1 if
     *         not found
     * @author Pradeep Paudyal
     */
    public static int lastDelimiterToWidth
        ( String strSource, String strDelimiters, FontMetrics fm, int iWidth){
        /** Note if you ever have to add some more logic to this code
          * or do some refactoring or whatever,
          * DO NOT DO fm.stringWidth(str) where str could be
          * a LONG LONG string. It takes a LOOOONNNNGGGG time for
          * FontMetrics to calculate width of a long string.
          */
        int iRC = -1;
        int iIndexToBegin = 0;
        int iWrapPoint = -1;

        if ((iWidth <= 0) || (strSource == null))
            return iRC;
        int iMaxCharsThatCanFit = findMaxCharsLengthPossible(fm, iWidth);
        if( strSource.length() <= iMaxCharsThatCanFit ){
            if( fm.stringWidth(strSource) > iWidth ) {
                iWrapPoint = firstCharacter
                    (strSource, iIndexToBegin, strDelimiters) ;
            }
        }else{
            iWrapPoint = firstCharacter
                (strSource, iIndexToBegin, strDelimiters) ;
        }
        while( iWrapPoint != -1 ){
            if ((iWrapPoint+1) < strSource.length()){

                String strBeforeWrapPoint = "";
                if (strSource.charAt(iWrapPoint) == ' ')
                    strBeforeWrapPoint=
                      strSource.substring(iIndexToBegin, iWrapPoint) ;
                else
                    strBeforeWrapPoint=
                      strSource.substring(iIndexToBegin, iWrapPoint+1) ;
                if( fm.stringWidth(strBeforeWrapPoint) < iWidth ) {
                    iRC = iWrapPoint;
                    iWrapPoint = firstCharacter
                        (strSource, iWrapPoint+1, strDelimiters) ;

                }else{
                    break;
                }
            }else{
                break;
            }
        }
        return iRC;
    }

    /**
     * Find the number of smallest width chars that can
     * fit in the given width with the provided
     * FontMetrics. This is useful to approximate
     * if a sub-string is longer that the
     * width without calculating the font metrics width
     * of the entire string.
     *
     */
    protected static int findMaxCharsLengthPossible
        (FontMetrics fm , int iWidth){

        int iRC = 0;
        int iSmallestCharWidth = fm.stringWidth(" ");
        if( iWidth > 0 ){
            iRC = ((iWidth/iSmallestCharWidth))+5;
        }
        //added  5 justto give a double size which shouldn't be
        //too bad
        return iRC;
    }


    /**
     * Expands a single string to a vector of strings by looking for '\n's.
     *
     * @param strSource source string to expand into multiple strings
     *
     * @return vector of strings resulting from the expansion
     */
    public static Vector expandNewlines(String strSource)
    {
        Vector vRC = new Vector() ;
        String strCandidate = strSource ;
        int iNextIndex = strCandidate.indexOf('\n') ;
        while (iNextIndex != -1) {
            vRC.addElement(strCandidate.substring(0, iNextIndex)) ;
            strCandidate = strCandidate.substring(iNextIndex+1) ;
            iNextIndex = strCandidate.indexOf('\n') ;
        }
        vRC.addElement(strCandidate) ;

        return vRC ;
    }


    /**
     * Removes the leading zeros if any exists by converting the String to an
     * Integer. If its an invalid number, it doesn't propagate the exception,
     * just nicely catches it, prints the stack trace and returns the original
     * string.
     */
    public static String removeLeadingZerosifInt(String strSource){
        String strRet = strSource;
        try{
            Integer integer = new Integer( strSource );
            strRet = integer.toString();
        }catch( Exception e ){
            SysLog.log(e);
        }
        return strRet;
    }

    /**
     * Look through the string and wrap it using the passed font metrics and
     * width.  The wrapping will search for instances of any character found
     * in strDelimiters and wrapped using that.
     *
     * @param source source string what should be wrapped
     * @param fm FontMetric of font used to display text
     * @param iWidth width of screen area
     * @param strDelimiters delimiters used
     *
     * @return vector of text lines
     * @see org.sipfoundry.util.TextUtils#wrapText(String strSource, FontMetrics fm, int iWidth,
         String strDelimiters, int iDesiredNumberOfLines )
     */
    public static Vector wrapText
        (String strSource, FontMetrics fm, int iWidth, String strDelimiters)
    {
        return wrapText( strSource, fm, iWidth, strDelimiters, -1 );
    }



    /**
     * Look through the string and wrap it using the passed font metrics and
     * width.  If the number of actual lines
     * exceeds the desired number of lines, "..." will be appended to
     * the last desired line.
     *
     *<p> The logic applied here for wrapping is as following:<br>
     * <li>1) First check to see if it is a a continuous string with
     * no delimiters within the width of the screen.
     * In that case, break the string at the width
     * of the screen, do hard-wrapping( prepend the next string with
     * the remaming portion of this string that didn't fit in the screen)
     * and continue handling the remaing of the string similar way( using
     * one of the three conditions mentioned ).
     * In case 1, if the string fits within the width in the first try,
     * the loop exist after that.
     *
     *<li>2) Break the string at the last delimiter if there is one before
     * the screen-width portion of the string.
     *
     * <li>3) If the number of desired lines is already reached, stop handling any
     * more remaining string and append "..."( even if existing chars have
     * to be replaced) to the last string added to the Vector.
     *
     * <p>Implementation note: The wrapped lines are added to a non-growing
     * vector whose size is determined by the number of desired lines.
     * The class NonGrowingVector has a method to tell if the limit was
     * tried to be exceeded( i.e. if addElement() was called after the
     * limit was reached).
     *
     * @param strSource source string what should be wrapped
     * @param fm FontMetrics of font used to display text
     * @param iWidth width of screen area
     * @param strDelimiters delimiters used
     * @param iDesiredNumberOfLines desired number of lines, -1 if none
     * @return vector of text lines
     * @see org.sipfoundry.util.NonGrowingVector
     */
    public static Vector wrapText
        (String strSource, FontMetrics fm, int iWidth,
         String strDelimiters, int iDesiredNumberOfLines )
    {
       /** Note: if you ever have to add some more logic to this code
        * or do some refactoring or whatever,
        * DO NOT DO fm.stringWidth(str) where str could be
        * a LONG LONG string. It takes a LOOOONNNNGGGG time for
        * FontMetrics to calculate width of a long string.
        */
        NonGrowingVector vRC= new NonGrowingVector(iDesiredNumberOfLines);

        if (iWidth <= 0)
            return vRC.getVector();
        int    iWrapPoint ;

        Vector vExpandedText = TextUtils.expandNewlines(strSource) ;
        int expandedTextSize = vExpandedText.size();
        for (int i=0; i< expandedTextSize; i++) {
            String strCandidate = (String) vExpandedText.elementAt(i) ;

            // Simple case does the existing string fit within our space?
            if (fm.stringWidth(strCandidate) < iWidth){
                vRC.addElement(strCandidate) ;
                strCandidate = null;
            }

            while (strCandidate != null) {
                iWrapPoint = lastDelimiterToWidth
                    (strCandidate, strDelimiters, fm, iWidth);

                //taking care of the case when the string is just one big
                //continuous word
                // if the string fits in the width, hardWrapStrArray[1]
                // will be null.
                if( iWrapPoint == -1 ){
                    String[] hardWrapStrArray =
                        breakStringForHardWrap(strCandidate, fm , iWidth );
                    if( hardWrapStrArray[0] != null ){
                        vRC.addElement( hardWrapStrArray[0]);
                    }
                    strCandidate = hardWrapStrArray[1];

                }else if( iWrapPoint >= 0 ){

                    String strBeforeWrapPoint=
                            strCandidate.substring(0, iWrapPoint+1) ;

                    String strAfterWrapPoint = null;
                    if ( (iWrapPoint+1) < strCandidate.length()){
                        strAfterWrapPoint=strCandidate.substring(iWrapPoint+1);
                    }

                    vRC.addElement( strBeforeWrapPoint );
                    //now resetting iWrapPoint and strCandidate
                    iWrapPoint = -1;
                    strCandidate = null;

                    if (strAfterWrapPoint != null){
                        strCandidate = strAfterWrapPoint;
                    }

                }
                if( vRC.wasSizeTriedToBeExceeded() ){
                    break; //break from the while loop
                }
            }//end of while loop

            if( vRC.wasSizeTriedToBeExceeded() ){
                break; //break from the for loop
            }
        }//end of for loop

        if( vRC.wasSizeTriedToBeExceeded() ){
            //add ... to the last line
            int iLastElementIndex = vRC.size()-1;
            StringBuffer buffer = new StringBuffer();
            String strLastLine = (String)(vRC.elementAt(iLastElementIndex));
            buffer.append( strLastLine.substring(0, strLastLine.length()-2));
            buffer.append("...");
            vRC.removeElementAt(iLastElementIndex);
            vRC.addElement(buffer.toString());
        }

        return vRC.getVector() ;
    }

    /*
    public static final int PREPENDING_DOTS_WRAP = 1;
    public static final int APPENDING_DOTS_WRAP  = 2;
    public static final int HARD_WRAP            = 3;
    public static final int TRUNCATING_WRAP      = 4;
    */

    /**
     * breaks the string into two strings , the first one that
     * fits in the width specified and the second one that is the remaining
     * portion of the original string.
     *
     * @param str source string to search
     * @param fm FontMetric of font used to display text
     * @param iWidth width of screen area
     * @return array of two strings which contains two strings, the first one
     * is the one that is broken at the iWidth and the remaining one is
     * the second one. If there is no remaining srting portion left, the second
     * element of the array will be null.
     *
     * @author Pradeep Paudyal
     */
    public static String[] breakStringForHardWrap
        (String str, FontMetrics fm, int iWidth){

        String[] strArray = new String[2];
        int iTruncate =
            findTruncatingLength( fm, str, iWidth );
        if( iTruncate != -1 ){
            strArray[0]= str.substring(0, iTruncate+1);
            if(( iTruncate+1) < str.length() )
                strArray[1]=str.substring(iTruncate+1) ;
        }else{
            strArray[0] = str;
        }
        return strArray;
    }

    /**
     * Finds the length of the string where the given string should
     * be truncated to fit in the given screen width. It finds the truncating
     * point using binary search. Since there may be no exact match
     * of the "width" of truncated string to the given width,
     * it gets the low value when the search ends and
     * does one more checking to see if the resultant "width" is  actually
     * not bigger than iWidth.
     *
     * @param fm FontMetrics used for displaying the string
     * @param strParam String which needs to fit in the iWidth
     * @param iWidth the width in which the string needs to fit
     * @return the length of the string where it needs to be truncated to
     *         fit in iWidth. -1 if strParam does not need to be truncated.
     */
    public static int findTruncatingLength(FontMetrics fm, String strParam,
            int iWidth)
    {
        int iRet = -1;

        if (iWidth <= 0)
            return iRet;

        if(  fm.stringWidth(strParam) > iWidth ){

            int iLow = 0;
            int iHigh = (strParam.length())-1;
            while( iLow <= iHigh ){
                int iMid = (iLow + iHigh) / 2;
                String strMiddle = strParam.substring(0, iMid+1 );
                if( fm.stringWidth(strMiddle) > iWidth ) {
                    iHigh = iMid -1;
                }else  if( fm.stringWidth(strMiddle) < iWidth ){
                    iLow = iMid + 1;
                }else{
                    return iMid;
                }
            }
            String strMiddle = strParam.substring(0, iLow+1 );
            while(  fm.stringWidth(strMiddle) > iWidth) {
                if( (strMiddle.length() > (iLow-1)) && ((iLow-1) > 0) ){
                    iLow = iLow -1;
                    strMiddle = strParam.substring(0, iLow+1 );
                }else{
                    break;
                }
            }
            iRet = iLow;
        }
        return iRet;
    }

    /**
     * This gives you back the chopped string with "..." being
     * appended if the width occupied by the string using the given
     * font metrics is bigger than iWidth.
     * <b> This does not break the string into two lines.
     * It just gives you back a string that will fit in ONE line
     * in the given iWidth. </b>
     * @param strParam string that is examined to get the string
     *                 that will fit in the given iWidth
     *
     * @param fm      FontMetrics that is used to display the string.
     *
     * @param iWidth  the width where the string needs to fit.
     *            string to denote that it didn't fit in iWidth.
     * @see org.sipfoundry.util.TextUtils#truncateToWidth(  String strParam, FontMetrics fm,
     *                              int iWidth, String strToAppend )
     */

    public static String truncateToWidth
        ( String strParam, FontMetrics fm, int iWidth){
        return truncateToWidth( strParam, fm, iWidth, "..." );
    }

    /**
     * This gives you back the chopped string with "strToAppend" being
     * appended if the width occupied by the string using the given
     * font metrics is bigger than the display width.
     * <b> This does not break the string into two lines.
     * It just gives you back a string that will fit in ONE line
     * in the given iWidth. </b>
     * @param strParam     string that is examined to get the string
     *                     that will fit in the given iWidth
     *
     * @param fm           FontMetrics that is used to display the string.
     *
     * @param iWidth       the width where the string needs to fit.
     *
     * @param strToAppend  string that is appeneded to the chopped
     *            string to denote that it didn't fit in the screen.
     *            e.g  "..." .
     */
    public static String truncateToWidth
        ( String strParam, FontMetrics fm, int iWidth, String strToAppend ){
        String strRet = strParam;
        int iTruncate = findTruncatingLength( fm, strParam, iWidth );
        if( iTruncate != -1 ){
            StringBuffer buffer = new StringBuffer();
            if( ( (iTruncate) > 0  ) &&
                ( (strRet.length()) > (iTruncate) ) ){
                strRet = strRet.substring(0, iTruncate+1);
            }

            int iAppendWidth = fm.stringWidth(strToAppend);
            //width of the chars  from the end of the string
            int iLastCharsWidth = 0;

            //counter for the position of character from the end of the string.
            int i = strRet.length() - 1;
            while( iLastCharsWidth <= iAppendWidth ){
                if( i > 1 )
                    iLastCharsWidth += fm.charWidth(strRet.charAt(i--));
                else{
                    break;
                }

            }
            if( ((i+1) > 0) &&
                (strRet.length() > (i+1)) ){
                strRet = strRet.substring(0, (i+1));
            }

            buffer.append( strRet );
            buffer.append(strToAppend);
            strRet = buffer.toString();
        }
        return strRet;
    }

    /**
    * parses a string like \"test case\" 5 test test
    * returns number of tokens
    */

    public static int parseTokens(String parseLine, Vector v)
    {
        String separator = " ";
        StringTokenizer tokenizer = new StringTokenizer(parseLine, separator, true);
        String token;
        int tokenCount = 0;

        while (tokenizer.hasMoreTokens())
        {
            token = tokenizer.nextToken();

            // ignore seperator if not in quotes
            if (!token.equals(" "))
            {
                // if we find a " or ' group all the enclosed tokens together
                if (token.startsWith("\""))
                {
                    while (!token.endsWith("\""))
                    {
                        token += tokenizer.nextToken();
                    }
                }
                else if (token.startsWith("'"))
                {
                    while (!token.endsWith("'"))
                    {
                        token += tokenizer.nextToken();
                    }
                }
                v.addElement(token);
                tokenCount++;
            }
        }

        return tokenCount;
     }

    /**
     * Removes all whitespace (space, tab, cr, lf) from the supplied string.
     *
     * @param strSource The source string that will be stripped of whitespace.
     *
     * @return The string cleaned of whitespace.  If the supplied source string
     *         is null, an empty string is returned.
     */
    public static String removeAllWhitespace(String strSource)
    {
        StringBuffer results = new StringBuffer() ;

        if (strSource != null)
        {
            int iLength = strSource.length() ;
            for (int i=0; i<iLength; i++)
            {
                /*
                 * NOTE: We are not using Charactyer.isWhitespace() because
                 *       that methods has a more liberal view of whitespace.
                 *       In particular, we do not want to strip file
                 *       separators.  This method should also be quicker
                 *       without the function overhead.
                 */
                char ch = strSource.charAt(i) ;
                if ((ch != ' ') && (ch != '\t') && (ch != '\r') && (ch != '\n'))
                {
                    results.append(ch) ;
                }
            }
        }
        return results.toString() ;
    }


    /**
     * Determines if the supplied character is dialable.  A dialable digit in
     * something within the set ('0'..'9', '#', '*').
     *
     * @param ch The character that is in question
     *
     * @return boolean true if the supplied character is dialable, otherwise
     *         false.
     */
    public static final boolean isDialableDigit(char ch)
    {
        boolean bDialable = false ;

        if (((ch >= '0') && (ch <= '9')) || (ch == '#') || (ch == '*') || 
            (ch == PhoneNumberParser.CHAR_WAIT) || (ch == PhoneNumberParser.CHAR_PAUSE))
        {
            bDialable = true ;
        }
        return bDialable ;
    }

   /**
     * checks if a string is null or just consists of space
     * @param str The string to check if it is null or just consists of space.
     * @return boolean true if the string is null or just consists of space,
     * false otherwise.
     */
    public static  boolean isNullOrSpace(String str ){
        boolean bRes = false;
        if(( str == null ) || (str.trim().length() == 0))
            bRes = true;
        return bRes;
    }
//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * utility method to print debug messages making it distinct from other
     * debug messages
     */
    public static void debug(String str ){
        System.out.println("******************************");
        System.out.println(str);
        System.out.println("******************************");
    }



}








