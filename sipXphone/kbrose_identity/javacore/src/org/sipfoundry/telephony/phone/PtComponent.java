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

import org.sipfoundry.telephony.* ;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.phone.capabilities.* ;

import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.telephony.phone.capabilities.* ;

public class PtComponent extends PtWrappedObject implements Component
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////    
    public static final int BUTTON          = 0 ;
    public static final int DISPLAY         = 1 ;
    public static final int GRAPHIC_DISPLAY = 2 ;
    public static final int HOOKSWITCH      = 3 ;
    public static final int LAMP            = 4 ;
    public static final int MICROPHONE      = 5 ;
    public static final int RINGER          = 6 ;
    public static final int SPEAKER         = 7 ;
    public static final int TEXT_DISPLAY    = 8 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** cached copy of the component name */
    private String m_name = null ;
    /** cached copy of the component type */
    private int    m_iType = -1 ;    
    
   
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use getComponent() from
     * the terminal to gain a reference to a component.
     */
    public PtComponent(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     *
     */
    public String getName()
    {
        if (m_name == null)
            m_name = JNI_getName(m_lHandle) ;
            
        return m_name ;
    }
        
        
    /**
     *
     */
    public int getType()
    {
        if (m_iType == -1)
            m_iType = JNI_getType(m_lHandle) ;
            
        return m_iType ;
    }
    

    /**
     *
     */
    public ComponentCapabilities getCapabilities()
    {
        return new PtComponentCapabilities() ;
    }
    
    
    /**
     *
     */
    public static PtComponent createComponent(long lHandle)
    {   
        PtComponent compRC = null ;
        PtComponent comp = new PtComponent(lHandle) ;
        
        // ::UGLY:: We need to create a PtComponent just so we can look up
        // the type.  When then lookup the actual type, create correct 
        // object and toss the PtComponent away.
        //
        // ::TODO:: Create a simple handle/type lookup routine to do this.
        
        switch (comp.getType()) {
            case PtComponent.SPEAKER:
                compRC = new PtPhoneSpeaker(lHandle) ;
                break ;                
            case PtComponent.MICROPHONE:
                compRC = new PtPhoneMicrophone(lHandle) ;
                break ;
            case PtComponent.HOOKSWITCH:
                compRC = new PtPhoneHookSwitch(lHandle) ;
                break ;
            case PtComponent.BUTTON:
                compRC = new PtPhoneButton(lHandle) ;
                break ;
            case PtComponent.DISPLAY:
                compRC = new PtPhoneDisplay(lHandle) ;
                break ;
            case PtComponent.LAMP:
                compRC = new PtPhoneLamp(lHandle) ;
                break ;                
            default:
                break ;
        }
        
        comp.clearHandle() ;
        return compRC ;   
    }


    /**
     * This is called when the VM decides that the object is no longer
     * useful (no more references are found to it) and it is time to
     * delete it.  At this point, we call down to the native world and
     * free the native object
     */
    protected void finalize()
        throws Throwable    
    {
        if (m_lHandle != 0) {
            JNI_finalize(m_lHandle) ;   
        }
        m_lHandle = 0 ;
    }    
    
    
    /**
     * In rare cases, we would like to clear the handle of an object so that 
     * it is not deleted on object finalization.
     */
    protected void clearHandle()
    {
        m_lHandle = 0 ;   
    }
            
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native String  JNI_getName(long lhandle) ;
    protected static native int     JNI_getType(long lhandle) ;
    protected static native void    JNI_finalize(long lHandle) ;    
}
