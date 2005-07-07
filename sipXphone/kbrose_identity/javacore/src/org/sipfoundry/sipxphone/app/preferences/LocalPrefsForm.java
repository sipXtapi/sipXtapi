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

import java.awt.Insets;
import java.util.Hashtable ;
import java.io.IOException ;

import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.sys.SystemDefaults ;
import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.util.PingerInfo ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp ;
import org.sipfoundry.util.* ;

/**
 * Preferences setting UI associated to local-config OR access to a set of config 
 * parameters that were deemed harmless enough for a (exactly one) client that 
 * didn't want to provide access to Call Handling settings but wanted access to 
 * bringing IX to the foreground.
 * 
 * @author Douglas Hubler
 */
public class LocalPrefsForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** return codes from Show Modal: Cancel, didn't do anything */
    public final static int CANCEL  = 0 ;
    /** return codes from Show Modal: Set */
    public final static int SET    = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** container that houses the controls */
    protected PScrollableComponentContainer m_contOptionList ;

    /** Forward on busy checkbox */
    protected PCheckbox     m_chkBringToFrontOnCall ;

    /** dispatches commands to various onXXX handlers */
    protected icCommandDispatcher m_dispatcher = new icCommandDispatcher() ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * stock constructor requiring a parent form
     */
    public LocalPrefsForm(PForm formParent)
    {
        super(formParent, "Local Preferences");
        setStringResourcesFile("LocalPrefsForm.properties") ;
        setTitle(getString("lblLocalPrefsTitle")) ;
        setIcon(getImage("imgPrefsAppsIcon")) ;

        initComponents() ;
        layoutComponents() ;
        initMenubar() ;
        setHelpText(getString("preference_local"), getString("preference_local_title")) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void initComponents()
    {
        // Create our scrollable container
        m_contOptionList  = new PScrollableComponentContainer(4, PButtonEvent.BID_R1) ;
        m_contOptionList.setBounds(28, 2, 129, 104) ;

        // Create our check boxes
        m_chkBringToFrontOnCall = new PCheckbox(getString("lblBringToFrontOnCall")) ;
        m_chkBringToFrontOnCall.setHint(getString("hint/preference/local_prefs/bring_to_front_on_call")) ;
        m_chkBringToFrontOnCall.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_chkBringToFrontOnCall.setState(PingerApp.getInstance().isBringToFrontOnCallEnabled());

        // Add components to scrollable container
        m_contOptionList.addComponent(m_chkBringToFrontOnCall) ;

        // finally add scrollable container to the device preferences form
        addToDisplayPanel(m_contOptionList) ;
    }

    protected void layoutComponents()
    {
        PContainer container = new PContainer() ;

        container.setLayout(null) ;

        m_contOptionList.setBounds(28, 2, 129, 104) ;
        container.add(m_contOptionList) ;

        addToDisplayPanel(container, new Insets(0, 0, 0, 0)) ;
    }

    protected void initMenubar()
    {
        // Initialize Bottom Menu
        PBottomButtonBar menuControl = getBottomButtonBar() ;

        PActionItem itemSet = new PActionItem(
                new PLabel(getString("lblGenericOk")),
                getString("hint/preferences/common/ok"),
                m_dispatcher,
                m_dispatcher.ACTION_SET) ;

        menuControl.setItem(PBottomButtonBar.B3, itemSet) ;
        getLeftMenuComponent().addItem(itemSet) ;

        PActionItem itemCancel = new PActionItem(
                new PLabel(getString("lblGenericCancel")),
                getString("hint/preferences/common/cancel"),
                m_dispatcher,
                m_dispatcher.ACTION_CANCEL) ;
        menuControl.setItem(PBottomButtonBar.B2, itemCancel) ;
        getLeftMenuComponent().addItem(itemCancel) ;
    }

    protected void onSet()
    {
        if (m_chkBringToFrontOnCall.getState() != PingerApp.getInstance().isBringToFrontOnCallEnabled())
        {
            try
            {
                String path = PingerConfig.getProfilePath(PingerConfig.LOCAL_CONFIG);
                OrderedHashtable localPrefs = PingerConfigFileParser.getConfigFile(path);
                localPrefs.put(PingerConfig.PHONESET_BRING_TO_FRONT_ON_CALL,
                        m_chkBringToFrontOnCall.getState() ? "ENABLE" : "DISABLE");
                PingerConfigFileParser.setConfigFile(path, localPrefs);
                PingerInfo.getInstance().reboot() ;
            }
            catch (IOException ioe)
            {
                // internal error.  Need more of a warning here? User
                // will not be able to do anything
                ioe.printStackTrace();
            }
        }
    }

    protected void onCancel()
    {
        closeForm(CANCEL) ;
    }

//////////////////////////////////////////////////////////////////////////////
// Inner classes
////

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
        }
    }
}
