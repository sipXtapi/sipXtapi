/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/linemanager/LineEditorForm.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.app.preferences.linemanager;


import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.SystemDefaults ;

/**
 * <<TODO:: DESCRIBE ME>>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LineEditorForm extends ComplexForm
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PTextField m_tfUserName ;
    private PTextField m_tfPassword ;
    private PTextField m_tfRealm ;
    private PTextField m_tfDisplayName ;
    private PTextField m_tfRegisterURL ;

    private PCheckbox  m_cbEnableLine ;
    private PCheckbox  m_cbApplyCallHandling ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public LineEditorForm(Application app)
    {
        super(app, "New Line") ;

        initComponents() ;
        addComponents() ;

        enableCommandBar(true) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    protected void initComponents()
    {
        m_tfUserName = new PTextField() ;
        m_tfUserName.setLabel("User Name", PLabel.ALIGN_WEST | PLabel.ALIGN_NORTH) ;
        m_tfUserName.setAlphanumericMode(true) ;

        m_tfPassword = new PTextField() ;
        m_tfPassword.setLabel("Password (optional)", PLabel.ALIGN_WEST | PLabel.ALIGN_NORTH) ;
        m_tfPassword.setAlphanumericMode(true) ;
        m_tfPassword.setEchoCharDelay('*') ;

        m_tfRealm = new PTextField() ;
        m_tfRealm.setLabel("Realm (optional)", PLabel.ALIGN_WEST | PLabel.ALIGN_NORTH) ;
        m_tfRealm.setAlphanumericMode(true) ;

        m_tfDisplayName = new PTextField() ;
        m_tfDisplayName.setLabel("Display Name (optional)", PLabel.ALIGN_WEST | PLabel.ALIGN_NORTH) ;
        m_tfDisplayName.setAlphanumericMode(true) ;

        m_tfRegisterURL = new PTextField() ;
        m_tfRegisterURL.setLabel("Register URL", PLabel.ALIGN_WEST | PLabel.ALIGN_NORTH) ;
        m_tfRegisterURL.setAlphanumericMode(true) ;

        m_cbEnableLine = new PCheckbox("Enable Line?") ;
        m_cbEnableLine.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;
        m_cbApplyCallHandling = new PCheckbox("Use Call Handling Settings?") ;
        m_cbApplyCallHandling.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL)) ;

    }


    protected void addComponents()
    {
        addComponent(m_tfUserName) ;
        addComponent(m_tfPassword) ;
        addComponent(m_tfRealm) ;
        addComponent(m_tfDisplayName) ;
        addComponent(m_tfRegisterURL) ;
        addComponent(m_cbEnableLine) ;
        addComponent(m_cbApplyCallHandling) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

}
