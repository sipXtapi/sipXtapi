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

package org.sipfoundry.sipxphone.app.transfer ;

import java.awt.* ;
import java.util.* ;
import java.text.* ;

import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.service.* ;


import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

import javax.telephony.callcontrol.events.* ;


/**
 * This Transfer failed form is displayed when the handset has been
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BlindTransferFailedForm extends PApplicationForm
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected icEventDispatcher      m_dispatcher = new icEventDispatcher() ;
    protected PContainer             m_container ;
    protected Component              m_calleeRenderer ;
    protected PLabel                 m_lblStatus ;
    protected String                 m_strStatus ;

    protected PCall                  m_callOriginal ;


    /**
     * Constructor requiring both the transferee (callOriginal) and the transfer
     * monitor call (call between the transferee and the transfer target).
     */
    public BlindTransferFailedForm(Application application,
                                   PCall       callOriginal,
                                   PAddress    address)
    {
        super(application, "Transfer Status") ;

        m_callOriginal = callOriginal ;

        setTitle("Transfer Status") ;


        initializeComponents() ;
        initMenus() ;

        updateCallerID(address) ;
        onTransferFailed() ;

        setHelpText(getString("transfer_fail"), getString("transfer_fail_title")) ;
    }


    /**
     * Updates the caller id portion of the screen
     */
    public void updateCallerID(PAddress address)
    {
        DefaultSIPAddressRenderer  renderer = new DefaultSIPAddressRenderer() ;
        Dimension                  dimSize = getSize() ;
        String                     strAddress = null ;

        if (address != null)
            strAddress = address.getAddress() ;

        Component comp = renderer.getComponent(this, strAddress, false) ;
        if (m_calleeRenderer != null) {
            m_container.remove(m_calleeRenderer) ;
        }

        m_calleeRenderer = comp ;
        m_calleeRenderer.setBounds(1, 54, dimSize.width-3, 27) ;
        m_container.add(m_calleeRenderer) ;

        // Force a repaint
        m_calleeRenderer.repaint() ;
    }


    /**
     * Invoked when the transfer has failed.  The Transfer control should
     * probably return to the orignal call, but we still give him the option
     * of leaving the call on hold.
     */
    public void onTransferFailed()
    {
        m_lblStatus.setText("Transfer Failed.  Press 'Close' to leave the transferee on hold or press 'Resume Call' to return to the call.") ;

        TransferUtils.unmarkBlindTransferCall(m_callOriginal) ;
        Shell.getCallManager().showCall(m_callOriginal) ;
        Shell.getCallManager().monitorCall(m_callOriginal) ;

// System.out.println("Original Call: " + m_callOriginal) ;
// m_callOriginal.dumpConnections() ;
// m_callOriginal.dumpConnectionsCache() ;

        TransferUtils.playFailedCallNotification(null) ;
    }


    public void onClose()
    {
        // If we are closing the form, make sure we turn off any tones
        if (m_callOriginal != null)
        {
            try {
                m_callOriginal.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
            TransferUtils.removeCallFromFocus(m_callOriginal) ;
        }
        Shell.getMediaManager().stopTone() ;
        closeForm() ;
    }


    public void onResumeCall()
    {
        TransferUtils.resumeBlindTransferCall(m_callOriginal) ;
        closeForm() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     *
     */
    private void initMenus()
    {
        PActionItem      items[] ;
        PBottomButtonBar menuControl = null ;

        // Initialize Menubar
        menuControl = getBottomButtonBar() ;
        menuControl.setItem(    PBottomButtonBar.B2,
                                new PLabel("Close"),
                                m_dispatcher.ACTION_CLOSE,
                                getString("hint/transfer/failed/close")) ;
        menuControl.setItem(    PBottomButtonBar.B3,
                                new PLabel("Resume Call"),
                                m_dispatcher.ACTION_RESUME_CALL,
                                getString("hint/transfer/failed/resume_call")) ;
        menuControl.addActionListener(m_dispatcher) ;

        // Initialize Left Menu
        items = new PActionItem[2] ;
        items[0] = new PActionItem(  new PLabel("Close"),
                                     getString("hint/transfer/failed/close"),
                                     m_dispatcher,
                                     m_dispatcher.ACTION_CLOSE) ;
        items[1] = new PActionItem(  new PLabel("Resume Call"),
                                     getString("hint/transfer/failed/resume_call"),
                                     m_dispatcher,
                                     m_dispatcher.ACTION_RESUME_CALL) ;
        setLeftMenu(items) ;
    }



    /**
     * Initialize the GUI
     */
    private void initializeComponents()
    {
        m_container = new PContainer() ;
        m_container.setLayout(null) ;

        m_lblStatus = new PLabel("") ;
        m_lblStatus.setAlignment(PLabel.ALIGN_WEST) ;
        m_lblStatus.setBounds(1, 0, 150, 54) ;

        m_container.add(m_lblStatus) ;

        addToDisplayPanel(m_container, new Insets(1,1,1,1)) ;
    }


    public void doLayout()
    {
        Dimension   dimSize = getSize() ;

        super.doLayout() ;

        m_lblStatus.setBounds(1, 0, 150, 54) ;
        m_calleeRenderer.setBounds(1, 54, dimSize.width-3, 27) ;
    }




//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////
    /**
     * Dispatches various action commands to various handlers
     */
    public class icEventDispatcher implements PActionListener
    {
        public final String ACTION_CLOSE       = "action_close" ;
        public final String ACTION_RESUME_CALL = "action_resume_call" ;

        public void actionEvent(PActionEvent event)
        {
            if (event.getActionCommand().equals(ACTION_CLOSE)) {
                onClose() ;
            } else if (event.getActionCommand().equals(ACTION_RESUME_CALL)) {
                onResumeCall() ;
            }
        }
    }
}
