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

package org.sipfoundry.stapi.event ;

import org.sipfoundry.stapi.* ;
import java.io.PrintStream ;

/**
 * This class is used to notify the application of a change in the state
 * of a call in the system.
 *
 * @see PConnectionListener
 * @see PCallManager
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PConnectionEvent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Indicates an event resulting from normal operation. */
    public static final int CAUSE_NORMAL					 = 100 ;
    /** Indicates an event that resulted from an unknown cause. */
    public static final int CAUSE_UNKNOWN					 = 101 ;
    /** Indicates an event that resulted when a call was terminated without
     *  the phone going on hook. */
    public static final int CAUSE_CALL_CANCELLED			 = 102 ;
    /** Indicates an event that resulted when a destination was unavailable. */
    public static final int CAUSE_DESTINATION_NOT_OBTAINABLE = 103 ;
    /** Indicates an event that resulted when a call encountered an
     *  incompatible destination. */
    public static final int CAUSE_INCOMPATIBLE_DESTINATION	 = 104 ;
    /** Indicates an event that resulted when a call encountered
     *  inter-digit timeout while dialing. */
    public static final int CAUSE_LOCKOUT					 = 105 ;
    /** Indicates an event resulting from the creation of a new call. */
    public static final int CAUSE_NEW_CALL					 = 106 ;
    /** Indicates an event that is the result of resources being unavailable. */
    public static final int CAUSE_RESOURCES_NOT_AVAILABLE	 = 107 ;
    /** Indicates an event that results when a call encounters network
     *  congestion. */
    public static final int CAUSE_NETWORK_CONGESTION		 = 108 ;
    /** Indicates an event that results when a call cannot reach its
     *  destination network. */
    public static final int CAUSE_NETWORK_NOT_OBTAINABLE	 = 109 ;
    /** Indicates an event that is part of a snapshot of the current
     *  state of the call. */
    public static final int CAUSE_SNAPSHOT					 = 110 ;
    /** Indicates an event that results when one call is put on hold and
     *  another retrieved in an atomic operation, typically on single line
     *  telephones. */
    public static final int CAUSE_ALTERNATE					 = 203 ;
    /** Indicates an event that results when a call encounters a busy endpoint. */
    public static final int CAUSE_BUSY						 = 203 ;
    /** Indicates an event that is related to the callback feature. */
    public static final int CAUSE_CALL_BACK					 = 204 ;
    /** Indicates an event resulting when a call is not answered before a
     *  timer elapses. */
    public static final int CAUSE_CALL_NOT_ANSWERED			 = 205 ;
    /** Indicates an event that occurs when a call is redirected by the call
     *  pickup feature. */
    public static final int CAUSE_CALL_PICKUP				 = 206 ;
    /** Indicates an event that is related to the conference feature. */
    public static final int CAUSE_CONFERENCE				 = 207 ;
    /** Indicates an event that is related to the do not disturb feature. */
    public static final int CAUSE_DO_NOT_DISTURB			 = 208 ;
    /** Indicates an event that is related to the park feature. */
    public static final int CAUSE_PARK						 = 209 ;
    /** Indicates an event that is related to the redirect feature. */
    public static final int CAUSE_REDIRECTED				 = 210 ;
    /** Indicates an event that results from a call encountering a reorder
     *  tone. */
    public static final int CAUSE_REORDER_TONE				 = 211 ;
    /** Indicates an event that is related to the transfer feature. */
    public static final int CAUSE_TRANSFER					 = 212 ;
    /** Indicates an event resulting when a call encounters the "all trunks
     *  busy" condition. */
    public static final int CAUSE_TRUNKS_BUSY				 = 213 ;
    /** Indicates an event that is related to the unhold feature. */
    public static final int CAUSE_UNHOLD					 = 214 ;
    /** Indicates an event that results when the call is not allowed. */
    public static final int CAUSE_NOT_ALLOWED				 = 1000 ;
    /** Indicates an event that results when the call to the destination's
     *  network is not allowed. */
    public static final int CAUSE_NETWORK_NOT_ALLOWED		 = 1001 ;

   /** @deprecated */
    public static final int DEFAULT_RESPONSE_CODE = 0;
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PCall     m_call ;
    private PAddress  m_address ;
    private int       m_iCause ;

    /** sip response code*/
    private int    m_iResponseCode = DEFAULT_RESPONSE_CODE;

    /** sip response text */
    private String m_strResponseText = null;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a PConnectionEvent object
     */
    public PConnectionEvent(PCall    call,
                            PAddress address,
                            int      iCause)
    {
        m_call = call ;
        m_address = address ;
        m_iCause = iCause ;
    }

    /**
     * Constructs a PConnectionEvent with
     * call, address, cause, sipResponseCode and sipResponseText.
     * @deprecated DO NOT EXPOSE
     */
    public PConnectionEvent(PCall    call,
                            PAddress address,
                            int      iCause,
                            int      iResponseCode,
                            String   strResponseText )
    {
        m_call = call ;
        m_address = address ;
        m_iCause = iCause ;
        m_iResponseCode = iResponseCode;
        m_strResponseText = strResponseText;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the PCall object that changed in the system.
     */
    public PCall getCall()
    {
        return m_call ;
    }


    /**
     * Get the phone number associated with the state change. This may be null
     * if there was no phone number.
     */
    public PAddress getAddress()
    {
        return m_address ;
    }

    /**
     * Get the cause of the failure or disconnect.
     */
    public int getCause()
    {
        return m_iCause ;
    }

   /**
    *  get the response code
    *  @deprecated DO NOT EXPOSE
    */
    public int getResponseCode(){
        return m_iResponseCode;
    }

    /**
     * get the response text
     * @deprecated DO NOT EXPOSE
     */
    public String getResponseText(){
        return m_strResponseText;
    }

    /**
     * Gets the cause of the event as a String.
     * @deprecated DO NOT EXPOSE
     */
    public static String getCauseAsString(int iCause){
       String strRet;
       switch( iCause ){
            case CAUSE_NORMAL:
                strRet="Normal ("+iCause+")" ; break;
            case CAUSE_UNKNOWN:
                strRet="Unknown ("+iCause+")" ; break;
            case CAUSE_CALL_CANCELLED:
                strRet="Call Cancelled ("+iCause+")" ; break;
            case CAUSE_DESTINATION_NOT_OBTAINABLE:
                strRet="Destination not obtainable ("+iCause+")" ; break;
            case CAUSE_INCOMPATIBLE_DESTINATION:
                strRet="Incompatible destination ("+iCause+")" ; break;
            case CAUSE_LOCKOUT:
                strRet="Lockout ("+iCause+")" ; break;
            case CAUSE_NEW_CALL:
                strRet="New Call ("+iCause+")" ; break;
            case CAUSE_RESOURCES_NOT_AVAILABLE:
                strRet="Resource not available ("+iCause+")"; break;
            case CAUSE_NETWORK_CONGESTION:
                strRet="Network Congestion ("+iCause+")"; break;
            case CAUSE_NETWORK_NOT_OBTAINABLE:
                strRet="Network not obtainable ("+iCause+")"; break;
            case CAUSE_SNAPSHOT:
                strRet="Snapshot ("+iCause+")"; break;
            case CAUSE_ALTERNATE:
                strRet="Alternate ("+iCause+")"; break;
            case CAUSE_CALL_BACK:
                strRet="Callback ("+iCause+")"; break;
            case CAUSE_CALL_NOT_ANSWERED:
                strRet="Call not answered ("+iCause+")"; break;
            case CAUSE_CALL_PICKUP:
                strRet="Call Pickup ("+iCause+")"; break;
            case CAUSE_CONFERENCE:
                strRet="Conference ("+iCause+")"; break;
            case CAUSE_DO_NOT_DISTURB:
                strRet="Do not disturb ("+iCause+")"; break;
            case CAUSE_PARK:
                strRet="Park ("+iCause+")"; break;
            case CAUSE_REDIRECTED:
                strRet="Redirected ("+iCause+")"; break;
            case CAUSE_REORDER_TONE:
                strRet="Reorder tone ("+iCause+")"; break;
            case CAUSE_TRANSFER:
                strRet="Transfer ("+iCause+")"; break;
            case CAUSE_TRUNKS_BUSY:
                strRet="Trunks busy ("+iCause+")"; break;
            case CAUSE_UNHOLD:
                strRet="Unhold ("+iCause+")"; break;
            case CAUSE_NOT_ALLOWED:
                strRet="Not allowed ("+iCause+")"; break;
            case CAUSE_NETWORK_NOT_ALLOWED:
                strRet="Network not allowed ("+iCause+")"; break;
            default:strRet="";break;
        }
        return strRet;
    }

    /**
     * Dump debugging output for this object
     *
     * @deprecated Hide
     */
    public void dump(PrintStream out)
    {
        out.println("PConnectionEvent") ;
        out.println("  callid       ="+m_call.getCallID()) ;
        out.println("  address      ="+m_address) ;
        out.println("  cause        ="+getCauseAsString(m_iCause)) ;
        out.println("  responseCode ="+getResponseCode()) ;
        out.println("  responseText ="+getResponseText()) ;

    }

    /**
     * returns a String with callid, address and causeCode.
     * @deprecated Hide
     */
    public String toString()
    {
        StringBuffer buffer= new StringBuffer();

        buffer.append("  call-id      = "+m_call.getCallID()) ;
        buffer.append("\n");
        buffer.append("  address      = "+m_address) ;
        buffer.append("\n");
        buffer.append("  cause        = "+getCauseAsString(m_iCause)) ;
        buffer.append("\n");
        buffer.append("  responseCode = "+getResponseCode()) ;
        buffer.append("\n");
        buffer.append("  responseText = "+getResponseText()) ;
        buffer.append("\n");

        return buffer.toString();
    }


}
