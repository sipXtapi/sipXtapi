/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/AuthorizeProfilesForm.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys ;

import org.sipfoundry.sipxphone.*;
import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.awt.form.*;
import org.sipfoundry.util.*;
import java.util.*;
import java.awt.* ;

/**
 * Encrypted profiles delivered from ConfigServer need username and password to 
 * decrypt
 *
 * @author Douglas Hubler
 */
public class AuthorizeProfilesForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private icCommandDispatcher m_controller;

    private String[] m_info;

    private PTextField m_userId;

    private PTextField m_password;

    private PDisplayArea m_msg;

    private int m_passwordAttempts;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public AuthorizeProfilesForm(Application app)
    {
        super(app, "Authorize Profiles");

        AppResourceManager resMgr = AppResourceManager.getInstance();
        setIcon(resMgr.getImage("imgSecurityIcon")) ;
        setTitle(getString("lblAuthProfilesTitle"));
        // QUESTION: Alison, do we need a help page?
        m_controller = new icCommandDispatcher();
        initComponents();
        initBottomButtonBar();
    }

    public void setPasswordAttempts(int passwordAttempts)
    {
        m_passwordAttempts = passwordAttempts;
    }

    public void setUserInfo(String[] info)
    {
        m_info = info;
        m_userId.setText(info[0]);
        m_password.setText(info[1]);
    }

    public void initComponents()
    {
        PContainer container = new PContainer();
        container.setLayout(null);
        addToDisplayPanel(container, new Insets(1, 1, 1, 1));
        Dimension size = container.getSize();

        PCommandBar commandBar = new PCommandBar();
        commandBar.setBounds(0, 0, 27, 108);
        container.add(commandBar);

        TextFieldActionEventAdapter textfieldAdapter = new TextFieldActionEventAdapter(commandBar);
        textfieldAdapter.populateCommandbar();
        textfieldAdapter.setAllowNumericMode(true);

        // Use scrollable, PContainer does not appear to have a
        // focus manager. Both text widgets get focus.
        PScrollableComponentContainer scroller = new PScrollableComponentContainer(4, PButtonEvent.BID_R1);
        scroller.setBounds(27, 0, 120, 98);
        container.add(scroller);

        PLabel msg = new PLabel("");
        msg.setAlignment(PLabel.ALIGN_CENTER);
        if (m_passwordAttempts == 0)
        {
            msg.setText(getString("lblAuthProfilesFirstPwdAttempt"));
        }
        else
        {
            msg.setText(getString("lblAuthProfilesInvalidPwd"));
        }
        scroller.addComponent(msg);

        PLabel info = new PLabel(getString("lblAuthProfilesUserMsg"));
        info.setAlignment(PLabel.ALIGN_CENTER);
        scroller.addComponent(info);

        m_userId = new PTextField("");
        m_userId.setLabel(getString("lblAuthProfilesUserId"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);
        scroller.addComponent(m_userId);
        textfieldAdapter.addTextfield(m_userId, false);

        m_password = new PTextField("");
        m_password.setLabel(getString("lblAuthProfilesPin"), PLabel.ALIGN_NORTH | PLabel.ALIGN_WEST);
        m_password.setAlphanumericMode(false);
        m_password.setEchoChar('*');
        scroller.addComponent(m_password);
        textfieldAdapter.addTextfield(m_password, false);
    }

    public void doLayout()
    {
        // Need these here, otherwise text input doesn't draw correctly
        m_userId.setBounds(0, 0, 90, 27);
        m_userId.doLayout();

        m_password.setBounds(0, 29, 90, 27);
        m_password.doLayout();

        m_userId.setFocus(true);

        super.doLayout();
    }

    private void initBottomButtonBar()
    {
        PBottomButtonBar bottom = getBottomButtonBar();
        
        // TODO: Write hints
        PActionItem cancel = new PActionItem(new PLabel(getString("lblGenericCancel")),
                                         getString("hint/core/system/cancelform"),
                                         m_controller,
                                         m_controller.ACTION_CANCEL);
        bottom.setItem(PBottomButtonBar.B2, cancel);

        PActionItem ok = new PActionItem(new PLabel(getString("lblGenericOk")),
                                         getString("hint/core/system/okform"),
                                         m_controller,
                                         m_controller.ACTION_OK);
        bottom.setItem(PBottomButtonBar.B3, ok);
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


//////////////////////////////////////////////////////////////////////////////
// Private Methods
////
    private void onOk()
    {
        m_info[0] = m_userId.getText();
        m_info[1] = m_password.getText();

        closeForm(0);
    }

    private void onCancel()
    {
        closeForm(1);
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    private class icCommandDispatcher implements PActionListener
    {
        final String ACTION_CANCEL = "action_cancel";
        final String ACTION_OK = "action_ok";
        
        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CANCEL)) {
                onCancel() ;
            } else  if (event.getActionCommand().equals(ACTION_OK)) {
                onOk() ;
            }
        }
    }
}


