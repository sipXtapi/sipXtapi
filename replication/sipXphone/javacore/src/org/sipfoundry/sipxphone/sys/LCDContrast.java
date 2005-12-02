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


package org.sipfoundry.sipxphone.sys ;

import java.io.* ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;

import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

/**
 * LCDContrast is a quick hack that allows us to muck with the LCD settings.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LCDContrast implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** cached low range value */        
    protected static transient int m_iLow = -1 ;
    /** cached high range value */
    protected static transient int m_iHigh = -1 ;
    /** cached nomincal range value */
    protected static transient int m_iNominal = -1 ;       
    
    
    /** cached reference to our phone display component */
    protected static transient PtPhoneDisplay m_phoneDisplay = null ;
    
    /** the actual volume */
    static int m_iLevel = -1 ;
    

//////////////////////////////////////////////////////////////////////////////
// Constructions
////    
    /**
     * There is no need to create an LCD Contrast object- all of the members
     * are static.  This is provided purely for serialization
     */
    public LCDContrast()
    {
        
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    
    /** 
     *
     */
    public static void incLCDContrast()
    {
        int iHigh = getLCDContrastHigh() ;
        int iContrast = getLCDContrast() ;
                
        if (iContrast < iHigh) {
            iContrast++ ;
            setLCDContrast(iContrast) ;
        }
    }


    /** 
     *
     */
    public static void decLCDContrast()
    {
        int iLow = getLCDContrastLow() ;
        int iContrast = getLCDContrast() ;
        
        if (iContrast > iLow) {
            iContrast-- ;
            setLCDContrast(iContrast) ;
        }        
    }


    /**
     *
     */
    public static int getLCDContrastLow()
    {
        if (PingerApp.isTestbedWorld()) {
            return 1 ;
        }
        
        if (m_iLow == -1) {
            PtPhoneDisplay display = getPhoneDisplay() ;
            if (display != null) {
                m_iLow = display.getContrastLow() ;
            } else {
                System.out.println("LCDContrast::Unable to get Phone Display") ;
            }
        }
            
        return m_iLow ;
    }

    
    /**
     *
     */
    public static int getLCDContrastHigh()
    {
        if (PingerApp.isTestbedWorld()) {
            return 31 ;
        }
        
        if (m_iHigh == -1) {
            PtPhoneDisplay display = getPhoneDisplay() ;
            if (display != null) {
                m_iHigh = display.getContrastHigh() ;
            } else {
                System.out.println("LCDContrast::Unable to get Phone Display") ;
            }
        }
            
        return m_iHigh ;
    }
    

    /**
     *
     */
    public static int getLCDContrastNominal()
    {
        if (PingerApp.isTestbedWorld()) {
            return 17 ;
        }
        
        if (m_iNominal == -1) {
            PtPhoneDisplay display = getPhoneDisplay() ;
            if (display != null) {
                m_iNominal = display.getContrastNominal() ;
            } else {
                System.out.println("LCDContrast::Unable to get Phone Display") ;
            }
        }
            
        return m_iNominal ;
    }


    /**
     *
     */
    public static int getLCDContrast()
    {        
        if (PingerApp.isTestbedWorld()) {
            return m_iLevel;
        }
        
        if (m_iLevel == -1) {            
            PtPhoneDisplay display = getPhoneDisplay() ;
            if (display != null) {
                m_iLevel = display.getContrast() ;
            } else {
                System.out.println("LCDContrast::Unable to get Phone Display") ;
            }
        }
        
        return m_iLevel ;
    }


    /**
     *
     */
    public static void setLCDContrast(int iLevel)
    {
        m_iLevel = iLevel ;
                
        if (PingerApp.isTestbedWorld()) {
            return ;
        }

        PtPhoneDisplay display = getPhoneDisplay() ;
        if (display != null) {            
            display.setContrast(m_iLevel) ;
        } else {
            System.out.println("LCDContrast::Unable to get Phone Display") ;
        }        
    }
    
    public static PtPhoneDisplay getPhoneDisplay()
    {
        // only lookup the phone display if we haven't already found it.
        if (m_phoneDisplay == null) {            
            Terminal terminal = ShellApp.getInstance().getTerminal() ;
		    ComponentGroup groups[] = ((PtTerminal) terminal).getComponentGroups() ;		    
    		
		    for (int i=0; (i<groups.length) && (m_phoneDisplay == null); i++) {
		        Component components[] = groups[i].getComponents() ;
		        for (int j=0; (j<components.length) && (m_phoneDisplay == null); j++) {
		            if (components[j] instanceof PtPhoneDisplay) {
		                m_phoneDisplay = (PtPhoneDisplay) components[j] ;
		            }
		        }
            }
        }
        
        return m_phoneDisplay ;        
    }
    
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
/*
    protected static native int  JNI_getLCDContrast() ;
    protected static native int  JNI_getLCDContrastLow() ;
    protected static native int  JNI_getLCDContrastHigh() ;
    protected static native int  JNI_getLCDContrastNominal() ;
    protected static native void JNI_setLCDContrast(int iLevel) ;    
*/
}
