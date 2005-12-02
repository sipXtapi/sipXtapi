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


package org.sipfoundry.sipxphone.app.conference ;

import java.util.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;

/**
 * The conference call list model aggregates both conference call participants
 * and potential conference call participants into a single list.  A potential
 * conference call participant is defined as named address or a held call.  It
 * is assume that only one conference can exist on the phone at any one time.
 * <p>
 * This list model will fire change notifications though the PListDataListener
 * interface whenever the aggregated list changes or an individual element of
 * the list changes (participant list changes or a held call drops, etc).
 * <p>
 * The list is sorted in a semi-predictable way.  Categories of participants/
 * potential participants are grouped together, however, we currently are not
 * providing sorting within groups (later, perhaps user selectable: duration,
 * name, etc).  Group order:
 * <ol>
 *    <li>Held calls</li>
 *    <li>Named Participants</li>
 *    <li>Conference Participants</li>
 *    <li>Dropped Participants</li>
 * </ol>
 * The theory is that any calls on hold should be dealt with as soon as
 * possible-- the user should not forget about them.  Named participants are
 * include if a user selected a conference group.  The conference controller
 * will want to select these and 'add' them sooner than later.  Lastly, the
 * conference participants are listed.  In most cases, the above two groups
 * will not exist.
 * <p>
 * TODO:
 * <ul>
 *   <li>held call need to be monitor and watched for drop/unhold</li>
 *   <li>entire list is reported as changed for any change, when we
 *       should only report the changed elements </li>
 *   <li>accessor methods must be added for named participants (conference
 *       groups)</li>
 * <ul>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConferenceCallListModel implements PListModel
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PCall  m_call ;             // call this list model is monitoring.
    private Vector m_vListeners ;       // list of data list listeners
    private Vector m_vNamedParticipants ;// possible conference participants
    private Vector m_vFailedConnections ;// list of failed connections

    private ConferenceCallDataItem m_participants[] ;   // list of participants
    private icConnectionListener m_connectionListener ; // monitors call for connection state changes
    private icHeldCallListener m_monitorHeldCalls ; // watch for held call state changes and refreshed world

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a conference call list model object.  Developers must call
     * setCall to gain any use of this list model.
     */
    public ConferenceCallListModel()
    {
        m_call = null ;
        m_connectionListener = new icConnectionListener() ;
        m_vNamedParticipants = new Vector() ;
        m_vFailedConnections = new Vector() ;
        m_monitorHeldCalls = new icHeldCallListener() ;
        m_participants = null ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the conference call who status should be exposed through this
     * list model
     *
     * @param call The conference call we will expose through this list model
     */
    public void setCall(PCall call)
    {
        if (call != m_call) {
            // If we are replacing a call, make sure we clean up
            if (m_call != null) {
                m_call.removeConnectionListener(m_connectionListener) ;
            }

            // Attach the new call
            if (call != null) {
                m_call = call ;
                m_call.addConnectionListener(m_connectionListener) ;
                update() ;
            }
        }
    }


    /**
     * Get the call used by this model
     *
     * @return call used by this model or null if not call is assigned
     */
    public PCall getCall()
    {
        return m_call ;
    }


    /**
     * What are the number of participants / connections in the coupled
     * call.
     *
     * @return the number of particpants in this conference call
     */
    public int getSize()
    {
        if (m_participants == null) {
            return 0 ;
        } else {
            return m_participants.length ;
        }
    }


    /**
     * Obtain the participant at position iIndex
     *
     * @param iIndex Index of the participant you're interested retreiving
     *
     * @return PAddress of the particpant at position iIndex
     */
    public Object getElementAt(int iIndex)
    {
        Object obj = null;
        if (m_participants == null) {
            return null ;
        } else {
            //this is a temprary check unless the real problem of listing invalid items in
            //this listmodel gets fixed( --pradeep )
            if( iIndex >=0 && iIndex < m_participants.length )
                obj =  m_participants[iIndex] ;
            else{
                System.out.println("INVALD INDEX PASSED TO getElementAt in ConferenceCallListModel ");
            }
        }
        return obj;
    }


    /**
     * Is the specified address an existing participant of this conference?
     */
    public boolean isActiveParticipant(PAddress address)
    {
        boolean bActive = false ;
        for (int i=0; i<getSize(); i++) {
            ConferenceCallDataItem item = (ConferenceCallDataItem) getElementAt(i) ;
            if (item != null) {
                if (address.equals(item.getAddress()) && (item.isActive())) {
                    bActive = true ;
                    break ;
                }
            }
        }

        return bActive ;
    }


    /**
     * Remove the specified inactive participants from this conference.  This
     * will blindly remove from the named list and then look though and do
     */
    public void removeInactiveParticipant(PAddress address)
    {
        remove(address) ;
    }


    public synchronized void remove(Object object)
    {
        m_vNamedParticipants.removeElement(object) ;
        m_vFailedConnections.removeElement(object) ;
        update() ;
    }


    public String getElementPopupTextAt(int iIndex)
    {
        return null ;
    }

    /**
     * Add the specified list listener to this list model.  The list data
     * listener is updated whenever a connection state changes and requires
     * a new GUI view.  If the listener is already a listening to this model
     * then the add is ignored.
     */
    public void addListDataListener(PListDataListener listener)
    {
        if (m_vListeners == null)
            m_vListeners = new Vector() ;

        if (!m_vListeners.contains(listener))
            m_vListeners.addElement(listener) ;
    }


    /**
     * Removed the specified listener from this conference list model.  If
     * the listener is not listening to this data model, the call is ignored.
     */
    public void removeListDataListener(PListDataListener listener)
    {
        if (m_vListeners != null) {
            m_vListeners.removeElement(listener) ;
        }
    }


    public void fireContentsChanged()
    {
        if (m_vListeners != null) {
            PListDataEvent event = new PListDataEvent(this, PListDataEvent.CONTENTS_CHANGED, 0, getSize()) ;
            Enumeration enum = m_vListeners.elements() ;
            while (enum.hasMoreElements()) {
                PListDataListener listener = (PListDataListener) enum.nextElement() ;
                listener.contentsChanged(event) ;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * @return An array of participants (addresses) for the specified call
     *         whose connection state does not match the specified exclusion
     *         state.
     */
    private PAddress[] getParticipantsExcludingState(PCall call, int iConnectionState)
    {
        PAddress addressesSource[] = call.getParticipants() ;
        PAddress addressesTarget[] = null ;

        // Null out any/all of the participants that match the excluded state.
        int iNumExcluded = 0 ;
        for (int i=0; i<addressesSource.length; i++) {
            if (call.getConnectionState(addressesSource[i]) == iConnectionState) {
                addressesSource[i] = null ;
                iNumExcluded++ ;
            }
        }

        // Build the target array
        if (iNumExcluded == 0) {
            // If nothing was excluded, use the source array as is.
            addressesTarget = addressesSource ;
        } else {
            // Copy the array over, skipping and null addresses
            addressesTarget = new PAddress[addressesSource.length - iNumExcluded] ;
            int iTargetIndex = 0 ;
            for (int i=0; i<addressesSource.length; i++) {
                if (addressesSource[i] != null) {
                    addressesTarget[iTargetIndex++] = addressesSource[i] ;
                }
            }
        }

        return addressesTarget ;
    }



    /**
     * Determine the number of non-named, non-failed or 'live' connections.
     */
    public int getLiveConnections()
    {
        int iLiveConnections = 0 ;

        if (m_call != null)
        {
            PAddress addresses[] = getParticipantsExcludingState(m_call,
                    PCall.CONN_STATE_FAILED) ;
            if (addresses != null)
            {
                iLiveConnections = addresses.length ;
            }
        }
        return iLiveConnections ;
    }


    /**
     * Updates the data model to reflect current participant list
     */
    public synchronized void update()
    {
        if (m_call != null) {
            // Get all of the addresses, excluding any failed connections.
            // When a connection fails, it's address is placed into the
            // m_vFailedConnections vector and populated later.
            PAddress addresses[] = getParticipantsExcludingState(m_call, PCall.CONN_STATE_FAILED) ;

            // PCall callsOnHold[] = Shell.getCallManager().getCallsByState(PCall.CONN_STATE_HELD) ;

            int iElements = m_vNamedParticipants.size() + m_vFailedConnections.size() + addresses.length /* callsOnHold.length */ ;
            int index = 0 ;
            m_participants = new ConferenceCallDataItem[iElements] ;

            // Add named participants
            for (int i=0; i<m_vNamedParticipants.size(); i++) {
                PAddress address = (PAddress) m_vNamedParticipants.elementAt(i) ;
                m_participants[index++] = new ConferenceCallDataItem(address) ;
            }

            // Add failed Participants
            for (int i=0; i<m_vFailedConnections.size(); i++) {
                PAddress address = (PAddress) m_vFailedConnections.elementAt(i) ;
                m_participants[index++] = new ConferenceCallDataItem(address, ConferenceCallDataItem.STATE_FAILED) ;
            }

            // Add 'real' participants
            for (int i=0; i<addresses.length; i++) {
                m_participants[index++] = new ConferenceCallDataItem(addresses[i], m_call) ;
            }

            // Sort Everything using the most inefficient sorting algorithm
            // known to man: Mr. BubbleSort
            //
            // TODO: Update this!
            for (int i=0; i<index; i++) {
                for (int j=0; j<index; j++) {
                    if (m_participants[i].getRenderedName().compareTo(m_participants[j].getRenderedName()) < 0) {
                        ConferenceCallDataItem temp = m_participants[i] ;
                        m_participants[i] = m_participants[j] ;
                        m_participants[j] = temp ;
                    }
                }
            }
        } else {
            m_participants = null ;
        }

        fireContentsChanged() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////
    public class icConnectionListener implements PConnectionListener
    {
        public void callCreated(PConnectionEvent event)
        {
//System.out.println("CONF: callCreated: " + event.getAddress()) ;

            update() ;
        }


        public void callDestroyed(PConnectionEvent event)
        {
//System.out.println("CONF: callDestroyed: " + event.getAddress()) ;

            update() ;
        }


        public void connectionTrying(PConnectionEvent event)
        {
//System.out.println("CONF: connectionTrying: " + event.getAddress()) ;

            update() ;
        }

        public void connectionUnknown(PConnectionEvent event)
        {
//System.out.println("CONF: connectionUnknown: " + event.getAddress()) ;

            update() ;
        }


        public void connectionOutboundAlerting(PConnectionEvent event)
        {
//System.out.println("CONF: connectionOutboundAlerting: " + event.getAddress()) ;

            update() ;
        }


        public void connectionInboundAlerting(PConnectionEvent event)
        {
//System.out.println("CONF: connectionInboundAlerting: " + event.getAddress()) ;

            update() ;
        }


        public void connectionConnected(PConnectionEvent event)
        {
//System.out.println("CONF: connectionConnected: " + event.getAddress()) ;

            update() ;
        }


        public void connectionFailed(PConnectionEvent event)
        {
//System.out.println("CONF: connectionFailed: " + event.getAddress()) ;


            m_vFailedConnections.addElement(event.getAddress()) ;
            update() ;


            // Drop any failed calls (Hack to deal with renegotiation errors)
            try
            {
                m_call.disconnect(event.getAddress()) ;
            }
            catch (Exception e)
            {
                // Ignore
            }
        }


        public void connectionDisconnected(PConnectionEvent event)
        {
//System.out.println("CONF: connectionDisconnected: " + event.getAddress()) ;

            // Do not add the local address to our list of named participants.
            if ((m_call.getLocalAddress() != null) &&
                    (!m_call.getLocalAddress().equals(event.getAddress())))
            {
                m_vFailedConnections.removeElement(event.getAddress()) ;
                m_vNamedParticipants.removeElement(event.getAddress()) ;
                m_vNamedParticipants.addElement(event.getAddress()) ;
                update() ;
            }
        }


        public void callHeld(PConnectionEvent event)
        {
//System.out.println("CONF: callHeld: " + event.getAddress()) ;

            update() ;
        }


        public void callReleased(PConnectionEvent event)
        {
//System.out.println("CONF: callReleased: " + event.getAddress()) ;

            update() ;
        }
    }


    private class icHeldCallListener implements PConnectionListener
    {
        public void callCreated(PConnectionEvent event) { /* stub */ }
        public void callDestroyed(PConnectionEvent event) { /* stub */ }
        public void connectionTrying(PConnectionEvent event)  { /* stub */ }
        public void connectionOutboundAlerting(PConnectionEvent event) { /* stub */ }
        public void connectionInboundAlerting(PConnectionEvent event) { /* stub */ }
        public void connectionConnected(PConnectionEvent event) { /* stub */ }
        public void connectionUnknown(PConnectionEvent event) { /* stub */ }
        public void callHeld(PConnectionEvent event) { /* stub */ }


        public void connectionFailed(PConnectionEvent event)
        {
            PCall call = event.getCall() ;

            if (call.getConnectionState() == PCall.CONN_STATE_FAILED) {
                call.removeConnectionListener(m_monitorHeldCalls) ;
            }
            update() ;
        }


        public void callReleased(PConnectionEvent event)
        {
            event.getCall().removeConnectionListener(m_monitorHeldCalls) ;
            update() ;
        }

        public void connectionDisconnected(PConnectionEvent event)
        {

            PCall call = event.getCall() ;
            if (call.getConnectionState() == PCall.CONN_STATE_DISCONNECTED) {
                call.removeConnectionListener(m_monitorHeldCalls) ;
            }
            update() ;
        }
    }
}
