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

public class PtComponentIntChangeEvent extends PtTerminalComponentEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the original value */
    protected int m_iOldValue ;
    /** the new value */
    protected int m_iNewValue ;
       
//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * constructor taking old and new values.  This is an immutable object and
     * thus no set accessors are defined.
     */
    protected PtComponentIntChangeEvent(Terminal  terminal, 
                                        Component component, 
                                        int       iOldValue, 
                                        int       iNewValue)
    {
        super(terminal, component) ;
        
        m_iOldValue = iOldValue ;
        m_iNewValue = iNewValue ;
    }
    
    
    /**
     * constructor taking old and new values.  This is an immutable object and
     * thus no set accessors are defined.
     *
     * NOTE: This contructor is used purely by JNI     
     */
    protected PtComponentIntChangeEvent(long lTerminal, 
                                        long lComponent, 
                                        int  iOldValue, 
                                        int  iNewValue)
    {
        super(lTerminal, lComponent) ;
        
        m_iOldValue = iOldValue ;
        m_iNewValue = iNewValue ;
    }
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public int getOldValue() 
    {
        return m_iOldValue ;
    }


    public int getNewValue()
    {
        return m_iNewValue ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    
    
//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////    
    
}
