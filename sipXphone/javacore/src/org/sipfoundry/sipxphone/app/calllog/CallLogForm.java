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


package org.sipfoundry.sipxphone.app.calllog ;

import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.app.calllog.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.core.*;
import org.sipfoundry.sipxphone.app.CoreAboutbox ;

import org.sipfoundry.stapi.* ;


/**
 * The call log form displays a call history of incoming and outgoing calls.
 * End users can customize their display by changing sorting and filtering
 * criteria.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallLogForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int OK = 0 ;
    public static final int CANCEL = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** call log list control */
    private PList m_listCallLog ;
    /** call log list model */
    private  PDefaultListModel m_listModel ;

    /** our left vertical command bar */
    private PCommandBar m_commandBar ;

    /** Filtering used to restrict display of incoming calls
     * (see constants in CallLegEntry) */
    protected int m_iCallLogInboundFilter ;

    /** Filtering used to restrict display of outgoing calls
     *  (see constants in CallLegEntry) */
    protected int m_iCallLogOutboundFilter ;

    /** Sort ordering (see constants in CallLog) */
    protected int m_iSortOrder ;

    //original values
    protected int m_iOldCallLogOutboundFilter;
    protected int m_iOldCallLogInboundFilter;
    protected int m_iOldSortOrder;


    /** action command dispatcher */
    private icCommandDispatcher m_commandDispatcher = new icCommandDispatcher() ;

    /** an instance of calllog listener */
    private icCallLogListener m_callLogListener = new icCallLogListener();


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a call log form under the specified application context
     */
    public CallLogForm(Application application)
    {
        super(application, "Call Log") ;

        setTitle("Call Log") ;
        setIcon(getImage("imgCallLogIcon")) ;

        setHelpText(getString("call_log"), getString("call_log_title")) ;
        setStringResourcesFile("CallLogForm.properties") ;

        // Load inbound Filter state
        try {
            Integer objInt = (Integer) getApplication().getObject("calllog_inbound_filter") ;
            m_iCallLogInboundFilter = objInt.intValue() ;
        } catch (Exception e) {
            m_iCallLogInboundFilter = CallLogManager.COMPLETE | CallLogManager.INCOMPLETE;
        }

        // Load outbound Filter state
        try {
            Integer objInt = (Integer) getApplication().getObject("calllog_outbound_filter") ;
            m_iCallLogOutboundFilter = objInt.intValue() ;
        } catch (Exception e) {
            m_iCallLogOutboundFilter = CallLogManager.COMPLETE | CallLogManager.INCOMPLETE;
         }

        // Load Sort Order
        try {
            Integer objInt = (Integer) getApplication().getObject("sortOrder") ;
            m_iSortOrder = objInt.intValue() ;
        } catch (Exception e) {
            //default should be reverse chronological order(fix of bug 810 )
            m_iSortOrder = CallLog.SORT_REVERSE_CHRONOLOGICAL;
        }

        m_iOldCallLogOutboundFilter = m_iCallLogOutboundFilter;
        m_iOldCallLogInboundFilter = m_iCallLogInboundFilter;
        m_iOldSortOrder = m_iSortOrder;
        // Create our event/command dispatcher
        icCommandDispatcher dispatcher = new icCommandDispatcher() ;

        // Create everything else
        initCallLog() ;
        initCommandBar() ;
        initMenus() ;

        // physically lay them out
        layoutComponents() ;

        updateFilter() ;
        if (m_listModel.getSize() > 0)
            m_listCallLog.setSelectedIndex(0) ;

        CallLogManager.getInstance().addCallLogListener(m_callLogListener);

    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Invoked when end user presses the "sort" button
     */
    public void onSort()
    {
        SimpleOptionForm options = new SimpleOptionForm(getApplication(), "Call Log Sort") ;

        options.setIcon(getImage("imgCallLogIcon")) ;
        options.setHelpText(getString("call_log"), getString("call_log_title")) ;

        // Add various options to the sorting option form
        options.groupOptions() ;
        options.addOption(getString("lblChronologicalOrder"),
                (m_iSortOrder == CallLog.SORT_CHRONOLOGICAL),
                getString("hint/calllog/sort/chronological")) ;
        options.addOption(getString("lblReverseChronologicalOrder"),
                (m_iSortOrder == CallLog.SORT_REVERSE_CHRONOLOGICAL),
                getString("hint/calllog/sort/reverse_chronological")) ;
        options.addOption(getString("lblAlphabeticalOrder"),
                (m_iSortOrder == CallLog.SORT_ALPHANUMERIC),
                getString("hint/calllog/sort/alphabetical")) ;
        options.addOption(getString("lblReverseAlphabeticalOrder"),
                (m_iSortOrder == CallLog.SORT_REVERSE_ALPHANUMERIC),
                getString("hint/calllog/sort/reverse_alphabetical")) ;

        // Give control to user
        if (options.showModal() == SimpleOptionForm.OK) {
            // Update filter if requested
            m_iSortOrder = options.getSelectedGroupOptionIndex() ;
            //default order should be reverse-chronological
            //fix of bug 810
            if (m_iSortOrder == -1)
                m_iSortOrder = CallLog.SORT_REVERSE_CHRONOLOGICAL ;

            m_listCallLog.setSelectedIndex(-1) ;
            updateFilter() ;
        }
    }


    /**
     * Invoked when end user presses the filter button
     */
    public void onFilter()
    {
        SimpleOptionForm options = new SimpleOptionForm
                            (getApplication(), "Call Log Filter") ;

        options.setIcon(getImage("imgCallLogIcon")) ;
        options.setHelpText(getString("call_log"), getString("call_log_title")) ;

        // Determine current state
        boolean bOutgoingCallsAnswered = true;
        boolean bOutgoingCallsMissed = true;
        boolean bIncomingCallsTaken = true;
        boolean bIncomingCallMissed = true;
        bIncomingCallsTaken =
         (( m_iCallLogInboundFilter & CallLogManager.COMPLETE )==CallLogManager.COMPLETE);
        bIncomingCallMissed =
          (( m_iCallLogInboundFilter & CallLogManager.INCOMPLETE)==CallLogManager.INCOMPLETE);
        bOutgoingCallsAnswered =
         (( m_iCallLogOutboundFilter & CallLogManager.COMPLETE )==CallLogManager.COMPLETE);
        bOutgoingCallsMissed =
          (( m_iCallLogOutboundFilter & CallLogManager.INCOMPLETE)==CallLogManager.INCOMPLETE);

        /*
        boolean bOutgoingCallsAnswered =
                ((m_iCallLogFilter & CallLogEntry.OUTBOUND) == CallLogEntry.OUTBOUND) ;
        boolean bOutgoingCallsMissed =
                ((m_iCallLogFilter & CallLogEntry.OUTBOUND_FAILED) == CallLogEntry.OUTBOUND_FAILED) ;
        boolean bIncomingCallsTaken =
                ((m_iCallLogFilter & CallLogEntry.INBOUND) == CallLogEntry.INBOUND) ;
        boolean bIncomingCallMissed =
                ((m_iCallLogFilter & CallLogEntry.INBOUND_MISSED) == CallLogEntry.INBOUND_MISSED) ;
        */
        // Prime Options List
        options.addOption(getString("lblFilterOutgoingCallsAnswer"),
                bOutgoingCallsAnswered,
                getString("hint/calllog/filter/outgoing_answer")) ;
        options.addOption(getString("lblFilterOutgoingCallsMissed"),
                bOutgoingCallsMissed,
                getString("hint/calllog/filter/outgoing_missed")) ;
        options.addOption(getString("lblFilterIncomingCallsTaken"),
                bIncomingCallsTaken,
                getString("hint/calllog/filter/incoming_taken")) ;
        options.addOption(
                getString("lblFilterIncomingCallsMissed"),
                bIncomingCallMissed,
                getString("hint/calllog/filter/incoming_missed")) ;

        // Give control to user
        if (options.showModal() == SimpleOptionForm.OK) {

            // Update the actual filter
            m_iCallLogOutboundFilter = 0;
            m_iCallLogInboundFilter = 0;

            if (options.getOption(getString("lblFilterOutgoingCallsAnswer")))
                m_iCallLogOutboundFilter |= CallLogManager.COMPLETE ;
            if (options.getOption(getString("lblFilterOutgoingCallsMissed")))
                m_iCallLogOutboundFilter |= CallLogManager.INCOMPLETE ;
            if (options.getOption(getString("lblFilterIncomingCallsTaken")))
                m_iCallLogInboundFilter |= CallLogManager.COMPLETE ;
            if (options.getOption(getString("lblFilterIncomingCallsMissed")))
                m_iCallLogInboundFilter |= CallLogManager.INCOMPLETE ;

             m_listCallLog.setSelectedIndex(-1) ;
             updateFilter() ;
        }
    }


    /**
     * Invoked when the user presses the delete element button
     */
    public void onDelete()
    {
        CallLegEntry data = (CallLegEntry) m_listCallLog.getSelectedElement() ;
        if (data != null) {
            MessageBox alert = new MessageBox(getApplication(), MessageBox.TYPE_WARNING) ;
            alert.setMessage(getString("lblWarningDeleteEntry")) ;
            if (alert.showModal() == MessageBox.OK) {
               CallLogManager.getInstance().removeEntry(data) ;
            }
        } else {
            MessageBox alert = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
            alert.setMessage(getString("lblErrorEmptyDelete")) ;
            alert.showModal() ;
        }
    }





    public void onMore(){
      CallLegEntry data = (CallLegEntry) m_listCallLog.getSelectedElement() ;
      final SimpleTextForm infoBox = new SimpleTextForm(getApplication(), "Call Log Info") ;
      infoBox.getBottomButtonBar().clearItem(PBottomButtonBar.B2);
      infoBox.setHelpText(getString("call_log"), getString("call_log_title")) ;
      PActionListener okListener = new PActionListener(){
                public void actionEvent(PActionEvent event){
                    if(  event.getActionCommand().equals(getString("lblGenericOk"))){
                        infoBox.closeForm();
                    }
                }
            };


      PActionItem okActionItem = new PActionItem
            (new PLabel(getString("lblGenericOk")),
             getString("hint/xdk/simpletextform/ok" ),
             okListener,
             (getString("lblGenericOk")));
      PActionItem[] actionItems = new PActionItem[1];
      actionItems[0] = okActionItem;
      infoBox.getBottomButtonBar().setItem(PBottomButtonBar.B3, okActionItem);
      infoBox.setLeftMenu(actionItems);
      infoBox.setWrappingDelimiters("@");
      infoBox.setText((new CallLegEntryDescriptor(data)).getDisplayString()) ;
      infoBox.showModal();

    }


    /**
     * Called when the end user presses the dial button
     */
    public void onDial()
    {
        PCallManager callManager = Shell.getInstance().getCallManager() ;
        CallLegEntry data = (CallLegEntry) m_listCallLog.getSelectedElement() ;
        if( data != null )
        {
            String strRemoteAddress = prepareAddressToDial( data );
            String strLocalAddress  = data.getLocalAddress().getAddress();
            strLocalAddress = SipParser.stripFieldParameters(strLocalAddress);

            try
            {
                Shell.getInstance().getDialingStrategy().dial(
                    PAddressFactory.getInstance().createAddress(strLocalAddress),
                    PAddressFactory.getInstance().createAddress(strRemoteAddress)) ;
            }
            catch (Exception e)
            {
                Shell.getInstance().showUnhandledException(e, true) ;
            }

            closeForm(OK) ;
        } else {
            MessageBox alert = new MessageBox(getApplication(), MessageBox.TYPE_ERROR) ;
            alert.setMessage(getString("lblErrorEmptyDial")) ;
            alert.showModal() ;
        }
    }



    /**
     * Invoked when the user presses the delete all button
     */
    public void onClearAll()
    {
        MessageBox alert = new MessageBox(getApplication(), MessageBox.TYPE_WARNING) ;
        alert.setMessage(getString("lblWarningDeleteAll")) ;
        if (alert.showModal() == MessageBox.OK) {
            CallLogManager.getInstance().removeAll();
            Shell.getCallManager().getMissedCallFeatureIndicator().clearAll();
        }
    }



    /**
     * Invoked when the end user presses the cancel button
     */
    public void onCancel()
    {
        //commit the data to the cache.ser file when exiting out of close
        //form. This keeps the data permanently .
        //CallLogManager.getInstance().commit();

        //remove the callLogListener added to CallManager when this form was
        //constructed. This avoids leaking listeners.
        CallLogManager.getInstance().removeCallLogListener(m_callLogListener);
        closeForm(CANCEL) ;
    }

     public void closeForm(int iExitCode){
       super.closeForm(iExitCode);
    }



   /**
    * called when the form is gaining focus
    *
    * @param formLosingFocus form that lost focus as a result of
    */
    public void onFocusGained(PForm formLosingFocus)
    {
        applyDialingStrategy() ;
    }

    public void onFormOpening()
    {
        // updateFilter() ;
    }


    /**
     * called when the form is losing focus
     *
     * @param formGainingFocus form that is gaining focus
     */
    public void onFocusLost(PForm formGainingFocus)
    {
        // Save State on focus loss
        try {
            if( m_iOldSortOrder != m_iSortOrder )
                getApplication().putObject("sortOrder", new Integer(m_iSortOrder)) ;
            if( m_iOldCallLogOutboundFilter != m_iCallLogOutboundFilter )
                getApplication().putObject("calllog_outbound_filter", new Integer(m_iCallLogOutboundFilter)) ;
            if( m_iOldCallLogInboundFilter != m_iCallLogInboundFilter )
                getApplication().putObject("calllog_inbound_filter", new Integer(m_iCallLogInboundFilter)) ;

        } catch (Exception e) {
            System.out.println("Unable to save call filter state: " + e) ;
            SysLog.log(e) ;
        }

    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private String prepareAddressToDial(CallLegEntry data)
    {
        String strRet = null;
        String strDialedAddress = (String) data.getOtherField("dialed_address") ;
        if (strDialedAddress != null)
        {
           strRet = strDialedAddress ;
        }
        else
        {
            strRet = data.getRemoteAddress().getAddress() ;
        }

        if( strRet != null )
            strRet = SipParser.stripFieldParameters(strRet);

        return strRet;

    }

    /**
     *
     */
    private void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_commandBar.setBounds(0, 0, 28, 108) ;
        container.add(m_commandBar) ;

        m_listCallLog.setBounds(28, 2, 129, 104) ;
        container.add(m_listCallLog) ;

        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }


    /**
     *
     */
    private void initCommandBar()
    {
        m_commandBar = new PCommandBar() ;

        m_commandBar.addActionListener(m_commandDispatcher) ;

        PActionItem actionFilter = new PActionItem(new PLabel(getImage("imgFilterIcon")),
            getString("hint/calllog/main/filter"),
            null,
            m_commandDispatcher.ACTION_FILTER) ;
        m_commandBar.addButton(actionFilter) ;

        PActionItem actionSort = new PActionItem(new PLabel(getImage("imgSortIcon")),
            getString("hint/calllog/main/sort"),
            null,
            m_commandDispatcher.ACTION_SORT) ;
        m_commandBar.addButton(actionSort) ;

        PActionItem actionDelete = new PActionItem(new PLabel(getImage("imgDeleteIcon")),
            getString("hint/calllog/main/delete"),
            null,
            m_commandDispatcher.ACTION_DELETE) ;
        m_commandBar.addButton(actionDelete) ;

       PActionItem actionMore = new PActionItem(new PLabel(getImage("imgOtherIcon")),
            getString("hint/calllog/main/info"),
            null,
            m_commandDispatcher.ACTION_MORE) ;
        m_commandBar.addButton(actionMore) ;

    }


    /**
     *  Initialize our pingtel task manager menus
     */
    private void initMenus()
    {

        PActionItem actionDial = new PActionItem
                (new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
                Shell.getInstance().getDialingStrategy().getActionHint(),
                m_commandDispatcher,
                m_commandDispatcher.ACTION_DIAL) ;

        PActionItem items[] = new PActionItem[4] ;

        // Initialize Left Menu
        items[0] = actionDial;

        items[1] = new PActionItem(new PLabel("Filter"),
            getString("hint/calllog/main/filter"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_FILTER) ;

        items[2] = new PActionItem(new PLabel("Sort"),
            getString("hint/calllog/main/sort"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_SORT) ;

        items[3] = new PActionItem(new PLabel("Info"),
            getString("hint/calllog/main/info"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_MORE) ;

        setLeftMenu(items) ;


        // Initialize Right menu

        PActionItem actionExit = new PActionItem(new PLabel("Exit"),
            getString("hint/core/system/exitform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CANCEL) ;

        PActionItem actionDeleteAll = new PActionItem(new PLabel("Delete All"),
            getString("hint/calllog/main/delete_all"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_CLEAR_ALL) ;

        items = new PActionItem[4] ;

        items[0] = new PActionItem(new PLabel("About"),
            getString("hint/core/system/aboutform"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_ABOUT) ;

        items[1]  = actionExit;

        items[2] = new PActionItem(new PLabel("Delete Entry"),
            getString("hint/calllog/main/delete"),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_DELETE) ;

        items[3] = actionDeleteAll ;
        setRightMenu(items) ;


        PBottomButtonBar menuControl = getBottomButtonBar() ;
        menuControl.setItem(PBottomButtonBar.B1, actionDeleteAll) ;
        menuControl.setItem(PBottomButtonBar.B2, actionExit) ;
        menuControl.setItem(PBottomButtonBar.B3, actionDial) ;
    }


    /**
     *
     */
    private void updateFilter()
    {

        //System.out.println("***********\n********UPDATE FILTER called\n**********");
        CallLog callLog = Shell.getCallLog() ;
        int iIndex = m_listCallLog.getSelectedIndex() ;

        // refresh our display
        try{
        m_listModel.removeAllElements() ;
        }catch(Exception e ){
            SysLog.log(e);
        }
        CallLegEntry entries[] = CallLogManager.getInstance().getEntriesByCallState
            (m_iCallLogInboundFilter, m_iCallLogOutboundFilter, m_iSortOrder) ;

        if (entries != null) {
            m_listModel.addElements(entries) ;
        }

        // Try to restore the initial selection if possible
        if ((iIndex == -1) && (m_listModel.getSize() > 0))
            m_listCallLog.setSelectedIndex(0) ;
        else {
            if (m_listModel.getSize() > iIndex)
                m_listCallLog.setSelectedIndex(iIndex) ;
            else {
                m_listCallLog.setSelectedIndex(m_listModel.getSize()-1) ;
            }
        }

        // Enable / Disable inappropriate items
        PBottomButtonBar bar = getBottomButtonBar() ;
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;

        if (m_listModel.getSize() == 0) {
            if (m_commandBar != null)
            {
                m_commandBar.enableByAction(m_commandDispatcher.ACTION_DELETE, false) ;
                m_commandBar.enableByAction(m_commandDispatcher.ACTION_MORE, false) ;
                bar.enableByAction(false,m_commandDispatcher.ACTION_CLEAR_ALL);
                leftMenu.enableItemsByAction(false,m_commandDispatcher.ACTION_MORE);
                rightMenu.enableItemsByAction(false,m_commandDispatcher.ACTION_DELETE);
                rightMenu.enableItemsByAction(false,m_commandDispatcher.ACTION_CLEAR_ALL);
            }
            if (bar != null)
                bar.clearItem(PBottomButtonBar.B3) ;
        } else {
            if (m_commandBar != null)
            {
                m_commandBar.enableByAction(m_commandDispatcher.ACTION_DELETE, true) ;
                m_commandBar.enableByAction(m_commandDispatcher.ACTION_MORE, true) ;
                bar.enableByAction(true,m_commandDispatcher.ACTION_CLEAR_ALL);
                leftMenu.enableItemsByAction(true,m_commandDispatcher.ACTION_MORE);
                rightMenu.enableItemsByAction(true,m_commandDispatcher.ACTION_DELETE);
                rightMenu.enableItemsByAction(true,m_commandDispatcher.ACTION_CLEAR_ALL);
            }
            if (bar != null) {
                PActionItem actionDial = new PActionItem(new PLabel(Shell.getInstance().getDialingStrategy().getAction()),
                    Shell.getInstance().getDialingStrategy().getActionHint(),
                    m_commandDispatcher,
                    m_commandDispatcher.ACTION_DIAL) ;
                bar.setItem(PBottomButtonBar.B3, actionDial) ;
            }
        }
        enableDialAction() ;

    }


    /**
     * This method enables/disables the "dial" label on any menus/bottom
     * button bars based on the state of the dial field.  If the dial field
     * has data then the action is enabled, otherwise if the dial field is
     * blank then the action is disabled.
     */
    protected void enableDialAction()
    {
        boolean bEnable = (m_listModel.getSize() > 0) ;
        enableMenusByAction(bEnable, m_commandDispatcher.ACTION_DIAL) ;
    }


    /**
     *
     */
    private void initCallLog()
    {
        m_listCallLog = new PList() ;
        m_listModel = new PDefaultListModel() ;
        m_listCallLog.setListModel(m_listModel) ;

        m_listCallLog.addActionListener(m_commandDispatcher) ;
        m_listCallLog.setItemRenderer(new CallLogEntryRenderer()) ;
    }


    /**
     * This form can morph slighly based on the current dialing strategy.
     * This method applies those morphing changes
     */
    private void applyDialingStrategy()
    {
        DialingStrategy strategy = Shell.getInstance().getDialingStrategy() ;

        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PActionItem actionDial = new PActionItem(new PLabel(strategy.getAction()),
            strategy.getActionHint(),
            m_commandDispatcher,
            m_commandDispatcher.ACTION_DIAL) ;
        menuControl.setItem(PBottomButtonBar.B3, actionDial) ;

        enableDialAction() ;
    }



//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////
    /**
     * Listens to action events and dispatches control to the appropriate
     * onXXX handler.
     */
    private class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_DIAL             = "action_dial" ;
        public final String ACTION_DELETE           = "action_delete" ;
        public final String ACTION_FILTER           = "action_filter" ;
        public final String ACTION_SORT             = "action_sort" ;
        public final String ACTION_CLEAR_ALL        = "action_clear_all" ;
        public final String ACTION_CANCEL           = "action_cancel" ;
        public final String ACTION_ABOUT           = "action_about" ;
        public final String ACTION_MORE           = "action_more" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_DIAL)) {
                onDial() ;
            } else  if (event.getActionCommand().equals(ACTION_DELETE)) {
                onDelete() ;
            } else  if (event.getActionCommand().equals(ACTION_ABOUT)) {
                SystemAboutBox box = new SystemAboutBox(getApplication());
                box.onAbout();
            } else  if (event.getActionCommand().equals(ACTION_FILTER)) {
                onFilter() ;
            } else  if (event.getActionCommand().equals(ACTION_SORT)) {
                onSort() ;
            } else  if (event.getActionCommand().equals(ACTION_CLEAR_ALL)) {
                onClearAll() ;
            } else  if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else if (event.getActionCommand().equals(PList.ACTION_DOUBLE_CLICK)) {
                // Default Double-click action
                onDial() ;
            } else if (event.getActionCommand().equals(ACTION_MORE)) {
                //on pressing more button
                onMore() ;
            }
        }
    }

    /**
     * an inner class that implements CallLogListener
     * to update CallLogModel whenever contents get changed.
     */
    private class icCallLogListener implements CallLogListener
    {

        /**
         * This will take care of refreshing call log form when entry is added,
         * or removed.
         */
        public void contentsChanged(CallLogEvent e)
        {
           updateFilter();
        }
    }
}

