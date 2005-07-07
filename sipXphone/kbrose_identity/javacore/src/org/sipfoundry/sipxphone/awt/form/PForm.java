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


package org.sipfoundry.sipxphone.awt.form ;

import java.awt.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;

/**
 * Abstract Pingtel Frame Interface.  A "form" ties an application to our UI
 * application framework.  It exposes manditory methods and
 * implementations for dealing with event notifications and GUI state changes.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** Flag that indicates that this form should ways be on top */
    public static final int ALWAYSONTOP      = 0x8000 ;


    /** form is application modal- control cannot be returned to any
        forms lower on the form stack until this form is closed */
    public static final int MODAL            = 0x0001 ;


    /** form can be pushed into and out of context regardless of any other
        applications and whatnot. */
    public static final int MODELESS         = 0x0002 ;



    /**
     *
     */
    public String getFormName() ;


    /**
     * A frame is not necessarily a GUI component and doesn't extend or
     * implement any gui classes.  This method is used to gain the top
     * level GUI element attached to or represented by this frame.
     */
    public Component getComponent() ;


    /**
     * A frame has an associated menu that is displayed whenever the user
     * brings up the pingtel menu and selects the menu tab.  Each of
     * these menus items is represented by a PActionItem.  The action item
     * contains the label, tooltip, action listener, and action command.
     *
     * LEFT MENU
     */
    public PActionItem[] getLeftMenu() ;


    /**
     * A frame has an associated menu that is displayed whenever the user
     * brings up the pingtel menu and selects the menu tab.  Each of
     * these menus items is represented by a PActionItem.  The action item
     * contains the label, tooltip, action listener, and action command.
     *
     * RIGHT MENU
     */
    public PActionItem[] getRightMenu() ;


    /**
     * This is a place holder and may change shape many times.  Our help
     * systems is current being thought of as context sensitive help where
     * all the text is displayed in one file (all for system or all for app?)
     * and is html based.  An application can jump to particular topics in the
     * file using <a name=xxx> type of thing.  This will probably change to a
     * url at some point, too.
     * <br><br>
     * For now: some text is displayed and that is it.
     */
    public String getHelpText() ;


    /**
     * Get the help title for this form.  The help title is displayed in the
     * titlebar area of the task manager when help for this form is displayed.
     */
    public String getHelpTitle() ;


    /**
     * Display this form.  Forms can be shown/displayed in either modal or
     * modeless state.  If modal then program executing with be suspended
     * until the dialog is dismissed.  The return code will be the button
     * exit code.
     *
     * @param application originating appliction
     */
    public int showModal() ;


    /**
     * Display this form.  Forms can be shown/displayed in either modal or
     * modeless state.  If modeless then program execute will return
     * immediately and the return code will be true for success or false if
     * a failure occures.
     *
     * @param application originating appliction
     */
    public boolean showModeless() ;


    /**
     * What display state is this form in? APP_MODAL, SYSTEM_MODAL, or
     * MODELESS
     */
    public int getDisplayState() ;


    /**
     * A form must know who what application context it is running within.
     * Implementors must not return null.
     */
    public Application getApplication() ;


    /**
     * Get the parent form if available.  This is the form that created this
     * form.
     */
    public PForm getParentForm() ;


    /**
     * Add a form listener to be notified of focus changes and initial startup/destruction
     */
    public void addFormListener(PFormListener listener) ;


    /**
     * Remove a form listener
     */
    public void removeFormListener(PFormListener listener) ;


    public void postFormEvent(Object event) ;
    
    /**
     * return an PFormOverlay object associated with this form. 
     */
    public PFormOverlay getFormOverlay();
}