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
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.sys.app.*;
import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.HookManager;
import org.sipfoundry.sipxphone.hook.ButtonHookData;
import org.sipfoundry.util.SysLog;
/**
 * This class implements the pingtel event dispatching.  The queue is
 * managed in PingtelEventQueue.  This guy talks to the Shell to see
 * who is the application in focus and fires events directly the
 * the outmost PContainer/PComponent.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PingtelEventDispatcher extends Thread
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static boolean DEBUG_PROCESS_EVENTS  = false ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    public static boolean s_bEnabled = true ;
    public static Object  objSync = new Object() ;
    public static boolean m_bBetweenButtonEvents = false ;

    /** reference to the event dispatcher thread */
    protected Thread m_threadInstance = null ;
    /** thread status - should we be running or not */
    protected boolean m_bRunning = true ;

    /** identifier used to track the number of event dispatcher thread */
    protected static int m_queueID = 1 ;

    protected static PButtonListener m_coreListener = null ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Stock no argument constructor
     */
    public PingtelEventDispatcher()
    {
        super("PT-EventQueue-" + m_queueID++) ;

        try {
            setPriority(Thread.NORM_PRIORITY+1) ;
        } catch (Exception e) {
            System.out.println("Error setting Priority: ") ;
            SysLog.log(e) ;
        }
    }


    /**
     * Enable the Pingtel Event Dispatcher
     */
    public static void enable()
    {
        synchronized (objSync) {
            s_bEnabled = true ;
        }
    }


    /**
     * Disable the Pingtel Event Dispatcher
     */
    public static void disable()
    {
        synchronized (objSync) {
            s_bEnabled = false ;
        }
    }


    /**
     * @return true if between button presses else false
     */
    public static boolean isBetweenButtonPresses()
    {
        return m_bBetweenButtonEvents ;
    }


    /**
     * Set the core listener.  The core listener is a special entity that gets
     * the first crack button events before anyone else does.
     */
    public static void setCoreListener(PButtonListener listener)
    {
        m_coreListener = listener ;
    }


    /**
     * starts up the Event Dispatcher
     */
    public static void startup()
    {
        PingtelEventDispatcher dispatcher = new PingtelEventDispatcher() ;
        dispatcher.start() ;
    }


    /**
     * get the next waiting event.
     */
    protected synchronized PingtelEvent getNextEvent()
        throws InterruptedException
    {
        while (!s_bEnabled) {
            System.out.println("PingtelEvent getNextEvent SLEEPING 250ms");
            Thread.sleep(250) ;
        }
        return PingtelEventQueue.nextEvent() ;
    }


    /**
     *
     */
    public void handleButtonEvent(PButtonEvent event)
    {
        // Fire off the event to our logger
        switch (event.getEventType()) {
            case PButtonEvent.BUTTON_UP:
                Logger.post("button", Logger.TRAIL_COMMAND, "buttonUp",
                        new Integer(event.getEventType()),
                        new Integer(event.getButtonID())) ;
                break ;
            case PButtonEvent.BUTTON_DOWN:
                Logger.post("button", Logger.TRAIL_COMMAND, "buttonDown",
                        new Integer(event.getEventType()),
                        new Integer(event.getButtonID())) ;
                break ;
            case PButtonEvent.KEY_UP:
                Logger.post("button", Logger.TRAIL_COMMAND, "keyUp",
                        new Integer(event.getEventType()),
                        new Integer(event.getButtonID())) ;
                break ;
            case PButtonEvent.KEY_DOWN:
                Logger.post("button", Logger.TRAIL_COMMAND, "keyDown",
                        new Integer(event.getEventType()),
                        new Integer(event.getButtonID())) ;
                break ;
        }

        //execute the button Hook
        //ShellApp installs CoreAppButtonHook which acts on the event first
        //and then the defaultButtonHook, which takes action only if the
        //event is not consumed by then.
        Shell.getHookManager().executeHook(HookManager.HOOK_BUTTON, new ButtonHookData(event)) ;

    }


    public void handleShellEvent(ShellEvent event)
    {
        ShellApp.getInstance().processShellEvent(event) ;
    }


    /**
     * thread execution loop.  This is where all the actual dispatching
     * actually happens.
     */
    public void run()
    {
        // store a reference to the event dispatcher thread
        m_threadInstance = Thread.currentThread() ;

        // Keep dispatching until stopDispatching is invoked.
        while (m_bRunning)
        {
            try
            {
                if (DEBUG_PROCESS_EVENTS)
                {
                    System.out.println("----") ;
                    System.out.println("-- <BEGIN> getNextEvent...") ;
                }
                PingtelEvent event = getNextEvent() ;

                if (DEBUG_PROCESS_EVENTS)
                {
                    System.out.println("-- <END>   getNextEvent...") ;
                    System.out.println("----") ;
                }

                if (DEBUG_PROCESS_EVENTS)
                {
                    System.out.println("====") ;
                    System.out.println("== <BEGIN> Processing Event") ;
                    System.out.println("==" + event) ;
                    try
                    {
                        Thread.sleep(250);
                    }
                    catch (Exception e)
                    {

                    }
                }

                if (event != null)
                {
                    try
                    {
                        if (event instanceof PButtonEvent)
                        {
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <BEGIN> Processing BUTTON Event") ;
                                System.out.println("==" + event) ;
                            }
                            handleButtonEvent((PButtonEvent) event) ;
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <END> Processing BUTTON Event") ;
                                System.out.println("==" + event) ;
                            }
                        }
                        else if (event instanceof ShellEvent)
                        {
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <BEGIN> Processing SHELL Event") ;
                                System.out.println("==" + event) ;
                            }
                            handleShellEvent((ShellEvent) event) ;
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <BEGIN> Processing SHELL Event") ;
                                System.out.println("==" + event) ;
                            }
                        }
                        else if (event instanceof Runnable)
                        {
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <BEGIN> Processing Runnable Event") ;
                                System.out.println("==" + event) ;
                            }
                            ((Runnable)event).run();
                            if (DEBUG_PROCESS_EVENTS)
                            {
                                System.out.println("====") ;
                                System.out.println("== <BEGIN> Processing Runnable Event") ;
                                System.out.println("==" + event) ;
                            }

                        }
                        else
                        {
                            System.out.println("PANIC: Unknown event in PingtelEventDispatcher") ;
                        }
                    }
                    catch (Exception e)
                    {
                        System.out.println("Error Caught by Event Dispatcher: " + e) ;
                        SysLog.log(e) ;

                        Shell.getInstance().showUnhandledException(e, false) ;
                    }
                    finally {
                        CountingSemaphore semaphore = event.getSemaphore() ;
                        if (semaphore != null) {
                            semaphore.up() ;
                        }
                    }
                    event = null ;
                }

                if (DEBUG_PROCESS_EVENTS)
                {
                    System.out.println("== <END>   Processing Event") ;
                    System.out.println("====") ;
                }
            }
            catch (ThreadDeath death)
            {
                System.out.println("++++++++++++++++++ EVENT DISPATCH THREAD DEATH !!!!!! ARRRRGGGGGGHHGHGHGHGHH!!!!") ;
            }
            catch (InterruptedException ie)
            {
            }
            catch (Throwable e)
            {
                System.err.println("Exception occurred during pingtel event dispatching:") ;
                SysLog.log(e) ;

                Shell.getInstance().showUnhandledException(e, false) ;
            }
        }
    }


    /**
     * Gracefully bring down the event dispatcher
     */
    public void stopDispatching()
    {
        // Bring down the Dispatcher
        if (DEBUG_PROCESS_EVENTS)
            System.out.println(" +++++++++++ STOP DISPATCHING WAS CALLED!");
        m_bRunning = false ;

        try {
            if (m_threadInstance != null) {
                m_threadInstance.interrupt() ;
            }
        } catch (Exception e) {
            SysLog.log(e) ;
        }

        if (Thread.currentThread() != this) {
            try {
                join() ;
            } catch(InterruptedException e) {
                // no way to really recover from this
            }
        }
    }
}
