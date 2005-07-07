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

public class PtPhoneButton extends PtComponent implements PhoneButton
{
//////////////////////////////////////////////////////////////////////////////
// Constants
//// 
   public static final int DIAL_0       = '0' ;
   public static final int DIAL_1       = '1' ;
   public static final int DIAL_2       = '2' ;
   public static final int DIAL_3       = '3' ;
   public static final int DIAL_4       = '4' ;
   public static final int DIAL_5       = '5' ;
   public static final int DIAL_6       = '6' ;
   public static final int DIAL_7       = '7' ;
   public static final int DIAL_8       = '8' ;
   public static final int DIAL_9       = '9' ;
   public static final int DIAL_POUND   = '#' ;
   public static final int DIAL_STAR    = '*' ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
//// 
    private String m_strInfo = null ;
   
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use getComponent() from
     * the terminal to gain a reference to a component.
     */
    public PtPhoneButton(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * invoke this button.  setInfo must be called prior to pressing.
     */ 
    public void buttonPress()
    {
        JNI_buttonPress(m_lHandle) ;        
    }
    
 
    /**
     *
     */
    public PhoneLamp getAssociatedPhoneLamp()
    {
        PtPhoneLamp lamp = null ;
        
        long lHandle = JNI_getAssociatedPhoneLamp(m_lHandle) ;
        if (lHandle != 0) {
            lamp = new PtPhoneLamp(lHandle) ;            
        }
        
        return lamp ;
    }
    
    
    /**
     * set the button identifier (so we know what button to press)
     */
    public void setInfo(String strButton)
    {
        JNI_setInfo2(m_lHandle, strButton) ;
        m_strInfo = null ;
    }
   

    /**
     * what button identifier has this button been set to?
     */
    public String getInfo()
    {
//        if (m_strInfo == null)
            m_strInfo = JNI_getInfo(m_lHandle) ;            
            
        return m_strInfo ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            


//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected native final static void   JNI_buttonPress(long lHandle) ;
    protected native final static void   JNI_setInfo2(long lHandle, String strInfo) ;
    protected native final static String JNI_getInfo(long lHandle) ;
    protected native final static long   JNI_getAssociatedPhoneLamp(long lHandle) ;
}
