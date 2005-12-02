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

 
package org.sipfoundry.sipxphone.awt.event ;
 
import java.awt.* ;
import org.sipfoundry.util.CountingSemaphore ;


public class PingtelEvent
{

//////////////////////////////////////////////////////////////////////////////
// Attributes
////     
    /** button event type */
    protected int m_iEventType ;
    
    /** Object that fired off the event */
    protected Object m_objSource ;
    
    /** has this event been consumed? */
    protected boolean m_bConsumed ;
        
    /** provides synchronization for posts */
    protected CountingSemaphore m_semaphore = null ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////         
    public PingtelEvent(Object source, int iEventType) 
    {
        m_objSource = source ;
        m_iEventType = iEventType ;
        
        m_bConsumed = false ;
    }
    

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////    
    /**
     * what type of event is this?  BUTTON_UP, BUTTON_DOWN, or BUTTON_REPEAT
     */
    public int getEventType()
    {
        return m_iEventType ;        
    }
    
    
    /**
     * Who/What is the source of this event?
     */
    public Object getSource()
    {
        return m_objSource ;   
    }
    
    
    /**
     * consume this event; this will stop the event from being dispatch to any
     * one else.
     */
    public void consume()
    {
        m_bConsumed  = true ;
    }
    
        
    
    /**
     * has this event been consumed?
     */
    public boolean isConsumed()
    {
        return m_bConsumed ;
    }           
    
        
    
    /**
     * reset consumption member variable to false
     */
    public void reset()
    {
        m_bConsumed = false ;
        
    }
    
    
    /**
     * @deprecated do not expose
     */
    public CountingSemaphore getSemaphore()
    {
        return m_semaphore ;
    }
    
    
    /**
     * @deprecated do not expose
     */
    public void setSemaphore(CountingSemaphore semaphore)
    {
        m_semaphore = semaphore ; 
    }
    
    
    public String toString()
    {
        return "PingtelEvent type=" + m_iEventType + " source=" + m_objSource ;
    }
}