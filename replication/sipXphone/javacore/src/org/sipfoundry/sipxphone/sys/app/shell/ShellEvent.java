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


package org.sipfoundry.sipxphone.sys.app.shell ;

import org.sipfoundry.util.CountingSemaphore ;
import org.sipfoundry.sipxphone.awt.event.PingtelEvent ;

/**
 * A shell event is either posted or sent to the shell whenever an action
 * is needed.  We are using events instead of direct method calls to avoid
 * various race conditions and deadlock conditions.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ShellEvent extends PingtelEvent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** unknown event type */
    public static final int SE_UNKNOWN = 0 ;
    /** show a form: objParam 1 is the form, objParam2 is the modal state */
    public static final int SE_SHOWFORM = 1 ;
    /** close a form: objParam 1 is the form */
    public static final int SE_CLOSEFORM = 2 ;
    /** Display an Hint: objParam1 is the hint text */
    public static final int SE_SHOWHINT = 3 ;
    /** Clear an Hint*/
    public static final int SE_CLEARHINT = 4 ;
    /** Display an Fly Over: objParam1 is the flyover text; objParam2 is the
        optional timeout */
    public static final int SE_SHOWFLYOVER = 5 ;
    /** Clear an Fly Over*/
    public static final int SE_CLEARFLYOVER = 6 ;
    /**  Do nothing... NON OPERATIONAL */
    public static final int SE_NOP = 7 ;

    /** check to see if the specified form is in focus */
    public static final int SE_FOCUSCHECK = 8 ;
    /** check to see if the specified form is stacked (displayed) */
    public static final int SE_STACKCHECK = 9 ;

    /** display some status on the screen */
    public static final int SE_SHOWSTATUS = 10 ;
    /** clear any status on the screen */
    public static final int SE_CLEARSTATUS = 11 ;
    /** Dial a number */
    public static final int SE_DIAL = 12 ;

    /** Ignore the call specified as objParam 1 */
    public static final int SE_IGNORECALL = 13 ;
    /** Monitor the call specified as objParam 1 */
    public static final int SE_MONITORCALL = 14 ;


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
    public ShellEvent(int iAction, Object objSource,  Object objParam1, Object objParam2)
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
            case SE_SHOWFORM:
                strRC += "SE_SHOWFORM" ;
                break ;
            case SE_CLOSEFORM:
                strRC += "SE_CLOSEFORM" ;
                break ;
            case SE_SHOWHINT:
                strRC += "SE_SHOWHINT" ;
                break ;
            case SE_CLEARHINT:
                strRC += "SE_CLEARHINT" ;
                break ;
            case SE_SHOWFLYOVER:
                strRC += "SE_SHOWFLYOVER" ;
                break ;
            case SE_CLEARFLYOVER:
                strRC += "SE_CLEARFLYOVER" ;
                break ;
            case SE_NOP:
                strRC += "SE_NOP" ;
                break ;
            case SE_FOCUSCHECK:
                strRC += "SE_FOCUSCHECK" ;
                break ;
            case SE_STACKCHECK:
                strRC += "SE_STACKCHECK" ;
                break ;
            case SE_SHOWSTATUS:
                strRC += "SE_SHOWSTATUS" ;
                break ;
            case SE_CLEARSTATUS:
                strRC += "SE_CLEARSTATUS" ;
                break ;
            case SE_DIAL:
                strRC += "SE_DIAL" ;
                break ;
            default:
                strRC += "SE_UNKNOWN" ;
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