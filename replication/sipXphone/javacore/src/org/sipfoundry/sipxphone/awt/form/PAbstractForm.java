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

import java.awt.Component;
import java.awt.Image;
import java.util.Enumeration;
import java.util.Vector;

import org.sipfoundry.util.AppResourceManager;
import org.sipfoundry.util.CountingSemaphore;
import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.Application;
import org.sipfoundry.sipxphone.awt.PActionItem;
import org.sipfoundry.sipxphone.awt.PContainer;
import org.sipfoundry.sipxphone.awt.PMenuComponent;
import org.sipfoundry.sipxphone.awt.event.PButtonEvent;
import org.sipfoundry.sipxphone.awt.event.PFormEvent;
import org.sipfoundry.sipxphone.awt.event.PFormListener;
import org.sipfoundry.sipxphone.sys.GenericEventQueue;
import org.sipfoundry.sipxphone.sys.Shell;
import org.sipfoundry.sipxphone.sys.app.ShellApp;
import org.sipfoundry.sipxphone.sys.app.shell.ShellEvent;


/**
 * PAbstractForm provides the default implementation that most (all) forms
 * derive from. Most of the PForm interface requirements are met here and
 * are reexposed as optional setXXX methods.
 *
 * @see PForm
 *
 * @author Robert J. Andreasen, Jr.
 */
