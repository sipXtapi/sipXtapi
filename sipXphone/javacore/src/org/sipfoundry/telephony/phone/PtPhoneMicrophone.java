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

public class PtPhoneMicrophone extends PtComponent implements PhoneMicrophone 
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
    public PtPhoneMicrophone(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
        
    public int getGain() 
    {
        return JNI_getGain(m_lHandle) ;
    }
    
    
    /**
     * Sets the microphone gain to a value between MUTE and FULL, inclusive. 
     */
    public void setGain(int iGain)
    {
        JNI_setGain(m_lHandle, iGain) ;
    }            
  
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            

        
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////

    protected native final static void JNI_setGain(long lHandle, int iGain) ;
    protected native final static int  JNI_getGain(long lHandle) ;
}
