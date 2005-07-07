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

public class PtCallControlConnection extends PtConnection implements CallControlConnection
{
    public PtCallControlConnection(long lHandle)
    {
        super(lHandle) ;
    }    
        
    public int getCallControlState()   
    {
        return CallControlConnection.IDLE ;
    }
    
    public void accept()
        throws InvalidStateException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException    
    {
        JNI_accept(m_lHandle) ;   
    }            
    
    
    public void reject()
        throws InvalidStateException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException    
    {
        JNI_reject(m_lHandle) ;
    }
        

    public Connection redirect(String destinationAddress)
        throws InvalidStateException, InvalidPartyException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException
    {        
        long lConnection = JNI_redirect(m_lHandle, destinationAddress) ;
        PtConnection connection = PtConnection.fromNativeObject(lConnection) ;        
        
        return connection ;
    }
        

    public void addToAddress(java.lang.String additionalAddress)
        throws InvalidStateException, MethodNotSupportedException, PrivilegeViolationException, ResourceUnavailableException
    {
        throw new MethodNotSupportedException() ;    
    }
        
    public Connection park(java.lang.String destinationAddress)
        throws InvalidStateException, MethodNotSupportedException, PrivilegeViolationException, InvalidPartyException, ResourceUnavailableException        
    {
        throw new MethodNotSupportedException() ;    
    }
    
    
    /**
     * @deprecated do not expose
     *
     * This is a convenience methods that places all of the terminal 
     * connections for this connection on hold.
     */
    public void hold()
        throws PrivilegeViolationException, ResourceUnavailableException, MethodNotSupportedException, InvalidStateException
    {
        TerminalConnection tcs[] = getTerminalConnections() ;
        System.out.println("") ;
        for (int i=0; i<tcs.length; i++) {
            System.out.println("PLACE TC ON HOLD: " + tcs[i]) ;
            ((PtCallControlTerminalConnection) tcs[i]).hold() ;
        }
        System.out.println("") ;
    }
    
    /**
     * @deprecated do not expose
     *
     * This is a convenience methods that removes all of the terminal 
     * connections for this connection from hold.
     */
    public void unhold()
        throws PrivilegeViolationException, ResourceUnavailableException, MethodNotSupportedException, InvalidStateException
    {
        TerminalConnection tcs[] = getTerminalConnections() ;
        for (int i=0; i<tcs.length; i++) {
            ((PtCallControlTerminalConnection) tcs[i]).unhold() ;
        }
    }
    
    
    
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////    
    protected static native void JNI_accept(long lHandle) ;
    protected static native void JNI_reject(long lHandle) ;
    protected static native long JNI_redirect(long lHandle, String destinationAddress) ;
}

