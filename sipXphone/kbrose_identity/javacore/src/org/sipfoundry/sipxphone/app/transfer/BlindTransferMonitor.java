/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/transfer/BlindTransferMonitor.java#2 $
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

import java.lang.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.util.SysLog;
/**
 * The blind tranfer monitor adds connection listeners to the call
 * kicks off a progress form and signals the invoker when complete.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BlindTransferMonitor extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

//////////////////////////////////////////////////////////////////////////////
// Attributes
///
    private Application m_appParent ;
    private PCall       m_callTransferee ;
    private PCall       m_callMonitor ;
    private Object      m_objNotify ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public BlindTransferMonitor(Application app,
                                PCall callTransferee,
                                PCall callMonitor,
                                Object objNotify)
    {
        m_appParent = app ;
        m_callTransferee = callTransferee ;
        m_callMonitor = callMonitor ;
        m_objNotify = objNotify ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public void run()
    {
        PAddress transferee = null ;
        if (m_callMonitor != null)
        {
            BlindTransferInProgressForm formProgress = new BlindTransferInProgressForm(m_appParent, m_callTransferee, m_callMonitor) ;

            PAddress participants[] = m_callTransferee.getParticipants() ;
            if (participants.length > 0)
            {
                transferee = participants[0] ;
            }

            formProgress.updateCallerID(transferee) ;
            formProgress.showModal() ;
        }
        else
        {
            Shell.getInstance().displayFlyOver("Transfer Completed: Press any button to continue.", 5000) ;

            try
            {
                m_callTransferee.disconnect() ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
        }

        // Notify interested parties that we are complete
        if (m_objNotify != null)
        {
            synchronized (m_objNotify)
            {
                m_objNotify.notifyAll() ;
            }
        }
    }
}
