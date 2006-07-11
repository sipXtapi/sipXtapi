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

package org.sipfoundry.telephony.callcontrol ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import javax.telephony.capabilities.* ;

import org.sipfoundry.telephony.* ;

public class PtCallControlTerminalConnection extends PtTerminalConnection implements CallControlTerminalConnection
{
    public PtCallControlTerminalConnection(long lHandle)
    {
        super(lHandle) ;
    }    


    /**
     * not implemented at time of wrapping
     */
    public int getCallControlState() 
    {
        return JNI_getCallControlState(m_lHandle) ;
    }


    /**
     * place place terminal connection on hold
     */
    public void hold()
        throws InvalidStateException,
                 MethodNotSupportedException,
                 PrivilegeViolationException,
                 ResourceUnavailableException
    {
        JNI_hold(m_lHandle) ;
    }


    /**
     * take terminal connection off hold
     */
    public void unhold()
            throws InvalidStateException,
                   MethodNotSupportedException,
                   PrivilegeViolationException,
                   ResourceUnavailableException
    {
        JNI_unhold(m_lHandle) ;
    }
        
 
    /**
     * not implemented at time of wrapping
     */
    public void join()
          throws InvalidStateException,
                 MethodNotSupportedException,
                 PrivilegeViolationException,
                 ResourceUnavailableException
    {
        JNI_join(m_lHandle) ;
    }

                 
    /**
     * not implemented at time of wrapping
     */
    public void leave()
           throws InvalidStateException,
                  MethodNotSupportedException,
                  PrivilegeViolationException,
                  ResourceUnavailableException                 
    {
        JNI_leave(m_lHandle) ;
    }
        
    
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////    
    protected static native int  JNI_getCallControlState(long lHandle) ;
    protected static native void JNI_hold(long lHandle) ;
    protected static native void JNI_unhold(long lHandle) ;
    protected static native void JNI_join(long lHandle) ;
    protected static native void JNI_leave(long lHandle) ;
}

