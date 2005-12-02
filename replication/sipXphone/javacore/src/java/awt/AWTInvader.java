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

package java.awt ;

/**
 * This is sort of a hack, but an important hack.
 *
 * This class "lives" in the java.awt package so that we can
 * expose some of the package protected implementation- namely
 * the EventDispatchThread.  We need to see if a thread is a
 * EventDispatchThread (and create an another event dispatcher)
 * before we suspend it.
 *
 * We will try to play nice and expose as little as possible.
 */
public class AWTInvader
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** an alternative event pump */
    private EventDispatchThread m_pump ;
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    

    /** 
     * is the passed thread a event dispatch thread?
     */
    public boolean isEventDispatcher(Thread t)
    {
        return (t instanceof EventDispatchThread) ;
    }
    
    
    /**
     * start up an alternative event dispatcher (proxy)
     */
    public void startEventDispatcher()
    {
        if (m_pump == null) {
            m_pump = new EventDispatchThread(Thread.currentThread().getThreadGroup(),
                "AWT-Dispatch-Proxy", Toolkit.getEventQueue()) ;
            m_pump.start() ;        
        } else
            throw new IllegalStateException("AWT-Dispatch-Proxy is already started") ;
    }
    
    
    /**
     * if you have an event dispatcher (proxy) running, stop it.
     */
    public void stopEventDispatcher()
    {
        if (m_pump != null) {
            m_pump.stopDispatching();
            m_pump = null ;            
        }   
    }
}
