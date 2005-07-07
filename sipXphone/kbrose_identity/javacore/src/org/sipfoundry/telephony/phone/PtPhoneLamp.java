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

package org.sipfoundry.telephony.phone ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.sipxphone.service.* ;

public class PtPhoneLamp extends PtComponent implements PhoneLamp
{
//////////////////////////////////////////////////////////////////////////////
// Constants
//// 
   
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use getComponent() from
     * the terminal to gain a reference to a component.
     */
    public PtPhoneLamp(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *
     */
    public PhoneButton getAssociatedPhoneButton()
    {
        PtPhoneButton button = null ;
        
        long lHandle = JNI_getAssociatedPhoneButton(m_lHandle) ;
        if (lHandle != 0) {
            button = new PtPhoneButton(lHandle) ;
        }               
        
        return button ;
    }
    
    
    /**
     *
     */
    public int getMode()
    {
        return JNI_getMode(m_lHandle) ;        
    }
    
    
    /**
     *
     */
    public int[] getSupportedModes() 
    {
        return JNI_getSupportedModes(m_lHandle) ;
    }
    
    
    /**
     *
     */
    public void setMode(int iMode)
    {
        JNI_setMode(m_lHandle, iMode) ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            

    public String toString()
    {
        String strState = "Unknown" ;
        int    iMode = getMode() ;
        
        switch (iMode) {
            case PhoneLamp.LAMPMODE_OFF:
                strState = "OFF" ;
                break ;
            case PhoneLamp.LAMPMODE_STEADY:
                strState = "STEADY" ;
                break ;
            case PhoneLamp.LAMPMODE_FLASH:
                strState = "FLASH" ;
                break ;
            case PhoneLamp.LAMPMODE_FLUTTER:
                strState = "FLUTTER" ;
                break ;
            case PhoneLamp.LAMPMODE_BROKENFLUTTER:
                strState = "BROKENFLUTTER" ;
                break ;
            case PhoneLamp.LAMPMODE_WINK:
                strState = "WINK" ;
                break ;
        }
        
        return "PtPhoneLamp: "+strState+" ("+iMode+")" ;
    }


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native final static long JNI_getAssociatedPhoneButton(long lHandle) ;
    protected native final static int JNI_getMode(long lHandle) ;
    protected native final static int[] JNI_getSupportedModes(long lHandle) ;
    protected native final static void JNI_setMode(long lHandle, int iMode) ;
}
