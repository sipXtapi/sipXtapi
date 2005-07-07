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


package org.sipfoundry.sipxphone.service.logger ;

import org.sipfoundry.sipxphone.service.* ;
import java.io.* ;
import java.net.* ;


/**
 * 
 */
public class LogCommand implements Serializable
{    
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String CMD_SUBSCRIBE   = "subscribe" ;
    public static final String CMD_UNSUBSCRIBE = "unsubscribe" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    public String strCommand ;        
    public String strOption ;


    public LogCommand()
    {
        strCommand = null ;
        strOption = null ;
    }

    
    public LogCommand(String strCommand, String strOption)
    {
        this.strCommand = strCommand ;
        this.strOption = strOption ;
    }        
    
    public String toString()
    {
        return strCommand + " " + strOption ;
        
    }
}