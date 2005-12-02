/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/callcontrol/PtCallControlForwarding.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.telephony.callcontrol;

import javax.telephony.callcontrol.* ;
/**
 * PtCallControlForwarding.java
 *
 * This class adds the concept of timeout value to forwarding rule
 * if the forwarding type is CallControlForwarding.FORWARD_ON_NOANSWER.
 * Created: Wed Oct 24 17:18:24 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class PtCallControlForwarding extends CallControlForwarding{

    /**
     * The timeout value in seconds if the forwarding type is
     * FORWARD_ON_NOANSWER which defaults to 24.
     */
   private int m_iNoAnswerTimeout = 24;



 /**
   * This constructor takes the address to apply this forwarding instruction,
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
    public PtCallControlForwarding(java.lang.String destAddress,
                             int type,
                             boolean internalCalls,
                             int iNoAnswerTimeout){
        super( destAddress, type, internalCalls );
        if( type == super.FORWARD_ON_NOANSWER ){
            m_iNoAnswerTimeout = iNoAnswerTimeout;
        }

    }

    /**
    * This constructor takes an address to apply the forwarding instruction for
    * a specific incoming telephone call, identified by a string address.
    * It also takes the type of forwarding desired for this specific address.
    * @param destAddress  destination address for the call forwarding operation
    * @param type  the type of the forwarding operation
    * @param caller  the address of the incoming caller
    * @param iNoAnswerTimeout timeout in seconds if the forwarding type
    *                           is FORWARD_ON_NOANSWER
    */
    public PtCallControlForwarding(java.lang.String destAddress,
                             int type,
                             java.lang.String caller, int iNoAnswerTimeout){
        super( destAddress, type, caller );
        if( type == super.FORWARD_ON_NOANSWER ){
            m_iNoAnswerTimeout = iNoAnswerTimeout;
        }
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
    public PtCallControlForwarding(java.lang.String destAddress,
                             int type,
                             int iNoAnswerTimeout){
        super( destAddress, type);
        if( type == super.FORWARD_ON_NOANSWER ){
            m_iNoAnswerTimeout = iNoAnswerTimeout;
        }

    }



    /**
     * Returns the "no answer timeout"  if the type was set to FORWARD_ON_NOANSWER
     * and "timeout" was set. If not set, the default value of 24 is returned.
     * @return The "no answer timoeut" of this forwarding instruction.
     */
    public int getNoAnswerTimeout(){
        return m_iNoAnswerTimeout;
    }



}// PtCallControlForwarding







