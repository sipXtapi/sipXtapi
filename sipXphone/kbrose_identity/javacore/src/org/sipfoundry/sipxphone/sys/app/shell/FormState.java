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


package org.sipfoundry.sipxphone.sys.app.shell ;

import java.awt.* ;
import java.util.* ;

import org.sipfoundry.sipxphone.awt.form.* ;

/**
 * <p>FormState is a package protected class that is used purely by the 
 * FormManager to record / monitor the state of a form for it's own 
 * management.  This should not be exposed or looked at form anyone or 
 * anywhere else.
 *
 * <p>We need to know what state a form to avoid situations where we close a 
 * form twice or a request comes in while in the middle of a state change.
 *
 * <p>These states are recorded for each form and checked at key points.  
 * This is an implementation helper and expected to be used properly, no 
 * guards are present for bad states or invalid state progressions.
 *
 * <p>The form state also maintaines the list of forms / states as a 
 * hashtable.  There are static methods on the form state objects that
 * allow manipulation.
 *
 * @author Robert J. Andreasen, Jr.
 */
class FormState 
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** the form state is unknown (closed?) */
    public static final int FS_UNKNOWN = -1 ;
    /** the form is being created / shown */
    public static final int FS_CREATING = 0 ;
    /** the form is about to be shown */
    public static final int FS_SHOWING = 1 ; 
    /** the form has been shown and may be active or not */
    public static final int FS_SHOWN = 2 ; 
    /** the form is being closed */
    public static final int FS_CLOSING = 3 ;

    
    /** the form gui state is known */
    public static final int FGS_UNKNOWN = -1 ;
    /** the form has been shown and is in focus */
    public static final int FGS_ACTIVE = 0 ;
    /** the form has been shown and is not in focus */
    public static final int FGS_DEACTIVE = 1 ;
    


//////////////////////////////////////////////////////////////////////////////
// Attributes
////    
    /** the static state hashtable- form state is stored here */
    protected static Hashtable m_htStates = new Hashtable() ;
    
    /** static gui state hashtable- gui form state is stored here */
    protected static Hashtable m_htGUIStates = new Hashtable() ;
                    
//////////////////////////////////////////////////////////////////////////////
// Construction
////    
    /**
     * There is no need to create a form state, the important stuff is static.
     */
    private FormState()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    
    /**
     * what is the state of the specified form?
     *
     * @param form the form we are interested in
     * @return int value representing state of the form.  See FS_* constants
     *         for more information.
     */
    public static int getFormState(PForm form)
    {
        int iRC = FS_UNKNOWN ;
        Integer state = (Integer) m_htStates.get(form) ;        
        
        if (state != null) {
            iRC = state.intValue() ;                                
        }
        
        return iRC ;        
    }
    
    
    /**
     * set the state of the specified form
     *
     * @param form the form we are interested in
     * @param iState the new state value.  This value should coincide with the FS_*
     *        constants defined in this class.     
     */
    public static void setFormState(PForm form, int iState)
    {
        m_htStates.put(form, new Integer(iState)) ;        
    }

    
    /**
     * clear form state associated with the specified form
     *
     * @param form the form we are interested in clearing
     */
    public static void clearFormState(PForm form) 
    {
        m_htStates.remove(form) ;
        
    }
    
    /**
     * what is the GUI state of the specified form?
     *
     * @param form the form we are interested in
     * @return int value representing state of the form.  See FGS_* constants
     *         for more information.
     */
    public static int getGUIFormState(PForm form)
    {
        int iRC = FGS_UNKNOWN ;
        Integer state = (Integer) m_htGUIStates.get(form) ;        
        
        if (state != null) {
            iRC = state.intValue() ;                                
        }
        
        return iRC ;        
    }
    
    
    /**
     * set the GUI state of the specified form
     *
     * @param form the form we are interested in
     * @param iState the new state value.  This value should coincide with the FS_*
     *        constants defined in this class.     
     */
    public static void setGUIFormState(PForm form, int iState)
    {
        m_htGUIStates.put(form, new Integer(iState)) ;        
    }

    
    /**
     * clear all GUI state record associated with the specified form
     *
     * @param form the form we are interested in clearing
     */
    public static void clearGUIFormState(PForm form) 
    {
        m_htGUIStates.remove(form) ;
        
    }    
}
