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

import org.sipfoundry.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.app.* ;


/**
 * The Form Manager is responsible for managing the physical display and the
 * stack of application 'Forms'.  This manager also interacts with the
 * PingtelEventDispatcher and PingtelEventQueue to process events.
 *
 * ::TODO:: Currently, an application can toss up forms and the forms appear
 * to stack other exiting forms.  There is no mechanism in place to validate
 * a form child/parent relationship.  In fact, it is possible that form could
 * be blocked on another modal form and have the original become active.  This
 * is definitely a no no.  I'm thinking about a "Activate Form" type of thing
 * that will push that form and it's child forms on top of the stack.
 */
public class FormManager extends java.awt.event.MouseAdapter
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Form  Z-Order */
    protected Stack m_vFormStack ;
    /** our window into the world- the AWT display*/
    protected Container m_pnlDisplay ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Default Constructor
     */
    public FormManager(Container pnlDisplay)
    {
        m_pnlDisplay = pnlDisplay ;
        m_pnlDisplay.addMouseListener(this);

        m_vFormStack = new Stack() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * What is the active form (form on top of stack)?
     *
     * NOTE: This method cannot be synchronized because it may
     *       be called as a side effect of a form event (and those
     *       events are handled on a the ShellApp's thread context).
     *
     * @return PForm that is active
     */
    public PForm getActiveForm()
    {
        PForm rcForm = null ;

        synchronized (m_vFormStack)
        {
            if (!m_vFormStack.empty()) {
                rcForm = (PForm) m_vFormStack.peek() ;
            }
        }

        return rcForm ;
    }


    /**
     * Place a form on top of the form stack.  If the the form is already
     * on the stack then it is moved to the top.
     *
     * @param form to place on top of the stack
     */
    public synchronized void pushForm(PForm form)
    {
        try {
            PingtelEventDispatcher.disable() ;

//System.out.println("+ FormManager::pushForm: " + form) ;
            if (form == null)
                throw new IllegalArgumentException("null pointer passed for form") ;

            PForm   activeForm = null ;
            boolean bCreated = false ;

            activeForm = getActiveForm() ;

            // Kick out if we in the midst of creating/showing this exact form
            int iFormState = FormState.getFormState(form) ;
            if ((iFormState == FormState.FS_CREATING) || (iFormState == FormState.FS_SHOWING)) {
                // Logger.message(this, Logger.Informational, "Ignore push, form is already creating or showing.") ;
                return ;
            }

            // Add if not already active!
            if (form != activeForm) {

                if (    (activeForm != null) &&
                        ((activeForm.getDisplayState() & PForm.ALWAYSONTOP) ==
                        PForm.ALWAYSONTOP)) {

                    // Add the new form to the stack, but then move the old
                    // active back ontop.

                    synchronized (m_vFormStack)
                    {
                        m_vFormStack.push(form) ;
                        m_vFormStack.removeElement(activeForm) ;
                        m_vFormStack.push(activeForm) ;
                    }
                    doCreateForm(form) ;
                } else {

                    if (FormState.getFormState(form) == FormState.FS_UNKNOWN) {
                        FormState.setFormState(form, FormState.FS_CREATING) ;
                    }

                    // Tell old form about state change
                    if (activeForm != null) {
                        doDeactivateForm(activeForm, form) ;
                    }

                    // Remove the form if it is already on the stack- so we can replace
                    // it on top of the stack
                    synchronized (m_vFormStack)
                    {
                        if (m_vFormStack.contains(form)) {
                            m_vFormStack.removeElement(form) ;
                        } else {
                            bCreated = true ;
                        }
                        m_vFormStack.push(form) ;
                    }

                    if (bCreated) {
                        doCreateForm(form) ;
                    }
                    FormState.setFormState(form, FormState.FS_SHOWING) ;
                    doActivateForm(form, activeForm) ;

                    FormState.setFormState(form, FormState.FS_SHOWN) ;
                }
            } else {
                System.out.println("Form already active: " + form) ;
            }
        } finally {
            PingtelEventDispatcher.enable() ;
        }
    }


    /**
     * NOTE: This method cannot be synchronized because it may
     *       be called as a side effect of a form event (and those
     *       events are handled on a the ShellApp's thread context).
     *
     * @return boolean true if form is stacked (on Form stack) otherwise false
     */
    public boolean isFormStacked(PForm form)
    {
        boolean bRC  ;

        synchronized (m_vFormStack)
        {
            bRC = m_vFormStack.contains(form) ;
        }

        return bRC ;
    }


    /**
     * NOTE: This method cannot be synchronized because it may
     *       be called as a side effect of a form event (and those
     *       events are handled on a the ShellApp's thread context).
     *
     * @return boolean true if form is in focus otherwise false
     */
    public boolean isFormInFocus(PForm form)
    {
        return (form == getActiveForm()) ;  // synchronization in getActiveForm
    }


    /**
     * remove the passed form from our stack
     */
    public synchronized void popForm(PForm form)
    {
        try {
            PingtelEventDispatcher.disable() ;

//System.out.println("+ FormManager::popForm: " + form) ;

            // Ignore the request if the form is already being closed...
            int iFormState = FormState.getFormState(form) ;
            if ((iFormState == FormState.FS_CLOSING) || (iFormState == FormState.FS_UNKNOWN)) {
                System.out.println("Ignore popForm, form is already closing or closed: " + form) ;
                // Logger.message(this, Logger.Informational, "Ignore popForm, form is already closing or closed.") ;
                return ;
            }

            if (getActiveForm() == form)
            {
                synchronized (m_vFormStack)
                {
                    form = (PForm) m_vFormStack.pop() ;
                }

                // Tell old form that it has lost focus ... and it was destroyed...
                FormState.setFormState(form, FormState.FS_CLOSING) ;
                doDeactivateForm(form, getActiveForm()) ;
                int iActiveFormState = FormState.getFormState(form) ;
                if (iActiveFormState != FormState.FS_UNKNOWN) {
                    doDestroyForm(form) ;
                }

                // If the stack ends up empty- yell and scream.
                PForm formActive = null ;
                synchronized (m_vFormStack)
                {
                    if (m_vFormStack.empty()) {
                        throw new IllegalStateException("Stack Frame is Empty") ;
                    } else {
                        // Otherwise tell other app about activation
                        formActive = getActiveForm() ;
                    }
                }
                doActivateForm(formActive, form) ;
            } else {
                PForm destroyForm = null ;
                synchronized (m_vFormStack)
                {
                    if (isFormStacked(form))
                    {
                        destroyForm = form ;
                        m_vFormStack.removeElement(destroyForm) ;
                    }
                }

                if (destroyForm != null)
                {
                    FormState.setFormState(destroyForm, FormState.FS_CLOSING) ;
                    doDestroyForm(destroyForm) ;
                }
            }
        } finally {
            PingtelEventDispatcher.enable() ;
        }
    }

    /**
     * recieves Mouse Events
     */
    public void mouseClicked(java.awt.event.MouseEvent e)
    {
    	PFormOverlay overlay = getActiveForm().getFormOverlay();
    	overlay.invokeButton(e);
    }

    /**
     * Debugging method useful for debugging / viewing the current frame stack
     */
    public void dumpStack()
    {
        PForm form ;

        System.out.println("Frame Stack: ") ;

        synchronized (m_vFormStack)
        {
            for (Enumeration e = m_vFormStack.elements() ; e.hasMoreElements() ;) {
                form = (PForm) e.nextElement() ;
                if (form != null) {
                    System.out.println("\t" + form.getFormName() + " -  " +
                            form.getApplication().getName()) ;
                }
            }
        }
    }

     /**
     *
     */
    private static PForm s_formPostedDown = null;

    public void firePingtelEvent(PButtonEvent event)
    {
        boolean bDown = false ;
        boolean bUp = false ;
        int     iType = event.getEventType() ;

        // Is this a button down?
        if ((iType == PButtonEvent.BUTTON_DOWN) ||
                (iType == PButtonEvent.KEY_DOWN)) {
            bDown = true ;
        }
        else if ((iType == PButtonEvent.BUTTON_UP) ||
                (iType == PButtonEvent.KEY_UP)) {
            bUp = true ;
        }

        if ((bUp == true) && (s_formPostedDown != null)) {
            // The shell has a chance to view this message, however,
            // the original form still MUST process this event
            ShellApp.getInstance().handlePingtelEvent(event) ;

            s_formPostedDown.postFormEvent(event) ;
            s_formPostedDown = null ;
        } else {
            // Shell aways gets first crack
            ShellApp.getInstance().handlePingtelEvent(event) ;
            if (!event.isConsumed()) {
                // Tell active app about the event
                PForm form = getActiveForm() ;
                if (form != null) {
                    form.postFormEvent(event) ;
                    if (bDown)
                        s_formPostedDown = form ;
                    else
                        s_formPostedDown = null ;
                }
            } else
                s_formPostedDown = null ;
        }
    }

    public PForm[] getFormChildren(PForm formParent)
    {
        Vector vChildren = new Vector()  ;
        PForm forms[] ;
        PForm form ;

        synchronized (m_vFormStack)
        {
            Enumeration e = m_vFormStack.elements();
            while (e.hasMoreElements())
            {
                form = (PForm) e.nextElement() ;
                if (form != null)
                {
                    if (form.getParentForm() == formParent)
                    {
                        vChildren.addElement(form) ;
                    }
                }
            }
        }

        forms = new PForm[vChildren.size()] ;
        vChildren.copyInto(forms) ;

        return forms;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /**
     * This helper is called whenever a form is activated (gets focus).  The
     * form is sized and tossed into the display.  A FE_FOCUS_GAINED event is
     * also fired off to it.
     */
    protected void doActivateForm(PForm form, PForm formOld)
    {
// System.out.println("+ FormManager::doActivateForm: " + form) ;

        if (FormState.getGUIFormState(form) != FormState.FGS_ACTIVE) {
            FormState.setGUIFormState(form, FormState.FGS_ACTIVE) ;

            Component comp = form.getComponent() ;
            comp.setBounds(0, 0, 160, 160) ;
            m_pnlDisplay.add(comp, -1) ;
            m_pnlDisplay.validate() ;

            form.postFormEvent(new PFormEvent(this, PFormEvent.FE_FOCUS_GAINED, formOld)) ;
            ApplicationManager.getInstance().activeForm(form) ;
            m_pnlDisplay.repaint() ;

            Logger.post("form", Logger.TRAIL_NOTIFICATION, "focus_gained", form.getFormName()) ;
        }
//System.out.println("- FormManager::doActivateForm: " + form) ;
    }


    /**
     * This helper is called whenever a component loses focus.  It is removed
     * from the display panel and fired off a FE_FOCUS_LIST event.
     *
     * @param form losing focus and becoming deactivated
     * @param form gaining focus as a result of the deactivation
     */
    protected void doDeactivateForm(PForm form, PForm formNew)
    {
//System.out.println("+ FormManager::doDeactivateForm: " + form) ;

        if (FormState.getGUIFormState(form) != FormState.FGS_DEACTIVE) {
            FormState.setGUIFormState(form, FormState.FGS_DEACTIVE) ;

            m_pnlDisplay.remove(form.getComponent()) ;
            form.postFormEvent(new PFormEvent(this, PFormEvent.FE_FOCUS_LOST, formNew)) ;
            Logger.post("form", Logger.TRAIL_NOTIFICATION, "focus_lost", form.getFormName()) ;
        }
//System.out.println("- FormManager::doDeactivateForm: " + form) ;
    }


    /**
     * Helper to fire off FE_FORM_CREATED notification
     */
    protected void doCreateForm(PForm form)
    {
//System.out.println("+ FormManager::doCreateForm: " + form) ;

        if (FormState.getFormState(form) != FormState.FS_SHOWING) {
            FormState.setFormState(form, FormState.FS_SHOWING) ;
            form.postFormEvent(new PFormEvent(this, PFormEvent.FE_FORM_CREATED, null)) ;
            ApplicationManager.getInstance().pushForm(form, form.getDisplayState()) ;
            Logger.post("form", Logger.TRAIL_NOTIFICATION, "created", form.getFormName()) ;
        }
//System.out.println("- FormManager::doCreateForm: " + form) ;
    }


    /**
     * Helper to fire off FE_FORM_DESTROYED notification
     */
    protected void doDestroyForm(PForm form)
    {
//System.out.println("+ FormManager::doDestroyForm: " + form) ;

        if (FormState.getFormState(form) != FormState.FS_UNKNOWN) {
            FormState.setFormState(form, FormState.FS_CLOSING) ;
            form.postFormEvent(new PFormEvent(this, PFormEvent.FE_FORM_DESTROYED, null)) ;
            ApplicationManager.getInstance().removeForm(form) ;
            Logger.post("form", Logger.TRAIL_NOTIFICATION, "destroyed", form.getFormName()) ;
        }

        // Clean up our state keeping records
        FormState.clearFormState(form) ;
        FormState.clearGUIFormState(form) ;
//System.out.println("- FormManager::doDestroyForm: " + form) ;
    }



}
