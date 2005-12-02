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


package org.sipfoundry.util ;

import org.sipfoundry.sipxphone.sys.app.ShellApp ;

import org.sipfoundry.sipxphone.awt.event.PActionEvent ;
import org.sipfoundry.sipxphone.awt.event.PActionListener ;
import org.sipfoundry.sipxphone.service.Timer ;


/**
 * A quick implementation of a Counting Semaphore.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CountingSemaphore
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private ShellApp m_app ; // = ShellApp.getInstance() ;
    private Object   m_objectMutex = new Object() ;
    private Object   m_objectCountMutex = new Object() ;
    private int      m_iCount = 0 ;
    private int      m_iInitial = 0 ;
    private boolean  m_bSafe = true ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public CountingSemaphore(int iInitial)
    {
        this(iInitial, false) ;
    }


    public CountingSemaphore(int iInitial, boolean bSafe)
    {
        m_iInitial = m_iCount = iInitial ;
        m_bSafe = bSafe ;

        if (m_bSafe == true) {
            m_app = ShellApp.getInstance() ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public boolean down()
    {
        boolean bSuccessful = true ;
        boolean bBlock = false ;

        synchronized (m_objectMutex)
        {
            synchronized (m_objectCountMutex)
            {
                m_iCount-- ;
                if (m_iCount < 0)
                {
                    bBlock = true ;
                }
            }

            if (bBlock)
            {
                if (m_bSafe)
                {
                    bSuccessful = m_app.safeWait(m_objectMutex) ;
                }
                else
                {
                    try
                    {
                        m_objectMutex.wait() ;
                    }
                    catch (InterruptedException ie)
                    {
                        bSuccessful = false ;
                    }
                }
            }
        }
        return bSuccessful ;
    }


    public boolean down(int iTimeout)
    {
        boolean bSuccessful = true ;
        boolean bBlock = false ;
        icThreadWaker waker = null ;

        synchronized (m_objectMutex)
        {
            synchronized (m_objectCountMutex)
            {
                m_iCount-- ;
                if (m_iCount < 0)
                {
                    bBlock = true ;
                }
            }

            if (bBlock)
            {
                if (iTimeout > 0)
                {
                    waker = new icThreadWaker(Thread.currentThread()) ;
                    Timer.getInstance().addTimer(iTimeout, waker, null) ;
                    if (m_bSafe)
                    {
                        bSuccessful = m_app.safeWait(m_objectMutex) ;
                    }
                    else
                    {
                        try
                        {
                            m_objectMutex.wait() ;
                        }
                        catch (InterruptedException ie)
                        {
                            bSuccessful = false ;
                        }
                    }
                    Timer.getInstance().removeTimers(waker) ;
                }
                else
                {
                    bSuccessful = false ;
                }
            }
        }

        // Adjust count if we kickout because of a timeout
        if (!bSuccessful)
        {
            m_iCount++ ;
        }

        return bSuccessful ;
    }



    public void up()
    {
        boolean bRelease = false ;

        synchronized (m_objectMutex)
        {
            synchronized (m_objectCountMutex)
            {
                m_iCount++ ;
                if (m_iCount >= 0)
                {
                    bRelease = true ;
                }
            }

            if (bRelease)
            {
                if (m_bSafe)
                {
                    m_app.safeNotify(m_objectMutex) ;
                }
                else
                {
                    m_objectMutex.notify() ;
                }
            }
        }
    }


    public int getCount()
    {
        return m_iCount ;
    }


    public String toString()
    {
        return "CountingSemaphore (initial="+ m_iInitial+" count="+m_iCount+")" ;
    }


    private class icThreadWaker implements PActionListener
    {
        Thread m_thread = null ;


        public icThreadWaker(Thread thread)
        {
            m_thread = thread ;
        }


        public void actionEvent(PActionEvent event)
        {
            if (m_thread.isAlive())
            {
                m_thread.interrupt() ;
            }
        }
    }
}
