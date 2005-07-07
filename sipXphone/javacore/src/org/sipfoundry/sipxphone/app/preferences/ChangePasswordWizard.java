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
import org.sipfoundry.sipxphone.sys.user.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;


/**
 * The Change Password Wizard is used to change the password of the particular
 * user.  It is assumed that the controller has already check for the proper
 * authentication.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ChangePasswordWizard extends SimpleTextInputForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public final static int STATE_1ST_TRY = 0 ;
    public final static int STATE_2ND_TRY = 1 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** the current state of the wizard */
    protected int    m_iState ;
    protected String m_strPassword ;
    protected String m_strUser ;

//////////////////////////////////////////////////////////////////////////////
// Constructions
////
    /**
     * Constructor
     *
     * @param formParent Parent of this form.
     * @param strUser The user who password you are going to change
     */
    public ChangePasswordWizard(PForm formParent, String strUser)
    {
        super(formParent, "Change Password") ;
        setStringResourcesFile("ChangePasswordWizard.properties") ;

        setTitle(getString("lblChangePasswordTitle")) ;

        setMode(SimpleTextInputForm.MODE_PASSWORD_TEXT) ;
        setHelpText(getString("enter_password"), getString("enter_password_title")) ;
        m_strUser = strUser ;
        initialize1stTry() ;
    }


    /**
     * Initialize the state of the wizard into the 1st try
     */
    protected void initialize1stTry()
    {
        m_iState  = STATE_1ST_TRY ;

        setLabel(getString("lblNewPassword")) ;
        setInstructions(getString("lblNewPasswordInstructions") + m_strUser + ".") ;
        setText("") ;
        m_strPassword = null ;
    }


    /**
     * Initialize the state of the wizard into the 2nd try
     */
    protected void initialize2ndTry()
    {
        m_iState = STATE_2ND_TRY ;

        setLabel(getString("lblConfirmPassword")) ;
        setInstructions(getString("lbConfirmPasswordInstructions") + m_strUser + ".") ;
        setText("") ;
    }


    /**
     * Invoked by the framework when the ok button is pressed
     */
    public void onOk()
    {
        String strPassword ;

        switch (m_iState) {
            case STATE_1ST_TRY:
                m_strPassword = getText() ;
                initialize2ndTry() ;
                break ;
            case STATE_2ND_TRY:
                strPassword = getText() ;
                if (strPassword.equals(m_strPassword)) {

                    try {
                        PUserManager userManager = new PUserManager() ;
                        userManager.changeUserPassword(m_strUser, m_strPassword) ;

                        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_INFORMATIONAL) ;
                        messageBox.setMessage(getString("lblSuccessfulPasswordChange")) ;
                        messageBox.showModal() ;
                        super.onOk() ;
                    } catch (Exception e) {
                        MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_ERROR) ;
                        messageBox.setMessage(getString("lblPasswordError") + "\n" + e.toString()) ;
                        messageBox.showModal() ;
                        initialize1stTry() ;
                    }


                } else {
                    MessageBox messageBox = new MessageBox(this, MessageBox.TYPE_INFORMATIONAL) ;
                    messageBox.setMessage(getString("lblPasswordMismatch")) ;
                    messageBox.showModal() ;
                    initialize1stTry() ;
                }
        }
    }
}
