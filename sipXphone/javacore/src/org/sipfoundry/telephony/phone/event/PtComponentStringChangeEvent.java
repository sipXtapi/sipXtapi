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

package org.sipfoundry.telephony.phone.event ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

public class PtComponentStringChangeEvent extends PtTerminalComponentEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the original value */
    protected String m_strOldValue ;
    /** the new value */
    protected String m_strNewValue ;
       
//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * constructor taking old and new values.  This is an immutable object and
     * thus no set accessors are defined.
     */
    public PtComponentStringChangeEvent(Terminal  terminal, 
                                        Component component, 
                                        String    strOldValue,
                                        String    strNewValue)
    {
        super(terminal, component) ;
        
        m_strOldValue = strOldValue ;
        m_strNewValue = strNewValue ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public String getOldValue() 
    {
        return m_strOldValue ;
    }


    public String getNewValue()
    {
        return m_strNewValue ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    
    
//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////    
    
}
