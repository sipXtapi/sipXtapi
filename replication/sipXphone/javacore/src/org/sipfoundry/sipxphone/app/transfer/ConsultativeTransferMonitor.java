/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/transfer/ConsultativeTransferMonitor.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.transfer;


import java.util.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.sipxphone.sys.Shell ;


/**
 * The consultative tranfer monitor adds connection listeners to the calls
 * involved in the transfer and fires off TransferControllerListener events.
 *
 * This monitor was created to reduce the logic/complex in the transfer
 * application and separate the 'ugly' code from the presentation.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConsultativeTransferMonitor
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final boolean DEBUG_ENABLED = false ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PAddress m_addressTransferee ;  // Address of the transferee
    private PCall    m_callTransferee ;     // Call between TC and transferee
    private PAddress m_addressTarget ;      // Address of target (derived)
    private PCall    m_callTarget ;         // Call between TC and target

    private Vector   m_vListeners ;         // Listeners to our event stream

    private boolean  m_bFinalState ;    // final state is abort, success, or
                                        // failure

    private icTransfereeConnectionMonitor m_transfereeMonitor ;  // tranferee connection monitor
    private icTargetConnectionMonitor m_targetMonitor ; // target connection monitor


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Constructs a ConsultativeTransferMonitor with all of the required
     * parameters.
     *
     * @param callTransferee The call with the TC and transferee
     * @param callTarget The call with the TC and the transferee
     */
    public ConsultativeTransferMonitor(PCall callTransferee,
                                       PCall callTarget)
    {
        m_callTransferee = callTransferee ;
        m_callTarget = callTarget ;

        m_addressTransferee = null ;
        m_addressTarget = null ;
        m_vListeners = new Vector() ;
        m_bFinalState = false ;

        if (m_callTransferee != null)
        {
            m_transfereeMonitor = new icTransfereeConnectionMonitor() ;
            m_callTransferee.addConnectionListener(m_transfereeMonitor) ;
        }

        if (m_callTarget != null)
        {
            m_targetMonitor = new icTargetConnectionMonitor() ;
            m_callTarget.addConnectionListener(m_targetMonitor) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Release any listeners added for the purpose of transfer monitoring.
     */
    public void releaseListeners()
    {
        if ((m_callTransferee != null) && (m_transfereeMonitor != null))
        {
            m_callTransferee.removeConnectionListener(m_transfereeMonitor) ;
        }

        if ((m_callTarget != null) && (m_targetMonitor != null))
        {
            m_callTarget.removeConnectionListener(m_targetMonitor) ;
        }
    }


    /**
     * Add the specified listener to this monitor.
     */
    public synchronized void addTransferListener(TransferControllerListener listener)
    {
        if (!m_vListeners.contains(listener))
        {
            m_vListeners.addElement(listener) ;
        }
    }


    /**
     * Removes the specified listener to this monitor.
     */
    public synchronized void removeTransferListener(TransferControllerListener listener)
    {
        m_vListeners.removeElement(listener) ;
    }


    /**
     * Sets the transferee address for the monitor.
     */
    public void setTransfereeAddress(PAddress address)
    {
        m_addressTransferee = address ;
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireConsultSucceeded()
    {
        debugln("ConsultativeTransferMonitor::fireConsultSucceeded") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.consultSucceeded(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireConsultFailed()
    {
        debugln("ConsultativeTransferMonitor::fireConsultFailed") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.consultFailed(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireConsultAborted()
    {
        debugln("ConsultativeTransferMonitor::fireConsultAborted") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.consultAborted(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireTransferSucceeded()
    {
        debugln("ConsultativeTransferMonitor::fireTransferSucceeded") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.transferSucceeded(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireTransferFailed()
    {
        debugln("ConsultativeTransferMonitor::fireTransferFailed") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.transferFailed(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireTransferAborted()
    {
        debugln("ConsultativeTransferMonitor::fireTransferAborted") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.transferAborted(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireTransfereeHungup()
    {
        debugln("ConsultativeTransferMonitor::fireTransfereeHungup") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.transfereeHungup(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


    /**
     * Fire this event to all interested TransferControllerListeners.
     */
    public synchronized void fireTransferTargetHungup()
    {
        debugln("ConsultativeTransferMonitor::fireTransferTargetHungup") ;

        Enumeration enumListeners = m_vListeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            TransferControllerListener listener = (TransferControllerListener)
                    enumListeners.nextElement() ;

            if (listener != null)
            {
                try
                {
                    listener.transferTargetHungup(createEvent());
                }
                catch (Throwable t)
                {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * If debugging is enabled, output passed string to System.out
     */
    protected void debugln(String strOutput)
    {
        if (DEBUG_ENABLED == true)
        {
            System.out.println(strOutput) ;
        }
    }

    protected void debugevent(PConnectionEvent event)
    {
        if (DEBUG_ENABLED == true)
        {
            event.dump(System.out);
        }
    }


    /**
     * A helper methods that sreate a new TransferControllerEvent give our
     * memeber variables.
     */
    private TransferControllerEvent createEvent()
    {
        return new TransferControllerEvent(m_addressTransferee,
                m_callTransferee, m_addressTarget, m_callTarget) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    /**
     * Monitors the call between the transfer controller and the transferee
     */
    public class icTransfereeConnectionMonitor extends PConnectionListenerAdapter
    {
        public void connectionConnected(PConnectionEvent event)
        {
            debugln("icTransfereeConnectionMonitor::connectionConnected") ;
            debugevent(event) ;
        }


        public void connectionFailed(PConnectionEvent event)
        {
            debugln("icTransfereeConnectionMonitor::connectionFailed") ;
            debugevent(event) ;
        }


        public void connectionUnknown(PConnectionEvent event)
        {
            debugln("icTransfereeConnectionMonitor::connectionUnknown") ;
            debugevent(event) ;
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
            debugln("icTransfereeConnectionMonitor::connectionDisconnected") ;
            debugevent(event) ;

            if (event.getAddress().equals(m_addressTransferee))
            {
                fireTransfereeHungup() ;
            }
            else
            {
                System.out.println("UNKNOWN connection disconnected!") ;
            }
        }
    }


    /**
     * Monitors the call between the transfer controller and the transfer target
     *
     * NOTE: This includes a 'ghost' connection that represent the transferee
     *       status.
     */
    public class icTargetConnectionMonitor extends PConnectionListenerAdapter
    {
        public void connectionTrying(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionTrying") ;
            debugevent(event) ;
        }


        public void connectionOutboundAlerting(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionOutboundAlerting") ;
            debugevent(event) ;

            // If we do not have a target address, this MUST before it.
            if (m_addressTarget == null)
                m_addressTarget = event.getAddress() ;

            PAddress address = event.getAddress() ;
            if (address.equals(m_addressTransferee))
            {

            }
            else
            {

            }
        }


        public void connectionConnected(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionConnected") ;
            debugevent(event) ;

            // If we do not have a target address, this MUST before it.
            if (m_addressTarget == null)
                m_addressTarget = event.getAddress() ;
            PAddress address = event.getAddress() ;

            debugln("     event address: " + address) ;
            debugln(" addressTransferee: " + m_addressTransferee) ;
            debugln("           compare: " + address.equals(m_addressTransferee)) ;

            if (address.equals(m_addressTransferee))
            {
                fireTransferSucceeded() ;
            }
            else
            {
                fireConsultSucceeded() ;
            }
        }


        public void connectionFailed(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionFailed") ;
            debugevent(event) ;

            // If we do not have a target address, this MUST before it.
            if (m_addressTarget == null)
                m_addressTarget = event.getAddress() ;
            PAddress address = event.getAddress() ;

            debugln("     event address: " + address) ;
            debugln(" addressTransferee: " + m_addressTransferee) ;
            debugln("           compare: " + address.equals(m_addressTransferee)) ;


            if (address.equals(m_addressTransferee))
            {
                fireTransferFailed() ;
            }
            else
            {
                fireConsultFailed() ;
            }
        }


        public void connectionUnknown(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionUnknown") ;
            debugevent(event) ;

            // If we do not have a target address, this MUST before it.
            if (m_addressTarget == null)
                m_addressTarget = event.getAddress() ;
            PAddress address = event.getAddress() ;

            debugln("     event address: " + address) ;
            debugln(" addressTransferee: " + m_addressTransferee) ;
            debugln("           compare: " + address.equals(m_addressTransferee)) ;


            if (address.equals(m_addressTransferee))
            {
                fireTransferSucceeded() ;
            }
            else
            {
            }
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
            debugln("icTargetConnectionMonitor::connectionDisconnected") ;
            debugevent(event) ;

            // If we do not have a target address, this MUST before it.
            if (m_addressTarget == null)
                m_addressTarget = event.getAddress() ;
            PAddress address = event.getAddress() ;

            debugln("     event address: " + address) ;
            debugln(" addressTransferee: " + m_addressTransferee) ;
            debugln("           compare: " + address.equals(m_addressTransferee)) ;


            if (address.equals(m_addressTransferee))
            {
                fireTransfereeHungup() ;
            }
            else
            {
                fireTransferTargetHungup() ;
            }
        }
    }
}
