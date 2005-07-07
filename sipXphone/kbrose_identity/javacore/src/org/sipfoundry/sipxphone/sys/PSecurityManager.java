/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/PSecurityManager.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys;

import org.sipfoundry.util.AppResourceManager;
import org.sipfoundry.util.SysLog;
import org.sipfoundry.sipxphone.service.Timer;
import org.sipfoundry.sipxphone.awt.form.*;
import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.*;

public class PSecurityManager
{
    //HACK: we are trying to introduce a level where a guest user
    //has no access to features like Restart, Version Check.
    protected boolean m_bGuestFeaturesMode = false;

    protected boolean m_bShowGuestAdminFeatures = false;

    protected String m_strPasswordForGuestAdmin;


    private static PSecurityManager m_instance = new PSecurityManager();


    protected PSecurityManager()
    {
        //check for the file adminmode.properties
        //if it exists and there is a key called
        //admin_mode_password, then
        //restricted featires mode is enabled.
        AppResourceManager.getInstance().addStringResourceFile("adminmode.properties");
        m_strPasswordForGuestAdmin =
            AppResourceManager.getInstance().getString("admin_mode_password");
        if( m_strPasswordForGuestAdmin != null &&
            !m_strPasswordForGuestAdmin.equals("admin_mode_password") )
        {
            m_bGuestFeaturesMode = true;
            m_bShowGuestAdminFeatures = false;
        }

    }

    /**
     * gets the singleton instance of this security manager.
     */
    public static PSecurityManager getInstance()
    {
        return m_instance;
    }

    /**
     * Is the phone in guest features mode? With this mode on,
     * users will not be able to execute functionalities lke
     * restart, "set to factory defaults", version check, etc
     * unless a pre-configured admin password in entered.
     */
    public boolean isInGuestFeaturesMode()
    {
        return m_bGuestFeaturesMode;
    }

    /**
     *  If the guest features mode is on, is this a guest user or
     *  an admin? A guest becomes Admin for guest features
     *  by entering a pre-configured password for the system.
     *  This admin still needs to put the regular admin password
     *  to change settings like network settings.
     */
    public boolean shouldShowGuestAdminFeatures()
    {
        return m_bShowGuestAdminFeatures;
    }


    public void showGuestAdminFeatures(boolean bEnable)
    {
        m_bShowGuestAdminFeatures = bEnable;
    }

    public int showAuthenticationForm(PForm formParent)
    {
        icPasswordAuthentication form = new icPasswordAuthentication(formParent);
        return (form.showModal());
    }



     public class icPasswordAuthentication extends SimpleTextInputForm
    {
        private icPACommandDispatcher m_dispatcher ;

        /** tip for the password form */
        private String m_strInstructions = "" ;

        /** the number of dial by url tips */
        protected int        m_iNumTips = - 1 ;

        public icPasswordAuthentication(PForm formParent)
        {
            super(formParent, "Authentication") ;
            AppResourceManager resourceManager = AppResourceManager.getInstance();
            m_dispatcher = new icPACommandDispatcher() ;

            setLabel("admin mode password: ") ;
            setMode(icPasswordAuthentication.MODE_PASSWORD_TEXT) ;
            this.setHelpText(resourceManager.getString("admin"),
                             resourceManager.getString("admin_title")) ;
            nextTip();
            this.setInstructions(m_strInstructions);
            //initMenus() ;
        }


        /**
         * loads the next tip.
         */
        protected void nextTip()
        {
              // Load number of tips if not already known
              if (m_iNumTips == -1) {
                  try {
                      m_iNumTips = Integer.parseInt(this.getString("lblNumDialByURLTips")) ;
                  } catch (Exception e) { /* burp */ }
              }

              int iTip = (int) (Math.random() * m_iNumTips) ;
              // Cycle if we have enough...
              if (m_iNumTips > 0) {
                  m_strInstructions = this.getString("lblDialByURLTip" + (iTip+1)) ;
              }
        }

        /**
         * on focus gained of this password authentication form,
         * load the next tip and show the tip.
         */
        public void onFocusGained(PForm formLosingFocus)
        {
            nextTip();
            this.setInstructions(m_strInstructions);
        }




        public void onOk()
        {

            String strPasswd = getText() ;
            try {
                if ( strPasswd.equals(m_strPasswordForGuestAdmin) ){
                    m_bShowGuestAdminFeatures = true;
                    super.onOk() ;
                } else {
                    setText("") ;
                    setInstructions("Invalid Admin mode password.  Please re-enter your password and try again.") ;
                }
            } catch (Exception e) {
                MessageBox error = new MessageBox(this, MessageBox.TYPE_ERROR) ;
                error.setMessage(this.getString("lblAuthenticationError") + "\n" + e.toString()) ;
                error.showModal() ;
                super.onOk() ;
            }
        }

        public void onCancel()
        {
            super.onCancel() ;
        }

        private class icPACommandDispatcher implements PActionListener
        {
            public final String ACTION_OK     = "ACTION_OK" ;
            public final String ACTION_CANCEL = "ACTION_CANCEL" ;
            public final String ACTION_ABOUT  = "ACTION_ABOUT" ;

            public void actionEvent(PActionEvent event)
            {
                if (event.getActionCommand().equals(ACTION_OK)) {
                    onOk() ;
                } else if (event.getActionCommand().equals(ACTION_CANCEL)) {
                    onCancel() ;
                }
            }
        }
    }




}
