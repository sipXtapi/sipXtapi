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

import org.sipfoundry.sipxphone.service.* ;

public class PtTerminalConnectionMonitor implements TerminalConnectionListener
{
    public PtTerminalConnectionMonitor()    
    {
        
    }
    
    /*
     * TerminalConnectionListener stuff
     */
          
    public void terminalConnectionActive(TerminalConnectionEvent event)
    {
        
    }
    
    public void terminalConnectionCreated(TerminalConnectionEvent event)
    {
        
    }

    public void terminalConnectionDropped(TerminalConnectionEvent event)
    {
        
    }

    public void terminalConnectionPassive(TerminalConnectionEvent event)
    {
        
    }

    public void terminalConnectionRinging(TerminalConnectionEvent event)
    {
        
    }

    public void terminalConnectionUnknown(TerminalConnectionEvent event)
    {
        
    }
        
    public void terminalConnectionTalking(TerminalConnectionEvent event)
    {
        
    }        
    
    /*
     * ConnectionListener stuff
     */    
    public void connectionAlerting(ConnectionEvent event)
    {
        
    }        
    
    public void connectionConnected(ConnectionEvent event)
    {
        
    }        
    
    public void connectionCreated(ConnectionEvent event)
    {
        
    }        
    
    public void connectionDisconnected(ConnectionEvent event)
    {
        
    }        
    
    public void connectionFailed(ConnectionEvent event)
    {
        
    }        
    
    public void connectionInProgress(ConnectionEvent event)
    {
        
    }        
    
    public void connectionUnknown(ConnectionEvent event)
    {
        
    }
    
    public void connectionOffered(ConnectionEvent event)
    {
        
    }        
    
    
    /*
     * Call Listener stuff
     */
    public void callActive(CallEvent event)
    {
        
    }        
    
    public void callInvalid(CallEvent event)
    {
        
    }        
    
    public void callEventTransmissionEnded(CallEvent event)
    {
        
    }        
    
    public void singleCallMetaProgressStarted(MetaEvent event)
    {
        
    }        
    
    public void singleCallMetaProgressEnded(MetaEvent event)
    {
        
    }        
    
    public void singleCallMetaSnapshotStarted(MetaEvent event)
    {
        
    }        
        
    public void singleCallMetaSnapshotEnded(MetaEvent event)
    {
        
    }        
    
    public void multiCallMetaMergeStarted(MetaEvent event)
    {
        
    }        
    
    public void multiCallMetaMergeEnded(MetaEvent event)
    {
        
    }        
    
    
    public void multiCallMetaTransferStarted(MetaEvent event)
    {
        
    }        
            
    public void multiCallMetaTransferEnded(MetaEvent event)
    {
        
    }                    
}
