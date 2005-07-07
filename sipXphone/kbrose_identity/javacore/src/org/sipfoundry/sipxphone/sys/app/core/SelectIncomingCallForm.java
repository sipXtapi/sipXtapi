/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/SelectIncomingCallForm.java#2 $
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

import javax.telephony.phone.* ;
import java.awt.Insets ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.sys.AudioSourceControl;
import org.sipfoundry.sipxphone.sys.Shell;
import org.sipfoundry.sipxphone.sys.app.CoreApp;
import org.sipfoundry.sipxphone.service.Timer ;


/**
 * The select incoming call form prompts the end user to select an incoming
 * ringing call.  A showModal return value of OK indiciates the user selected an
 * incoming ringing call, while a showModal return value of CANCEL indicates the process
 * should be aborted.
 *
 */
public class SelectIncomingCallForm extends PApplicationForm
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
     * Creates the SelectIncomingCallForm with the specified application, title,
     * ok button label and ok hint text.
     */
    public SelectIncomingCallForm(  Application application,
                                String strTitle,
                                String strOKLabel,
                                String strOKHint)
    {
        super(application, "Incoming Calls") ;

        setTitle(strTitle) ;

        initControls() ;
        layoutControls() ;
        initBottomButtonBar(strOKLabel, strOKHint) ;

        m_strOKLabel = strOKLabel ;
        m_strOKHint = strOKHint ;
        initializeMenus(m_strOKLabel, m_strOKHint) ;

        setHelpText(getString("incoming_calls"), getString("incoming_calls_title")) ;

        m_connectionListener = new icConnectionListener() ;
        populateIncomingCalls() ;
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
     * copycat from onIgnore of IncomingCallForm
     */
    public void onIgnore()
    {
        try {
             PCall selectedCall = (PCall) m_listControl.getSelectedElement() ;
             ((CoreApp) getApplication()).ignoreCall( selectedCall);
             m_listModel.removeElement( selectedCall );
            AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;
            selectedCall.stopSound() ;
            selectedCall.stopTone() ;
            asc.enableRinger(false) ;
            Shell.getCallManager().zombieCall(selectedCall) ;
            Shell.getCallManager().callNotRinging(selectedCall);
            selectedCall.placeOnHold() ;
            Shell.getInstance().setMessageIndicator(PhoneLamp.LAMPMODE_OFF, "Incoming Call") ;
        } catch (PSTAPIException e) {
            // ::TODO:: Handle this failure condition
            SysLog.log(e) ;
        }

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
        m_listControl.setItemRenderer(new DefaultIncomingCallRenderer()) ;
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
     * Fill list model with incoming ringing calls
     */
    private void populateIncomingCalls()
    {
        System.out.println(".....................inside populate incoming calls" );
        PCallManager callManager = Shell.getCallManager() ;

        clearIncomingCallsList() ;
        PCall incomingCalls[] = callManager.getCallsByState(PCall.CONN_STATE_INBOUND_ALERTING) ;
        sortIncomingCalls(incomingCalls) ;
        for (int i=0; i<incomingCalls.length; i++) {
            m_listModel.addElement(incomingCalls[i]) ;
            incomingCalls[i].addConnectionListener(m_connectionListener) ;
        }
    }


    /**
     * clears the list model of incoming ringing calls
     */
    private void clearIncomingCallsList()
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
     * It has a Cancel, OK and Ignore
     */
    private void initBottomButtonBar(String strLabelOK,
                                       String strHintOK)
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem actionCancel = new PActionItem(new PLabel("Close"),
            getString("hint/core/system/closeform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, actionCancel) ;

        PActionItem actionOk = new PActionItem(new PLabel(strLabelOK),
            strHintOK,
            m_commandDispatcher,
            m_commandDispatcher.ACTION_OK) ;
        menuControl.setItem(PBottomButtonBar.B3, actionOk) ;

        PActionItem actionIgnore =  new PActionItem( new PLabel(getString("lblIncomingCallIgnore")),
                                        getString("hint/coreapp/incomingcall/ignore"),
                                        m_commandDispatcher,
                                        m_commandDispatcher.ACTION_IGNORE) ;

        menuControl.setItem(PBottomButtonBar.B1, actionIgnore) ;
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

        items = new PActionItem[3] ;

        items[0] = new PActionItem(new PLabel(strLabelOK),
            strHintOK,
            m_commandDispatcher,
            m_commandDispatcher.ACTION_OK) ;

        items[1] = new PActionItem(new PLabel("Close"),
            getString("hint/core/system/closeform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;

         items[2] =  new PActionItem( new PLabel(getString("lblIncomingCallIgnore")),
                                        getString("hint/coreapp/incomingcall/ignore"),
                                        m_commandDispatcher,
                                        m_commandDispatcher.ACTION_IGNORE) ;

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
    private static void sortIncomingCalls(PCall calls[])
    {
        // Yes, this is an grossly inefficient sorting algorithm, however, we
        // should never have more than a few calls on hold.
        for (int i=0; i<calls.length; i++) {
            for (int j=0; j<calls.length; j++) {
                if (calls[j].getDurationSinceFirstRang() < calls[i].getDurationSinceFirstRang()) {
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
        public final String ACTION_IGNORE = "action_ignore" ;


        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_OK)) {
                onOk() ;
            } else  if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                onOk() ;
            }else if (event.getActionCommand().equals(ACTION_IGNORE)) {
                onIgnore() ;
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
     * Connection listener that looks for events in PConnectionListener.
     * For anything other than callCreated, connectionTrying,
     * connectionOutboundAlerting, connectionInboundAlerting, the calls are
     * are removed from the incoming calls list.
     */
    private class icConnectionListener extends PConnectionListenerAdapter
    {

        public void callCreated(PConnectionEvent event) {
        }
        public void callDestroyed(PConnectionEvent event) {
            m_listModel.removeElement(event.getCall()) ;
        }
        public void callHeld(PConnectionEvent event) {
            m_listModel.removeElement(event.getCall()) ;
        }
        public void callReleased(PConnectionEvent event) {
            m_listModel.removeElement(event.getCall()) ;
        }

        public void connectionTrying(PConnectionEvent event) {
        }
        public void connectionOutboundAlerting(PConnectionEvent event) {
        }
        public void connectionInboundAlerting(PConnectionEvent event) {
        }
        public void connectionConnected(PConnectionEvent event) {
            m_listModel.removeElement(event.getCall()) ;
        }
        public void connectionFailed(PConnectionEvent event) {
            m_listModel.removeElement(event.getCall()) ;
        }
        public void connectionUnknown(PConnectionEvent event) {
                 m_listModel.removeElement(event.getCall()) ;
        }
        public void connectionDisconnected(PConnectionEvent event) {
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
            clearIncomingCallsList() ;
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
            enableMenusByAction((m_listModel.getSize() > 0), m_commandDispatcher.ACTION_IGNORE) ;
        }

        public void intervalRemoved(PListDataEvent e)
        {
            enableMenusByAction((m_listModel.getSize() > 0), m_commandDispatcher.ACTION_OK) ;
            enableMenusByAction((m_listModel.getSize() > 0), m_commandDispatcher.ACTION_IGNORE) ;
        }

        public void contentsChanged(PListDataEvent e) { }
    }
}

