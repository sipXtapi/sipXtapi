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

import javax.telephony.callcontrol.capabilities.*  ;
import javax.telephony.capabilities.*  ;

import org.sipfoundry.telephony.capabilities.* ;

public class PtCallControlAddressCapabilities extends PtAddressCapabilities implements CallControlAddressCapabilities
{
    public boolean canSetForwarding()
    {
        return false ;
    }
    
    
    public boolean canGetForwarding()    
    {
        return false ;
    }
    
    
    public boolean canCancelForwarding()
    {
        return false ;
    }
    
    
    public boolean canGetDoNotDisturb()
    {
        return false ;
    }
    
    
    public boolean canSetDoNotDisturb()
    {
        return false ;
    }
    
    
    public boolean canGetMessageWaiting()
    {
        return false ;
    }
    
    
    public boolean canSetMessageWaiting()
    {
        return false ;
    }
}
