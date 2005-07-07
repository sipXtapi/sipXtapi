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

package org.sipfoundry.telephony.callcontrol.capabilities ;

import javax.telephony.*  ;
import javax.telephony.capabilities.*  ;
import javax.telephony.callcontrol.capabilities.*  ;

import org.sipfoundry.telephony.capabilities.* ;

public class PtCallControlTerminalConnectionCapabilities extends PtTerminalConnectionCapabilities implements CallControlTerminalConnectionCapabilities
{
    public boolean canHold()
    {
        return true ;   
    }


    public boolean canUnhold()
    {
        return true ;
    }
    
    
    public boolean canJoin()    
    {
        return false ;                
    }
    
    
    public boolean canLeave()
    {
        return false ;
    }
}
