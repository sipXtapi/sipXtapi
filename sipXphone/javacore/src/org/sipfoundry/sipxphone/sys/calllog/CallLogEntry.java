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


package org.sipfoundry.sipxphone.sys.calllog ;

import java.util.* ;
import java.text.* ;
import java.io.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.stapi.* ;


/**
 * A CallLogEntry is the data structure used and stored for the call log.  
 * Each entry represents a single call attempt (either inbound or outbound).
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallLogEntry implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Unknown call status */
    public static final int UNKNOWN = 0 ;
    /** outbound call was made successfully */
    public static final int OUTBOUND = 1 ;
    /** outbound call failed for one of many reasons */
    public static final int OUTBOUND_FAILED = 2 ;
    /** inbound call was received and answered successfully */
    public static final int INBOUND = 4 ;
    /** inbound call was received but now answered */
    public static final int INBOUND_MISSED = 8 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** address of caller/callee */
    protected PAddress m_address ;
    /** state of call (UNKNOWN, OUTBOUND, OUTBOUND_FAILED, INBOUND, ) */
    protected int m_iCallStatus ;
    /** time and date of call origination */
    protected Date m_timeStamp ;
    /** duration of call in seconds */
    protected int m_iDuration ;   
                                
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor
     */
    protected CallLogEntry()
    {
        m_address = new PAddress("Unknown") ;
        m_iCallStatus = UNKNOWN ;
        m_timeStamp = null ;
        m_iDuration = 0 ;
    }
    
    
    /**
     * CallLogEntry constructor.  A CallLogEntry should be immutable and thus
     * no set accessor methods are exposed.
     * 
     * @param address address of inbound or outbound call
     * @param iCallStatus status of call
     * @param startTime time stamp of call
     * @param iDuration duration of call in seconds
     */
    public CallLogEntry(PAddress address,
                        int      iCallStatus,
                        Date     timeStamp,
                        int      iDuration) 
    {
        m_address = address ;
        m_iCallStatus = iCallStatus ;
        m_timeStamp = timeStamp ;
        m_iDuration = iDuration ;
        
        if (m_timeStamp == null)
            m_timeStamp = Calendar.getInstance().getTime() ;
    }
       
       
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get the call status of this log entry.  The call status indicates
     * whether the call was inbound or outbound and answered or failed,
     * respectfully.
     *
     * @return status of called
     */
    public int getCallStatus()
    {
        return m_iCallStatus ;    
    }
    
    
    /**
     * Get the call status of this log entry in a string form.  This works
     * identically to getCallStatus() except it returns a string instead
     * of an int.  This is implemented as a convenience and subject to 
     * change.
     *
     * @return string status of called
     * @see #getCallStatus
     */
    public String getCallStatusAsString()
    {
        String strRC = "Unknown" ;
        
        switch (m_iCallStatus) {
             case OUTBOUND:
                strRC = "outbound" ;
                break ;
             case OUTBOUND_FAILED:
                strRC = "failed" ;
                break ;
             case INBOUND:
                strRC = "inbound" ;
                break ;
             case INBOUND_MISSED:            
                strRC = "missed" ;
                break ;
        }
        
        return strRC ;
    }
            
    
    /**
     * Get the start time of the call.  This is the time stamp of when the 
     * call first connected if successfully or the time stamp of the attempt
     * if the called failed or was unanswered.
     *
     * @return timestamp of call
     */
    public Date getTimeStamp()
    {
        return m_timeStamp ;
    }
    
    
    /**
     * Get the start time of the call as a string.  This works identically to
     * getStartTime except it returns a preformatted string as a convenience.
     * The string is formatted using the short flavor of the date appended
     * with short flavor of the time.
     * 
     * @return timestamp of call formatted as a string
     * @see #getTimeStamp
     */
    public String getTimeStampAsString()
    {
        DateFormat df = DateFormat.getDateInstance(DateFormat.SHORT) ;
        DateFormat tf = DateFormat.getTimeInstance(DateFormat.SHORT) ;

       return df.format(getTimeStamp()) + " " + tf.format(getTimeStamp()) ;         
    }    
    
    
    /**
     * Get the duration of the call in seconds.  If the call was not 
     * successful (missed or failed) then the duration will be 0.
     * 
     * @return call duration in seconds
     */
    public int getDuration()
    {
        return m_iDuration ;
    }
    
    
    /**
     * Get the duration of the call in seconds formatted as a String.  This
     * works identically as getDuration, but added as a convenience.  The 
     * format of the resulting string is: hh:mm:ss
     * 
     * @return call duration in seconds formatted as a string
     */    
    public String getDurationAsString()
    {
        String rcStr ;
        int iTotalSeconds = m_iDuration ;
        
        int iHours = (iTotalSeconds / (60*60)) ;
        iTotalSeconds -= (iHours * (60*60)) ;
        int iMinutes = (iTotalSeconds / 60) ;
        iTotalSeconds -= (iMinutes * 60) ;
        int iSeconds = iTotalSeconds ;
                           
        rcStr = (iHours > 9) ? Integer.toString(iHours) : "0" + Integer.toString(iHours) ;
        rcStr += ":" ;
        rcStr += (iMinutes > 9) ? Integer.toString(iMinutes) : "0" + Integer.toString(iMinutes) ;
        rcStr += ":" ;
        rcStr += (iSeconds > 9) ? Integer.toString(iSeconds) : "0" + Integer.toString(iSeconds) ;
                                    
        return rcStr ;                
    }
    
    
    /**
     * Get the address of inbound or outbound call.
     *
     * @return address of inbound or outbound call
     */
    public PAddress getAddress()
    {
        return m_address ;
    }    
}