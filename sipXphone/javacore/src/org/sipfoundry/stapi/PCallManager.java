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
 
package org.sipfoundry.stapi ;

import java.util.Vector ;
import java.util.Hashtable ;
import java.awt.Image ;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.callcontrol.* ;

import org.sipfoundry.util.SysLog;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.util.SipParser ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.app.core.MissedCallFeatureIndicator ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.featureindicator.* ;
import org.sipfoundry.sipxphone.service.Timer ;
import org.sipfoundry.sipxphone.service.Logger ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.sys.util.PingerInfo ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp ;
import org.sipfoundry.sipxphone.app.preferences.CallHandlingManager ;

import org.sipfoundry.sip.SipLine ;
import org.sipfoundry.sip.SipSession ;
import org.sipfoundry.sip.SipLineManager ;

import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;


/**
 * The PCallManager class acts as a repository for all of the active calls within the
 * system. Users can query this object for all calls or calls of certain types.
 * <p>
 * A 'global' call (connection) listener can also be added. This listener
 * will allow the application to be notified when the state of a call in the
 * system changes. For example, if someone hangs up on an active call, the application
 * can listen for such events using a {@link PConnectionListener}.
 *
 * @see PConnectionListener
 */
public class PCallManager
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected Hashtable m_htCallList = new Hashtable() ;
    protected Vector m_vCallList = new Vector() ;
    protected Vector m_vListenerList ;
    protected icListenerForwarder m_listenerForwarder ;
    protected Vector m_vZombieCalls ;
    protected Vector m_vHiddenCalls ;
    protected Hashtable m_heldCallFeatureIndicators ;
    protected IncomingCallFeatureIndicator m_incomingCallFeatureIndicator ;
    protected MissedCallFeatureIndicator   m_missedCallFeatureIndicator   ;

    /**
     * key constant to hold redirected address
     * @deprecated DO NOT EXPOSE
     */
    public static final String REDIRECTED_ADDRESS = "redirected_address";

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor guards against user construction
     */
    protected PCallManager()
    {
        m_listenerForwarder = new icListenerForwarder() ;
        m_vZombieCalls = new Vector() ;
        m_vHiddenCalls = new Vector() ;
        m_heldCallFeatureIndicators = new Hashtable();
        m_incomingCallFeatureIndicator = new IncomingCallFeatureIndicator();
        m_missedCallFeatureIndicator   = new MissedCallFeatureIndicator()   ;
        // Add terminal Listener to pick up new calls!
        Terminal terminal = ShellApp.getInstance().getTerminal() ;
        if (terminal != null) {
            try
            {
                if (Logger.isEnabled())
                {
                    terminal.addCallListener(new icTerminalListener()) ;
                }
                else
                {
                    // If logging is disabled, we only care about connection
                    // offering and call alerting.

                    // connectionOffering = 0x00040000
                    // callActive = 0x00100000
                    // ! 0x00140000 = FFEBFFFF
                    ((PtTerminal) terminal).addCallListener(new icTerminalListener(), 0xFFEBFFFF) ;
                }
            }
            catch (Exception e)
            {
                System.out.println("STAPI: ERROR: PCallManager cannot add terminal listener:") ;
                SysLog.log(e) ;
            }
        } else {
            System.out.println("STAPI: ERROR: PCallManager cannot get terminal reference.") ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
     /**
     * gets the singleton instance of MissedCallFeatureIndicator.
     * @deprecated DO NOT EXPOSE
     */
     public MissedCallFeatureIndicator getMissedCallFeatureIndicator(){
        return m_missedCallFeatureIndicator;
     }


    /**
     * Query for all calls within the system.
     *
     * @return Array of all calls in the system.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to query for all calls.
     */
    public PCall[] getCalls()
        throws SecurityException
    {
        PCall rc[] = null ;

        synchronized (m_vCallList)
        {
            // Copy all non-hidden items into an array
            int iActualCalls = 0;
            rc = new PCall[m_vCallList.size()] ;
            for (int i=0; i<m_vCallList.size(); i++)
            {
                PCall call = (PCall) m_vCallList.elementAt(i) ;
                if (!isHiddenCall(call))
                {
                    rc[iActualCalls++] = call ;
                }
            }

            // Resize the array if needed
            if (rc.length != iActualCalls)
            {
                PCall rcSized[] = new PCall[iActualCalls] ;
                for (int i=0; i<iActualCalls; i++)
                {
                    rcSized[i] = rc[i] ;
                }
                rc = rcSized ;
            }
        }

        return rc ;
    }


    /**
     * What is the active or 'in focus' call? A call is considered in focus if
     * it is exclusively using one of the primary audio devices.
     *
     * @return The {@link PCall} that is active or in focus.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to query for the infocus call.
     */
    public PCall getInFocusCall()
        throws SecurityException
    {
        /*
         * Fun begins here: We don't actually know what the active call is!
         * We need to figure it out based on the state of all of our calls.
         *
         * This is a temp solution until we get some more APIs...
         */

        PCall calls[] = getCalls() ;
        int iNumCalls = calls.length ;


        //
        // Next look for a connected call
        //
        for (int i=0; i<iNumCalls; i++) {
            if (calls[i].getConnectionState() == PCall.CONN_STATE_CONNECTED) {
                if (!isZombieCall(calls[i]) && (!calls[i].isDNDSet()))
                    if (!calls[i].isDropping())
                        return calls[i] ;
            }
        }

        //
        // Next looks for a outbound or inbound call
        //
        for (int i=0; i<iNumCalls; i++) {
            if (    (calls[i].getConnectionState() == PCall.CONN_STATE_IDLE) ||
                    (calls[i].getConnectionState() == PCall.CONN_STATE_TRYING) ||
                    (calls[i].getConnectionState() == PCall.CONN_STATE_INBOUND_ALERTING) ||
                    (calls[i].getConnectionState() == PCall.CONN_STATE_OUTBOUND_ALERTING)) {
                if (!isZombieCall(calls[i]) && (!calls[i].isDNDSet()) && !calls[i].m_bTransferControllerRelated)
                    if (!calls[i].isDropping())
                        return calls[i] ;
            }
        }

        //
        // Finally look for a call that's failed
        //
        for (int i=0; i<iNumCalls; i++) {
            if (    (calls[i].getConnectionState() == PCall.CONN_STATE_DISCONNECTED) ||
                   (calls[i].getConnectionState() == PCall.CONN_STATE_FAILED)) {
                if (!isZombieCall(calls[i]) && (!calls[i].isDNDSet()))
                    if (!calls[i].isDropping())
                        return calls[i] ;
            }
        }

        return null ;
    }

    /**
     * Is this call state a valid state?
     * @return boolean True if the state value falls within the valid range
     *         else false.
     *
     */
    private boolean isValidState(int iState)
    {
        boolean bIsValid = false;
        if (iState >= PCall.CONN_STATE_UNKNOWN && iState <= PCall.CONN_STATE_MAX)
            bIsValid = true;

        return bIsValid;
    }

    /**
     * Query for all calls in a particular connection state. See {link PCall}
     * for a list of call states.
     * <p>
     * This method will only return a call where all connections match the
     * requested call state. For example, if one call participant is in the
     * CONN_STATE_CONNECTED state, and the other participant is in the
     * CONN_STATE_OUTBOUND_ALERTING state, then
     * the call would not be classified as either state.
     *
     * @param iState The state constant you wish to filter on. All connections
     *        must match the state to be returned.
     *
     * @return The PCall[] array of PCalls matching the requested iState state.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to query for the calls.
     *
     * @exception IllegalArgumentException Thrown if the specified state id
     *            is invalid
     *
     * @see PCall
     */
    public PCall[] getCallsByState(int iState)
        throws IllegalArgumentException, SecurityException
    {
        PCall rc[] = null ;

        if (isValidState(iState) == false)
            throw new IllegalArgumentException("Invalid State passed. ouch.");

        if (m_vListenerList != null) {
            Vector vCallsMatchingState = new Vector() ;


            /*
             * Step 1: Toss all of the calls that pass our state check in
             *         check in vector.
             */
            PCall allCalls[] = getCalls() ;
            for (int i=0; i<allCalls.length; i++) {
                if (    (allCalls[i].getConnectionState() == iState) &&
                        (!isZombieCall(allCalls[i])) &&
                        (!allCalls[i].m_bTransferControllerRelated))

                {
                    vCallsMatchingState.addElement(allCalls[i]) ;
                }
            }


            /*
             * Step 2: Convert the Vector into an array
             */
            rc = new PCall[vCallsMatchingState.size()] ;
            for (int i=0; i<vCallsMatchingState.size(); i++)
            {
                rc[i] = (PCall) vCallsMatchingState.elementAt(i) ;
            }
        }
        return rc ;
    }


    /**
     * Register a JTAPI call with the CallManager and request tracking
     * of the call states. This method should be used if a call is created using
     * JTAPI (not the PCall::createCall method) and the creator wants to
     * turn control of the call over to the PCallManager and core system.
     *
     * If the call is already being monitor by the core system, then a
     * reference to that call is returned.
     *
     * @param call Specifies the call that should be monitored by the
     *             CallManager.
     *
     * @return The newly created PCall object representing the original
     *         JTAPI call object.
     *
     * @see PCall
     */
    public PCall monitorJTAPICall(javax.telephony.Call call)
    {
        ShellApp shell = ShellApp.getInstance() ;
        String strCallId = ((PtCall)call).getCallID() ;

        PCall newCall = getCallByCallID(strCallId) ;
        if (newCall == null)
        {
            newCall = new PCall(call, false) ;
        }
        shell.monitorCall(newCall) ;

        return newCall ;

    }


    /**
     * @deprecated do not expose
     *
     */
    protected PCall createCall()
        throws PCallResourceUnavailableException
    {
        ShellApp            shellApp = ShellApp.getInstance() ;
        AudioSourceControl  asc = shellApp.getAudioSourceControl() ;
        Provider            provider = shellApp.getProvider() ;
        PhoneState          phoneState = PhoneState.getInstance() ;

        PCall callRC = null ;
        PtCallControlCall call = null ;

        if (!PingerApp.checkLowMemoryCondition(true)) {
            synchronized (m_vCallList) {
                try {
                    // Create the call
                    call = (PtCallControlCall) provider.createCall() ;
                    System.out.println("<<CREATE " + ((PtCall) call).getCallID() + ">>") ;
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi", Logger.TRAIL_COMMAND, "createCall") ;
                    }
                } catch (Exception e) {
                    System.out.println("STAPI:: Error while creating call: " + e) ;
                    SysLog.log(e) ;
                    if (call != null) {
                        try {
                            System.out.println("<<DROP " + ((PtCall) call).getCallID() +">>") ;
                            call.drop() ;
                        } catch (Exception eat) {
                            /* gulp */
                        }
                        call = null ;
                    } else {
                        throw new PCallResourceUnavailableException() ;
                    }
                }

                if (call != null)
                {
                    callRC = new PCall(call, true) ;
                    callRC.setState(PCall.CONN_STATE_IDLE, null, PtConnectionEvent.CAUSE_NORMAL, true, true) ;
                    callRC.m_bWasOutboundCall = true ;
                }
            }
        } else {
            throw new PCallResourceUnavailableException() ;
        }
        return callRC ;
    }


    /**
     * Ignore the specified call. This method will force the core system to
     * ignore the state changes of the specified call. For example, the
     * system will not display the establishing call or call in progress forms
     * if the call is ignored prior to invoking the <i>connect()</i> statement.
     * <p>
     * Additionally, if the specified call is the infocus call, the any core
     * forms displaying status about that call will be closed. If the call
     * is unknown or already ignored, nothing is done (silent error).
     *
     * @param call The call that will be ignored by the core system
     */
    public void ignoreCall(PCall call)
        throws SecurityException
    {
        ShellApp shell = ShellApp.getInstance() ;
        shell.ignoreCall(call) ;
    }

    /**
     * Ignore the specified call. This method will force the core system to
     * ignore the state changes of the specified call. For example, the
     * system will not display the establishing call or call in progress forms
     * if the call is ignored prior to invoking the <i>connect()</i> statement.
     * <p>
     * Additionally, if the specified call is the infocus call, the any core
     * forms displaying status about that call will be closed. If the call
     * is unknown or already ignored, nothing is done (silent error).
     *
     * @param strCallID Calls with this callid that will be ignored by the
     *        core system
     *
     * @deprecated DO NOT EXPOSE
     */
    public void ignoreCallByID(String strCallID)
        throws SecurityException
    {
        ShellApp shell = ShellApp.getInstance() ;
        shell.ignoreCallByID(strCallID) ;
    }



    /**
     * Monitor the specified STAPI called. This method should be invoked if
     * the specified call was previously ignored by invoking the <i>ignoreCall</i>
     * method. This will cause the core system to being listening to state
     * changes raise core forms as needed. If the call is already monitored
     * by the core system, nothing is done (silent error).
     *
     * @param call The call that will be monitored by the core system
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to perform this operation.
     */
    public void monitorCall(PCall call)
        throws SecurityException
    {
        ShellApp shell = ShellApp.getInstance() ;
        shell.monitorCall(call) ;

        // Make sure the held call screen is in sync with the call status
        if (call.getConnectionState() == PCall.CONN_STATE_HELD)
        {
            if (!isZombieCall(call) && !isHiddenCall(call))
            {
                onHeldCall(call);
                updateHeldStatus() ;
                m_incomingCallFeatureIndicator.callNotRinging(call) ;
            }
        }
        else if (call.getConnectionState() == PCall.CONN_STATE_CONNECTED)
        {
            if (!isZombieCall(call) && !isHiddenCall(call))
            {
                onReleasedHeldCall(call);
                m_incomingCallFeatureIndicator.callNotRinging(call) ;
            }
        }
        else if (call.getConnectionState() == PCall.CONN_STATE_INBOUND_ALERTING)
        {
            if (!isZombieCall(call) && !isHiddenCall(call))
            {
                m_incomingCallFeatureIndicator.callRinging(call) ;
            }
        }else{
            if (!isZombieCall(call) && !isHiddenCall(call))
            {
                m_incomingCallFeatureIndicator.callNotRinging(call) ;
            }
        }

    }


    /**
     * At various times a call is no longer useful; however, that call must be
     * tracked until death. Those calls are moved into a 'zombie' state.
     *
     * @deprecated do not expose
     */
    public void zombieCall(PCall call)
    {
        if (call != null)
        {
            String strCallID = call.toString() ;

            if (!isZombieCall(call))
            {
                System.out.println("ZOMBIE: " + call) ;
                m_vZombieCalls.addElement(strCallID) ;
                // Removed this from the the held call indicators
                onReleasedHeldCall(call);
                m_incomingCallFeatureIndicator.callNotRinging(call) ;
                updateHeldStatus() ;
            }
        }
        else
        {
            throw new IllegalArgumentException("passed null to zombie call") ;
        }
    }


    /**
     * This method will resurrect a zombie call back to life. This method
     * will not update the GUI state, however, the core app will start to
     * monitor it's state changes.
     *
     * @deprecated do not expose
     */
    public void resurrectCall(PCall call)
    {
        if (call != null)
        {
            String strCallID = call.toString() ;
            if (m_vZombieCalls.contains(strCallID))
            {
                System.out.println("RESURRECT: " + call) ;
                m_vZombieCalls.removeElement(strCallID) ;

                if (call.getConnectionState() == PCall.CONN_STATE_HELD)
                {
                    // Add this to the the held call indicators
                    onHeldCall(call);
                    updateHeldStatus() ;
                }
                else
                {
                    // remove this from  the held call indicators
                    onReleasedHeldCall(call);
                    updateHeldStatus() ;
                }
            }
        }
        else
        {
            throw new IllegalArgumentException("passed null to resurrect call") ;
        }
    }


    /**
     * Is the specified call a zombie call?
     *
     * @deprecated do not expose
     */
    public boolean isZombieCall(PCall call)
    {
        boolean bIsZombie = false ;

        if (call != null)
        {
            String strCallID = call.toString() ;
            bIsZombie = m_vZombieCalls.contains(strCallID) ;
        }
        return bIsZombie ;
    }


    /**
     * Hides call from the various enumerating methods on the PCallManager.
     *
     * @deprecated do not expose
     */
    public void hideCall(PCall call)
    {
        if (call != null)
        {
            String strCallID = call.toString() ;

            if (!m_vHiddenCalls.contains(strCallID))
            {
                System.out.println("HIDE: " + call) ;
                m_vHiddenCalls.addElement(strCallID) ;
                // Removed this from the the held call indicators
                onReleasedHeldCall(call);
                m_incomingCallFeatureIndicator.callNotRinging(call);
                updateHeldStatus() ;
            }
        }
        else
        {
            throw new IllegalArgumentException("passed null to hideCall") ;
        }
    }


    /**
     * Shows a call previously hidden via the hideCall method.
     *
     * @deprecated do not expose
     */
    public void showCall(PCall call)
    {
        if (call != null)
        {
            String strCallID = call.toString() ;

            if (m_vHiddenCalls.contains(strCallID))
            {
                System.out.println("SHOW: " + call) ;
                m_vHiddenCalls.removeElement(strCallID) ;

                if (call.getConnectionState() == PCall.CONN_STATE_HELD)
                {
                    // Add this to the the held call indicators
                    onHeldCall(call);
                    updateHeldStatus() ;
                }
                else
                {
                    onReleasedHeldCall(call);
                    updateHeldStatus() ;
                }
            }
        }
        else
        {
            throw new IllegalArgumentException("passed null to showCall") ;
        }
    }


    /**
     * Has the specified call been hidden from view?
     *
     * @deprecated do not expose
     */
    public boolean isHiddenCall(PCall call)
    {
        boolean bIsHidden = false ;

        if (call != null)
        {
            String strCallID = call.toString() ;
            bIsHidden = m_vHiddenCalls.contains(strCallID) ;
        }
        else
        {
            throw new IllegalArgumentException("passed null to isHiddenCall") ;
        }

        return bIsHidden ;

    }


    /**
     * Add a call listener to this {link PCall}. After adding, the listener
     * will receive notification of state changes for all calls known
     * to the CallManager.
     *
     * @param listener Call listener that will receive notifications.
     */
    public void addConnectionListener(PConnectionListener listener)
    {
        if (m_vListenerList == null) {
            m_vListenerList = new Vector() ;
        }

        synchronized (m_vListenerList) {
            if (!m_vListenerList.contains(listener)) {
                m_vListenerList.insertElementAt(listener, 0) ;
            }
        }
    }


    /**
     * Remove a call listener from the CallManager.
     *
     * @param listener Call listener that wishes to suspend notifications.
     */
    public void removeConnectionListener(PConnectionListener listener)
    {
        if (m_vListenerList != null) {
            synchronized (m_vListenerList) {
                m_vListenerList.removeElement(listener) ;
            }
        }
    }

    /**
    * @deprecated This is for internal use only
    */
    public void dump()
    {
        System.out.println("Call List: ") ;
        PCall calls[] = getCalls() ;

        for (int i=0; i<calls.length; i++) {

            if (isZombieCall(calls[i]))
            {
                System.out.print("ZOMBIE ") ;
            }

            if (isHiddenCall(calls[i]))
            {
                System.out.print("HIDDEN ") ;
            }

            switch (calls[i].getConnectionState())
            {
                case PCall.CONN_STATE_CONNECTED:
                    System.out.print("CONNECTED: ") ;
                    break ;
                case PCall.CONN_STATE_TRYING:
                    System.out.print("   TRYING: ") ;
                    break ;
                case PCall.CONN_STATE_OUTBOUND_ALERTING:
                    System.out.print(" ALERTING: ") ;
                    break ;
                case PCall.CONN_STATE_INBOUND_ALERTING:
                    System.out.print("  RINGING: ") ;
                    break ;
                case PCall.CONN_STATE_IDLE:
                    System.out.print("     IDLE: ") ;
                    break ;
                case PCall.CONN_STATE_HELD:
                    System.out.print("     HELD: ") ;
                    break ;
                case PCall.CONN_STATE_FAILED:
                    System.out.print("   FAILED: ") ;
                    break ;
                case PCall.CONN_STATE_DISCONNECTED:
                    System.out.print("  DROPPED: ") ;
                    break ;
            }

            System.out.println(calls[i].toString()) ;
            calls[i].dumpConnections() ;
            calls[i].dumpConnectionsCache() ;
        }
    }


    /**
     * Returns whether forwarding should be allowed for the called address.  If a line is
     * not found for the called address, the device line is checked.  If no device
     * line is defined, the method returns false.
     *
     * @deprecated do not expose
     */
    public boolean isForwardingAllowed(SipSession session)
    {
        boolean bAllowForwarding = false ;

        SipLineManager manager = SipLineManager.getInstance() ;
        SipLine line = manager.getLineBySession(session) ;
        if (line != null)
        {
            bAllowForwarding = line.isAllowForwardingEnabled() ;
        }
        else
        {
            // If we can not find a defined line, use the settings for the device line
            SipLine lines[] = manager.getLines() ;
            if (lines != null)
            {
                for (int i=0; i<lines.length; i++)
                {
                    if (lines[i].getUser().equalsIgnoreCase("Device"))
                    {
                        bAllowForwarding = lines[i].isAllowForwardingEnabled() ;
                    }
                }
            }
        }
        return bAllowForwarding ;
    }


    /**
     * @deprecated do not expose, ever.
     *
     * Find a call by the PTAPI/TAO call id. This is sort of an expensive
     * operation and should not be used randomly. If you find that you're
     * using this call a lot- we are probably doing something wrong.
     */
    public PCall getCallByCallID(String strCallID)
    {
        PCall callRC = null ;

        synchronized (m_vCallList) {
            for (int i=0; i<m_vCallList.size(); i++) {
                PCall candidate = (PCall) m_vCallList.elementAt(i) ;
                String strCandidateID = candidate.getCallID() ;
                if ((strCandidateID != null) && strCandidateID.equals(strCallID)) {
                    callRC = candidate ;
                    break ;
                }
            }
        }
        return callRC ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Query for all calls within the system (including hidden)
     *
     * @return Array of all calls in the system.
     */
    protected PCall[] getAllCalls()
    {
        PCall rc[] = null ;

        synchronized (m_vCallList)
        {
            // Copy all non-hidden items into an array
            rc = new PCall[m_vCallList.size()] ;
            for (int i=0; i<m_vCallList.size(); i++)
            {
                rc[i] = (PCall) m_vCallList.elementAt(i) ;
            }
        }

        return rc ;
    }


    /**
     * get all of the call listeners registered to this call.
     *
     * @return PCallListener array holding call listeners registered to this call
     */
    protected PConnectionListener[] getConnectionListeners()
    {
        PConnectionListener[] rc = null ;

        // If we have a list return that list other wise return an empty array
        if (m_vListenerList != null) {
            synchronized (m_vListenerList) {

                int iItems = m_vListenerList.size() ;

                rc = new PConnectionListener[iItems] ;
                for (int i=0;i<iItems;i++) {
                    rc[i] = (PConnectionListener) m_vListenerList.elementAt(i) ;
                }
            }
        } else {
            rc = new PConnectionListener[0] ;
        }

        return rc ;
    }


    /**
     * Register a call with the CallManager.
     */
    protected void registerCall(PCall call)
    {
        synchronized (m_vCallList) {
            if (!m_htCallList.containsKey(call.getCallID())) {
                m_htCallList.put(call.getCallID(), call) ;
                m_vCallList.addElement(call) ;
                call.addConnectionListener(m_listenerForwarder) ;
            }
        }
    }


    /**
     * Deregister a call with the CallManager.
     */
    protected void deregisterCall(PCall call)
    {
        synchronized (m_vCallList) {
            call.removeConnectionListener(m_listenerForwarder) ;
            m_htCallList.remove(call.getCallID()) ;
            m_vCallList.removeElement(call) ;
            m_vZombieCalls.removeElement(call.toString()) ;
            m_vHiddenCalls.removeElement(call.toString()) ;
        }
    }

    /**
     * Updated the held LED to reflect the state of the held calls on this
     * phone.
     */
    protected void updateHeldStatus()
    {
        PCall heldCalls[] = getCallsByState(PCall.CONN_STATE_HELD) ;
        AudioSourceControl asc = PhoneHelper.getInstance().getAudioSourceControl() ;

        PhoneLamp lamp = asc.getButtonLamp("HOLD") ;
        if (lamp != null) {
            int iMode = PhoneLamp.LAMPMODE_OFF ;
            if ((heldCalls != null) && (heldCalls.length > 0)) {
                iMode = PhoneLamp.LAMPMODE_STEADY ;
            }
            try {
                lamp.setMode(iMode) ;
            } catch (InvalidArgumentException e) {
                SysLog.log(e) ;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Inner/Nested Classes
////

    /**
     * icListenerForward is added to a call when registered and refires all
     * notifications to 'Global' connection listeners (ones added to Call
     * Manager)
     */
    protected class icListenerForwarder implements PConnectionListener
    {
        /**
         * A call has been created by the PCall.createCall() method. The call is
         * in the idle state and the PAddress within the event object is not yet
         * valid.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callCreated(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].callCreated(event) ;
            }
        }


        /**
         * A call has been destroyed and is no longer valid. This is an
         * appropriate time to remove listeners and close applications/forms that
         * are monitoring specific calls.
         *
         * @param event The {@link PConnectionEvent} object containing both the call
         *        and address objects associated with the state change, and a
         *        cause ID if a failure has occurred.
         */
        public void callDestroyed(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].callDestroyed(event) ;
            }

            updateHeldStatus() ;
            onReleasedHeldCall(event.getCall());
            m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
            m_vZombieCalls.removeElement(event.getCall().toString()) ;
            m_vHiddenCalls.removeElement(event.getCall().toString()) ;
        }



        /**
         * A call/address connect() attempt has been made, however, the
         * target has not yet responded. The state can proceed to outbound
         * alerting, connected, or failed.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionTrying(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionTrying(event) ;
            }
        }


        /**
         * A call/address connect() has been acknowledged by the target and is
         * alerting or (ringing). The state can proceed to disconnected,
         * connected, or failed.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionOutboundAlerting(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionOutboundAlerting(event) ;
            }
        }


        /**
         * An inbound call has been detected and is ringing the local terminal.
         * The user should pick up the phone to answer or can progamatically answer
         * the call by calling answer() on the call object held within the event
         * object. The staet canb proceed to connected, failed, or disconnected.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionInboundAlerting(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionInboundAlerting(event) ;
            }

            PCall call = event.getCall() ;
            if (!isZombieCall(call) && !isHiddenCall(call) &&
                !ShellApp.getInstance().getCoreApp().isCallIgnored(call) )
            {
                m_incomingCallFeatureIndicator.callRinging(call) ;
            }
        }


        /**
         * A connection has been established. The state can
         * proceed to Disconnected, held, or failed.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionConnected(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionConnected(event) ;
            }
            m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
        }


        /**
         * A connection has moved into an unsupported or unknown state. See the
         * cause code within the connection event for more information. This
         * state should not be considered a failure and most applications should
         * ignore this state change.
         *
         * @param event The PConnectionEvent object containing both the call and
         *        address objects associated with the state change, and a cause ID
         *        if one is available.
         */
        public void connectionUnknown(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionUnknown(event) ;
            }
            if (!isZombieCall(event.getCall()) && !isHiddenCall(event.getCall()))
            {
                m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
            }
        }


        /**
         * A connection has failed for one of various reasons. See the cause
         * within the connection event for more information. The state will not
         * change after this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionFailed(PConnectionEvent event)
        {


            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionFailed(event) ;
            }
            if (!isZombieCall(event.getCall()) && !isHiddenCall(event.getCall()))
            {
                m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
            }
        }

        /**
         * A connection has been disconnected for one of various reasons. See the
         * cause within the connection event for more information. The state will
         * not change after this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void connectionDisconnected(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++) {
                listeners[i].connectionDisconnected(event) ;
            }
             if (!isZombieCall(event.getCall()) && !isHiddenCall(event.getCall()))
            {
                m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
            }

             
             PCall call = event.getCall();
             if (call != null)
             {
                 HeldCallFeatureIndicator feature = getHeldCallFeatureIndicator(call.getCallID());
                 if (feature != null &&  feature.isIndicatorInstalled())
                 {
                     updateHeldStatus() ;
                     PCall heldCalls[] = getCallsByState(PCall.CONN_STATE_HELD);
                     if ((heldCalls == null) || (heldCalls.length == 0)) 
                     {
                         onReleasedHeldCall(call);
                     }
                 }
            }
        }


        /**
         * The entire call (all connections) have been placed on hold. At this
         * point no audio will be exchanged between all participants of the call.
         * The state can change to connected, failed, or dropped from this point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callHeld(PConnectionEvent event)
        {
            PCall call = event.getCall() ;

            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++)
            {
                listeners[i].callHeld(event) ;
            }

            updateHeldStatus() ;
            if (!isZombieCall(call) && !isHiddenCall(call))
            {
                onHeldCall(call);
                m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
            }
        }


        /**
         * A call has been released (taken off) hold. At this point audio is once
         * again be exchanged between all participants of the call. The state is
         * connected and proceed to disconnected, failed, or dropped from this
         * point.
         *
         * @param event Connection Event object containing both call and
         *        address object of the state change and a cause id if an
         *        failure has occurred.
         */
        public void callReleased(PConnectionEvent event)
        {
            // Simply refire to additional listeners
            PConnectionListener listeners[] = getConnectionListeners() ;
            for (int i=0; i<listeners.length; i++)
            {
                listeners[i].callReleased(event) ;
            }

            updateHeldStatus() ;

            onReleasedHeldCall(event.getCall());

            m_incomingCallFeatureIndicator.callNotRinging(event.getCall()) ;
        }
    }


    /**
     * This terminal listener sits around and wait for new/unknown incoming
     * connections. Upon notification of a new call, this code kicks off the
     * offering accept/decline/redirect decision making and adds the call to
     * our state worlds.
     * <p>
     * This listener also watches for random call failures. If a call fails
     * and aren't trackiing it- we squash it like a bug it is!
     *
     */
    protected class icTerminalListener extends PtTerminalConnectionAdapter
    {
        public void connectionOffered(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionOffered",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }

            // Only do work if this is an inbound connection
            PtConnection connection = (PtConnection) (event.getConnection()) ;
            if (((PtConnectionEvent) event).isLocalConnection())
            {
                PingerApp.checkLowMemoryCondition(false) ;

                PingerInfo pingerInfo = PingerInfo.getInstance() ;
                CallHandlingManager manager = Shell.getCallHandlingManager();
                PtCallControlCall call = ((PtCallControlCall) event.getCall()) ;
                String strCallID = call.getCallID() ;
                PCall stapiCall = getCallByCallID(strCallID) ;

                // Verify that the connection is still valid.  We have seen
                // cases where a call comes in and is aborted before java
                // ever hears about it.  In those cases, the connection state
                // is failed or disconnected.
                if (    (stapiCall.getConnectionState() == PCall.CONN_STATE_FAILED) ||
                        (stapiCall.getConnectionState() == PCall.CONN_STATE_DISCONNECTED))
                {
                    System.out.println("Cannot answer an offer for a failed call "+strCallID+" , dropping...") ;
                    try
                    {
                        stapiCall.disconnect() ;
                    }
                    catch (Exception e)
                    {
                        SysLog.log(e) ;
                    }
                    return ;
                }

                try
                {
                    boolean bMaxConnectionsHit = pingerInfo.getSimultaneousConnections() >= (pingerInfo.getMaxSimultaneousConnections()) ;
                    boolean bAcceptingCalls = ShellApp.getInstance().getCoreApp().isAcceptCallsEnabled() ;

                    String redirectedAdddress = null;
                    String strConnectionName = connection.getAddress().getName() ;

                    PAddress address = PAddressFactory.getInstance().createAddress(call.getCallingAddress().getName()) ;
                    PAddress addressCalled = null ;
                    if (stapiCall != null)
                    {
                        addressCalled = stapiCall.getCalledAddress() ;
                    }
                    else
                    {
                        addressCalled = PAddressFactory.getInstance().createAddress(connection.getFromURI()) ;
                    }
                    PAddress addressConnection = PAddressFactory.getInstance().createAddress(strConnectionName) ;

                    boolean  bForwardingAllowed ;
                    if (stapiCall != null)
                    {
                        bForwardingAllowed = isForwardingAllowed((SipSession) stapiCall.getSession(addressConnection)) ;
                    }
                    else
                    {
                        bForwardingAllowed = isForwardingAllowed(connection.getSession()) ;
                    }

                    StringBuffer buffer = new StringBuffer();
                    buffer.append("\n");
                    buffer.append("-------New inbound call-------") ;
                    buffer.append("\n");
                    buffer.append(" CallID: "+ strCallID );
                    buffer.append("\n");
                    buffer.append("  Calling Address: " + address) ;
                    buffer.append("\n");
                    buffer.append("    Local Address: " + addressCalled)  ;
                    buffer.append("\n");
                    buffer.append(" Allow Forwarding: " + bForwardingAllowed) ;
                    buffer.append("\n");
                    buffer.append("  Accepting calls: " + bAcceptingCalls) ;
                    buffer.append("\n");
                    buffer.append("  Max Connections: " + bMaxConnectionsHit) ;
                    buffer.append("\n");
                    System.out.println(buffer.toString());
                    // Make sure we aren't over the hardwired connection limit or blocking calls
                    if (!bAcceptingCalls || bMaxConnectionsHit)
                    {
                        // If Forward all, Forward to that address
                        if (manager.isForwardingAllEnabled() && bForwardingAllowed)
                        {
                            redirectedAdddress = manager.getTranslatedSipForwardingAll();
                            ((CallControlConnection) event.getConnection()).redirect(manager.getTranslatedSipForwardingAll()) ;
                        }
                        // If Forward Busy, Forward...
                        else
                        {
                            if (manager.isForwardOnBusyEnabled() && bForwardingAllowed)
                            {
                                redirectedAdddress =  manager.getTranslatedSipForwardingBusy();
                                ((CallControlConnection) event.getConnection()).redirect(manager.getTranslatedSipForwardingBusy()) ;
                            }
                            else
                            {
                                ((CallControlConnection) connection).reject() ;
                            }
                        }
                    }
                    // Otherwise let the application framework decide whether to take the call or not
                    else
                    {
                        CallFilterHookData data = new CallFilterHookData(address, addressCalled, strCallID) ;
                        Shell.getHookManager().executeHook(HookManager.HOOK_CALL_FILTER, data) ;
                        switch (data.getAction()) {
                            case CallFilterHookData.ACTION_ACCEPT:
// System.out.println("ACCEPTING CONNECTION") ;
                                ((CallControlConnection) connection).accept() ;

                                // If we have enabled forward on no answer,
                                // then add a forward on answer listener to
                                // track the call and forward it when
                                // appropriate.
                                if ((manager.isForwardOnNoAnswerEnabled()) &&
                                        bForwardingAllowed)
                                {
                                    String strRedirectURL = manager.getTranslatedSipForwardingNoAnswer() ;
                                    int    iRedirectAfterSeconds = manager.getPhonesetNoAnswerTimeout() ;
                                    if (iRedirectAfterSeconds <= 0)
                                        iRedirectAfterSeconds = 24 ;

                                    if (strRedirectURL != null)
                                    {
                                        new icForwardOnNoAnswerHandler(call, strRedirectURL, iRedirectAfterSeconds) ;
                                    }
                                }

                                break ;
                            case CallFilterHookData.ACTION_DECLINE:
// System.out.println("REJECTING CONNECTION") ;
                                ((CallControlConnection) connection).reject() ;
                                break ;
                            case CallFilterHookData.ACTION_REDIRECT:
                                System.out.println("REDIRECTING CONNECTION") ;
                                redirectedAdddress =  data.getRedirectAddress().getAddress();
                                ((CallControlConnection) connection).redirect(data.getRedirectAddress().getAddress()) ;
                                break ;
                        }
                    }
                    if ( (stapiCall != null ) && (redirectedAdddress != null) )
                        stapiCall.setCallData(REDIRECTED_ADDRESS, redirectedAdddress);
                } catch (Exception e) {
                    System.out.println("Error trying to invoke call filter hook: " + e.toString()) ;
                    SysLog.log(e) ;
                }
            }
        }

        public void connectionAlerting(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionAlerting",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }
        }

        public void connectionConnected(ConnectionEvent event)
        {
        	
        	// HACK: In some cases, the lower layers fire off a local 
        	// connection event before the upper layer is able to add a 
        	// listener.  In theory, adding a JTAPI listener is supposed to
        	// update the listener by firing off relevant events.   This works
        	// around this by looking for a local connection event w/ a 
        	// missing call.  If this situation exists, the STAPI call is 
        	// created and local state primed to connected. 
			if (((PtConnectionEvent) event).isLocalConnection())
			{
				PtCall jtapiCall = (PtCall) event.getCall() ;
				PCall call = getCallByCallID(jtapiCall.getCallID()) ;
				if (call == null)
				{
					call = new PCall(event.getCall(), false) ;
					call.setLocalConnectionState(PCall.CONN_STATE_CONNECTED) ;
				}
								
			}
        	
        	
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionConnected",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }

        }

        public void connectionCreated(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionCreated",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }
        }

        public void connectionDisconnected(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionDisconnected",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }
        }

        public void connectionInProgress(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionInProgress",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }
        }

        public void connectionUnknown(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionUnknown",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }
        }

        public void connectionFailed(ConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "connectionFailed",
                        SipParser.stripParameters((PtAddress) event.getConnection().getAddress()),
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }

/*
            PtCall call = (PtCall) event.getCall() ;
            String callID = call.getCallID() ;
            if ((callID != null) && (callID.length() > 0))
            {
                if (getCallByCallID(callID) == null) {
                try
                {
                    ((PtCallControlCall)call).drop() ;
                } catch (Exception e) { }
            }
*/
        }


        public void terminalConnectionActive(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionActive",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void terminalConnectionCreated(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionCreated",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void terminalConnectionDropped(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionDropped",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void terminalConnectionPassive(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionPassive",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void terminalConnectionUnknown(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionUnknown",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }

        public void terminalConnectionHeld(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionHeld",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }

        }


        public void terminalConnectionTalking(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionTalking",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void terminalConnectionRinging(TerminalConnectionEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "terminalConnectionRinging",
                        ((PtCall) event.getCall()).getCallID(),
                        (((PtTerminalConnection) event.getTerminalConnection()).isLocal()) ? "Local" : "Remote",
                        String.valueOf(event.getCause())) ;
            }
        }


        public void callActive(CallEvent event)
        {
            if (Logger.isEnabled())
            {
                Logger.post("jtapi",
                        Logger.TRAIL_NOTIFICATION,
                        "callActive",
                        ((PtCall) event.getCall()).getCallID(),
                        String.valueOf(event.getCause())) ;
            }

            PtCall call = (PtCall)event.getCall() ;
            String callID = call.getCallID() ;

            // If the call is not known, add it to our tracking system.
            if (getCallByCallID(callID) == null)
            {
                PCall newCall = new PCall(call, false) ;
                int iConnState = newCall.getConnectionState() ;
                if (    (iConnState == PCall.CONN_STATE_DISCONNECTED) ||
                        (iConnState == PCall.CONN_STATE_FAILED))
                {
                    // Whack-a-mole: If the call is already dead, then drop it.
                    try
                    {
                        newCall.disconnect();
                    }
                    catch (Exception e)
                    {
                        SysLog.log(e) ;
                    }
                }
                else
                {
                    newCall.setState(PCall.CONN_STATE_IDLE, null, PtConnectionEvent.CAUSE_NORMAL, true, true) ;

                    if (ShellApp.getInstance().getCoreApp().isCallIgnored(newCall))
                    {
                        newCall.setDND(true) ;
                        newCall.addConnectionListener(new icCallKiller()) ;
                    }
                    else
                    {
                        // There is a chance that the call has been created by the
                        // lower layer in reponse to a transfer or some third party
                        // call control.  If that is the case, we need toss up the
                        // appropriate status dialog -- monitor call should do
                        // that.

                        MetaEvent metaEvent = event.getMetaEvent() ;
                        if ((metaEvent != null) &&
                                (metaEvent.getID() == 134)) // MULTICALL_META_TRANSFER_STARTED
                        {
                            PAddress addr[] = newCall.getParticipants() ;
                            if (addr.length > 0)
                            {
                                monitorCall(newCall) ;
                            }
                        }
                    }
                }
            }
        }
    }

    // effectively adds a call to the home screen
    private void onHeldCall(PCall call)
    {
        //m_heldCallFeatureIndicator.callHeld(call) ;
        if (call != null && !call.m_bTransferControllerRelated)
        {
            HeldCallFeatureIndicator feature = getHeldCallFeatureIndicator(call.getCallID());
            if (feature == null)
            {
                feature = new HeldCallFeatureIndicator(call);

                FeatureIndicatorManager manager = Shell.getFeatureIndicatorManager();
                manager.installIndicator(feature,  FeatureIndicatorManager.VIEWSTYLE_LINE) ;
                m_heldCallFeatureIndicators.put(call.getCallID(), feature);
            }
            else
            {
                feature.refreshDisplayName(call);
            }
        }
    }

    // effectively removes a call from the home screen
    private void onReleasedHeldCall(PCall call)
    {
        if (call != null)
        {
            HeldCallFeatureIndicator feature = getHeldCallFeatureIndicator(call.getCallID());
            if (feature != null)
            {
                m_heldCallFeatureIndicators.remove(call.getCallID());
                FeatureIndicatorManager manager = Shell.getFeatureIndicatorManager();
                manager.removeIndicator(feature);
            }
        }
    }

    private HeldCallFeatureIndicator getHeldCallFeatureIndicator(String callId)
    {
        //return m_heldCallFeatureIndicator;
        return (HeldCallFeatureIndicator)m_heldCallFeatureIndicators.get(callId);
    }

    protected class HeldCallFeatureIndicator implements FeatureIndicator
    {
        private String m_callId;

        private String m_displayName;

        public HeldCallFeatureIndicator(PCall call)
        {
            m_callId = call.getCallID();
            refreshDisplayName(call);
        }

        public void refreshDisplayName(PCall call)
        {
            String displayName;
            PAddress participants[] = call.getParticipants() ;
            if (participants.length > 1) 
            {
                displayName = participants.length + " Party Conference" ;
            } 
            else if (participants.length == 1) 
            {
                // PERFORMANCE ALERT : If this gets called alot
                DefaultSIPAddressRenderer r = new DefaultSIPAddressRenderer();
                displayName = r.getDisplayText(participants[0].getAddress(), 1);
            }
            else
            {
                displayName = "";
            }

            m_displayName = "On Hold:" + displayName;
        }

        public Image getIcon()
        {
            return null ;
        }


        public String getShortDescription()
        {
            return null ;
        }

        public java.awt.Component getComponent()
        {
            PLabel label = new PLabel(m_displayName, PLabel.ALIGN_EAST) ;
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;

            return label ;
        }


        public String getHint()
        {
            return "Take call off hold" ;
        }


        public void buttonPressed()
        {
            PCall call = getCallByCallID(m_callId);
            ShellApp.getInstance().getCoreApp().onReleaseCall(call);
        }

        public boolean isIndicatorInstalled(){
            return getHeldCallFeatureIndicator(m_callId) != null;
        }
    }

    /**
     * @deprecated
     * delegates the call to incoming call feature indicator which
     * removes the call from the incoming ringing calls list.
     */
    public void callNotRinging(PCall call){
        m_incomingCallFeatureIndicator.callNotRinging(call);
    }

    /**
     *  Feature Indicator to show incoming ringing calls.
     */
    protected class IncomingCallFeatureIndicator implements FeatureIndicator
    {
        FeatureIndicatorManager m_manager ;
        Vector vRingingCalls ;

        public IncomingCallFeatureIndicator()
        {
            m_manager = Shell.getFeatureIndicatorManager() ;
            vRingingCalls = new Vector() ;
        }


        public void callRinging(PCall call)
        {
            if (!vRingingCalls.contains(call.getCallID()) && !call.m_bTransferControllerRelated) {
                vRingingCalls.addElement(call.getCallID()) ;
                if (vRingingCalls.size() == 1)
                    m_manager.installIndicator(this, FeatureIndicatorManager.VIEWSTYLE_LINE) ;
                else
                    m_manager.refreshIndicator(this) ;

            }
        }


        public void callNotRinging(PCall call)
        {
            if (vRingingCalls.contains(call.getCallID())) {
                vRingingCalls.removeElement(call.getCallID()) ;
                if (vRingingCalls.size() == 0) {
                    m_manager.removeIndicator(this) ;
                } else {
                    m_manager.refreshIndicator(this) ;
                }
            }
        }


        public Image getIcon()
        {
            return null ;
        }


        public String getShortDescription()
        {
            return null ;
        }


        public java.awt.Component getComponent()
        {
            PLabel label = new PLabel("Ringing Calls: " +
                Integer.toString(vRingingCalls.size()), PLabel.ALIGN_EAST) ;
            label.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE)) ;

            return label ;
        }


        public String getHint()
        {
            return "Show Incoming Calls" ;
        }


        public void buttonPressed()
        {
            ShellApp.getInstance().getCoreApp().doAnswerRingingCall() ;
        }
    }


    /**
     * This class monitors a call and waits for all of it's connections to
     * fail or disconnect. At that point, the big club is pulled out and
     * the call is hammer.
     */
    private class icCallKiller extends PConnectionListenerAdapter
    {
        public void connectionDisconnected(PConnectionEvent event)
        {
            PCall call = event.getCall() ;
            if (call.getConnectionState() == PCall.CONN_STATE_DISCONNECTED) {
                try {
                    call.disconnect() ;
                } catch (PCallStateException e) {
                    SysLog.log(e) ;
                }
            }
        }

        public void connectionFailed(PConnectionEvent event)
        {
            PCall call = event.getCall() ;
            if (call.getConnectionState() == PCall.CONN_STATE_FAILED) {
                try {
                    call.disconnect() ;
                } catch (PCallStateException e) {
                    SysLog.log(e) ;
                }
            }
        }
    }



    /**
     *
     */
    private class icForwardOnNoAnswerHandler implements PActionListener
    {
        private PtCallControlCall   m_call ;
        private int                 m_iForwardAfterSeconds ;
        private String              m_strForwardURL ;

        public icForwardOnNoAnswerHandler(PtCallControlCall call, String strForwardURL, int iForwardAfterSeconds)
        {
            m_call = call ;
            m_strForwardURL = strForwardURL ;
            m_iForwardAfterSeconds = iForwardAfterSeconds ;

            Timer t = Timer.getInstance() ;
            t.addTimer(m_iForwardAfterSeconds*1000, this, null) ;
        }


        public void actionEvent(PActionEvent event)
        {
            Connection connections[] = m_call.getConnections() ;

            // look through all of our connections
            for (int i=0; i<connections.length; i++)
            {
                TerminalConnection termConnections[] = connections[i].getTerminalConnections() ;
                // and each of the terminal connections
                for (int j=0;j<termConnections.length; j++)
                {
                    if (!((PtTerminalConnection) termConnections[j]).isLocal())
                    {
                        if (termConnections[j].getState() == CallControlTerminalConnection.RINGING)
                        {
                            try
                            {
                                PCall stapiCall = getCallByCallID(m_call.getCallID());
                                if( stapiCall != null )
                                    stapiCall.setCallData( REDIRECTED_ADDRESS, m_strForwardURL);
                                ((CallControlConnection) connections[i]).redirect(m_strForwardURL) ;
                            }
                            catch (Exception e)
                            {
                                Shell.getInstance().showUnhandledException(e, false) ;
                            }
                        }
                    }
                }
            }
        }
    }
}