public abstract class PAbstractForm extends PContainer implements PForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final int FS_INVALID  = 0 ;
    private static final int FS_OPENING  = 1 ;
    private static final int FS_OPENED   = 2 ;
    private static final int FS_CLOSING  = 3 ;
    private static final int FS_CLOSED   = 4 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** vector of form listeners */
    protected Vector m_vFormListeners = new Vector() ;

    /** return code passed to close() */
    protected int    m_iExitCode ;

    /** has this form been closed? */
    protected boolean m_bClosed ;

    /** The left menu component */
    protected PMenuComponent m_menuLeft = null ;
    /** The right menu component */
    protected PMenuComponent m_menuRight = null ;

    /** the source/originating application */
    protected Application m_application ;

    /** form specific properties file */
    protected String m_strPropertiesFile = null ;

    /** help text for this form */
    protected String m_strHelpText = null ;
    /** help title for the form */
    protected String m_strHelpTitle = null ;

    /** The title of the form */
    protected String m_strFormName = null ;

    /** The forms parent (optional) */
    protected PForm  m_formParent ;

    /** The forms display state (MODAL, MODELESS, ALWAYSONTOP) */
    protected int    m_iDisplayState = 0 ;
    
    protected PFormOverlay m_formOverlay = null;

    /** The form's event queue */
    private GenericEventQueue m_eventQueue ;

    /** The form's event processor */
    private icFormEventProcessor m_eventProcessor ;

    /** Object used to synchronized form startup.  We need to make sure that
        the event process doesn't start until the form is shown.  */
    private CountingSemaphore  m_semStartUp = new CountingSemaphore(0, false) ;

    /** Object used to synchronized form down down  */
    private CountingSemaphore  m_semShutDown = new CountingSemaphore(1, false) ;

    /** Queue of re-start request for this form.  If a showModeless request is
        received while closing down, we simple queue the request and invoke a
        call to showModeless once the form has closed. */
    private Vector m_vRestartModelessForm ;

    /** Is this form in the process of being closed down? */
    protected boolean m_bClosingDown = false ;

    /** Count used to number the form event queue threads under vxWorks */
    public static int FORM_EVENT_QUEUE_COUNT = 0;

    /** The current form state (FS_) */
    private int m_iFormState ;

    /** List of button hooks */
    private Vector m_vButtonHooks ;

    /** Should child forms inherit these button hooks? */
    protected boolean m_bInheritFormButtonHooks ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Default constructor requiring an application context.
     */
    public PAbstractForm(Application application)
    {
        m_application = application ;
        m_eventQueue = new GenericEventQueue() ;
        m_strFormName = null ;
        m_formParent = null ;
        m_bClosed = true ;
        m_iExitCode = -1 ;

        m_vRestartModelessForm = new Vector(3) ;
        m_iFormState = FS_INVALID ;
    }


    /**
     * Default constructor requiring an application context and form name
     */
    public PAbstractForm(Application application, String strFormName)
    {
        m_application = application ;
        m_eventQueue = new GenericEventQueue() ;
        m_strFormName = strFormName ;
        m_formParent = null ;
        m_bClosed = true ;
        m_iExitCode = -1 ;

        m_vRestartModelessForm = new Vector(3) ;
        m_iFormState = FS_INVALID ;
    }


    /**
     * Default constructor requiring an application context and form parents
     */
    public PAbstractForm(Application application, PForm formParent)
    {
        m_application = application ;
        m_formParent = formParent ;
        m_eventQueue = new GenericEventQueue() ;
        m_strFormName = null ;
        m_bClosed = true ;
        m_iExitCode = -1 ;

        m_vRestartModelessForm = new Vector(3) ;
        m_iFormState = FS_INVALID ;
        inheritFormButtonListeners(formParent) ;
    }


    /**
     * Default constructor requiring an application context, parent, and form
     * name.
     */
    public PAbstractForm(Application application, PForm formParent,
            String strFormName)
    {
        m_application = application ;
        m_formParent = formParent ;
        m_eventQueue = new GenericEventQueue() ;
        m_strFormName = strFormName ;
        m_bClosed = true ;
        m_iExitCode = -1 ;

        m_vRestartModelessForm = new Vector(3) ;
        m_iFormState = FS_INVALID ;
        inheritFormButtonListeners(formParent) ;
    }



    /**
     * Return the native component responsible for rendering this form.
     */
    public Component getComponent()
    {
        return this ;
    }


    /**
     * Convenience method that invokes closeForm with a parameter of 0.
     */
    public void closeForm()
    {
        closeForm(0) ;
    }


    /**
     * Close the form with the specified exit code
     *
     * @param iExitCode the return code returned through showModal()
     */
    public void closeForm(int iExitCode)
    {
        // If the form is opening or opened, then post the CLOSE
        m_semShutDown.down() ;
        if ((m_iFormState == FS_OPENING) || (m_iFormState == FS_OPENED)) {
            m_iFormState = FS_CLOSING ;
            m_iExitCode = iExitCode ;
            ShellApp.getInstance().postEvent(new ShellEvent(ShellEvent.SE_CLOSEFORM, this, this, null)) ;
        }

        // Warning if INVALID or already closed
        if ((m_iFormState == FS_CLOSED) && (m_iFormState == FS_INVALID)) {
            System.out.println("WARNING: Form is already closed: " + this) ;
            Thread.dumpStack() ;
        }
        m_semShutDown.up() ;
    }


    /**
     * Is this form in a closed (none-displayed) state? This is true when
     * the form is first created but not yet shown, and after
     * closeForm() has been invoked.
     */
    public boolean isClosed()
    {
        return m_bClosed ;
    }


    /**
     * Set the left column of menu items that display when a user
     * displays the pingtel menu.
     */
    public void setLeftMenu(PActionItem[] items)
    {
        m_menuLeft = new PMenuComponent(items) ;
    }


    /**
     * Called by the framework when the system needs a list of action items
     * to populate the left side of the pingtel menu.
     */
    public PActionItem[] getLeftMenu()
    {
        PActionItem items[] = null ;

        if (m_menuLeft != null) {
            items = m_menuLeft.getItems() ;
        }

        return items ;
    }


    /**
     * Get the left menu component for this form. Each form can populate a
     * left and right menu. These two menus display when the end user
     * displays the "Menu" tab off the task manager/"More" button. The data
     * return from this method is the same as <i>getLeftMenu()</i>, however, wrapped
     * in a data object with various convenience methods.
     */
    public PMenuComponent getLeftMenuComponent()
    {
        if (m_menuLeft == null)
            m_menuLeft = new PMenuComponent() ;

        return m_menuLeft ;
    }


    /**
     * Set the right column of menu items that display when a user
     * displays the pingtel menu.
     */
    public void setRightMenu(PActionItem[] items)
    {
        m_menuRight = new PMenuComponent(items) ;
    }



    /**
     * Called by the framework when the system needs a list of action items
     * to populate the right side of the pingtel menu.
     */
    public PActionItem[] getRightMenu()
    {
        PActionItem items[] = null ;

        if (m_menuRight != null) {
            items = m_menuRight.getItems() ;
        }

        return items ;
    }


    /**
     * Get the right menu component for this form. Each form can populate a
     * left and right menu. These two menus display together when the end user
     * displays the "Menu" tab off the task manager/"More" button. The data
     * return from this method is the same as <i>getLeftMenu()</i>, however, wrapped
     * in a data object with various convenience methods.
     */
    public PMenuComponent getRightMenuComponent()
    {
        if (m_menuRight == null)
            m_menuRight = new PMenuComponent() ;

        return m_menuRight ;
    }



    /**
     * Set a specific string resource properties file for this application.
     * The specified properties file is loaded and pushed
     * to the top of the stack whenever the form moves into focus.
     */
    public void setStringResourcesFile(String strPropertiesFile)
    {
        m_strPropertiesFile = strPropertiesFile ;
        if( m_strPropertiesFile != null ){
            AppResourceManager.getInstance()
                  .addStringResourceFile( m_strPropertiesFile, m_application);
        }
    }


    /**
     * Set the help text to display when the end user displays the help
     * tab while this form is in focus.
     *
     * @param strText This form's context specific help text
     */
    public void setHelpText(String strText)
    {
        m_strHelpText = strText ;
    }


    /**
     * Set the help text and title to display when the end user
     * displays the help tab while this form is in focus.
     *
     * @param strText This form's context specific help text
     * @param strTitle This form's context specific help title
     */
    public void setHelpText(String strText, String strTitle)
    {
        m_strHelpText = strText ;
        m_strHelpTitle = strTitle ;
    }


    /**
     * Get this form's context-specific help text. This method is called by
     * the framework when the help tab displays and this form is in focus.
     */
    public String getHelpText()
    {
        if (m_strHelpText == null) {
            m_strHelpText = AppResourceManager.getInstance().getString("lblDefaultNoHelpText") ;
        }

        return m_strHelpText ;
    }


    /**
     * Get this form's context specific help title. This method is called by
     * the framework when the help tab displays and this form is in focus.
     */
    public String getHelpTitle()
    {
        if (m_strHelpTitle == null) {
            m_strHelpTitle = AppResourceManager.getInstance().getString("lblDefaultNoHelpTitle") ;
        }
        return m_strHelpTitle ;
    }


    /**
     * Look up a string from the resource manager. This is added as a helper
     * and calls AppResourceManager directly.
     */
    public String getString(String strKey)
    {
        return AppResourceManager.getInstance().getString(strKey) ;
    }


    /**
     * Look up an image from the resource manager. This is added as a helper
     * and calls AppResourceManager directly.
     */
    public Image getImage(String strKey)
    {
        return AppResourceManager.getInstance().getImage(strKey) ;
    }


    /**
     * Add a form listener to be notified of focus changes and initial
     * startup/destruction.
     */
    public void addFormListener(PFormListener listener)
    {
        synchronized (m_vFormListeners) {
            if (!m_vFormListeners.contains(listener)) {
                m_vFormListeners.insertElementAt(listener, 0) ;
            }
        }
    }


    /**
     * Remove a form listener.
     */
    public void removeFormListener(PFormListener listener)
    {
        synchronized (m_vFormListeners) {
            m_vFormListeners.removeElement(listener) ;
        }
    }


    /**
     * @deprecated do not expose.  This is used by the framework to post events
     * to this form.
     */
    public void postFormEvent(Object objEvent)
    {
        if (m_eventProcessor == null) {
            System.out.println("WARNING: Posting Event to a form without an event processor: " + m_strFormName) ;
        }
//System.out.println("POSTING EVENT to " + m_strFormName + ": " + objEvent) ;
        m_eventQueue.postEvent(objEvent) ;
    }


    /**
     * Get the form's parents form.
     */
    public PForm getParentForm()
    {
        return m_formParent ;
    }


    /**
     * What is the application associated with this form?
     */
    public Application getApplication()
    {
        return m_application ;
    }


    /**
     * Display this form in a modeless state where control is immediately
     * returned to the caller (non-blocking). Forms that are already
     * displayed, but not the active form, are placed into focus.
     *
     * @return true if a new form is displayed else false.
     */
    public synchronized boolean showModeless()
    {
        return showModeless(false) ;
    }


    /**
     * Display this form in a modeless state where control is immediately
     * returned to the caller (non-blocking). Forms that are already
     * displayed, but not the active form, are placed into focus.
     *
     * @param  bAlwaysOnTop boolean that forces this form to be placed on top
     *         of all other forms until this form is closed.
     *
     *
     * @return true if a new form is displayed else false.
     */
    public synchronized boolean showModeless(boolean bAlwaysOnTop)
    {
        boolean bNewFormShown = false ;

        // If the form is currently closing, queue the request until after the
        // form closes.
        if (m_iFormState == FS_CLOSING)
        {
            synchronized (m_vRestartModelessForm)
            {
                m_vRestartModelessForm.addElement(new Boolean(bAlwaysOnTop)) ;
            }
            return true ;
        }


        // If the form is not opening, then process this request.  Otherwise,
        // we can simple ignore the request because the form will be shown
        // shortly.
        if (m_iFormState != FS_OPENING) {

            // If the form state is closed or invalid, then we are creating
            // a new form.
            if ((m_iFormState == FS_CLOSED) || (m_iFormState == FS_INVALID)) {
//                System.out.println("Showing new form!");
                bNewFormShown = true ;
            }


            // Load the form's resource file
            if (m_strPropertiesFile != null) {
                AppResourceManager.getInstance().topStringResourceFile( m_strPropertiesFile, m_application);
            }


            // Set the form state
            if (bAlwaysOnTop) {
                m_iDisplayState = PForm.MODELESS | PForm.ALWAYSONTOP ;
            } else {
                m_iDisplayState = PForm.MODELESS ;
            }

            // Create the event process if we don't already have one?  (can this happen??)
            if (m_eventProcessor == null) {
                m_eventProcessor = new icFormEventProcessor(this) ;
            }

            // The shell app will dispatch this event to the form manager for
            // processing.
            ShellApp.getInstance().postEvent(
                    new ShellEvent(ShellEvent.SE_SHOWFORM, this, this,
                    new Integer(m_iDisplayState))) ;


            m_iFormState = FS_OPENING ;


            // Make sure the event process is also started.
            if (!m_eventProcessor.isAlive()) {
                m_eventProcessor.start() ;


                // Wait for the processor to start up
                m_semStartUp.down() ;
            }
        }
        return bNewFormShown ;
    }



    /**
     * Display this form in a modal state where the control is not returned
     * to the caller until the form is closed by the end user. If the form
     * is already being displayed, then an IllegalStateException exception is
     * thrown.
     *
     * @return The value specified when invoking closeForm.
     *
     * @exception IllegalStateException thrown if the form is already being
     *            displayed.
     */
    public synchronized int showModal()
        throws IllegalStateException
    {
        return showModal(false) ;
    }


    /**
     * Display this form in a modal state where the control is not returned
     * to the caller until the form is closed by the end user. If the form
     * is already being displayed, then an IllegalStateException exception is
     * thrown.
     *
     * @param  bAlwaysOnTop boolean that forces this form to be placed on top
     *         of all other forms until this form is closed.
     *
     * @return The value specified when invoking closeForm.
     *
     * @exception IllegalStateException thrown if the form is already being
     *            shown.
     *
     */
    public synchronized int showModal(boolean bAlwaysOnTop)
        throws IllegalStateException
    {

        // Load the form's resource file
        if (m_strPropertiesFile != null) {
            AppResourceManager.getInstance()
                .topStringResourceFile(m_strPropertiesFile, m_application);
        }

        // Set the form state
        if (bAlwaysOnTop) {
            m_iDisplayState = PForm.MODAL | PForm.ALWAYSONTOP ;
        } else {
            m_iDisplayState = PForm.MODAL ;
        }


        // Create the event process
        if ((m_eventProcessor != null) && m_eventProcessor.isAlive()) {
            throw new IllegalStateException("form is already being displayed") ;
        } else {
            m_eventProcessor = new icFormEventProcessor(this) ;

            m_iFormState = FS_OPENING ;

            // The shell app will dispatch this event to the form manager for
            // processing.
            ShellApp.getInstance().sendEvent(
                        new ShellEvent(ShellEvent.SE_SHOWFORM, this, this,
                        new Integer(m_iDisplayState))) ;
            m_iFormState = FS_OPENED ;

            m_eventProcessor.start() ;
            ShellApp.getInstance().safeJoin(m_eventProcessor) ;
        }
        return m_iExitCode ;
    }


    /**
     * Is this form in focus?
     */
    public boolean isInFocus()
    {
//System.out.println(";;;; isInFocus: " + m_strFormName) ;
        boolean bRC = false ;

        ShellEvent event = new ShellEvent(ShellEvent.SE_FOCUSCHECK, this, this, null) ;
        ShellApp.getInstance().sendEvent(event) ;
        Boolean boolRC = (Boolean) event.getReturnValue() ;
        if (boolRC != null) {
            bRC = boolRC.booleanValue() ;
        }

        return bRC ;
    }


    /**
     * Is this form currently being displayed?
     */
    public boolean isStacked()
    {
//System.out.println(";;;; isStacked") ;
        boolean bRC = false ;

        ShellEvent event = new ShellEvent(ShellEvent.SE_STACKCHECK, this, this, null) ;
        ShellApp.getInstance().sendEvent(event) ;
        Boolean boolRC = (Boolean) event.getReturnValue() ;
        if (boolRC != null) {
            bRC = boolRC.booleanValue() ;
        }

        return bRC ;
    }


    /**
     * What display state is this form in? MODAL or MODELESS.
     */
    public int getDisplayState()
    {
        return m_iDisplayState ;
    }


    /**
     * Set the form name
     */
    public void setFormName(String strFormName)
    {
        m_strFormName = strFormName ;
    }


    /**
     * Get the form name
     */
    public String getFormName()
    {
        if (m_strFormName == null)
            return "unknown" ;
        else
            return m_strFormName ;
    }


    /**
     * Get the form's string representation (form name)
     */
    public String toString()
    {
        if (m_strFormName == null)
        {
            //was originally calling super().toString but when the versioncheck app
            //is visible and then the phone goes off then on hook,
            //this line will lock.  So we changed it to this, and it works.
            //go figure.  If YOU know why, feel free to fix it.
            return getClass().getName()+'@'+Integer.toHexString(hashCode());
        }
        else
        {
            return getFormName() ;
        }
    }


    /**
     *
     * @param bInheritFormButtonHooks Should child forms inherit any button
     *        hooks. NOTE: removeFormButtonHook will *NOT* remove hooks added
     *        to button hooks.
     */
    public void addFormButtonHook(PFormButtonHook hook, boolean bInheritFormButtonHooks)
    {
        m_bInheritFormButtonHooks = bInheritFormButtonHooks ;

        if (hook == null)
            throw new IllegalArgumentException() ;

        if (m_vButtonHooks == null)
            m_vButtonHooks = new Vector() ;

        if (!m_vButtonHooks.contains(hook))
        {
            synchronized (m_vButtonHooks)
            {
                m_vButtonHooks.addElement(hook);
            }
        }
    }


    /**
     *
     */
    public void removeFormButtonHook(PFormButtonHook hook)
    {
        if (hook == null)
            throw new IllegalArgumentException() ;

        if (m_vButtonHooks != null)
        {
            synchronized (m_vButtonHooks)
            {
                m_vButtonHooks.removeElement(hook) ;
            }
        }
    }


    /**
     *
     */
    public PFormButtonHook[] getFormButtonHooks()
    {
        PFormButtonHook[] hooks ;

        if ((m_vButtonHooks != null) && (m_vButtonHooks.size() > 0))
        {
            synchronized (m_vButtonHooks)
            {
                hooks = new PFormButtonHook[m_vButtonHooks.size()] ;
                m_vButtonHooks.copyInto(hooks) ;
            }
        }
        else
        {
            hooks = new PFormButtonHook[0] ;
        }
        return hooks ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Fire off a form event to all interested listeners.  Listeners must be
     * added via the addFormListener interface.
     */
    protected void fireFormEvent(PFormEvent event)
    {
        // fire off the event to all the listeners
        for (int i=0; i<m_vFormListeners.size(); i++) {
            PFormListener listener = (PFormListener) m_vFormListeners.elementAt(i) ;
            if (listener != null) {
                try {
                    switch (event.getType()) {
                        case PFormEvent.FE_FORM_CREATED:
                            listener.formOpening(event) ;
                            break ;
                        case PFormEvent.FE_FORM_DESTROYED:
                            listener.formClosing(event) ;
                            m_bClosed = true ;
                            break ;
                        case PFormEvent.FE_FOCUS_GAINED:
                            //top the resource file if it is set when the form comes
                            //into focus.
                            if(  m_strPropertiesFile != null ){
                              AppResourceManager.getInstance()
                                 .topStringResourceFile( m_strPropertiesFile, m_application);
                            }
                            listener.focusGained(event) ;
                            break ;
                        case PFormEvent.FE_FOCUS_LOST:
                            listener.focusLost(event) ;
                            break ;
                    }
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    protected void inheritFormButtonListeners(PForm formParent)
    {
        if (formParent instanceof PAbstractForm)
        {
            if (((PAbstractForm) formParent).m_bInheritFormButtonHooks)
            {
                PFormButtonHook hooks[] = ((PAbstractForm) formParent).getFormButtonHooks() ;
                for (int i=0; i<hooks.length; i++)
                {
                    addFormButtonHook(hooks[i], true) ;
                }
            }
        }
    }

    /**
     * Sets the PFormOverlay object associated with this form
     */
    public void setFormOverlay(PFormOverlay overlay)
    {
    	this.m_formOverlay = overlay;
    }
    
    /**
     * Returns the PFormOverlay object associated with this form.
     * If the current overlay object is null, a new default overlay is created,
     * initialized, and returned.
     */
    public PFormOverlay getFormOverlay()
    {
    	if (m_formOverlay == null)
    	{
    		m_formOverlay = new PFormOverlay();
    		m_formOverlay.initialize();
    	}
    	return m_formOverlay;
    }

    /**
     * Overloaded finilize that cleans up form resources
     */
    protected void finalize() throws Throwable
    {
// System.out.println("Finalizing form: "+ m_strFormName) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     *  This class serves as the event process for all form and button events.
     *  This form also enforces a few assumptions:
     *  <ul>
     *      <li>Event Queue does not show down if a button down is received
     *          without a button up.</li>
     *      <li>Event Queue will not process button events until the form has
     *          been created</li>
     *  </ul>
     */
    protected class icFormEventProcessor extends Thread
    {
    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
        /** Form who owns this event queue */
        protected PForm m_formOwner ;

        /** Queued button events (button events receieved before the form
            was created. */
        protected GenericEventQueue m_eqButtons = null ;

        /** Should the event processor stop processing?  Is it done */
        protected boolean m_bStopProcessing = false ;
        /** Has the form been offical created? */
        protected boolean m_bFormCreated = false ;
        /** Have we receieved a button down, however, still awaiting a button
            up */
        protected boolean m_bAwaitingButtonUp = false ;


    //////////////////////////////////////////////////////////////////////////
    // Constrction
    ////
        /**
         * Constructs a form event queue with the supplied form owner
         *
         * @param formOwner The form that this event queue serves
         */
        public icFormEventProcessor(PForm formOwner)
        {
            super("form-evtq-"+Integer.toString(++FORM_EVENT_QUEUE_COUNT)) ;
            m_formOwner = formOwner ;

            // We want button presses to be responsive, so make sure we have
            // a slight edge over normal threads.  That edge is assured by
            // increasing our priority by one unit.
            try {
                setPriority(Thread.NORM_PRIORITY+1) ;
            } catch (Exception e) {
                System.out.println("Error setting Priority: ") ;
                SysLog.log(e) ;
            }
        }


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
        /**
         * Process the event loop until 'done'.
         */
        public void run()
        {
            Object            objEvent = null ;

//System.out.println("STARTING UP EVENT PROCESSOR FOR " + m_strFormName) ;

            // Note that we have started up the event queue
            m_iFormState = FS_OPENED ;
            m_semStartUp.up();

            // Process events
            do {
                try {
                    objEvent = m_eventQueue.getNextEvent() ;
                    if (objEvent != null) {
//System.out.println("BEGIN PROCESSING EVENT to " + m_strFormName + ": " + objEvent) ;

                        // Dispatch FORM events
                        if (objEvent instanceof PFormEvent) {
                            processFormEvent((PFormEvent) objEvent) ;
                            objEvent = null ;
                        }
                        // Dispatch BUTTON Events
                        else if (objEvent instanceof PButtonEvent) {
                            processButtonEvent((PButtonEvent) objEvent) ;
                            objEvent = null ;
                        }
                        // Otherwise this is an unknown event
                        else {
                            System.out.println("PANIC: Unknown event passed to form: " + objEvent) ;
                            objEvent = null ;
                        }
                    }
                } catch (Exception e) {
                    SysLog.log(e) ;
                }
//System.out.println("END PROCESSING EVENT to " + m_strFormName + ": " + objEvent) ;
            } while (!m_bStopProcessing || m_bAwaitingButtonUp) ;

            // Clean up and notify threads waiting for us to shutdown
            m_formOwner = null ;
            m_eventProcessor = null ;

//System.out.println("SHUT DOWN EVENT PROCESSOR FOR " + m_strFormName) ;
            objEvent = null ;

            m_iFormState = FS_CLOSED ;


            synchronized (m_vRestartModelessForm)
            {
                if (m_vRestartModelessForm.size() > 0)
                {
                    Boolean boolAlwaysOnTop =
                           (Boolean) m_vRestartModelessForm.firstElement() ;
                    m_vRestartModelessForm.removeElementAt(0) ;
                    showModeless(boolAlwaysOnTop.booleanValue()) ;
                }
            }
        }


    //////////////////////////////////////////////////////////////////////////
    // Impelmentation
    ////

        /**
         * Process form events.  For the most part this entails firing off the
         * form events to interested listeners.
         */
        protected void processFormEvent(PFormEvent event)
        {
            // Inform interested Parties
            fireFormEvent(event) ;

            // Take internal actions
            switch (event.getType()) {
                case PFormEvent.FE_FORM_CREATED:

                    // Note that we have created the form and flush/process any
                    // button events received before this point
                    m_bFormCreated = true ;
                    if (m_eqButtons != null) {
                        while (!m_eqButtons.isEmpty()) {
                            try {
                                PButtonEvent evtButton = (PButtonEvent)
                                        m_eqButtons.getNextEvent() ;
                                processButtonEvent(evtButton) ;
                            } catch (Exception e) {
                                SysLog.log(e) ;
                            }
                        }
                    }
                    m_eqButtons = null ;

                    break ;
                case PFormEvent.FE_FORM_DESTROYED:
                    m_bStopProcessing = true ;

                    //remove the resource file loaded by this form
                    try {
                        // Clean any resources that we may loaded
                        if (m_strPropertiesFile != null) {
                            AppResourceManager.getInstance().removeStringResourceFile(m_strPropertiesFile)  ;
                        }
                    } catch (Throwable t) {
                        System.out.println("PANIC: Exception thrown while trying to clean up form!") ;
                        SysLog.log(t);
                    }

                    break ;
                case PFormEvent.FE_FOCUS_GAINED:
                    break ;
                case PFormEvent.FE_FOCUS_LOST:
                    break ;
            }
        }


        /**
         * Process and inbound button events.  If the form has not been
         * created yet, then queue those button events.
         */
        protected void processButtonEvent(PButtonEvent event)
        {
            // Do not allow the event process to close if we have an
            // outstanding button event.  We never want to receive an DOWN and
            // never an UP
            int iType = event.getEventType() ;
            if ((iType == PButtonEvent.BUTTON_DOWN) ||
                    (iType == PButtonEvent.KEY_DOWN))
            {
                m_bAwaitingButtonUp = true ;
            }
            else
            {
                m_bAwaitingButtonUp = false ;
            }

            // If the form has not been created yet, then queue up the button
            // presses in another queue
            if (!m_bFormCreated)
            {
                if (m_eqButtons == null)
                    m_eqButtons = new GenericEventQueue() ;
                m_eqButtons.postEvent(event) ;
            }
            else
            {
                boolean bEventConsumed = false ;
                if (m_vButtonHooks != null)
                {
                    Enumeration enum = m_vButtonHooks.elements() ;
                    while (enum.hasMoreElements() && !bEventConsumed)
                    {
                        PFormButtonHook hook = (PFormButtonHook) enum.nextElement() ;
                        bEventConsumed = hook.buttonEvent(event, m_formOwner) ;
                    }
                }


                if (!bEventConsumed)
                {
                    /*
                     * Dispatch the event to the container and let the event
                     * trickle downwards. Containers should let their
                     * children / components service the notification before
                     * acting themself.
                     */
                    Component comp = getComponent() ;
                    if (comp instanceof PContainer)
                    {
                        ((PContainer) comp).processPingtelEvent((PButtonEvent) event) ;
                    }
                }
            }
        }
    }
   
}
