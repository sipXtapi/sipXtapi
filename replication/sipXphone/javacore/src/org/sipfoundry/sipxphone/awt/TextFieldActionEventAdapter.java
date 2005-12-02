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


package org.sipfoundry.sipxphone.awt ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import java.util.* ;

/**
 * <strong>DO NOT EXPOSE: For internal use only</strong>
 * <br><br>
 * This adapter provides some helper functionality and coupling between a
 * command bar and a text field.  Upon construction, the text field is
 * added as an action listener to the command bar.  Afterwards, all
 * know action events (ACTION_BACKSPACE, ACTION_BACKWARD, ACTION_FOREWARD, and
 * ACTION_CAPSLOCK will be automatically processed and directed on the text
 * field.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class TextFieldActionEventAdapter implements PActionListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
/////
    public static final String ACTION_BACKSPACE = "action_taa_backspace" ;
    public static final String ACTION_BACKWARD  = "action_taa_backward" ;
    public static final String ACTION_FORWARD   = "action_taa_forward" ;
    public static final String ACTION_CAPSLOCK  = "action_taa_capslock" ;
    public static final String ACTION_IPMODE    = "action_taa_ipmode" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
/////
    protected PTextField      m_tfTextfield ;
    protected PCommandBar     m_cbActions ;
    protected icFocusListener m_focusListener ;
    protected Hashtable       m_htCapsLock ;

    private boolean m_bAllowNumericMode = false;

//////////////////////////////////////////////////////////////////////////////
// Construction
/////
    public TextFieldActionEventAdapter(PTextField textfield, PCommandBar commandbar, boolean bEnableCapsLock)
    {
        m_cbActions = commandbar ;
        m_focusListener = new icFocusListener() ;

        if (m_cbActions != null) {
            m_cbActions.addActionListener(this) ;
        }
        m_htCapsLock = new Hashtable() ;
        addTextfield(textfield, bEnableCapsLock) ;
    }


    public TextFieldActionEventAdapter(PCommandBar commandbar)
    {
        m_cbActions = commandbar ;
        m_focusListener = new icFocusListener() ;
        m_htCapsLock = new Hashtable() ;

        if (m_cbActions != null) {
            m_cbActions.addActionListener(this) ;
        }
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
/////
    public void addTextfield(PTextField textfield, boolean bEnableCapsLock)
    {
        textfield.addFocusListener(new icFocusListener()) ;
        m_htCapsLock.put(textfield, new Boolean(bEnableCapsLock)) ;
        if (m_tfTextfield == null) {
            m_tfTextfield = textfield ;
        }
    }


    /**
     * dispatch the action event to some sort of handler.
     */
    public void actionEvent(PActionEvent event)
    {
        if (event.getActionCommand().equals(ACTION_BACKSPACE)) {
            onProxyBackspace() ;
        } else if (event.getActionCommand().equals(ACTION_BACKWARD)) {
            onProxyBackward() ;
        } else if (event.getActionCommand().equals(ACTION_FORWARD)) {
            onProxyForward() ;
        } else if (event.getActionCommand().equals(ACTION_CAPSLOCK)) {
            onProxyCapslock() ;
        }
    }


    /**
     * Populate the command bar with text field related stuff
     */
    public void populateCommandbar()
    {
        AppResourceManager res = AppResourceManager.getInstance() ;

        // Initialize Command Bar
        m_cbActions.removeAllButtons() ;

        m_cbActions.addButton(new PLabel(res.getImage("imgBackspaceIcon")), ACTION_BACKSPACE, res.getString("hint/core/edit/backspace")) ;
        m_cbActions.addButton(new PLabel(res.getImage("imgCharBackwardIcon")), ACTION_BACKWARD, res.getString("hint/core/edit/moveleft")) ;
        m_cbActions.addButton(new PLabel(res.getImage("imgCharForwardIcon")), ACTION_FORWARD, res.getString("hint/core/edit/moveright")) ;
        m_cbActions.addButton(new PLabel(res.getImage("imgUprCase")), ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_toupper")) ;
    }

    public void setAllowNumericMode(boolean ballowNumeric)
    {
        m_bAllowNumericMode = ballowNumeric;
    }

    /**
     * Enable/disable the caps lock button
     */
    public void enableCapslock(boolean bEnable, PTextField textfield)
    {
        m_htCapsLock.put(textfield, new Boolean(bEnable)) ;
        if (m_tfTextfield == textfield)
            m_cbActions.enableByAction(ACTION_CAPSLOCK, bEnable) ;
    }

   /**
     * Enable/disable button by id
     */
    public void enableButtonByAction(String actionString, boolean bEnable)
    {
        m_cbActions.enableByAction(actionString, bEnable) ;
    }


    /**
     * Enable/disable all of the command bar buttons
     */
    public void enableCommandbar(boolean bEnable)
    {
        boolean bEnableCapsLock = isCapsLockEnabled(m_tfTextfield) ;

        m_cbActions.enableByAction(ACTION_BACKSPACE, bEnable) ;
        m_cbActions.enableByAction(ACTION_BACKWARD, bEnable) ;
        m_cbActions.enableByAction(ACTION_FORWARD, bEnable) ;
        m_cbActions.enableByAction(ACTION_CAPSLOCK, (bEnable && bEnableCapsLock)) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
/////
    protected boolean isCapsLockEnabled(PTextField textfield)
    {
        boolean bRC = false ;
        Boolean enabled ;

        if (textfield != null) {
            enabled = (Boolean) m_htCapsLock.get(textfield) ;
            if (enabled != null) {
                bRC = enabled.booleanValue() ;
            }
        }

        return bRC ;
    }

    protected void onProxyBackspace()
    {
        if (m_tfTextfield != null) {
            m_tfTextfield.backspace() ;
        }
    }


    protected void onProxyBackward()
    {
        if (m_tfTextfield != null) {
            m_tfTextfield.moveCaretBackward() ;
        }
    }


    protected void onProxyForward()
    {
        if (m_tfTextfield != null) {
            m_tfTextfield.moveCaretForward() ;
        }
    }


    protected void onProxyCapslock()
    {
        AppResourceManager res = AppResourceManager.getInstance() ;

        if (m_tfTextfield != null)
        {

            if (m_bAllowNumericMode)
            {
                if (m_tfTextfield.getNumericMode())
                {
                    m_tfTextfield.setNumericMode(false);
                    if (m_cbActions != null)
                    {
                        m_cbActions.updateButtonLabelByAction(ACTION_CAPSLOCK, new PLabel(res.getImage("imgUprCase"))) ;
                        m_cbActions.updateButtonHintByAction(ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_toupper")) ;
                    }
                }
                else
                if (m_tfTextfield.getCapsLock())
                {
                    m_tfTextfield.setCapsLock(!m_tfTextfield.getCapsLock()) ;
                    m_tfTextfield.setNumericMode(true);
                    if (m_cbActions != null)
                    {
                        m_cbActions.updateButtonLabelByAction(ACTION_CAPSLOCK, new PLabel(res.getImage("imgLwrCase"))) ;
                        m_cbActions.updateButtonHintByAction(ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_tolower")) ;
                    }
                }
                else
                {
                    m_tfTextfield.setCapsLock(!m_tfTextfield.getCapsLock()) ;
                    m_tfTextfield.setNumericMode(false);
                    if (m_cbActions != null)
                    {
                        m_cbActions.updateButtonLabelByAction(ACTION_CAPSLOCK, new PLabel(res.getImage("imgallnum"))) ;
                        m_cbActions.updateButtonHintByAction(ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_ipmode")) ;
                    }
                }
            }
            else
            {
                m_tfTextfield.setCapsLock(!m_tfTextfield.getCapsLock()) ;

                if (m_tfTextfield.getCapsLock())
                {
                    if (m_cbActions != null)
                    {
                        m_cbActions.updateButtonLabelByAction(ACTION_CAPSLOCK, new PLabel(res.getImage("imgallnum"))) ;
                        m_cbActions.updateButtonHintByAction(ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_tolower")) ;
                    }
                }
                else
                {
                    if (m_cbActions != null)
                    {
                        m_cbActions.updateButtonLabelByAction(ACTION_CAPSLOCK, new PLabel(res.getImage("imgUprCase"))) ;
                        m_cbActions.updateButtonHintByAction(ACTION_CAPSLOCK, res.getString("hint/core/edit/capslock_toupper")) ;
                    }
                }
            }
        }
    }


    public class icFocusListener implements PFocusListener
    {
        /**
         * Invoked when a component gains input focus
         */
        public void focusGained(PFocusEvent e)
        {
            m_tfTextfield = (PTextField) e.getSource() ;
            enableCommandbar(true) ;
        }

        /**
         * Invoked when a component loses input focus
         */
        public void focusLost(PFocusEvent e)
        {
            if (m_tfTextfield == e.getSource()) {
                m_tfTextfield = null ;
                enableCommandbar(false) ;
            }
        }
    }
}
