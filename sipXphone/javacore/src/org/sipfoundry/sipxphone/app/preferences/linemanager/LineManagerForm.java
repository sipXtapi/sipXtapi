/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/linemanager/LineManagerForm.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.preferences.linemanager ;

import java.lang.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sip.* ;
import org.sipfoundry.sip.event.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;


/**
 * The Line Manager form displays status for each of our lines.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LineManagerForm extends SimpleListForm
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private boolean m_bReadOnly ;               // Are we in read-only mode?
    private icActionDispatcher m_dispatcher ;   // action dispatcher
    private LineListModel m_model ;             // data model

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor requiring both an application context and a read-only flag.
     */
    public LineManagerForm(PForm formParent, boolean bReadOnly)
    {
        super(formParent, "Line Manager") ;

        m_bReadOnly = bReadOnly ;
        m_dispatcher = new icActionDispatcher() ;

        setStringResourcesFile("LineManager.properties") ;
        setHelpText(getString("preference_linemgr"), getString("preference_linemgr_title")) ;

        initBars() ;
        setItemRenderer(new LineRenderer()) ;

        m_model = new LineListModel() ;
        setDefaultListModel(m_model) ;
        addListListener(new icListListener()) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Prompts the user for all the information required for a new line
     * submission.
     */
    public void onAddLine()
    {
        LineEditorForm form = new LineEditorForm(getApplication()) ;
        form.showModal() ;
    }


    /**
     * Forces the selected line to re-register
     */
    public void onReregisterLine()
    {
        onEnableLine() ;
    }


    public void onEditLine()
    {

    }

    public void onDeleteLine()
    {
        SipLine line = (SipLine) getSelectedElement() ;
        if (line != null)
        {
            SipLineManager.getInstance().setDefaultLine(line) ;
        }
    }


    public void onDefaultLine()
    {
        SipLine line = (SipLine) getSelectedElement() ;
        if (line != null)
        {
            SipLineManager.getInstance().setDefaultLine(line) ;
        }
    }


    public void onEnableLine()
    {
        SipLine line = (SipLine) getSelectedElement() ;
        if (line != null)
        {
            line.enable(true) ;
        }
    }

    public void onDisableLine()
    {
        SipLine line = (SipLine) getSelectedElement() ;
        if (line != null)
        {
            line.enable(false) ;
        }
    }


    /**
     * Invoked up form opening (NOTE: this is not focus change).
     */
    public void onFormOpening()
    {
        // Refresh the model and add the model as a line lister.
        m_model.refresh() ;
        SipLineManager.getInstance().addLineListener(m_model) ;
    }


    /**
     * Invoked up form closing (NOTE: this is not focus change).
     */
    public void onFormClosing()
    {
        // Remove the model as a line listener.
        SipLineManager.getInstance().removeLineListener(m_model) ;
    }


    /**
     * Display Information of the selected line, using the fomrat:
     *
     * <pre>
     * Line Information
     *
     * Registration:
     * User:
     * URL:
     * Display Name: (if present)
     *
     * Status:
     * SIP Code: (if failed)
     * Authentication Scheme (if failed)
     * Authentication Realm: (if failed)
     *
     * Forwarding:
     * </pre>
     */
    public void onInfoLine()
    {
        SipLine line = (SipLine) getSelectedElement() ;
        if (line != null)
        {
            //
            // Gather Settings
            //
//System.out.println("INFO FORM: UserEnteredUrl = " + line.getUserEnteredUrl());

            SipParser parser = new SipParser(line.getUserEnteredUrl()) ;

            String strDisplayName = parser.getDisplayName() ;
            String strUser = parser.getUser() ;
            parser.setDisplayName("") ;
            parser.setUser("") ;
            String strRegisterURL = parser.render() ;

            String strStatus = line.getStateAsString(line.getState()) ;

            String strResponseText = line.getFailedStatusText() ;
            if ((strResponseText == null) || (strResponseText.length() == 0))
            {
                strResponseText = getString("lblNotAvailable") ;
            }
            int iResponseCode = line.getFailedStatusCode() ;
            String strResponseCode = (String) ((iResponseCode == 0)
                    ? "" : "(" + Integer.toString(iResponseCode) + ")") ;

            String strAuthScheme = line.getFailedAuthScheme() ;
            if (strAuthScheme == null)
                strAuthScheme = getString("lblNotAvailable") ;

            String strAuthRealm = line.getFailedAuthRealm() ;
            if (strAuthRealm == null)
                strAuthRealm = getString("lblNotAvailable") ;

            String strRegistration ;
            if (line.getState() == SipLine.LINE_STATE_PROVISIONED)
                strRegistration = getString("lblProvision") ;
            else
                strRegistration = getString("lblRegister") ;

            String strVisible = line.isShowLineEnabled() ? getString("lblYes") : getString("lblNo") ;
            String strAllowForwarding = line.isAllowForwardingEnabled() ?
                    getString("lblEnabled") : getString("lblDisabled") ;

            //
            // Build Result String
            //
            StringBuffer results = new StringBuffer() ;

            results.append(getString("lblLineInformation-Registration")) ;
            results.append(strRegistration) ;

            results.append(getString("lblLineInformation-User")) ;
            results.append(strUser) ;

            results.append(getString("lblLineInformation-RegisterURL")) ;
            results.append(strRegisterURL) ;

            if (strDisplayName != null && strDisplayName.length() > 0)
            {
                results.append(getString("lblLineInformation-DisplayName")) ;
                results.append(strDisplayName) ;
            }

            if ((line.getState() == SipLine.LINE_STATE_FAILED) ||
                    (line.getState() == SipLine.LINE_STATE_EXPIRED))
            {
                results.append(getString("lblLineInformation-Status")) ;
                if (line.getState() == SipLine.LINE_STATE_EXPIRED)
                    results.append(getString("lblFailed")) ;
                else
                    results.append(getString("lblPending")) ;


                results.append(getString("lblLineInformation-ResponseCode")) ;
                results.append(strResponseText) ;
                results.append(" ") ;
                results.append(strResponseCode) ;

                results.append(getString("lblLineInformation-AuthScheme")) ;
                results.append(strAuthScheme) ;

                results.append(getString("lblLineInformation-AuthRealm")) ;
                results.append(strAuthRealm) ;
            }
            else
            {
                results.append(getString("lblLineInformation-Status")) ;
                results.append(strStatus) ;
            }

            results.append(getString("lblLineInformation-Forwarding")) ;
            results.append(strAllowForwarding) ;

            results.append(getString("lblLineInformation-CallOutAsLine"));
            SipLine defaultLine = SipLineManager.getInstance().getDefaultLine() ;
            if (defaultLine.equals(line))
                results.append(getString("lblYes")) ;
            else
                results.append(getString("lblNo")) ;

            //
            // Display Results
            //
            SimpleTextForm formDisplay = new SimpleTextForm(getApplication(), getString("lblLineInformationTitle")) ;
            formDisplay.getBottomButtonBar().clearItem(PBottomButtonBar.B2) ;
            formDisplay.getLeftMenuComponent().removeItemAt(1) ;
            formDisplay.setText(results.toString()) ;
            formDisplay.setHelpText(getString("preference_linemgr"), getString("preference_linemgr_title")) ;
            formDisplay.showModal() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implemenation
////

    /**
     * Initialize the bottom button bar and command bar.
     */
    protected void initBars()
    {
        // Init the command bar
        enableCommandBar(true) ;

        PCommandBar commandBar = getCommandBar() ;
        PBottomButtonBar bottomBar = getBottomButtonBar() ;
        PMenuComponent leftMenu = getLeftMenuComponent() ;
        PMenuComponent rightMenu = getRightMenuComponent() ;


        // Clear bottom button bar and menus
        bottomBar.clearItem(PBottomButtonBar.B1);
        bottomBar.clearItem(PBottomButtonBar.B2);
        bottomBar.clearItem(PBottomButtonBar.B3);
        leftMenu.removeAllItems() ;
        rightMenu.removeAllItems() ;

/*
        // "Add" Action
        PActionItem actionAdd = new PActionItem(
                new PLabel(getImage("imgAddIcon")),
                "hint/preferences/linemanager/editor/add",
                m_dispatcher,
                m_dispatcher.ACTION_ADD_LINE) ;
        commandBar.addButton(actionAdd) ;

        actionAdd = new PActionItem(
                new PLabel("Add Line"),
                "hint/preferences/linemanager/editor/add",
                m_dispatcher,
                m_dispatcher.ACTION_ADD_LINE) ;
        leftMenu.addItem(actionAdd) ;

        // "Edit" Action
        PActionItem actionEdit = new PActionItem(
                new PLabel(getImage("imgEditIcon")),
                "hint/preferences/linemanager/editor/edit",
                m_dispatcher,
                m_dispatcher.ACTION_EDIT_LINE) ;
        commandBar.addButton(actionEdit) ;

        actionEdit = new PActionItem(
                new PLabel("Edit Line"),
                "hint/preferences/linemanager/editor/edit",
                m_dispatcher,
                m_dispatcher.ACTION_EDIT_LINE) ;
        leftMenu.addItem(actionEdit) ;

        // "Delete" Action
        PActionItem actionDelete = new PActionItem(
                new PLabel(getImage("imgDeleteIcon")),
                "hint/preferences/linemanager/editor/delete",
                m_dispatcher,
                m_dispatcher.ACTION_DELETE_LINE) ;
        commandBar.addButton(actionDelete) ;

        actionDelete = new PActionItem(
                new PLabel("Remove Line"),
                "hint/preferences/linemanager/editor/delete",
                m_dispatcher,
                m_dispatcher.ACTION_DELETE_LINE) ;
        leftMenu.addItem(actionDelete) ;
*/
        // "Info" Action
        PActionItem actionInfo = new PActionItem(
                new PLabel(getImage("imgOtherIcon")),
                getString("hint/preferences/linemanager/editor/info"),
                m_dispatcher,
                m_dispatcher.ACTION_INFO_LINE) ;
        commandBar.addButton(actionInfo) ;

        actionInfo = new PActionItem(
                new PLabel(getString("lblLineInfo")),
                getString("hint/preferences/linemanager/editor/info"),
                m_dispatcher,
                m_dispatcher.ACTION_INFO_LINE) ;
        leftMenu.addItem(actionInfo) ;

        // Re-Register Action
        PActionItem actionReregister = new PActionItem(
                new PLabel(getString("lblForceRegister")),
                getString("hint/preferences/linemanager/editor/force_register"),
                m_dispatcher,
                m_dispatcher.ACTION_REREGISTER_LINE) ;
        // commandBar.addButton(actionInfo) ;
        leftMenu.addItem(actionReregister) ;
        bottomBar.setItem(PBottomButtonBar.B1, actionReregister);


        // "Cancel" Action
        PActionItem actionExit = new PActionItem(new PLabel(getString("lblGenericCancel")),
                getString("hint/core/system/cancelform"),
                m_dispatcher,
                m_dispatcher.ACTION_EXIT) ;

        bottomBar.setItem(PBottomButtonBar.B2, actionExit);
        rightMenu.addItem(actionExit) ;

        setActionEnable(m_dispatcher.ACTION_ADD_LINE, false) ;
        setActionEnable(m_dispatcher.ACTION_EDIT_LINE, false) ;
        setActionEnable(m_dispatcher.ACTION_DELETE_LINE, false) ;
        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, false) ;
        setActionEnable(m_dispatcher.ACTION_INFO_LINE, false) ;
/*
        // "Make Default" Action
        PActionItem actionDefault = new PActionItem(
                new PLabel("Def Out"),
                "hint/preferences/linemanager/editor/default",
                m_dispatcher,
                m_dispatcher.ACTION_DEFAULT_LINE) ;
        commandBar.addButton(actionDefault) ;
        leftMenu.addItem(actionDefault) ;
*/
    }


    protected void setActionEnable(String strAction, boolean bEnable)
    {
        enableMenusByAction(bEnable, strAction) ;

        PCommandBar commandBar = getCommandBar() ;
        if (commandBar != null)
        {
            commandBar.enableByAction(strAction, bEnable) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested/Inner Classes
////

    /**
     * This inner classes listens for list selection changes and enables/
     * disables the appropriate actions
     */
    private class icListListener implements PListListener
    {
        SipLineManager manager = SipLineManager.getInstance() ;
        SipLine lineDefault = manager.getDefaultLine() ;

        public void selectChanged(PListEvent event)
        {
            SipLine line = (SipLine) getSelectedElement() ;
            if (line != null)
            {
                // Enable everything that should be enabled given that an
                // item is selected.
                setActionEnable(m_dispatcher.ACTION_EDIT_LINE, true) ;
                setActionEnable(m_dispatcher.ACTION_DELETE_LINE, true) ;
                setActionEnable(m_dispatcher.ACTION_INFO_LINE, true) ;

                // Only allow setting the default line, if it isn't already
                // the default line
                if ((lineDefault == null) && !lineDefault.equals(line))
                    setActionEnable(m_dispatcher.ACTION_DEFAULT_LINE, true) ;
                else
                    setActionEnable(m_dispatcher.ACTION_DEFAULT_LINE, false) ;

                // Next enable/disable everything that may be state dependent.
                int iState = line.getState() ;
                switch (iState)
                {
                    case SipLine.LINE_STATE_EXPIRED:
                    case SipLine.LINE_STATE_FAILED:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, true) ;
                        break ;
                    case SipLine.LINE_STATE_DISABLED:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, false) ;
                        break ;
                    case SipLine.LINE_STATE_PROVISIONED:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, false) ;
                        break ;
                    case SipLine.LINE_STATE_REGISTERED:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, true) ;
                        break ;
                    case SipLine.LINE_STATE_TRYING:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, false) ;
                        break ;
                    case SipLine.LINE_STATE_UNKNOWN:
                        setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE, true) ;
                        break ;
                }
            }
            else
            {
                // If nothing is selected, disable all.
                setActionEnable(m_dispatcher.ACTION_EDIT_LINE, false) ;
                setActionEnable(m_dispatcher.ACTION_DELETE_LINE, false) ;
                setActionEnable(m_dispatcher.ACTION_DEFAULT_LINE, false) ;
                setActionEnable(m_dispatcher.ACTION_INFO_LINE, false) ;
                setActionEnable(m_dispatcher.ACTION_REREGISTER_LINE , false) ;
            }
        }
    }

    /**
     * Helper class that listens for action commands and dispatches to various
     * onXXX handlers.
     */
    private class icActionDispatcher implements PActionListener
    {
        public final String ACTION_ADD_LINE = "ACTION_ADD_LINE" ;
        public final String ACTION_EDIT_LINE = "ACTION_EDIT_LINE" ;
        public final String ACTION_DELETE_LINE = "ACTION_DELETE_LINE" ;
        public final String ACTION_DEFAULT_LINE = "ACTION_DEFAULT_LINE" ;
        public final String ACTION_INFO_LINE = "ACTION_INFO_LINE" ;
        public final String ACTION_ENABLE_LINE = "ACTION_ENABLE_LINE" ;
        public final String ACTION_DISABLE_LINE = "ACTION_DISABLE_LINE" ;
        public final String ACTION_REREGISTER_LINE = "ACTION_REREGISTER_LINE" ;
        public final String ACTION_EXIT = "ACTION_EXIT" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_ADD_LINE))
            {
                onAddLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_EDIT_LINE))
            {
                onEditLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_DELETE_LINE))
            {
                onDeleteLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_DEFAULT_LINE))
            {
                onDefaultLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_INFO_LINE))
            {
                onInfoLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_REREGISTER_LINE))
            {
                onReregisterLine() ; ;
            }
            else if (event.getActionCommand().equals(ACTION_ENABLE_LINE))
            {
                onEnableLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_DISABLE_LINE))
            {
                onDisableLine() ;
            }
            else if (event.getActionCommand().equals(ACTION_EXIT))
            {
                closeForm(OK) ;
            }
        }
    }
}
