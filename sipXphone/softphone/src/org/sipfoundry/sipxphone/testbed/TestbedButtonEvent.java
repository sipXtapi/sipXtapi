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

 
package org.sipfoundry.sipxphone.testbed ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

public class TestbedButtonEvent
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
    /** Button Event Type: Button Repeat */
    public static final int BUTTON_REPEAT  = 3 ;


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
    
    
    /*
     * Temporary Buttons (Scroll up and Scroll down will be replaced with
     * some sort of scroll wheel events or something)
     */
    public static final int BID_SCROLL_UP  = 0x1000 ;
    public static final int BID_SCROLL_DOWN= 0x1001 ;

  
    protected static final String COMMAND_L1 = "L1" ;
    protected static final String COMMAND_L2 = "L2" ;
    protected static final String COMMAND_L3 = "L3" ;
    protected static final String COMMAND_L4 = "L4" ;
    protected static final String COMMAND_R1 = "R1" ;
    protected static final String COMMAND_R2 = "R2" ;
    protected static final String COMMAND_R3 = "R3" ;
    protected static final String COMMAND_R4 = "R4" ;
    protected static final String COMMAND_B1 = "B1" ;
    protected static final String COMMAND_B2 = "B2" ;
    protected static final String COMMAND_B3 = "B3" ;
    
    protected static final String COMMAND_1 = "1" ;
    protected static final String COMMAND_2 = "2" ;
    protected static final String COMMAND_3 = "3" ;
    protected static final String COMMAND_4 = "4" ;
    protected static final String COMMAND_5 = "5" ;
    protected static final String COMMAND_6 = "6" ;
    protected static final String COMMAND_7 = "7" ;
    protected static final String COMMAND_8 = "8" ;
    protected static final String COMMAND_9 = "9" ;
    protected static final String COMMAND_0 = "0" ;
    protected static final String COMMAND_POUND = "POUND" ;
    protected static final String COMMAND_STAR = "STAR" ;

    protected static final String COMMAND_MORE      = "MORE" ;
    protected static final String COMMAND_HDSET     = "HDSET" ;
    protected static final String COMMAND_MUTE      = "MUTE" ;
    protected static final String COMMAND_VOL_UP    = "VOL_UP" ;
    protected static final String COMMAND_VOL_DOWN  = "VOL_DOWN" ;
    protected static final String COMMAND_XFER      = "XFER" ;
    protected static final String COMMAND_CONF      = "CONF" ;
    protected static final String COMMAND_HOLD      = "HOLD" ;
    protected static final String COMMAND_SPKR      = "SPKR" ;
    
    protected static final String COMMAND_SCROLL_UP = "SCROLL_UP" ; 
    protected static final String COMMAND_SCROLL_DOWN = "SCROLL_DOWN" ;
    
    
    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////     
    /** button event type */
    protected int m_iEventType ;
    
    /** ID of the button */
    protected int m_iButtonID ;
    
//////////////////////////////////////////////////////////////////////////////
// Construction
////         
    public TestbedButtonEvent()
    {
       m_iEventType = 0;
       m_iButtonID  = 0;
    }
    
    public TestbedButtonEvent(int iButtonID, int iEventType) 
    {
        m_iEventType = iEventType ;
        m_iButtonID = iButtonID ;
        
        postButtonEvent() ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * get the button id associated with this event
     */
    public int getButtonId()
    {         
        return m_iButtonID ;
    }

   public int getButtonId (String strButton)
    {
        if (strButton.equalsIgnoreCase(COMMAND_L1)) return PButtonEvent.BID_L1 ;
        else if (strButton.equalsIgnoreCase(COMMAND_L2)) return PButtonEvent.BID_L2 ;
        else if (strButton.equalsIgnoreCase(COMMAND_L3)) return PButtonEvent.BID_L3 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_L4)) return PButtonEvent.BID_L4 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_R1)) return PButtonEvent.BID_R1 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_R2)) return PButtonEvent.BID_R2 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_R3)) return PButtonEvent.BID_R3 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_R4)) return PButtonEvent.BID_R4 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_B1)) return PButtonEvent.BID_B1 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_B2)) return PButtonEvent.BID_B2 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_B3)) return PButtonEvent.BID_B3 ; 

        else if (strButton.equalsIgnoreCase(COMMAND_1)) return PButtonEvent.BID_1 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_2)) return PButtonEvent.BID_2 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_3)) return PButtonEvent.BID_3 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_4)) return PButtonEvent.BID_4 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_5)) return PButtonEvent.BID_5 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_6)) return PButtonEvent.BID_6 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_7)) return PButtonEvent.BID_7 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_8)) return PButtonEvent.BID_8 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_9)) return PButtonEvent.BID_9 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_0)) return PButtonEvent.BID_0 ; 
        else if (strButton.equalsIgnoreCase(COMMAND_POUND)) return PButtonEvent.BID_POUND ; 
        else if (strButton.equalsIgnoreCase(COMMAND_STAR)) return PButtonEvent.BID_STAR ; 
        
        else if (strButton.equalsIgnoreCase(COMMAND_MORE)) return PButtonEvent.BID_PINGTEL ; 
        else if (strButton.equalsIgnoreCase(COMMAND_HDSET)) return PButtonEvent.BID_HEADSET ; 
        else if (strButton.equalsIgnoreCase(COMMAND_MUTE)) return PButtonEvent.BID_MUTE ; 
        else if (strButton.equalsIgnoreCase(COMMAND_VOL_UP)) return PButtonEvent.BID_VOLUME_UP ; 
        else if (strButton.equalsIgnoreCase(COMMAND_VOL_DOWN)) return PButtonEvent.BID_VOLUME_DN ; 
        else if (strButton.equalsIgnoreCase(COMMAND_XFER)) return PButtonEvent.BID_TRANSFER ; 
        else if (strButton.equalsIgnoreCase(COMMAND_CONF)) return PButtonEvent.BID_CONFERENCE ; 
        else if (strButton.equalsIgnoreCase(COMMAND_HOLD)) return PButtonEvent.BID_HOLD ; 
        else if (strButton.equalsIgnoreCase(COMMAND_SPKR)) return PButtonEvent.BID_SPEAKER ; 
        
        else if (strButton.equalsIgnoreCase(COMMAND_SCROLL_UP)) return PButtonEvent.BID_SCROLL_UP ; 
        else if (strButton.equalsIgnoreCase(COMMAND_SCROLL_DOWN)) return PButtonEvent.BID_SCROLL_DOWN ; 
        
        return -1; 
    }
    
    
    /**
     * what type of event is this?  BUTTON_UP, BUTTON_DOWN, or BUTTON_REPEAT
     */
    public int getEventType()
    {
        return m_iEventType ;        
    }
    
    /**
     *  Post the event to the PsPhoneTask object
     */
    public void postButtonEvent()
    {
        JNI_postButtonEvent(m_iButtonID, m_iEventType);       
    }
    
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    
    //native method to post button messages to phone task
    protected native void   JNI_postButtonEvent(int m_iButtonId, int m_iEventType) ;
}
