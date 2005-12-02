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


package org.sipfoundry.sipxphone.app.conference ;

import java.awt.Insets ;
import java.awt.Dimension;
import java.util.Vector ;

import org.sipfoundry.util.SysLog;
import org.sipfoundry.util.SipParser ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.sys.*;
import org.sipfoundry.sipxphone.sys.app.core.DTMFGenerator ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.PConnectionEvent ;
import org.sipfoundry.stapi.event.PConnectionListener;
import org.sipfoundry.sipxphone.app.CoreAboutbox ;

/**
 * The conference status form displays all the active conference participants
 * along with potential conference participants.  A potential conference
 * participant can be a held call, member of a conference group, or a
 * previously connected participants (hung up on or disconnected).
 * <p>
 * Much of the display logic is contains in the ConferenceStatusRenderer,
 * ConferenceCallDataItem, and ConferenceCallListModel.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConferenceStatusForm extends PApplicationForm implements DialingStrategyListener
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PList                   m_listControl ;     // list control used in the simple form
    private Vector                  m_vListListeners ;  // list selection listener
    private PCommandBar             m_commandBar ;      // command bar (null if not enabled)
    private boolean                 m_bInitialized ;    // Has the conference application been fully initialized?
    private PCall                   m_call ;            // The conference call this form is observing
    private icDisplayContainer      m_contDisplay ;     // container housing visual controls
    private ConferenceCallListModel m_listModel ;       // list control data model
    private icCommandDispatcher     m_commandDispatcher ;   // dispatches action commands to various handlers
    private icConnectionListener    m_connectionListener ;  // Connection Listener
    private DTMFGenerator           m_DTMFGenerator = new DTMFGenerator() ;

    /** tip to display if there are <= 2 items in the list */
    private PLabel m_labelTip ;

    /** boolean to denote if tip was already added or not*/
    private boolean m_bLabelTipAdded  = false;

    //listener to listen to data add/remove in list
    protected icListDataListener  m_listDataListener
        = new icListDataListener();

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor, takes an application object and conference call
     *
     * @param application Reference to the invoking application.
     * @param call Established conference call or a call that will become
     *        a conference.
     */
    public ConferenceStatusForm(Application application, PCall call)
    {
        super(application, "Conference Status") ;

        setTitle(getString("lblConferenceStatusTitle")) ;

        m_commandDispatcher = new icCommandDispatcher() ;
        m_vListListeners = new Vector() ;
        m_bInitialized = false ;

        // Create all of our components.
        initControls() ;
        initBottomButtonBar() ;
        initMenus() ;

        // Physically lay out everything
        layoutComponents() ;
        m_call = call ;

        initializeListModel(m_call) ;
        m_connectionListener = new icConnectionListener(m_call) ;

        setHelpText(getString("conference_status"), getString("conference_status_title")) ;


        addButtonListener(m_DTMFGenerator) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Enable or disable the command bar.
     *
     * @param bEnable True to enable and show the command bar, or false to disable
     *        and hide it.
     *
     * @see org.sipfoundry.sipxphone.app.conference.ConferenceStatusForm#getCommandBar
     */
    public void enableCommandBar(boolean bEnable)
    {
        if (bEnable == true) {
            if (m_commandBar == null) {
                m_commandBar = new PCommandBar() ;
                m_contDisplay.add(m_commandBar) ;
                doLayout() ;
            }
        } else {
            if (m_commandBar != null) {
                m_contDisplay.remove(m_commandBar) ;
                m_commandBar.removeAllButtons() ;
                m_commandBar = null ;
                doLayout() ;
            }
        }
    }


    /**
     * Get a reference to the command bar.  By default, the command bar is
     * disabled and developers must call <i>enableCommandBar(true)</i> prior to
     * invoking this accessor method.
     *
     * @return Reference to the command bar, or null if not enabled.
     *
     * @see org.sipfoundry.sipxphone.app.conference.ConferenceStatusForm#enableCommandBar
     */
    public PCommandBar getCommandBar()
    {
        return m_commandBar ;
    }


    /**
     * called when the form is gaining focus
     *
     * @param formLosingFocus form that lost focus as a result of
     */
    public void onFocusGained(PForm formLosingFocus)
    {
        if (m_bInitialized != true) {
            if (m_call.getParticipants().length == 0) {
                onAdd() ;
            }
            m_bInitialized = true ;
        }
        onUpdateSelection() ;
    }


    public void onExit()
    {
        closeForm() ;

        try {
            if (m_call != null)
                m_call.disconnect() ;
        } catch (PSTAPIException e) {
            SysLog.log(e) ;
        }
    }


    /**
     * Add a participant to the conference call.  There is some magic maximum
     * number of participants that we must maintain.  If we are equal to or
     * above that limit, we must complain to the user.
     */
    public void onAdd()
    {
        PCall call = m_call ;   // Store a reference to the call, so it cannot
                                // be changed while processing.
        if (call != null)
        {
            if (call.isConferenceLimitReached())
            {
                MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(getString("lblErrorMaxConfParticipants")) ;

                messageBox.showModal() ;
            }
            else
            {
                Shell.getInstance().dial() ;
            }
        }
    }


    /**
     * Disconnect the selected conference participant.  The end user must
     * confirm the action before proceeding.
     */
    public void onDisconnect()
    {
        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;

        messageBox.setTitle(getString("lblDisconnectWarningTitle")) ;
        messageBox.setMessage(getString("lblDisconnectWarningMessage")) ;

        if (messageBox.showModal() == messageBox.OK) {
            ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;
            if (item != null) {
                PAddress address = item.getAddress() ;
                try {
                    PCall call = item.getConferenceCall() ;
                    if (call != null)
                        call.disconnect(address) ;
                } catch (PCallException e) {
                    SysLog.log(e) ;
                }
            }
        }
    }


    /**
     * Remove a dropped conference participant or conference group member.
     * This method will remove the entry from the call status form.
     */
    public void onRemove()
    {
        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;

        messageBox.setTitle(getString("lblRemoveWarningTitle")) ;
        messageBox.setMessage(getString("lblRemoveWarningMessage")) ;

        if (messageBox.showModal() == messageBox.OK) {
            ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;
            if (item != null) {
                PAddress address = item.getAddress() ;
                m_listModel.remove(address) ;

            }
        }
    }


    /**
     * Includes or joins a held call into this conference.  This current
     * implementation assumes that the include target is a held conference
     * participant on the current call.
     */
    public void onInclude()
    {
        ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;
        PAddress address = item.getAddress() ;
        try {
            beginWaitIcon() ;
            m_call.releaseFromHold(address) ;
        } catch (PCallStateException cse) {
            displayTakeCallOffHoldError() ;
            SysLog.log(cse) ;
        } catch (Exception e) {
            SysLog.log(e) ;
        } finally {
            endWaitIcon() ;
        }

        item.setType(ConferenceCallDataItem.TYPE_CONNECTION) ;
        m_listModel.fireContentsChanged() ;
    }


    /**
     * Place the currently selected conference participant on hold.  This
     * assumes that the participant isn't already on hold.
     */
    public void onHold()
    {
        ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;
        if (item != null) {
            PAddress address = item.getAddress() ;
            try {
                beginWaitIcon() ;
                m_call.placeOnHold(address) ;
            } catch (PCallStateException cse) {
                displayPlaceCallOnHoldError() ;
                SysLog.log(cse) ;
            } catch (Exception e) {
                SysLog.log(e) ;
            } finally {
                endWaitIcon() ;
            }


            item.setState(ConferenceCallDataItem.STATE_HELD) ;
            item.setType(ConferenceCallDataItem.TYPE_HELDCALL) ;
            m_listModel.fireContentsChanged() ;
        }
    }


    /**
     * Redials a dropped/failed call or conference group member.
     */
    public void onRedial()
    {
        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_WARNING) ;
        DialingStrategy strategy = Shell.getInstance().getDialingStrategy() ;

        messageBox.setTitle(getString("lblRedialWarningTitle")) ;
        messageBox.setMessage(getString("lblRedialWarningMessage")) ;

        if (messageBox.showModal() == messageBox.OK) {
            ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;
            if (item != null) {
                PAddress address = item.getAddress() ;
                SipParser parser = new SipParser(address.getAddress()) ;
                parser.removeAllFieldParameters() ;
                parser.removeAllHeaderParameters() ;
                parser.removeAllURLParameters() ;
                m_listModel.remove(address) ;
                try
                {
                    strategy.dial(PAddressFactory.getInstance().createAddress(parser.render())) ;
                }
                catch (Exception e)
                {
                    Shell.getInstance().showUnhandledException(e, false) ;
                }
            }
        }
    }


    /**
     * Is the specified address already added to our conference?  This method
     * is a hack.  We cannot handle adding multple participants with the
     * same address to a conference yet, so are looking before we leap.
     */
    public boolean isActiveParticipant(PAddress address)
    {
        boolean bIsDuplicate = false ;

        if ((m_listModel != null) && (address != null)) {
            if (m_listModel.isActiveParticipant(address)) {
                bIsDuplicate = true ;
            }
        }
        return bIsDuplicate ;
    }


    /**
     * Removes the specified address from the conference list providing
     * that the address is inactive (not in an established/held/trying/ringing
     * state.
     */
    public void removeInactiveParticipant(PAddress address)
    {
        if ((m_listModel != null) && (address != null)) {
            m_listModel.removeInactiveParticipant(address) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Updates the bottom buttons based on the selected list element.  Please
     * note that B1 always has the "add" action (initalized in
     * initBottomButtonBar)
     */
    protected void onUpdateSelection()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        ConferenceCallDataItem item = (ConferenceCallDataItem) m_listControl.getSelectedElement() ;

        if (item != null)
        {
            item.updateState();

            switch (item.getState())
            {
                case ConferenceCallDataItem.STATE_UNKNOWN:
                    menuControl.clearItem(PBottomButtonBar.B2) ;
                    menuControl.clearItem(PBottomButtonBar.B3) ;

                    enableMenusByAction(false, m_commandDispatcher.ACTION_REMOVE) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_DISCONNECT) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_REDIAL) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_HOLD) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_INCLUDE) ;
                    break ;
                case ConferenceCallDataItem.STATE_FAILED:
                case ConferenceCallDataItem.STATE_UNAVAILABLE:
                    menuControl.setItem(PBottomButtonBar.B2, new PLabel(getString("lblActionRemove")), m_commandDispatcher.ACTION_REMOVE, getString("hint/conference/status/remove")) ;
                    menuControl.setItem(PBottomButtonBar.B3, new PLabel(getString("lblActionRedial")), m_commandDispatcher.ACTION_REDIAL, getString("hint/conference/status/redial")) ;

                    enableMenusByAction(true, m_commandDispatcher.ACTION_REMOVE) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_DISCONNECT) ;
                    enableMenusByAction(true, m_commandDispatcher.ACTION_REDIAL) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_HOLD) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_INCLUDE) ;
                    break ;
                case ConferenceCallDataItem.STATE_TRYING:
                case ConferenceCallDataItem.STATE_RINGING:
                    menuControl.setItem(PBottomButtonBar.B2, new PLabel(getString("lblActionDisconnect")), m_commandDispatcher.ACTION_DISCONNECT, getString("hint/conference/status/disconnect")) ;
                    menuControl.clearItem(PBottomButtonBar.B3) ;

                    enableMenusByAction(false, m_commandDispatcher.ACTION_REMOVE) ;
                    enableMenusByAction(true, m_commandDispatcher.ACTION_DISCONNECT) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_REDIAL) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_HOLD) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_INCLUDE) ;
                    break ;
                case ConferenceCallDataItem.STATE_CONNECTED:
                    menuControl.setItem(PBottomButtonBar.B2, new PLabel(getString("lblActionDisconnect")), m_commandDispatcher.ACTION_DISCONNECT, getString("hint/conference/status/disconnect")) ;
                    menuControl.setItem(PBottomButtonBar.B3, new PLabel(getString("lblActionHold")), m_commandDispatcher.ACTION_HOLD, getString("hint/conference/status/hold")) ;

                    enableMenusByAction(false, m_commandDispatcher.ACTION_REMOVE) ;
                    enableMenusByAction(true, m_commandDispatcher.ACTION_DISCONNECT) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_REDIAL) ;
                    enableMenusByAction(true, m_commandDispatcher.ACTION_HOLD) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_INCLUDE) ;
                    break ;
                case ConferenceCallDataItem.STATE_HELD:
                    menuControl.setItem(PBottomButtonBar.B2, new PLabel(getString("lblActionInclude")), m_commandDispatcher.ACTION_INCLUDE, getString("hint/conference/status/include")) ;
                    menuControl.clearItem(PBottomButtonBar.B3) ;

                    enableMenusByAction(false, m_commandDispatcher.ACTION_REMOVE) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_DISCONNECT) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_REDIAL) ;
                    enableMenusByAction(false, m_commandDispatcher.ACTION_HOLD) ;
                    enableMenusByAction(true, m_commandDispatcher.ACTION_INCLUDE) ;
                    break ;
            }
        } else {
            menuControl.setItem(PBottomButtonBar.B2,
                new PLabel(getString("lblActionExit")),
                             m_commandDispatcher.ACTION_EXIT,
                             getString("hint/conference/status/exit"));
            menuControl.clearItem(PBottomButtonBar.B3) ;

            enableMenusByAction(false, m_commandDispatcher.ACTION_REMOVE) ;
            enableMenusByAction(false, m_commandDispatcher.ACTION_DISCONNECT) ;
            enableMenusByAction(false, m_commandDispatcher.ACTION_REDIAL) ;
            enableMenusByAction(false, m_commandDispatcher.ACTION_HOLD) ;
            enableMenusByAction(false, m_commandDispatcher.ACTION_INCLUDE) ;
        }
    }


    /**
     * Initialize this form's controls
     */
    private void initControls()
    {
        m_listControl = new PList() ;
        m_listControl.setItemRenderer(new ConferenceStatusRenderer()) ;
        m_listModel = new ConferenceCallListModel() ;
        m_listControl.setListModel(m_listModel) ;
        m_listControl.addListListener(new icListListener()) ;
        m_listControl.repaint() ;
    }


    /**
     * Initialize the list model with the specified call
     */
    private void initializeListModel(PCall call)
    {
        PAddress participants[] = call.getParticipants() ;

        System.out.println("") ;
        System.out.println("Conference Participants: ") ;
        System.out.println("") ;

        for (int i=0; i<participants.length; i++) {
            System.out.println(participants[i]) ;
        }
        System.out.println("") ;

        m_listModel.setCall(call) ;
        m_listControl.setListModel(m_listModel) ;
        m_listModel.addListDataListener(m_listDataListener) ;
    }


    /**
     * Initialize the bottom button bar.  The bottom button bar is dynamically
     * updated by the onUpdateSelection method.  This method should initialize
     * static/non-changing buttons.
     */
    private void initBottomButtonBar()
    {
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        menuControl.addActionListener(m_commandDispatcher) ;
        menuControl.setItem(PBottomButtonBar.B1, new PLabel("Add"), m_commandDispatcher.ACTION_ADD, getString("hint/conference/status/add")) ;
    }


    private void initMenus()
    {
        PActionItem items[] ;

        // Exit     Disconnect
        // Add      Remove
        // Include  Hold
        //          Redial


        // Set Left Menu Items
        items = new PActionItem[4] ;
        items[0] = new PActionItem( new PLabel(getString("lblActionAdd")),
                                    getString("hint/conference/status/add"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_ADD) ;

        items[1] = new PActionItem( new PLabel(getString("lblActionDisconnect")),
                                    getString("hint/conference/status/disconnect"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_DISCONNECT) ;

        items[2] = new PActionItem( new PLabel(getString("lblActionRemove")),
                                    getString("hint/conference/status/remove"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_REMOVE) ;
        items[3] = new PActionItem( new PLabel(getString("lblActionRedial")),
                                    getString("hint/conference/status/redial"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_REDIAL) ;


        setLeftMenu(items) ;

        // Set Right Menu Items
        items = new PActionItem[4] ;

        items[0] = new PActionItem( new PLabel("About"),
                                    getString("hint/core/system/aboutform"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_ABOUT) ;
        items[1] = new PActionItem( new PLabel(getString("lblActionExit")),
                                    getString("hint/conference/status/exit"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_EXIT) ;

        items[2] = new PActionItem( new PLabel(getString("lblActionHold")),
                                    getString("hint/conference/status/hold"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_HOLD) ;
        items[3] = new PActionItem( new PLabel(getString("lblActionInclude")),
                                    getString("hint/conference/status/include"),
                                    m_commandDispatcher,
                                    m_commandDispatcher.ACTION_INCLUDE) ;

        setRightMenu(items) ;
    }



    /**
     * Physically layout the components in this form
     */
    private void layoutComponents()
    {
        // perform first time initialization of display container if needed
        if (m_contDisplay == null) {
            m_contDisplay = new icDisplayContainer() ;
            m_contDisplay.setLayout(null) ;
            m_contDisplay.add(m_listControl) ;

            if (m_commandBar != null)
                m_contDisplay.add(m_commandBar) ;
        }

        addToDisplayPanel(m_contDisplay, new Insets(0,0,0,0)) ;
    }


    public void dialingAborted(PCall call)
    {
        if (call.getConnectionState() == PCall.CONN_STATE_IDLE) {
            try {
                call.disconnect() ;
            } catch (PCallException e) {
                SysLog.log(e) ;
            }
            closeForm() ;
        }
    }

    public void dialingInitiated(PCall call, PAddress address)
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void displayPlaceCallOnHoldError()
    {
        String msg = getString("lblErrorCallHoldFailed") ;

        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_ERROR) ;
        messageBox.setMessage(msg) ;

        messageBox.showModal() ;
    }


    protected void displayTakeCallOffHoldError()
    {
        String msg = getString("lblErrorCallUnholdFailed") ;

        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_ERROR) ;
        messageBox.setMessage(msg) ;

        messageBox.showModal() ;
    }



//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    /**
     * Display containers holds our command bar and list control
     */
    private class icDisplayContainer extends PContainer
    {
        public void doLayout()
        {
            Dimension dim = this.getSize() ;

            if (m_commandBar != null) {
                m_commandBar.setBounds(0, 0, 28, dim.height-2) ;
                m_listControl.setBounds(28, 0, dim.width-30, dim.height-2) ;
            } else {
                m_listControl.setBounds(0, 0, dim.width-1, dim.height-2) ;
            }
            super.doLayout() ;
        }
    }


   /**
     * The command dispatcher sits arounds and waits for action event.  Once
     * an action event is received, it is dispatched to the appropriate onXXX
     * handler.
     */
    private class icCommandDispatcher implements PActionListener
    {
        protected final String ACTION_ADD        = "action_add" ;
        protected final String ACTION_DISCONNECT = "action_drop" ;
        protected final String ACTION_REMOVE     = "action_remove" ;
        protected final String ACTION_REDIAL     = "action_redial" ;
        protected final String ACTION_INCLUDE    = "action_include" ;
        protected final String ACTION_HOLD       = "action_hold" ;
        protected final String ACTION_EXIT       = "action_exit" ;
        protected final String ACTION_ABOUT       = "action_about" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ADD)) {
                onAdd() ;
            } else if (event.getActionCommand().equals(ACTION_DISCONNECT)) {
                onDisconnect() ;
            } else if (event.getActionCommand().equals(ACTION_REMOVE)) {
                onRemove() ;
            } else if (event.getActionCommand().equals(ACTION_REDIAL)) {
                onRedial() ;
            } else if (event.getActionCommand().equals(ACTION_INCLUDE)) {
                onInclude() ;
            } else if (event.getActionCommand().equals(ACTION_HOLD)) {
                onHold() ;
            } else if (event.getActionCommand().equals(ACTION_ABOUT)) {
                CoreAboutbox.display(getApplication(),"Conference") ;
            } else if (event.getActionCommand().equals(ACTION_EXIT)) {
                onExit() ;
            }
        }
    }


    /**
     * This list data listener is listens for data model changes and forces
     * an update of the bottom button bar by invoking the onUpdateSelection
     * method.  This listener also looks for a special case empty list
     * situation.  If meet, we close the form.
     * If there are no items in the list, then a "prompt" is displayed.
     */
    private  class icListDataListener implements PListDataListener
    {
        /**
         *  An interval was added/inserted into the data model
         *  Note: ConfereceCallListModel just calls "contentsChanged"
         *  even when an item is added or removed. So the handling
         *  of drawing info message( prompt ) is called in
         *  contentsChanged method.
         */
        public void intervalAdded(PListDataEvent e)
        {
        }

        /**
         *  An interval was deleted from the data model
         *  Note:ConfereceCallListModel just calls "contentsChanged"
         *  even when an item is added or removed. So the handling
         *  of drawing info message( prompt ) is called in
         *  contentsChanged method.
         */
        public void intervalRemoved(PListDataEvent e)
        {
            onUpdateSelection() ;
        }

        /** ConfereceCallListModel just calls "contentsChanged"
         *  even when an item is added or removed. So the handling
         *  of drawing info message( prompt ) is called in
         *  contentsChanged method.
         */
        public void contentsChanged(PListDataEvent e) {
              handleDrawingInfoMessage();
              onUpdateSelection() ;
        }

        /** draw the info message( prompt ).
         *  A label with the prompt is put on top
         *  of the list.
         */
        private void drawInfoMessage()
        {
            if (m_labelTip == null)
            {
                m_labelTip = new PLabel(getString("lblConferenceTip"),
                        PLabel.ALIGN_SOUTH) ;
            }

            Dimension dim = m_listControl.getSize() ;
            int iHeight = dim.height ;
            int iWidth = dim.width ;
            m_labelTip.setBounds(0, ((iHeight/2)-4), iWidth, ((iHeight/2)+4) );
            if( iWidth != 0 && iHeight != 0 )
            {
                m_listControl.add(m_labelTip);
                m_bLabelTipAdded = true;
            }
        }


        /**
         * remove the label with the prompt
         */
        private void removeInfoMessage()
        {
            if( m_labelTip != null )
                m_listControl.remove(m_labelTip);
            m_bLabelTipAdded = false;
        }


        /** add or remove the prompt depending on
         *  the number of items in the list.
         */
        public void handleDrawingInfoMessage()
        {
            if (m_listModel.getSize() > 0)
            {
                if (m_bLabelTipAdded == true)
                {
                    removeInfoMessage();
                }
            }
            else
            {
                if (m_bLabelTipAdded == false)
                {
                    drawInfoMessage();
                }
            }
        }
    }


    /**
     * Called when the fom is closing.
     */
    public void onFormClosing()
    {
        // clear the list model (this will detach the call listener in the
        // list model)
        m_listModel.setCall(null) ;
    }


    /**
     * This list listener observes selection changes and forces an update of
     * the context sensitive bottom button bar by invoking the
     * onUpdateSelection method.
     */
    private class icListListener implements PListListener
    {
        public void selectChanged(PListEvent event)
        {
            onUpdateSelection() ;
        }
    }


    /**
     * This connection listener is listening for held/release state changes
     * or call destruction.
     *
     * Upon call destruction, the listener closes the conference status form.
     * Upon hold/release, the status form is closed / restarted.
     */
    private class icConnectionListener implements PConnectionListener
    {
        private PCall m_call ;

        public icConnectionListener(PCall call)
        {
            m_call = call ;
            m_call.addConnectionListener(this) ;
        }


        /**
         * The entire call (all connections) has been placed on hold.  At this
         * point, no audio will be exchanged between the participants of the call.
         * The state can change to connected, failed, or dropped from this call state.
         *
         * @param event PConnectionEvent object containing both the call and
         *        address objects associated with the state change, and a cause ID if a
         *        failure has occurred.
         */
        public void callHeld(PConnectionEvent event)
        {
            // Close the conference application
            closeForm() ;
        }


        /**
         * A call has been released from (taken off) hold.  In this call state, audio
         * is once again exchanged between all of the participants of the call.
         * The state is connected and can change to disconnected, failed, or dropped
         * from this call state.
         *
         * @param event PConnectionEvent object containing both the call and
         *        address objects associated with the state change, and a cause ID if a
         *        failure has occurred.
         */
        public void callReleased(PConnectionEvent event)
        {
            // Remove ourself as a listener
            m_call.removeConnectionListener(this) ;
            ApplicationManager.getInstance().activateCoreApplication(ApplicationRegistry.CONFERENCE_APP) ;
        }


        /**
         * A call has been destroyed and is no longer valid.  This is an
         * appropriate time to remove listeners and close applications/forms that
         * are monitoring specific calls.
         *
         * @param event The {@link PConnectionEvent} object containing both the call
         *        and address objects associated with the state change, and a
         *        cause ID if a failure has occurred.
         */
        public void callDestroyed(PConnectionEvent event)
        {
            // Close the conference application
            m_call.removeConnectionListener(this) ;
            closeForm() ;
        }

        public void callCreated(PConnectionEvent event) { /* stub */ } ;
        public void connectionTrying(PConnectionEvent event)  { /* stub */ } ;
        public void connectionOutboundAlerting(PConnectionEvent event)  { /* stub */ } ;
        public void connectionInboundAlerting(PConnectionEvent event)  { /* stub */ } ;
        public void connectionConnected(PConnectionEvent event) { /* stub */ } ;
        public void connectionFailed(PConnectionEvent event) { /* stub */ } ;
        public void connectionUnknown(PConnectionEvent event) { /* stub */ } ;
        public void connectionDisconnected(PConnectionEvent event) { /* stub */ } ;
    }
}


