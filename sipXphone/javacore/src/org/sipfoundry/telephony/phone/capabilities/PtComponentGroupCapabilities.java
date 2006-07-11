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

package org.sipfoundry.telephony.phone.capabilities ;

import javax.telephony.*  ;
import javax.telephony.capabilities.*  ;
import javax.telephony.phone.capabilities.*  ;

import org.sipfoundry.telephony.capabilities.* ;

public class PtComponentGroupCapabilities implements ComponentGroupCapabilities
{        
    public boolean canActivate()
    {
        return true ;
    }
        
    public boolean canActivate(Address address)
    {
        return false ;
    }
}
