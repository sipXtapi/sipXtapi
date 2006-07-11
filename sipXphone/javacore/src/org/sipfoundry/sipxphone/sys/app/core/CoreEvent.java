/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/core/CoreEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys.app.core ;

import org.sipfoundry.util.CountingSemaphore ;
import org.sipfoundry.sipxphone.awt.event.PingtelEvent ;

/**
 * A core event is either posted or sent to the core app whenever an action
 * is needed.  We are using events instead of direct method calls to avoid
 * various race conditions and deadlock conditions.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CoreEvent extends PingtelEvent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** unknown event type */
    public static final int CE_UNKNOWN = 0 ;
    /** show a form: objParam 1 is the form, objParam2 is the modal state */

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** event specific parameter 1 */
    protected Object m_objParam1 ;
    /** event specific parameter 2 */
    protected Object m_objParam2 ;

    /** the calling/invoking thread */
    protected Thread m_thread ;

    protected Object m_objRC ;

    protected CountingSemaphore m_semaphore = null ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * constructor taking event type
     *
     * @param iAction int action type
     * @param objSource Object source
     * @param objParam1 Object parameter (action specific and optional)
     * @param objParam2 Object parameter (action specific and optional)
     *
     * @todo the thread should be recorded when the event is actually
     *       posted or sent instead of creation.
     */
    public CoreEvent(int iAction, Object objSource,  Object objParam1, Object objParam2)
    {
        super(objSource, iAction) ;

        m_objParam1 = objParam1 ;
        m_objParam2 = objParam2 ;
        m_thread = Thread.currentThread() ;
        m_objRC = null ;
    }


//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////
    /**
     * what is the action-specific optional parameter 1
     *
     * @return Object optional parameter 1
     */
    public Object getObjParam1()
    {
        return m_objParam1 ;
    }


    /**
     * what is the action-specific optional parameter 2
     *
     * @return Object optional parameter 2
     */
    public Object getObjParam2()
    {
        return m_objParam2 ;
    }


    public String toString()
    {
        String strRC = super.toString() + "\n  " ;

        switch (getEventType()) {
            default:
                strRC += "CE_UNKNOWN" ;
                break ;
        }

        strRC += "\n  objParam1=" + m_objParam1 ;
        strRC += "\n  objParam2=" + m_objParam2 ;
        strRC += "\n  thread   =" + m_thread ;

        return strRC ;
    }



    /**
     * what thread was used/active when this event was created?
     *
     * @return Thread used to create/send/post event
     */
    public Thread getThread()
    {
        return m_thread ;
    }


    public void setReturnValue(Object objReturnValue)
    {
        m_objRC = objReturnValue ;
    }


    public Object getReturnValue()
    {
        return m_objRC ;
    }
}
