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
import javax.telephony.callcontrol.capabilities.* ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.capabilities.* ;

import org.sipfoundry.sipxphone.service.* ;

public class PtCallControlAddress extends PtAddress implements CallControlAddress
{
    /**
     * creates a PtCallControlAddress using the given handle.
     */
    public PtCallControlAddress(long lHandle)
    {
        super(lHandle) ;
    }


    /**
     * Sets the forwarding characteristics for this Address.
     */
    public void setForwarding(CallControlForwarding[] instructions)
        throws MethodNotSupportedException, InvalidStateException, InvalidArgumentException
    {
        int iLength = instructions.length ;

        // 1) Convert to our adapters and pull out PTAPI handles
        PtCallControlForwardingAdapter adapter ;
        long lInstructions[] = new long[iLength] ;
        for (int i=0; i<iLength; i++) {
            adapter = PtCallControlForwardingAdapter
                .fromCallControlForwarding((PtCallControlForwarding)instructions[i]) ;
            lInstructions[i] = adapter.getHandle() ;
        }
        JNI_setForwarding(getHandle(), lInstructions) ;
    }


    /**
     * Returns an array of forwarding instructions currently effective for this
     * Address. If there are no effective forwarding instructions, this method
     * returns null.
     */
    public CallControlForwarding[] getForwarding()
        throws MethodNotSupportedException
    {
        CallControlForwarding rc[] = null ;
        long lHandles[] = JNI_getForwarding(getHandle()) ;
        if (lHandles != null) {
            int iLength = lHandles.length ;

            rc = new CallControlForwarding[iLength] ;
            for (int i=0; i<iLength; i++) {
                PtCallControlForwardingAdapter adapter =
                    new PtCallControlForwardingAdapter(lHandles[i]) ;
                rc[i] = adapter.toCallControlForwarding() ;
            }
        }

        return rc ;
    }


    /**
     *Cancels all of the forwarding instructions on this Address.
     */
    public void cancelForwarding()
        throws MethodNotSupportedException, InvalidStateException
    {
        JNI_cancelForwarding(getHandle()) ;
    }


    /**
     *
     */
    public boolean getDoNotDisturb()
        throws MethodNotSupportedException
    {
        return JNI_getDoNotDisturb(getHandle()) ;
    }

    /**
     *
     */
    public void setDoNotDisturb(boolean enable)
        throws MethodNotSupportedException, InvalidStateException
    {
        JNI_setDoNotDisturb(getHandle(), enable) ;
    }


    /**
     *
     */
    public boolean getMessageWaiting()
        throws MethodNotSupportedException
    {
        return JNI_getMessageWaiting(getHandle()) ;
    }


    /**
     *
     */
    public void setMessageWaiting(boolean enable)
        throws MethodNotSupportedException, InvalidStateException
    {
        JNI_setMessageWaiting(getHandle(), enable) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native void    JNI_setForwarding(long lHandle, long instructions[]) ;
    protected static native long[]  JNI_getForwarding(long lHandle) ;
    protected static native void    JNI_cancelForwarding(long lHandle) ;
    protected static native boolean JNI_getDoNotDisturb(long lHandle) ;
    protected static native void    JNI_setDoNotDisturb(long lHandle, boolean bEnable) ;
    protected static native boolean JNI_getMessageWaiting(long lHandle) ;
    protected static native void    JNI_setMessageWaiting(long lHandle, boolean bEnable) ;
}

