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

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;

import org.sipfoundry.sipxphone.service.* ;

public class PtTerminalComponentEvent extends PtTerminalEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the actual component */
    protected Component m_component ;
    protected String    m_strParam ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking originating terminal and component.  This is 
     * an immutable object and thus no set accessors are defined.
     */    
    protected PtTerminalComponentEvent(Terminal terminal, Component component)
    {
        super(terminal) ;
        m_component = component ;        
    }
    
    
    /**
     * constructor taking originating terminal and component handles.  This
     * is an immutable object and thus no set accessors are defined.
     *
     * NOTE: This contructor is used purely by JNI
     */
    protected PtTerminalComponentEvent(long terminal, long component)
    {
        super((terminal == 0) ? null : new PtTerminal(terminal)) ;
        
        m_component = PtComponent.createComponent(component) ;
    }
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public Component getComponent()
    {   
        return m_component ;    
    }
    
        
    public void setStringParam(String strParam)
    {
        m_strParam = strParam ;
    }
    
    
    public String getStringParam()
    {
        return m_strParam ;
    }
    
}
