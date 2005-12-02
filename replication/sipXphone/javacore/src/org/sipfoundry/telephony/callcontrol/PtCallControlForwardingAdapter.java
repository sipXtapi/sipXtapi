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

/**
 * Provides an adapter between the JTAPI CallControlForwarding OBJECT and
 * our PTAPI object.  This would have been a lot easier if the
 * CallControlForwarding class was an interface...
 *
 * This is an internal object and should never be exposed to outside
 * developers.
 */
public class PtCallControlForwardingAdapter extends PtWrappedObject
{


    /**
     * constructs a PtCallControlForwardingAdapter object using the given  handle.
     */
    public PtCallControlForwardingAdapter(long lHandle)
    {
        super(lHandle) ;
    }


    /**
     * This constructor takes the address to apply the forwarding instruction
     * and timeout in seconds if the forwarding type is FORWARD_ON_NOANSWER.
     * @param destAddress  destination address for the call forwarding operation
     * @param iNoAnswerTimeout timeout in seconds if the forwarding type
     *                           is FORWARD_ON_NOANSWER
     */
    public PtCallControlForwardingAdapter(String destAddress,
                                        int iNoAnswertimeout)

    {
        super(JNI_constructType
            (destAddress, CallControlForwarding.FORWARD_UNCONDITIONALLY, iNoAnswertimeout)) ;
    }


   /**
     * This constructor takes the address to apply this forwarding instruction
     *  ,the type of forwarding for all incoming calls and timeout in seconds if
     *  the forwarding type is FORWARD_ON_NOANSWER.
     * @param destAddress  destination address for the call forwarding operation
     * @param type  the type of the forwarding operation
     * @param iNoAnswerTimeout timeout in seconds if the forwarding type
     *                           is FORWARD_ON_NOANSWER
     */
    public PtCallControlForwardingAdapter(String destAddress,
                                          int type,
                                          int iNoAnswertimeout)
    {
        super(JNI_constructType(destAddress, type, iNoAnswertimeout)) ;
    }


   /**
   * This constructor takes the address to apply the forwarding instruction,
   * the type of forwarding desired for this address, a boolean flag
   * indicating whether this instruction should apply to internal (true) or
   * external (false) calls, and timeout in seconds if
   * the forwarding type is FORWARD_ON_NOANSWER.
   * @param destAddress  destination address for the call forwarding operation
   * @param type  the type of the forwarding operation
   * @param internalCalls  flag for internal call forwarding
   * @param iNoAnswerTimeout timeout in seconds if the forwarding type
   *                           is FORWARD_ON_NOANSWER
   */
    public PtCallControlForwardingAdapter(String destAddress,
                                          int type,
                                          boolean internalCalls,
                                          int iNoAnswertimeout)
    {
        super(JNI_constructInternalExternalType
            (destAddress, type, internalCalls, iNoAnswertimeout)) ;
    }


   /**
    * This constructor takes an address to apply the forwarding instruction for
    * a specific incoming telephone call, identified by a string address.
    * It also takes the type of forwarding desired for this specific address.
    * @param destAddress  destination address for the call forwarding operation
    * @param type  the type of the forwarding operation
    * @param caller  the address of the incoming caller
    * @param iNoAnswerTimeout timeout in seconds if the forwarding type
    *                           is FORWARD_ON_NOANSWER.
    */
    public PtCallControlForwardingAdapter(String destAddress,
                                          int type,
                                          String caller, int iNoAnswertimeout)
    {
        super(JNI_constructSpecific
            (destAddress, type, caller, iNoAnswertimeout)) ;
    }


