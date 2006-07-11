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

/*
 * CheckTextfieldUtils is a collection of methods to validate the 
 * contents of any user edited textfield. 
 */
public class CheckTextfieldUtils
{
   /**
    * Validate the textfield contains a valid IP Address.
    * 
    * @param strParam - parameter of format xxx.xxx.xxx.xxx
    * @param Returns 0 - OK, 
    *                1 - wrong no. periods, 
    *                2 - values not between 0 and 255
    *                3 - value is null
    *                4 - 0.0.0.0
    */ 
    public static int checkIpAddress(String strParam)
    {
        /* Rules:
            - IP address has 3 periods
            - each value is 0-255
        */
        int iResponse = 0 ;         // Response code - 0 is OK, else 1 or 2
        String strSeparator = ".";  // separator between values
        int iCurrPos = 0 ;          // position within string
        int iSeparatorPos  = 0 ;    // position of separator within string
        int iSeparatorCount = 0 ;   // count of how many separators we have
        int iTestValue = 0 ;        // value of int - should be between 0 and 255
        final int iNumSeparators = 3;   // each address has 3 separators
        String strParamCopy = strParam ;
    
        // check for nulls, spaces
        if ((strParamCopy==null)||(strParamCopy=="")||(strParamCopy.indexOf(" ")!=-1)) {
            return 3 ;
        }
        
        // check for 3 periods
        for (int i = 0 ; i<strParamCopy.length(); i++) {
            if (strParamCopy.charAt(i) == strSeparator.charAt(0)) {
                iSeparatorCount++ ;
                System.out.println("Separator: " + iSeparatorCount) ;
            }                
        }
        
        if (iSeparatorCount != iNumSeparators) {
            // not right number of periods
            iResponse = 1 ;
            return iResponse;
        }            
            
        // Check each value is between 0 and 255
        
        for (int i = 0 ; i < iNumSeparators ; i++ ) {
            iSeparatorPos = strParamCopy.indexOf(strSeparator, iCurrPos) ;
            if (iSeparatorPos != -1)
            {
                iTestValue = new Integer(strParamCopy.substring(iCurrPos, iSeparatorPos)).intValue() ;
                if ((iTestValue < 0) || (iTestValue >255))
                    iResponse = 2 ;
                strParamCopy = strParamCopy.substring(iSeparatorPos + 1, strParamCopy.length()) ;
            } 
        }
        // after last separator
        try {
            iTestValue = new Integer(strParamCopy.substring(iCurrPos, strParamCopy.length() )).intValue() ;
            if ((iTestValue < 0) || (iTestValue >255))
                        iResponse = 2 ;
        } catch (NumberFormatException e) {
            // No int after last period
            iResponse = 2 ;
        }
                   
        if (strParam.equals("0.0.0.0")) {
            return 4 ;
        }
        
       return iResponse ;
        
    }
    
    public static int checkDomainName(String strDomain)
    {
        return 0 ;   
    }
    
    
    /**
     * determine whether the specified port is valid or not.  A valid port
     * is non-null and a valid between 1 and 65535.
     */
    public static boolean validPort(String strPort)
    {
        boolean bValid = false ;
                      
        if ((strPort != null) && (strPort.length() > 0)) {
            int iPort = 0 ;  
            try {
                iPort = Integer.parseInt(strPort) ;
                if ((iPort > 0) && (iPort <= 65535))
                    bValid = true ;
                
            } catch (Exception e) { /* burp */ }
        }        
        return bValid ;                                
    }      
}
