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
import java.util.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;

/**
 *
 *
 */
abstract public class PApplicationForm extends PAbstractForm
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** title bar control */
    protected PTitlebar   m_titlebar ;

    /** menu control */
    protected PBottomButtonBar m_menuControl ;

    /** where we actually place our data*/
    protected PBorderedContainer m_displayPane ;

    boolean m_bAutoUpdateDateTime ;

    icDateTimeUpdateCallback m_timeUpdater ;

    icWaitIconHandler m_waitIconHandler ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    public PApplicationForm(Application application)
    {
        super(application) ;

        addFormListener(new icFormAdapter()) ;
        m_bAutoUpdateDateTime = false ;

        createComponents() ;
        layoutComponents() ;
    }


    public PApplicationForm(Application application, String strFormName)
    {
        super(application, strFormName) ;

        addFormListener(new icFormAdapter()) ;
        m_bAutoUpdateDateTime = false ;

        createComponents() ;
        layoutComponents() ;
    }


    public PApplicationForm(PForm formParent)
    {
        super(formParent.getApplication(), formParent) ;

        addFormListener(new icFormAdapter()) ;
        m_bAutoUpdateDateTime = false ;

        createComponents() ;
        layoutComponents() ;
    }


    public PApplicationForm(PForm formParent, String strFormName)
    {
        super(formParent.getApplication(), formParent, strFormName) ;

        addFormListener(new icFormAdapter()) ;
        m_bAutoUpdateDateTime = false ;

        createComponents() ;
        layoutComponents() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * This method will enable or disable any "more" menu items or bottom bar
     * buttons on this form that match the specified action.  This makes no
     * attempt to modified any command bar buttons.
     *
     * @param bEnable the new enable state; true to enable or false to disable
     * @param strAction the action command whose state is changed
     */
    public void enableMenusByAction(boolean bEnable, String strAction)
    {
        // Enable/Disable items on the bottom button bar
        if (m_menuControl != null)
            m_menuControl.enableByAction(bEnable, strAction) ;

        // Enable/Disable items on the left menu
        PMenuComponent menuLeft = getLeftMenuComponent() ;
        if (menuLeft != null) {
            menuLeft.enableItemsByAction(bEnable, strAction) ;
        }

        // Enable/Disable items on the right menu
        PMenuComponent menuRight = getRightMenuComponent() ;
        if (menuRight != null) {
            menuRight.enableItemsByAction(bEnable, strAction) ;
        }
    }


    /**
     * STUB: The form automatically adds a call listener and invokes this
     * method on focus/form changes.  This allows derived classes to simply
     * define these methods.
     */
    public void onFocusGained(PForm formLosingFocus)
    {

    }


    /**
     * STUB: The form automatically adds a call listener and invokes this
     * method on focus/form changes.  This allows derived classes to simply
     * define these methods.
     */
    public void onFocusLost(PForm formGainingFocus)
    {

    }


    /**
     * STUB: The form automatically adds a call listener and invokes this
     * method on focus/form changes.  This allows derived classes to simply
     * define these methods.
     */
    public void onFormOpening()
    {
    }


    /**
     * STUB: The form automatically adds a call listener and invokes this
     * method on focus/form changes.  This allows derived classes to simply
     * define these methods.
     */
    public void onFormClosing()
    {
    }



    /**
     *
     */
    private void createComponents()
    {
        AppResourceManager resMgr = AppResourceManager.getInstance() ;
        // Create Titlebar
        m_titlebar = new PTitlebar(PTitlebar.APP_FRAME) ;
        m_titlebar.setIcon(resMgr.getImage("imgAppFrameIcon")) ;

        m_displayPane = new PBorderedContainer(PBorderedContainer.APP_BORDER) ;
        m_menuControl = new PBottomButtonBar() ;
    }


    /**
     *
     */
    public PBottomButtonBar getBottomButtonBar()
    {
        return m_menuControl ;
    }


    /**
     *
     */
    private void layoutComponents()
    {
        setLayout(null) ;

        add(m_titlebar) ;
        m_titlebar.setBounds(0, 0, 160, 26) ;
        add(m_displayPane) ;
        m_displayPane.setBounds(0, 25, 160, 108) ;
        add(m_menuControl) ;
        m_menuControl.setBounds(0, 133, 160, 26) ;
    }

    public void removeFromDisplayPanel(Component comp)
    {
        m_displayPane.remove(comp) ;
    }

    public Component addToDisplayPanel(Component comp)
    {
        return m_displayPane.add(comp) ;
    }


    public Component addToDisplayPanel(Component comp, Insets insets)
    {
        m_displayPane.setInsets(insets) ;
        return addToDisplayPanel(comp) ;
    }


    /**
     *
     */
    public void setTitle(String strTitle)
    {
        m_titlebar.setTitle(strTitle) ;
    }


    /**
     *
     */
    public String getTitle()
    {
        return m_titlebar.getTitle() ;
    }


    /**
     * Update the title bar to display the current time and date.  Optionally,
     * you can ask the routine to keep updating the date/time.
     *
     * @param bAutoUpdate If set the form will automatically update the
     *        time/date every minute.  If not set the form will perform
     *        a one time date/time set.
     */
    public void setTimeDateTitle(boolean bAutoUpdate)
    {
        Timer timer = Timer.getInstance() ;

        // Clear any previously set timers
        if (m_timeUpdater != null) {
            timer.removeTimers(m_timeUpdater) ;
            m_timeUpdater = null ;
        }

        // Set new timer
        m_bAutoUpdateDateTime = bAutoUpdate ;
        if ((m_bAutoUpdateDateTime) && isInFocus()) {
            m_timeUpdater = new icDateTimeUpdateCallback() ;

            // Figure out when the minute will change, and set the timer for 16 ms after it.
            int iFireNext = (int) (60000 - (Calendar.getInstance().getTime().getTime() % 60000) + 16) ;

            // We will set a one time timer and then the
            // icDateTimeUpdateCallback is smart enough to put us in auto
            // rearm mode.
            Timer.getInstance().resetTimer(iFireNext, m_timeUpdater, null,false) ;
        }

        // Set the time/date for now.
        m_titlebar.setTimeDate() ;
    }



    public void stopTimeDateAutoUpdate()
    {
        Timer timer = Timer.getInstance() ;

        m_bAutoUpdateDateTime = false ;

        // Clear any previously set timers
        if (m_timeUpdater != null) {
            timer.removeTimers(m_timeUpdater) ;
            m_timeUpdater = null ;
        }
    }


    /**
     *
     */
    public void setIcon(Image imgIcon)
    {
        if (m_waitIconHandler != null)
        {
            m_waitIconHandler.setIcon(imgIcon);
        }
        else
        {
            m_titlebar.setIcon(imgIcon) ;
        }
    }


    public void beginWaitIcon()
    {
        if (m_waitIconHandler == null)
        {
            m_waitIconHandler = new icWaitIconHandler() ;
            m_waitIconHandler.setIcon(m_titlebar.getIcon()) ;
        }
        m_waitIconHandler.begin() ;
    }


    public void endWaitIcon()
    {
        if (m_waitIconHandler != null)
        {
            m_waitIconHandler.end() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     * This class waits for time events tell it that time has changed.
     */
    private class icDateTimeUpdateCallback implements PActionListener
    {
        private boolean m_bInitialized = false ;

        public void actionEvent(PActionEvent event)
        {
            // Set Time
            m_titlebar.setTimeDate() ;

            // If we haven't initialized, reset the timer for a auto-rearm
            // every 60 ms
            if (m_bInitialized == false) {
                Timer.getInstance().resetTimer(60000, this, null, true) ;
                m_bInitialized = true ;
            }
        }
    }


    /**
     * This class waits for notification that it is time to refresh the icon
     * because we are in a busy state.
     */
    private class icWaitIconHandler implements PActionListener
    {
        private boolean m_bEnabled ;
        private boolean m_cursorIcon ;
        private Image   m_imgOriginal ;
        private Image   m_imgHourglass ;
        private int     m_iRefCount ;


        public icWaitIconHandler()
        {
            m_bEnabled = false ;
            m_cursorIcon = false ;
            m_imgOriginal = null ;
            m_imgHourglass = getImage("imgHourglass") ;
            m_iRefCount = 0 ;
        }

        public synchronized void begin()
        {
            if (m_iRefCount == 0)
            {
                setEnabled(true) ;
            }
            m_iRefCount++ ;
        }


        public synchronized void end()
        {
            m_iRefCount-- ;

            if (m_iRefCount <= 0)
            {
                m_iRefCount = 0 ;
                if (isEnabled())
                {
                    setEnabled(false) ;
                }
            }
        }


        public synchronized void setIcon(Image imgIcon)
        {
            m_imgOriginal = imgIcon ;
            if (!m_cursorIcon)
            {
                m_titlebar.setIcon(m_imgOriginal) ;
            }
        }


        public synchronized boolean isEnabled()
        {
            return m_bEnabled ;
        }


        public synchronized void actionEvent(PActionEvent event)
        {
            if (m_cursorIcon)
            {
                m_titlebar.setIcon(m_imgOriginal) ;
            }
            else
            {
                m_titlebar.setIcon(m_imgHourglass) ;
            }
            m_cursorIcon = !m_cursorIcon ;
        }


        public synchronized void setEnabled(boolean bEnable)
        {
            Timer timer = Timer.getInstance() ;

            if (bEnable != m_bEnabled)
            {
                if (bEnable)
                {
                    timer.addTimer(1500, this, null, true) ;
                }
                else
                {
                    timer.removeTimers(this) ;
                    if (m_cursorIcon)
                    {
                        m_titlebar.setIcon(m_imgOriginal) ;
                        m_cursorIcon = false ;
                    }
                }
                m_bEnabled = bEnable ;
            }
        }
    }


    protected class icFormAdapter extends PFormListenerAdapter
    {
        public void formOpening(PFormEvent event)
        {
            onFormOpening() ;
        }

        public void formClosing(PFormEvent event)
        {
            onFormClosing() ;
        }


        public void focusLost(PFormEvent event)
        {
            // If we are in auto update mode, then we need clear the
            // timer on focus loss.
            if ((m_bAutoUpdateDateTime) && (m_timeUpdater != null)) {
                Timer.getInstance().removeTimers(m_timeUpdater) ;
            }

            // Fire off default focus lost event
            onFocusLost(event.getFormGainingFocus()) ;
        }


        public void focusGained(PFormEvent event)
        {
            // If we are in auto update mode, then we need to reset
            // the timer on focus gain.
            if (m_bAutoUpdateDateTime) {
                setTimeDateTitle(true) ;
            }

            // Fire off default focus gained event
            onFocusGained(event.getFormLosingFocus()) ;
        }
    }
}