    /**
     * Converts a CallControlForward object into this adapter / PTAPI object
     */
    public static PtCallControlForwardingAdapter fromCallControlForwarding
        (PtCallControlForwarding forwarding)
    {
        PtCallControlForwardingAdapter adapter = null ;

        switch (forwarding.getFilter()) {
            case CallControlForwarding.ALL_CALLS:
//                System.out.println("fromCallControlForwarding: ALL_CALLS " + forwarding.getDestinationAddress()) ;
                adapter = new PtCallControlForwardingAdapter
                    (forwarding.getDestinationAddress(),
                     forwarding.getType(),
                     forwarding.getNoAnswerTimeout()) ;
                break ;

            case CallControlForwarding.EXTERNAL_CALLS:
//                System.out.println("fromCallControlForwarding: EXTERNAL_CALLS " + forwarding.getDestinationAddress()) ;
                adapter = new PtCallControlForwardingAdapter
                    (forwarding.getDestinationAddress(),
                     forwarding.getType(),
                     false,
                     forwarding.getNoAnswerTimeout()) ;
                break ;

            case CallControlForwarding.INTERNAL_CALLS:
//                System.out.println("fromCallControlForwarding: INTERNAL_CALLS " + forwarding.getDestinationAddress()) ;
                adapter = new PtCallControlForwardingAdapter
                    (forwarding.getDestinationAddress(),
                     forwarding.getType(),
                     true,
                     forwarding.getNoAnswerTimeout()) ;
                break ;

            case CallControlForwarding.SPECIFIC_ADDRESS:
//                System.out.println("fromCallControlForwarding: SPECIFIC_ADDRESS " + forwarding.getDestinationAddress()) ;
                adapter = new PtCallControlForwardingAdapter
                    (forwarding.getDestinationAddress(),
                     forwarding.getType(),
                     forwarding.getSpecificCaller(),
                     forwarding.getNoAnswerTimeout()) ;
                break ;
        }
        return adapter ;
    }


    /**
     * Convert to a call control forward object
     */
    public PtCallControlForwarding toCallControlForwarding()
    {
        PtCallControlForwarding forwarding = null ;

        switch (getFilter()) {
            case CallControlForwarding.ALL_CALLS:
                forwarding = new PtCallControlForwarding
                    (getDestinationAddress(), getType(), getNoAnswerTimeout()) ;
                break ;
            case CallControlForwarding.EXTERNAL_CALLS:
                forwarding = new PtCallControlForwarding
                    (getDestinationAddress(), getType(), false, getNoAnswerTimeout()) ;
                break ;
            case CallControlForwarding.INTERNAL_CALLS:
                forwarding = new PtCallControlForwarding
                    (getDestinationAddress(), getType(), true, getNoAnswerTimeout()) ;
                break ;
            case CallControlForwarding.SPECIFIC_ADDRESS:
                forwarding = new PtCallControlForwarding
                    (getDestinationAddress(), getType(), getSpecificCaller(), getNoAnswerTimeout()) ;
                break ;
            default:
                System.out.println("Unknown/Invalid Filter: " + getFilter()) ;
                break ;
        }

        return forwarding ;
    }


    /**
     * gets the destination address of the forwarding rule.
     */
    public String getDestinationAddress()
    {
        return JNI_getDestinationAddress(getHandle()) ;
    }


    /**
     * gets the type of forwarding rule
     */
    public int getType()
    {
        return JNI_getType(getHandle()) ;
    }


    /**
     * gets the filter.
     */
    public int getFilter()
    {
        return JNI_getFilter(getHandle()) ;
    }


    /**
     * gets the specific caller.
     */
    public String getSpecificCaller()
    {
        return JNI_getSpecificCaller(getHandle()) ;
    }

    /**
     * gets the timeout value in seconds for the
     * phone to timout after not answering a call
     * before forwarding ietieout value of forwarding type
     * FORWARD_ON_NOANSWER.
     */
     public int getNoAnswerTimeout(){
        return JNI_getNoAnswerTimeout(getHandle());
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
        JNI_finalize(m_lHandle) ;
        m_lHandle = 0 ;
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
    protected static native long   JNI_constructUnconditional
        (String strDestAddress, int iNoAnswerTimeout) ;

    protected static native long   JNI_constructType
        (String strDestAddress, int iType, int iNoAnswerTimeout) ;

    protected static native long   JNI_constructInternalExternalType
        (String strDestAddress, int iType, boolean bInternalCalls, int iNoAnswerTimeout) ;

    protected static native long   JNI_constructSpecific
        (String strDestAddress, int iType, String strCaller, int iNoAnswerTimeout) ;

    protected static native String JNI_getDestinationAddress(long lHandle) ;
    protected static native int    JNI_getType(long lHandle) ;
    protected static native int    JNI_getFilter(long lHandle) ;
    protected static native String JNI_getSpecificCaller(long lHandle) ;
    protected static native int    JNI_getNoAnswerTimeout(long lHandle);
    protected static native void   JNI_finalize(long lHandle) ;
}
