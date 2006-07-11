/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/RestartConfirmationForm.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.app.core;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;


/**
 * Displays a form asking the user to either accept or decline a reboot
 * request.  If the user does not respond within a designated amount of time,
 * the form will automatically accept.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class RestartConfirmationForm extends MessageBox implements PActionListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected int m_iSeconds ;  // Number of seconds until we auto-accept


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Constructs the RestartConfirmation form
     *
     * @param app The application that will display this form
     * @param iWaitSeconds the number of seconds to wait before automatically
     *        closing the form (and accepting the restart)
     * @param strReason The reason for restarting.
     */

    public RestartConfirmationForm(Application app, int iWaitSeconds, String strReason)
    {
        super(app, MessageBox.TYPE_WARNING) ;
        setTitle(getString("lblRestartRequestTitle"));
        setMessage(strReason + ": " + getString("lblRestartRequestText")) ;

        m_iSeconds = iWaitSeconds ;

        addFormListener(new icFormListener()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public void actionEvent(PActionEvent event)
    {
        if (isStacked())
        {
            closeForm(MessageBox.OK) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * What is the action listener for the RestartConfirmationForm.
     */
    protected PActionListener getActionListener()
    {
        return this ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    class icFormListener extends PFormListenerAdapter
    {
        public void focusGained(PFormEvent event)
        {
            Timer.getInstance().resetTimer(m_iSeconds*1000, getActionListener(), null) ;
        }
    };


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}