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


package org.sipfoundry.telephony ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

public class PtEvent implements Event
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** PINGTEL SPECIFIC: Cause code indicating the call is unauthorized by endpoint. */
    public static final int CAUSE_NOT_ALLOWED           = 1000 ;
    /** PINGTEL SPECIFIC: Cause code indicating the call is unauthorized by network/servers. */
    public static final int CAUSE_NETWORK_NOT_ALLOWED	= 1001 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** what is the cause code of this event? */
    protected int m_iCause = CAUSE_UNKNOWN ;    // WARNING: This attribute is modified in JNI

    /** Meta event associated with this event */
    protected PtMetaEvent m_metaEvent = null ;  // WARNING: This attribute is modified in JNI

    /** Id of the meta event */
    protected int m_iID = -1 ;                  // WARNING: This attribute is modified in JNI

   /**
    *  SIP response text
    *  @deprecated DO NOT EXPOSE YET
    *  // WARNING: This attribute is modified in JNI
    */
    protected String m_strResponseText = null;

    /**
     * sip response code
     * @deprecated DO NOT EXPOSE YET
     * // WARNING: This attribute is modified in JNI
     */
    //TO DO: change 0 to some final CONSTANT value
    protected int m_iResponseCode = 0;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default no argument constructor
     */
    protected PtEvent()
    {
    }


    /**
     * Get the cause code associated with this event.  The cause code explains
     * why or how an event was dispatched.  See the cause codes in the JTAPI
     * Event object and the pingtel specific event code listed above.
     *
     * @return the
     */
    public int getCause()
    {
        return m_iCause ;
    }

    /**
     * get SIP response text
     * @deprecated DO NOT EXPOSE
     */
    public String getResponseText(){
        return m_strResponseText;
    }

    /**
     * get SIP response code
     * @deprecated DO NOT EXPOSE
     */
    public int getResponseCode(){
        return m_iResponseCode;
    }


    /** not implemented */
    public int getID()
    {
        return m_iID ;
    }


    /** not implemented */
    public Object getSource()
    {
        return null ;
    }



    /** not implemented */
    public MetaEvent getMetaEvent()
    {
        return m_metaEvent ;
    }
}
