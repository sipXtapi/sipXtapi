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


package org.sipfoundry.sipxphone.app.preferences ;

import javax.telephony.* ;

import java.awt.Insets ;
import java.util.Enumeration ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;


/**
 * The UserMaintenanceForm
 *
 * @author Robert J. Andreasen, Jr.
 */
public class UserMaintenanceForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** return codes from Show Modal: Cancel, do do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Dial */
    public final static int SET    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PList m_list ;
    protected PDefaultListModel m_listModel ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring an application context
     */
    public UserMaintenanceForm(PForm formParent)
    {
        super(formParent, "User Maintenance") ;
        setStringResourcesFile("UserMaintenanceForm.properties") ;
        setTitle(getString("lblUserMaintenanceTitle")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;


        initComponents() ;
        layoutComponents() ;
        initMenubar() ;

        setHelpText(getString("usermaint_select"),
            getString("usermaint_select_title")) ;
    }


    protected void initComponents()
    {
        m_listModel = new PDefaultListModel() ;
        m_list = new PList(m_listModel) ;
        m_list.addActionListener(m_dispatcher) ;
        m_list.setItemRenderer(new PDefaultItemRenderer(PDefaultItemRenderer.EAST)) ;

        m_listModel.addElement("Change admin password") ;
    }


    protected void layoutComponents()
    {
        addToDisplayPanel(m_list, new Insets(1, 1, 1, 1)) ;
    }


    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;
        PMenuComponent menuComponent = getLeftMenuComponent() ;

        PActionItem itemOK = new PActionItem(
                new PLabel(getString("lblGenericOk")),
                getString("hint/preferences/common/ok"),
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;
        menuControl.setItem(PBottomButtonBar.B3, itemOK) ;
        menuComponent.addItem(itemOK) ;

        PActionItem itemCancel = new PActionItem(
                new PLabel("Cancel"),
                getString("hint/preferences/common/cancel"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, itemCancel) ;
        menuComponent.addItem(itemCancel) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Invoked when the user has finished making changes and ready to commit.
     */
    protected void onSet()
    {
        switch (m_list.getSelectedIndex()) {
            case 0:
                ChangePasswordWizard wiz = new ChangePasswordWizard(this, "admin") ;
                wiz.showModal() ;
                break ;
        }
    }


    protected void onCancel()
    {
        closeForm(CANCEL) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

    /**
     * Dispatches action commands to various onXXX handlers
     */
    private class icCommandDispatcher implements PActionListener
    {
        public final String ACTION_CANCEL    = "action_cancel" ;
        public final String ACTION_SET       = "action_set" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            }
            else if (event.getActionCommand().equals(ACTION_SET)) {
                onSet() ;
            }
            else if (event.getActionCommand().equals(m_list.ACTION_DOUBLE_CLICK)) {
                onSet() ;
            }
        }
    }
}
