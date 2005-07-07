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

import java.awt.* ;
import java.util.* ;

import org.sipfoundry.sipxphone.awt.event.* ;

/**
 * A CheckboxGroup is used to group a number of like check boxes together.
 * The resulting group acts like a set of mutually exclusive Radio Buttons. 
 * Exactly one check box in a group can be in the "on" or checked 
 * state at any given time. Changing the state of any member of 
 * the group to "on" will automatically change the state of any other 
 * members to the "off" state.
 *
 * @see org.sipfoundry.sipxphone.awt.PCheckbox
 * @author Robert J. Andreasen, Jr.
 */
public class PCheckboxGroup
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private Vector    m_vMembers ;
    private PCheckbox m_cbSelected ;
        
//////////////////////////////////////////////////////////////////////////////
// Constructions
////
    /**
     * Construct an empty check box group.
     */
    public PCheckboxGroup()
    {        
        m_vMembers = new Vector() ;
        m_cbSelected = null ;
    }
    
    
    /**
     * Gets this group's selected check box.  A check box is considered 
     * selected if its state is "on" (true).
     *
     * @return The selected check box or null if there is no selection within 
     *         this group.
     */
    public PCheckbox getSelectedCheckbox()
    {
        return m_cbSelected ;
    }
    
    
    /**
     * Sets the specified check box to be the selected check box. Specifying 
     * null will unselect all check boxes and leave all of the check boxes 
     * in the group in the "off" (false) state.
     *
     * @param checkbox The check box that will be selected (state changed to "on")
     *        or null to clear all selection.
     */
    public void setSelectedCheckbox(PCheckbox checkbox)
    {        
        PCheckbox cbOld = m_cbSelected ;        
        m_cbSelected = checkbox ;
        
        if (cbOld != null) {
            cbOld.repaint() ;
        }
        
        if (m_cbSelected != null) {
            m_cbSelected.repaint() ;
        }
    }
    
    
//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Returns the number of check boxes within this check box group.
     *
     * @return The number of check boxes within this check box group.
     */
    protected int getSize()
    {
        return m_vMembers.size() ;
    }


    /**
     * Adds the specified checkbox to this check box group.
     * 
     * @param checkbox The check box that will become a member of this group.
     */     
    protected void addCheckbox(PCheckbox checkbox)
    {
        if (!m_vMembers.contains(checkbox))
            m_vMembers.addElement(checkbox) ;
        
    }

    
    /**
     * Removes the specified checkbox from this check box group.
     * 
     * @param checkbox The check box that will cease to be a member of this group.
     */     
    protected void removeCheckbox(PCheckbox checkbox)
    {
        m_vMembers.removeElement(checkbox) ;
    }
}
