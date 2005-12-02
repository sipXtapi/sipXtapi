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

 
package org.sipfoundry.sipxphone.awt.event ;
 
import java.awt.* ;

public class PButtonEvent extends PingtelEvent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** Button Event Type: Invalid Button */
    public static final int INVALID = 0 ;
    /** Button Event Type: Button Down */
    public static final int BUTTON_DOWN    = 1 ;
    /** Button Event Type: Button Up */
    public static final int BUTTON_UP      = 2 ; 
    /** Button Event Type: Key Down */
    public static final int KEY_DOWN    = 3 ;
    /** Button Event Type: Key Up */
    public static final int KEY_UP      = 4 ; 
    /** Button Event Type: Button Repeat */
    public static final int BUTTON_REPEAT  = 5 ;


    /*
     * Button key codes
     */
     
    public static final int BID_UNDEFINED  = 0x0000 ; 

    /*
     * Button key codes: Normal Dialing Buttons
     */
    public static final int BID_0          = '0' ;
    public static final int BID_1          = '1' ;
    public static final int BID_2          = '2' ;
    public static final int BID_3          = '3' ;
    public static final int BID_4          = '4' ;
    public static final int BID_5          = '5' ;
    public static final int BID_6          = '6' ;
    public static final int BID_7          = '7' ;
    public static final int BID_8          = '8' ;
    public static final int BID_9          = '9' ;
    public static final int BID_POUND      = '#' ;
    public static final int BID_STAR       = '*' ;

    /*
     * Button key codes: Special Buttons
     */    
    public static final int BID_PINGTEL    = 0x0100 ;     
    public static final int BID_REDIAL     = 0x0101 ;
    public static final int BID_CONFERENCE = 0x0102 ;
    public static final int BID_TRANSFER   = 0x0103 ;
    public static final int BID_HOLD       = 0x0104 ;
    public static final int BID_SPEAKER    = 0x0105 ;
    public static final int BID_MUTE       = 0x0106 ;
    public static final int BID_HEADSET    = 0x0107 ;
    
    public static final int BID_VOLUME_UP  = 0x0201 ;
    public static final int BID_VOLUME_DN  = 0x0202 ;        
       

    /*
     * Button key codes: Left buttons surronding the LCD Display
     */           
    public static final int BID_L1         = 0x0300 ;
    public static final int BID_L2         = 0x0301 ;
    public static final int BID_L3         = 0x0302 ;
    public static final int BID_L4         = 0x0303 ;

    /*
     * Button key codes: Right buttons surronding the LCD Display
     */              
    public static final int BID_R1         = 0x0400 ;
    public static final int BID_R2         = 0x0401 ;
    public static final int BID_R3         = 0x0402 ;
    public static final int BID_R4         = 0x0403 ;

    /*
     * Button key codes: Bottom buttons surronding the LCD Display
     */                  
    public static final int BID_B1         = 0x0500 ;
    public static final int BID_B2         = 0x0501 ;
    public static final int BID_B3         = 0x0502 ;
    
    
    /**
     * Button ID fo the hook switch
     */
    public static final int BID_HOOKSWITCH = 0x0600 ;
    
        
    /*
     * Temporary Buttons (Scroll up and Scroll down will be replaced with
     * some sort of scroll wheel events or something)
     */
    public static final int BID_SCROLL_UP  = 0x1000 ;
    public static final int BID_SCROLL_DOWN= 0x1001 ;

    //special keys feed by keyboard
    public static final int BID_HOME        = 0x1002 ;
    public static final int BID_END         = 0x1003 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////     
    /** ID of the button */
    protected int m_iButtonID ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////         
    public PButtonEvent(Object source, int iEventType, int iButtonID) 
    {
        super(source, iEventType) ;
        
        m_iButtonID = iButtonID ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * get the button id associated with this event
     */
    public int getButtonID()
    {         
        return m_iButtonID ;
    }
            
            
    public String toString()
    {   
        return super.toString() + " button id=" + m_iButtonID ;
    }
}