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
  
public class PtTerminalEvent extends PtEvent implements TerminalEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////    
    /** reference to the terminal associated with this event */
    protected Terminal m_terminal ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking originating terminal.  This is an immutable object
     * and thus no set accessors are defined.
     */        
    public PtTerminalEvent(Terminal terminal)  
    {
        m_terminal = terminal ;        
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    
    public Terminal getTerminal()
    {
        return m_terminal ;        
    }
}
