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

 
package org.sipfoundry.sipxphone.awt ;

import org.sipfoundry.sipxphone.awt.event.* ;

/**
 * PActionItem is a convenience class that stores information needed by a 
 * menu control, button bar, or command bar.  
 *
 * @author Robert J. Andreasen, Jr. 
 */
public class PActionItem implements Cloneable
{        
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** action Command send back to action listener */
    private String m_strActionCommand ;
    /** action Command send back to action listener */
    private PActionListener m_listener ;
    /** label (what we are going to display) */
    private PLabel m_label ;
    /* hint for this command */
    private String m_strHint ;    
    /** Is this action item enabled? */
    private boolean m_bEnabled ;
    
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /** 
     * Constructor, takes a label and hint text.
     * 
     * @param label Text or graphic that represents the action.
     * @param strHint Hint text that displays when the user holds down the button. 
     *        To include a title as well as text, use the 
     *        format <hint title>|<hint text>.
     */
    public PActionItem(PLabel label, String strHint)
    {
        m_strActionCommand = null ;
        m_listener = null ;
        m_label = label ;
        m_strHint = strHint ;
        m_bEnabled = true ;
    }

    
    /** 
     * Constructor, takes a label, hint text, listener, and action command.
     *
     * @param label Text or graphic that represents the action.
     * @param strHint Hint text that displays when the user holds down the button.
     * @param listener Listener interested in receiving button press 
     *        notifications.
     * @param strActionCommand Identification string sent to listeners.
     */
    public PActionItem( PLabel          label, 
                        String          strHint,
                        PActionListener listener,
                        String          strActionCommand)
    {
        m_strActionCommand = strActionCommand ;
        m_listener = listener ;
        m_label = label ;
        m_strHint = strHint ;        
        m_bEnabled = true ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////
    /**
     * Return the action command string, or null if not set.
     */
    public String getActionCommand()
    {
        return m_strActionCommand ;        
    }

    
    /**
     * Return the action listener, or null if not set.
     */
    public PActionListener getActionListener()
    {
        return m_listener ;
    }
    

    /**
     * Return the hint text used when a user holds down a button.
     */
    public String getHint()
    {
        return m_strHint ;
    }
    
    
    /**
     * Return the text or graphic representing the action.
     */
    public PLabel getLabel() 
    {
        PLabel label = null ;
        
        return m_label ;
    }
    
    
    /**
     * Set the enabled state of this action item.  A boolean state of true
     * indicates that the action is enabled, active, or on.  A state of false
     * indicates that the action is disabled or off.
     *
     * @param bEnable the new enable state for this item
     */
    public void setEnabled(boolean bEnable)
    {
        m_bEnabled = bEnable ;
    }

    
    /**
     * Return whether or not this action item is enabled
     */
    public boolean isEnabled()
    {
        return m_bEnabled ;
    }
}
