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

public class PtCallControlCallCapabilities extends PtCallCapabilities implements CallControlCallCapabilities
{
    public boolean canDrop()
    {
        return true ;   
    }
    
        
    public boolean canOffHook()
    {
        return false ;   
    }
    
    
    public boolean canSetConferenceController()
    {
        return false ;   
    }

    
    public boolean canSetTransferController()
    {
        return false ;   
    }

    
    public boolean canSetTransferEnable()
    {
        return false ;   
    }

    
    public boolean canSetConferenceEnable()
    {
        return false ;   
    }

    
    public boolean canTransfer()
    {
        return true ;        
    }


    public boolean canTransfer(Call call)
    {
        return false ;   
    }


    public boolean canTransfer(java.lang.String destination)
    {
        return true ;        
    }
    
    
    public boolean canConference()
    {
        return false ;   
    }

    
    public boolean canAddParty()
    {
        return false ;   
    }

    
    public boolean canConsult()
    {
        return false ;   
    }


    public boolean canConsult(TerminalConnection tc, java.lang.String destination)
    {
        return false ;   
    }


    public boolean canConsult(TerminalConnection tc)
    {
        return false ;   
    }

}
