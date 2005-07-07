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

public class PtCallControlTerminalCapabilities extends PtTerminalCapabilities implements CallControlTerminalCapabilities
{
    public boolean canGetDoNotDisturb()
    {
        return false ;
    }
    
    
    public boolean canSetDoNotDisturb()
    {
        return false ;
    }
    
    
    public boolean canPickup()
    {
        return false ;
    }
    
    
    public boolean canPickup(Connection connection, Address address)
    {
        return false ;
    }
    
    
    public boolean canPickup(TerminalConnection tc, Address address)
    {
        return false ;
    }
    
    
    public boolean canPickup(Address address1, Address address2)
    {
        return false ;
    }
    
    
    public boolean canPickupFromGroup()
    {
        return false ;
    }
    
    
    public boolean canPickupFromGroup(java.lang.String group, Address address)
    {
        return false ;
    }
    
    
    public boolean canPickupFromGroup(Address address)
    {
        return false ;   
    }        
}
