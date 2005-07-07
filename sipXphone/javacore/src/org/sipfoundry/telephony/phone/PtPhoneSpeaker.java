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

public class PtPhoneSpeaker extends PtComponent implements PhoneSpeaker 
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
    public PtPhoneSpeaker(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
        
    public int getVolume() 
    {
        return JNI_getVolume(m_lHandle) ;
    }
    
    
    public void setVolume(int iVolume)
    {
        JNI_setVolume(m_lHandle, iVolume) ;
    }  
    
    
    public int getDefaultVolume()
    {
        return JNI_getNominalVolume(m_lHandle) ;        
    }
  
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            

        
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////

    protected native final static void JNI_setVolume(long lHandle, int iVolume) ;
    protected native final static int  JNI_getVolume(long lHandle) ;
    protected native final static int  JNI_getNominalVolume(long lHandle) ;
}
