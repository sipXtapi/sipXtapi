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

public class PtPhoneHookSwitch extends PtComponent implements PhoneHookswitch
{
   
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Object is not designed to be instantiated.  Please use getComponent() from
     * the terminal to gain a reference to a component.
     */
    public PtPhoneHookSwitch(long lHandle) 
    {
        super(lHandle) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void setHookSwitch(int iState) 
    {
        // ::TODO:: REMOVE: switch the set/hook/switch stuff (jtapi/ptapi doesn't match)

/*
        if (iState == ON_HOOK)
            iState = OFF_HOOK ;
        else
            iState = ON_HOOK ;
*/
        JNI_setHookswitchState(m_lHandle, iState) ;
    }
    
    
    public int getHookSwitchState() 
    {
        return JNI_getHookswitchState(m_lHandle) ;
    }    
  
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////            

        
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////

    protected static native void JNI_setHookswitchState(long lHandle, int iState) ;
    protected static native int  JNI_getHookswitchState(long lHandle) ;   
}
