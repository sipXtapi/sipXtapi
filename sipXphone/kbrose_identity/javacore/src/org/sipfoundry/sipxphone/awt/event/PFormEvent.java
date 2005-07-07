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

import org.sipfoundry.sipxphone.awt.form.* ;

/**
 * A form event is sent whenever something significant happens to a form.
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PFormEvent extends PEvent
{
    /** the form has been created, but not yet shown */
    public static final int FE_FORM_CREATED     = 0 ;
    /** the form is about to be destroyed */
    public static final int FE_FORM_DESTROYED   = 1 ;
    /** the form has gained focus */
    public static final int FE_FOCUS_GAINED     = 2 ;
    /** the form has lost focus */
    public static final int FE_FOCUS_LOST       = 3 ;
    
    /** what type of form event is this? */
    protected int m_iType ;
    
    /** form that focus is being sent to or taken from for FE_FOCUS_LOST and 
        FE_FOCUS_GAINED events, respectfully. */
    protected PForm m_formFocus ;
    
        
    /** 
     * constructor taking a source and action command
     *
     * @param objSource source object
     * @param action command for this event
     * @param other form gaining or losing focus to/from target form
     */
    public PFormEvent(Object objSource, int iType, PForm formFocus)
    {
        super(objSource) ;
        
        m_iType = iType ;
        m_formFocus = formFocus ;
    }        
    
    
    /** 
     * @return what type of event is this?
     */
    public int getType()
    {
        return m_iType ;        
    }
    
 
    /** 
     * who is the form that is losing focus in a FE_FOCUS_GAINED event.  This
     * method will return null in any other event situations.
     *
     * @return the form who is losing focus in a FE_FOCUS_GAINED event
     */
    public PForm getFormLosingFocus()
    {
        PForm form = null ;
        
        if (m_iType == FE_FOCUS_GAINED) {
            form = m_formFocus ;                        
        }
        
        return form ;        
    }
    
    
    /** 
     * who is the form that is gaining focus in a FE_FOCUS_LOST event.  This
     * method will return null in any other event situations.
     *
     * @return the form who is gaining focus in a FE_FOCUS_LOST event
     */
    public PForm getFormGainingFocus()
    {
        PForm form = null ;
        
        if (m_iType == FE_FOCUS_LOST) {
            form = m_formFocus ;                        
        }
        
        return form ;                
    }
}