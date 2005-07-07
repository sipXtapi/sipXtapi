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


package org.sipfoundry.sipxphone.app.speeddial ;

import org.sipfoundry.util.* ;


/**
 *
 */
public class SpeeddialEntry
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Speed dial number*/
    protected String m_sNumber;

    /** Speed dial SIP URL*/
    protected SipParser m_sipParser;
//////////////////////////////////////////////////////////////////////////////
// Constructor
////
    /**
     *
     */
    private SpeeddialEntry (){
        m_sNumber   = "";
        m_sipParser = new SipParser(null);
    }

    public SpeeddialEntry (String sNumber, SipParser sipParser){
        m_sNumber   = sNumber;
        m_sipParser = sipParser;

    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *
     */

    public String getSpeeddialNumber(){
        return m_sNumber ;
    }

    public String getUserName(){
        return m_sipParser.getDisplayName() ;
    }

    public String getUserId(){
        return m_sipParser.getUser() ;
    }

    public String getAddress(){
        return m_sipParser.getHost() ;
    }

    public String getSIPURL()
    {
        return m_sipParser.render() ;
    }
}
