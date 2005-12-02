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

 
package org.sipfoundry.sipxphone.awt.event ;

/**
 * PActionEvent is the event object passed as part of the PActionListener
 * interface.  It allows the listeners to query the action command and 
 * discover more details of the event.
 *
 * @see PActionListener
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PActionEvent extends PEvent
{
    /** action command associated with this event */
    protected String m_strActionCommand ;        
        
    /** 
     * Constructor, takes a source and action command.
     *
     * @param objSource Source object.
     * @param action Command for this event.
     */
    public PActionEvent(Object objSource, String strActionCommand)
    {
        super(objSource) ;
        
        m_strActionCommand = strActionCommand ;
    }        
    
    
    /** 
     * @return Action command associated with this event.
     */
    public String getActionCommand()
    {
        return m_strActionCommand ;        
    }
}