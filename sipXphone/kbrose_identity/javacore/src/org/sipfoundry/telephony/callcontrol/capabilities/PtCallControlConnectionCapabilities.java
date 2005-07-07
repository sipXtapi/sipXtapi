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

public class PtCallControlConnectionCapabilities extends PtConnectionCapabilities implements CallControlConnectionCapabilities
{    
    public boolean canRedirect()
    {
        return false ;   
    }
    
    
    public boolean canAddToAddress()
    {
        return false ;   
    }
    
    
    public boolean canAccept()
    {
        return true ;   
    }
    
    
    public boolean canReject()
    {
        return false ;   
    }
    
    
    public boolean canPark()
    {
        return false ;   
    }    
}
