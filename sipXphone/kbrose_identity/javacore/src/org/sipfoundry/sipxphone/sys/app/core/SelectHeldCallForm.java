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


package org.sipfoundry.sipxphone.sys.app.core ;

import java.awt.* ;
import java.util.Vector ;
import java.text.* ;

import javax.telephony.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.* ;


/**
 * The select held call form prompts the end user to select a held call.  A
 * shoeModal return value of OK indiciates the user selected a held call,
 * while a showModal return value of CANCEL indicates the process should be
 * aborted.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SelectHeldCallForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int CANCEL  = 0 ;
    public static final int OK = 1 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** list control used in the simple form */
    private PList m_listControl = null ;
    /** list control data model */
    private PDefaultListModel m_listModel = null ;
    /** selected call */
    private PCall m_selectedCall = null ;
    /** are we currently in focus? */
    private boolean m_bInFocus = false ;
    /** action command dispatcher */
    private icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;
    /** listener for callReleased / callDestroyed events */
    private icConnectionListener m_connectionListener ;
    private PActionListener      m_tickHandler ;
    private String               m_strOKLabel ;
    private String               m_strOKHint ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Creates the SelectHeldCallForm with the specified application, title,
     * ok button label and ok hint text.
     */
    public SelectHeldCallForm(  Application application,
                                String strTitle,
                                String strOKLabel,
                                String strOKHint)
    {
        super(application, "Held Calls") ;

        setTitle(strTitle) ;

        initControls() ;
        layoutControls() ;
        initBottomButtonBar(strOKLabel, strOKHint) ;

        m_strOKLabel = strOKLabel ;
        m_strOKHint = strOKHint ;
        initializeMenus(m_strOKLabel, m_strOKHint) ;

        setHelpText(getString("hold"), getString("hold_title")) ;

        m_connectionListener = new icConnectionListener() ;
        populateHeldCalls() ;
        m_tickHandler = new icTimerTickHandler() ;
        addFormListener(new icFormListener()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        m_bInFocus = true ;
        Timer.getInstance().resetTimer(1000, m_tickHandler, null) ;
    }


    /**
     * called when the form has lost focus.
     *
     * @param formGainingFocus form that is gaining focus over this form
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        Timer.getInstance().removeTimers(m_tickHandler) ;
        m_bInFocus = false ;
    }


    /**
     * Gets the call selected by the end user in this form.
     *
     * @return selected held call or null if no calls were selected.
     */
    public PCall getSelectedCall()
    {
        return m_selectedCall ;
    }


    /**
     * Invoked when the end user presses the button in location B3.  This
     * method stores the selected call and then closes the form with a
     * return value of OK
     */
    public void onOk()
    {
        m_selectedCall = (PCall) m_listControl.getSelectedElement() ;
        closeForm(OK) ;
    }


    /**
     * Invoked when the end user presses the button in location B2.  This
     * method clears the selected call and then closes the form with a
     * return value of CANCEL
     */
    public void onCancel()
    {
        m_selectedCall = null ;
        closeForm(CANCEL) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Initialize our controls.
     */
    private void initControls()
    {
        m_listControl = new PList() ;
        m_listModel = new PDefaultListModel() ;
        m_listControl.setListModel(m_listModel) ;
        m_listModel.addListDataListener(new icListDataListener()) ;
        m_listControl.setItemRenderer(new DefaultHeldCallRenderer()) ;
        m_listControl.addActionListener(m_commandDispatcher) ;
    }


    /**
     * Physically layout or components
     */
    private void layoutControls()
    {
        addToDisplayPanel(m_listControl, new Insets(1,1,1,1)) ;
    }


    /**
     * Fill list model with held calls
     */
    private void populateHeldCalls()
    {
        PCallManager callManager = Shell.getCallManager() ;

        clearHeldCallsList() ;
        PCall heldCalls[] = callManager.getCallsByState(PCall.CONN_STATE_HELD) ;
        sortHeldCalls(heldCalls) ;
        for (int i=0; i<heldCalls.length; i++) {
            m_listModel.addElement(heldCalls[i]) ;
            heldCalls[i].addConnectionListener(m_connectionListener) ;
        }
    }


    private void clearHeldCallsList()
    {
        // First remove any/all call listeners
        m_listModel.getSize() ;
        for (int i=0; i<m_listModel.getSize(); i++) {
            PCall call = (PCall) m_listModel.getElementAt(i) ;
            call.removeConnectionListener(m_connectionListener) ;
        }

        // Now empty the list
        m_listModel.removeAllElements() ;
    }



    /**
     * Initialize the button button bar
     */
    private void initBottomButtonBar(String strLabelOK,
                                       String strHintOK)
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionCancel = new PActionItem(new PLabel("Cancel"),
            getString("hint/core/system/cancelform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;

        PActionItem actionOk = new PActionItem(new PLabel(strLabelOK),
            strHintOK,
            m_commandDispatcher,
            m_commandDispatcher.ACTION_OK) ;

        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;
    }


    /**
     * Initialize the menus that are displayed when a user hits the pingtel
     * key and selects the menu tab.
     */
    private void initializeMenus(String strLabelOK,
                                 String strHintOK)
    {
        PActionItem items[] ;

        // Initialize left menu

        items = new PActionItem[2] ;

        items[0] = new PActionItem(new PLabel(strLabelOK),
            strHintOK,
            m_commandDispatcher,
            m_commandDispatcher.ACTION_OK) ;

        items[1] = new PActionItem(new PLabel("Cancel"),
            getString("hint/core/system/cancelform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;

        setLeftMenu(items) ;
    }


    /**
     * Helper method that swaps the elements within an array of calls.
     */
    private static void swapCallsInArray(PCall calls[], int pos1, int pos2)
    {
        PCall holder ;

        holder = calls[pos1] ;
        calls[pos1] = calls[pos2] ;
        calls[pos2] = holder ;
    }


    /**
     * Sort the passed array of calls by duration.  Calls are be placed in
     * ascending order with the shortest duration (least time on hold) in array
     * index 0 and long duration (most time on hold) in the last index.  Calls
     * that are not on hold are sorted before all held calls.
     *
     * @param the array of calls to sort
     */
    private static void sortHeldCalls(PCall calls[])
    {
        // Yes, this is an grossly inefficient sorting algorithm, however, we
        // should never have more than a few calls on hold.
        for (int i=0; i<calls.length; i++) {
            for (int j=0; j<calls.length; j++) {
                if (calls[j].getHoldDuration() < calls[i].getHoldDuration()) {
                    swapCallsInArray(calls, i, j) ;
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

    /**
     * Listens for action commands and hands off to appropriate onXXX handler
     */
    private class icCommandDispatcher implements PActionListener
    {
        /** action command: exit from the release calls form */
        public final String ACTION_CANCEL = "action_cancel" ;
        /** action command: release a call from hold */
        public final String ACTION_OK = "action_ok" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_OK)) {
                onOk() ;
            } else  if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                onOk() ;
            }
        }
    }


    /**
     * Forces renderers to redraw on timer heart beat.
     */
    private class icTimerTickHandler implements PActionListener
    {
        public void actionEvent(PActionEvent event)
        {
            if (m_bInFocus == true) {
                m_listControl.refreshRenderers() ;
                Timer.getInstance().resetTimer(1000, this, null) ;
            }
        }
    }


    /**
     * Connection listener that looks for callReleased and/or callDestroyed
     * messages.  If found, those calls are moved from the held call list
     */
    private class icConnectionListener extends PConnectionListenerAdapter
    {
        public void callReleased(PConnectionEvent event)
        {
            m_listModel.removeElement(event.getCall()) ;
        }


        public void callDestroyed(PConnectionEvent event)
        {
            m_listModel.removeElement(event.getCall()) ;
        }
    }


    private class icFormListener implements PFormListener
    {
        public void formOpening(PFormEvent event) { }
        public void focusGained(PFormEvent event) { }
        public void focusLost(PFormEvent event) { }


        public void formClosing(PFormEvent event)
        {
            clearHeldCallsList() ;
        }
    }


    /**
     * This list data listener looks for model changes and updates the
     * "Release" button appropriately.
     */
    private class icListDataListener implements PListDataListener
    {
        public void intervalAdded(PListDataEvent e)
        {
            enableMenusByAction((m_listModel.getSize() > 0), m_commandDispatcher.ACTION_OK) ;
        }

        public void intervalRemoved(PListDataEvent e)
        {
            enableMenusByAction((m_listModel.getSize() > 0), m_commandDispatcher.ACTION_OK) ;
        }

        public void contentsChanged(PListDataEvent e) { }
    }
}

