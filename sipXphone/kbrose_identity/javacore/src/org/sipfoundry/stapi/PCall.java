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

import java.io.IOException ;
import java.io.File ;
import java.io.InputStream ;
import java.io.FileOutputStream ;

import java.net.URL;
import java.net.MalformedURLException;

import java.util.Hashtable ;
import java.util.Date ;
import java.util.Vector ;
import java.util.Enumeration ;
import java.util.Random ;
import java.util.Stack ;
import java.util.Calendar ;
import java.util.EmptyStackException ;

import javax.telephony.callcontrol.* ;
import javax.telephony.* ;

import org.sipfoundry.sip.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.telephony.* ;
import org.sipfoundry.telephony.callcontrol.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.ShellApp ;
import org.sipfoundry.sipxphone.sys.util.PingerInfo ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.Logger ;
import org.sipfoundry.sipxphone.service.Timer ;

import org.sipfoundry.siplite.CallProviderImpl;

/**
 * The PCall class is used by the developer to manipulate calls. If you are
 * familiar with JTAPI, a PCall represents the Call, Connection, and
 * TerminalConnection objects. PCall removes much of the complexity
 * of the JTAPI interfaces, providing for simpler call handling.
 * <p>
 * A call can have a variety of states, such as Connected, Held, and Failed.
 * For example, to dial a number, you would first create a PCall using
 * <i>createCall</i>, then call the <i>connect</i> method on the call.
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PCall
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /*
     * NOTE: The order of the following connection states is important. The order
     *       is used by <i>getConnectionState()</i> to determine what state is 'best'
     *       Please do not change the order without verifying that
     *       <i>getConnectionState()</i> is still doing the right thing.
     */

    /** The call state is unknown. */
    public final static int CONN_STATE_UNKNOWN           = -1 ;
    /** The attempted connection has failed. */
    public final static int CONN_STATE_FAILED            = 0 ;
    /** The connection has been disconnected. */
    public final static int CONN_STATE_DISCONNECTED      = 1 ;
    /** The call has been created but is not yet connected. */
    public final static int CONN_STATE_IDLE              = 2 ;
    /** A connection attempt has been made, but the far end has not yet started ringing. */
    public final static int CONN_STATE_TRYING            = 3 ;
    /** An outgoing call is being made and the far end terminal is ringing. */
    public final static int CONN_STATE_OUTBOUND_ALERTING = 4 ;
    /** There is an incoming call that is ringing. */
    public final static int CONN_STATE_INBOUND_ALERTING  = 5 ;
    /** The connection has been established. */
    public final static int CONN_STATE_CONNECTED         = 6 ;
    /** The call is on hold. */
    public final static int CONN_STATE_HELD              = 7 ;
    /** !!!!!! IMPORTANT !!!!maximum state value. */
    /*  Used for parameter testing  */
    /** should match the last value above                */
    protected final static int CONN_STATE_MAX            = 7 ;


    /** DTMF tone id: 0 key */
    public static final int DTMF_0     = '0' ;
    /** DTMF tone id: 1 key */
    public static final int DTMF_1     = '1' ;
    /** DTMF tone id: 2 key */
    public static final int DTMF_2     = '2' ;
    /** DTMF tone id: 3 key */
    public static final int DTMF_3     = '3' ;
    /** DTMF tone id: 4 key */
    public static final int DTMF_4     = '4' ;
    /** DTMF tone id: 5 key */
    public static final int DTMF_5     = '5' ;
    /** DTMF tone id: 6 key */
    public static final int DTMF_6     = '6' ;
    /** DTMF tone id: 7 key */
    public static final int DTMF_7     = '7' ;
    /** DTMF tone id: 8 key */
    public static final int DTMF_8     = '8' ;
    /** DTMF tone id: 9 key */
    public static final int DTMF_9     = '9' ;
    /** DTMF tone id: star (*) key */
    public static final int DTMF_STAR  = '*' ;
    /** DTMF tone id: pound (#) key */
    public static final int DTMF_POUND = '#' ;
    /** DTMF tone id: A key */
    public static final int DTMF_A     = 'A' ;
    /** DTMF tone id: B key */
    public static final int DTMF_B     = 'B' ;
    /** DTMF tone id: C key */
    public static final int DTMF_C     = 'C' ;
    /** DTMF tone id: D key */
    public static final int DTMF_D     = 'D' ;
    /** DTMF tone id: FLASH key */
    public static final int DTMF_FLASH = 'E' ;  //need to think about it.


    /** Call Creation Hint: A normal outbound call. */
    public static final int CREATE_HINT_NORMAL     = 0x00000000 ;
    /** Call Creation Hint: The call is being created for the purpose of
        a multi-party conference call. */
    public static final int CREATE_HINT_CONFERENCE = 0x00000001 ;


    /** Anonymous Display Names are treated different within the call log,
        this constant is used to check for that anomaly */
    private static final String ANONYMOUS_DISPLAY_NAME =  "Anonymous";

    /** how long before preium sound is disabled */
    private final int DISABLE_PREMIUM_SOUND_SEC = 80 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to JTAPI provider */
    protected Provider              m_provider ;
    /** reference to JTAPI home terminal */
    protected Terminal              m_terminal ;
    /** reference to JTAPI call */
    protected PtCallControlCall     m_call ;
    /** reference to original JTAPI call (in cases of call drop) */
    protected PtCallControlCall     m_callOriginal ;
    /** reference to old obsolute phone state monitor */
    protected PhoneState            m_phoneState ;
    /** reference to old obsolute audio source control */
    protected AudioSourceControl    m_audioSourceControl ;

    /** internal call monitor */
    protected icCallMonitor         m_icCallMonitor ;
    /** list of call listeners*/
    protected Vector                m_vConnectionListenersList ;

    /** list of DTMF listeners with specific originator address*/
    protected Hashtable             m_htDTMFListenersList = new Hashtable();

     /** list of DTMF listeners*/
    protected Vector                m_vDTMFListenersList = new Vector();

    /** date/time the call was connected */
    protected Date                  m_dateConnected = null ;

    /** date/time the call started ringing for incoming calls*/
    protected Date                  m_dateStartedRinging = null ;

    /** date/time the call was placed on hold */
    protected Date                  m_datePlacedOnHold = null ;
    /** total call duration */
    protected int                   m_iCallDuration = 0 ;


    /** Was the call an outbound call? */
    protected boolean               m_bWasOutboundCall ;
    /** Did the call ever make it to established? */
    protected boolean               m_bWasEstablishedCall ;
    /** Are we forcing a drop()? */
    protected boolean               m_bWasDropped ;
    /** Is this call a conference?  A call is considered a conference if
        we ever issue a second connect() upon it */
    protected boolean               m_bIsDropping ;
    protected boolean               m_bIsConference ;

    /** cached list of connection states */
    protected ConnectionStateCache  m_cacheStates ;

    /** the original callee or caller address */
    protected PAddress              m_originalAddress ;
    /** Cache of java sip session (container for from, to, call-id, contact) */
    protected Hashtable             m_htSessionCache ;
    /** hack varable that forces uses to ignore the disconnect() method
        until told otherwise */
    protected boolean               m_bIgnoreExternalDrops ;
    /** The Local call address */
    protected PAddress              m_addressLocal = null ;
    /** the connection state */
    protected int                   m_iLocalState ;

    /** hack to see if this call has been DNDed.  If DNDed we ignore it in
        other parts of the system */
    protected boolean               m_bDNDing ;
    /** a cache of the call id */
    protected String                m_strCallID ;
    /** hash table containing list of other participants so that we can fill
        in the call log properly */

    protected ActionHandler         m_hangupHandler ;

    /** The cached called address (address caller used to dial us if an inbound call */
    protected PAddress              m_addressCacheCalled ;

    /**
     * This is a hack so that we know this call is a transfer and displays
     * the proper state. This should be removed once we add support for
     * meta events.  This flag is set for the transferee's new call.
     *
     * @deprecated Hide
     *
     */
    public boolean                  m_bTransfereeRelated ;

    /**
     * This is another set of hack state variable that tells us that this call
     * is part of a transfer and that we are indeed the transfer controller.
     *
     * The OriginalCall and MonitorCall flags are set if the call was part of
     * a transfer (on the transfer controller side).
     *
     * @deprecated Hide
     */
    public boolean                  m_bTransferControllerRelated ;

    /**
     * @deprecated Hide
     */
    public boolean                  m_bTransferControllerOriginalCall ;

    /**
     * @deprecated Hide
     */
    public boolean                  m_bTransferControllerMonitorCall ;

    protected boolean               m_bAutoDrop = true ;
    protected Object                m_objHold = new Object() ;



    /** class responsble for disabling sound for a demo copy of ixpressa */
    private icPremiumSoundDisabler m_premiumSoundDisabler ;
    /** has premium sound already been disabled */
    private boolean m_bDisabledPremiumSound ;

    /**
     * has the internal listener to listnen to dtmf tones been added?
     */
    private boolean m_bInternalDtmfListenerAdded = false;

    /** Are we recording this call? */
    protected boolean              m_bWasRecording ;

    /** boolean to enable call state debugging as well as print other debug
     *  information.
     */
    protected static boolean m_sCallDebug = false ;

    /** "Music on Hold" player that plays music when a call is on hold
     *  if the feature is enabled.
     */
    protected PMediaPlayer m_musicOnHoldPlayer;

    protected boolean m_bMusicOnHoldPlaying = false;



//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Protected constructor taking JTAPI call- Construction is restricted to
     * <i>createCall</i> method or classes residing within the stapi package.
     */
    protected PCall(Call call, boolean bNewCall)
    {
        String strJavaCallDebug =
           PingerConfig.getInstance().getValue(PingerConfig.JAVA_CALL_DEBUG) ;
        if( strJavaCallDebug != null && strJavaCallDebug.equalsIgnoreCase("enable"))
        {
          m_sCallDebug = true;
        }

        setLocalConnectionState(PCall.CONN_STATE_UNKNOWN) ;
        m_premiumSoundDisabler = null ;
        m_hangupHandler = null ;
        m_provider = ShellApp.getInstance().getProvider() ;
        m_terminal = ShellApp.getInstance().getTerminal() ;
        m_cacheStates = new ConnectionStateCache() ;
        m_audioSourceControl = ShellApp.getInstance().getAudioSourceControl() ;
        m_bIgnoreExternalDrops = false ;
        m_bTransfereeRelated = false ;
        m_bTransferControllerRelated = false ;
        m_bDNDing = false ;
        m_bWasRecording = false;

        m_phoneState = PhoneState.getInstance() ;
        m_bWasOutboundCall = false ;
        m_bWasEstablishedCall = false ;
        m_bWasDropped = false ;
        m_bIsConference = false ;
        m_bIsDropping = false ;

        m_bTransferControllerOriginalCall = false ;
        m_bTransferControllerMonitorCall = false ;
        m_htSessionCache = new Hashtable() ;

        m_callOriginal = m_call = (PtCallControlCall) call ;
        m_strCallID = m_call.getCallID() ;

        m_originalAddress = getLocalAddress() ;

        STAPIFactory.getInstance().getCallManager().registerCall(this) ;
        attachLocalCallMonitor() ;
        if( m_sCallDebug )
          addConnectionListener(new icConnectionStateDebugger()) ;

        // OPTIMIZATION: Do not refresh the connection states unless needed
        if (!bNewCall)
            refreshConnectionStates() ;

        m_musicOnHoldPlayer = null;

    }


    /**
     * Create a new call. The new call is placed in the CONN_STATE_IDLE
     * state until the <i>connect</i> method is called. Once the <i>connect</i>
     * method is invoked, the state of the call is advanced to CONN_STATE_TRYING,
     * etc.
     *
     * @return A new PCall object.
     *
     * @exception SecurityException Thrown when a user/application does not have
     *            permission to create a call.
     *
     * @exception PCallResourceUnavailableException Thrown if a call cannot be
     *            created. This may occur if the phone has reached the maximum
     *            number of remote connections.
     */
    public static PCall createCall()
        throws SecurityException, PCallResourceUnavailableException
    {
        return PCall.createCall(CREATE_HINT_NORMAL) ;
    }

    /**
     * Create a new call. The new call is placed in the CONN_STATE_IDLE
     * state until the <i>connect</i> method is called. Once the <i>connect</i>
     * method is invoked, the state of the call is advanced to CONN_STATE_TRYING,
     * etc.
     *
     * @param creationHint Defines the intended purpose of the call. In most
     *        cases CREATE_HINT_NORMAL should be used. If a call is created
     *        for the purpose of local conferencing, specifying the
     *        CREATE_HINT_CONFERENCE hint may reduce protocol signally later
     *        in the call life cycle.
     *
     * @return A new PCall object.
     *
     * @exception SecurityException Thrown when a user/application does not have
     *            permission to create a call.
     *
     * @exception PCallResourceUnavailableException Thrown if a call cannot be
     *            created. This may occur if the phone has reached the maximum
     *            number of remote connections.
     */
    public static PCall createCall(int creationHint)
        throws SecurityException, PCallResourceUnavailableException
    {
        PCall call = Shell.getCallManager().createCall() ;
        switch (creationHint)
        {
            case CREATE_HINT_NORMAL:
                break ;
            case CREATE_HINT_CONFERENCE:
                if (Shell.getXpressaSettings().getProductID() ==
                        XpressaSettings.PRODUCT_XPRESSA)
                {
                    // Limit the call to low-cpu codecs
                    ((PtCallControlCall) call.getJTAPICall()).setCodecCPULimit(
                            PtCallControlCall.CODEC_CPU_LOW, false) ;
                }
                break ;
        }

        return call ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

   /**
     * Add a DTMF listener to this PCall. After it is added, the listener
     * will receive notification of all incoming DTMF tone events in this call.
     *
     * @param listener The DTMF listener that will receive notifications.
     * @see PDTMFListener
    */
   public void addDTMFListener(PDTMFListener listener ){

        synchronized (m_vDTMFListenersList) {
            if (!m_vDTMFListenersList.contains(listener)) {
                m_vDTMFListenersList.addElement(listener) ;
            }
        }
   }

   /**
     * Add a DTMF listener to this PCall for tones originating from a
     * specified address. After  the listener is added, it
     * will receive notification of all the
     * incoming DTMF tones from the specified address in this call.
     *
     * @param listener The DTMF listener that will receive notifications from
     *                 the specified address.
     * @param address The originating address of the DTMF tones.
     * @see PDTMFListener
    */
   public void addDTMFListener(PDTMFListener listener, PAddress address ){

        synchronized (m_htDTMFListenersList) {
            if (!m_htDTMFListenersList.contains(listener)) {
                m_htDTMFListenersList.put(listener, address) ;
            }
        }
   }

    /**
     * Remove a DTMF listener from this call.
     *
     * @param listener The DTMF listener that no longer needs notifications from
     *                 this call.
     *
     * @see PDTMFListener
     */
   public void removeDTMFListener(PDTMFListener listener){
      if (m_vDTMFListenersList != null) {
            synchronized (m_vDTMFListenersList) {
                m_vDTMFListenersList.removeElement(listener) ;
            }
      }
      if (m_htDTMFListenersList != null) {
            synchronized (m_htDTMFListenersList) {
                m_htDTMFListenersList.remove(listener) ;
            }
      }
   }


        private boolean isValidDtmf(int toneId)
        {
            return (toneId >= DTMF_0 && toneId <= PCall.DTMF_FLASH);
        }


        private void postDtmfDown(icToneDescriptor toneDesc)
        {
            if (isValidDtmf(toneDesc.iToneID))
            {
                // TODO: if remote, which of the connected calls did it come from
                toneDesc.dtmfSource = toneDesc.bRemote ? null : getLocalAddress();

                postDTMFEvent(new PDTMFEvent(toneDesc.iToneID,
                    PDTMFEvent.TYPE_BUTTON_DOWN, 0, toneDesc.dtmfSource));
                toneDesc.lStartTime = System.currentTimeMillis();
            }
        }

        private void postDtmfUp(icToneDescriptor toneDesc)
        {
            if (isValidDtmf(toneDesc.iToneID))
            {
                int duration = (int)(System.currentTimeMillis() - toneDesc.lStartTime);
                postDTMFEvent(new PDTMFEvent(toneDesc.iToneID,
                    PDTMFEvent.TYPE_BUTTON_UP, duration, toneDesc.dtmfSource));
            }
        }


        /**
    * posts the DTMFEvenet, basically fires up the PDTMFListeners.
    * @deprecated DO NOT EXPOSE
    */
   public void postDTMFEvent(PDTMFEvent event){
       for (Enumeration e = m_vDTMFListenersList.elements() ; e.hasMoreElements() ;) {
            PDTMFListener listener = (PDTMFListener) e.nextElement() ;
            if (listener != null) {
                if( event.getType() == PDTMFEvent.TYPE_BUTTON_DOWN )
                    listener.buttonDown( event );
                else
                    listener.buttonUp( event );
            }
        }
        for (Enumeration e = m_htDTMFListenersList.keys() ; e.hasMoreElements() ;) {
            PDTMFListener listener = (PDTMFListener) e.nextElement() ;
            if (listener != null) {
                if( event.getAddress() != null ){
                    if( ((PAddress)m_htDTMFListenersList.get(listener)) .equals(event.getAddress()) ){
                        if( event.getType() == PDTMFEvent.TYPE_BUTTON_DOWN )
                            listener.buttonDown( event );
                        else
                            listener.buttonUp( event );
                    }
                }
            }
        }
   }

    /**
     * Dial a phone number. This method is used to connect the phone to
     * the specified address (phone number).
     * <p>
     * This method can be called multiple times if a conference if desired.
     *
     * @param PAddress The phone number to call.
     *
     * @exception SecurityException Thrown when an user/application does
     *            not have permissions to call the number.
     *
     * @exception PCallResourceUnavailableException Thrown if the connection
     *            cannot be created. This may occur if the phone has reached
     *            the maximum number of connections.
     *
     * @exception PCallAddressException Thrown if the specified address is
     *            invalid or unsupported.
     */
    public void connect(PAddress remoteAddress)
        throws SecurityException,
               PCallResourceUnavailableException,
               PCallAddressException
    {
        connect( null, remoteAddress );
    }

    /**
     * Dial a phone number. This method is used to connect the phone to
     * the specified remote address (phone number) using the local address
     * as outbound line. If localAddress is null, then the default outbound line
     * is used.
     * <p>
     * This method can be called multiple times if a conference if desired.
     *
     * @param PAddress The phone number to call.
     *
     * @exception SecurityException Thrown when an user/application does
     *            not have permissions to call the number.
     *
     * @exception PCallResourceUnavailableException Thrown if the connection
     *            cannot be created. This may occur if the phone has reached
     *            the maximum number of connections.
     *
     * @exception PCallAddressException Thrown if the specified address is
     *            invalid or unsupported.
     * @deprecated DO NOT EXPOSE.
     */

    public void connect(PAddress localAddress, PAddress remoteAddress)
        throws SecurityException,
               PCallResourceUnavailableException,
               PCallAddressException
    {
        PingerInfo pingerInfo = PingerInfo.getInstance() ;
        PAddress originalAddress = remoteAddress ;

        // Validate Parameter
        if (remoteAddress == null)
            throw new PCallAddressException("null address") ;

        // Validate State
        if (m_call == null)
            throw new PCallResourceUnavailableException("call unavailable") ;

        // Log the request
        if (Logger.isEnabled())
        {
            Logger.post("stapi",
                    Logger.TRAIL_COMMAND,
                    "connect",
                    SipParser.stripParameters(remoteAddress.getAddress())) ;
        }

        // Note if this is now a conference
        if (getConnectionState() != CONN_STATE_IDLE)
        {
            m_bIsConference = true ;
            stopMusicOnHold();
        }

        // Disable any tones/sounds.
        try
        {
            stopTone() ;
            stopSound() ;
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }

        // Determine if this address has wait or pauses and will require
        // second stage dialing.
        String strMassageAddress = remoteAddress.getAddress() ;
        if (PhoneNumberScrubber.looksLikePhoneNumber(strMassageAddress))
        {
            PhoneNumberParser parser = new PhoneNumberParser(strMassageAddress) ;
            if (parser.hasMoreActions())
            {
                new SecondStageDialer(this, parser) ;
                strMassageAddress = parser.getNumber() ;
                remoteAddress = PAddressFactory.getInstance().createAddress(strMassageAddress) ;
            }
        }

         // Record last called address.
        setCallData("last_called", originalAddress.getAddress()) ;


        // Add a conference participant
        try
        {
            // Prime Call State
            setState(PCall.CONN_STATE_TRYING, remoteAddress, PConnectionEvent.CAUSE_NORMAL, false, false) ;
            if (m_bIsConference)
            {
                if (!isConferenceLimitReached())
                {
                    // Add conference participant
                    synchronized (m_call)
                    {
                        m_call.addParty(remoteAddress.getAddress()) ;
                        m_originalAddress = remoteAddress ;
                    }
                }
                else
                {
                    throw new PCallResourceUnavailableException("max conference reached") ;
                }
            }
            else
            {
                m_originalAddress = remoteAddress ;
                synchronized (m_call)
                {
                    // strOutboundIdentity is your outbound line address you
                    // are using to make this call.  If it is null, the default
                    // outbound line is used.

                    Address outboundLineAddress = null;
                    if( localAddress != null)
                    {
                        outboundLineAddress =
                            m_provider.getAddress(localAddress.getAddress());
                        if (    (outboundLineAddress.getName() == null) ||
                                (outboundLineAddress.getName().length() == 0) )
                        {
                            outboundLineAddress = null ;
                        }

                        if( outboundLineAddress != null )
                        {
                            m_addressLocal = localAddress;
                        }
                    }

                    if( outboundLineAddress == null)
                    {
                        //try to get the default line.
                        SipLine defaultSipLine  =
                            SipLineManager.getInstance().getDefaultLine();
                        if( defaultSipLine != null )
                        {
                            outboundLineAddress =
                                m_provider.getAddress(defaultSipLine.getIdentity());
                        }

                        if( outboundLineAddress == null )
                        {
                            Address addresses[] = m_terminal.getAddresses() ;
                            outboundLineAddress = addresses[0];
                        }
                    }

                    m_call.connect(m_terminal, outboundLineAddress, remoteAddress.getAddress()) ;
                }
            }
        }
        catch (InvalidStateException ise)
        {
            throw new PCallAddressException(
                    AppResourceManager.getInstance().getSystemString("lblErrorNoDirectoryServer")) ;
        }
        catch (ResourceUnavailableException e)
        {
            throw new PCallResourceUnavailableException("max simultaneous connections reached") ;
        }
        catch (IllegalStateException e)
        {
            throw e ;
        }
        catch (PCallResourceUnavailableException rua)
        {
            throw rua ;
        }
        catch (Exception e)
        {
            SysLog.log(e) ;
        }

    }


    /**
     * Returns the last address dialed by the phone user for this call. This address is the
     * dial string as entered by the user (or by an application) before any post
     * processing. Examples of post processing include digitmap expansion, ENUM look
     * ups, converting digits to an SIP Address, etc.  In addition to the last party
     * called, this method will return the last party added to a conference call or the
     * address of a party to whom a call was transferred.
     *
     * @return The last called address or null if no outbound calls or transfers have
     *         been placed.
     */
    public String getLastCalledAddress()
    {
        String strLastCalled = null ;

        strLastCalled = (String) getCallData("last_called") ;

        return strLastCalled ;
    }


    /**
     * Returns the address called if this was an inbound call. Otherwise, false.
     *
     * @deprecated do not expose
     */
    public PAddress getCalledAddress()
    {
        if (m_addressCacheCalled == null)
        {
            SipSession session = (SipSession) getSession(getLocalAddress()) ;

            if (session != null)
            {
                try
                {
                    m_addressCacheCalled = PAddressFactory.getInstance().createAddress(session.getFromURL()) ;
                }
                catch (Exception e)
                {
                    SysLog.log(e) ;
                }
            }
            else
            {
                System.out.println("JERR: Unable to find local session?") ;
            }
        }

        return m_addressCacheCalled ;
    }


    /**
     * Hang up on all participants in the call.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to hang up the call.
     *
     * @exception PCallStateException Thrown if the call has already been
     *            disconnected.
     */
    public void disconnect()
        throws SecurityException, PCallStateException
    {
        if ((m_hangupHandler == null) || (!m_hangupHandler.performAction("ACTION_HANGUP", this, null)))
        {
            // Get the Listener list, address, and duration before we drop the
            // call or make any changes.  Once we drop the call, none of the
            // the member variables can be trusted.
            PConnectionListener listeners[] = getConnectionListeners() ;
            int      iNumListeners = listeners.length ;
            PAddress participants[] = getParticipants() ;
            PAddress originalAddress = m_originalAddress ;
            int      iDuration = getDuration() ;


            if (m_bIgnoreExternalDrops)
            {
                System.out.println("HACK: Placing call on hold instead of disconnecting!") ;
                placeOnHold() ;
                return ;
            }

            if (m_bIsDropping == false)
                m_bIsDropping = true ;
            else
                return ;

            if ((m_call != null) && !m_bWasDropped)
            {
                // Disable Audio device if we are in focus
                PCall call_in_focus = Shell.getCallManager().getInFocusCall();
                if (call_in_focus == null || call_in_focus == this)
                {
                    try
                    {
                        Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
                    }
                    catch (PMediaLockedException e) { }
                }


                // Make sure we stop all tones (i.e. ring tone)
                stopTone() ;
                stopSound() ;

                // Force a calculation of call duration
                getDuration() ;

                //if we were recording then signal it to stop.
                if (m_bWasRecording)
                    stopRecording(m_strCallID);

                //destry the music_on_hold player if it was ever created.
                if( m_musicOnHoldPlayer != null )
                {
                    try{
                        m_musicOnHoldPlayer.close();
                        m_musicOnHoldPlayer = null;
                    }catch(Exception e )
                    {
                        e.printStackTrace();
                    }
                }

                if (Logger.isEnabled())
                {
                    Logger.post("stapi", Logger.TRAIL_COMMAND, "drop") ;
                }

                try
                {
                    synchronized (m_call)
                    {
                        m_call.drop() ;
                    }
                }
                catch (Exception e)
                {
                    System.out.println("Error droping call: " + e.toString()) ;
                    SysLog.log(e);
                }


                /*
                 * After this point (calling m_call.drop), the call can be
                 * invalidated at any point.  You can no longer rely on any
                 * global variables.
                 */

                // Ignore the call to stop the core application from displaying
                // a connection disconnected status form if we don't have any
                // active participants (meaning the other side has dropped)
                // Shell.getCallManager().ignoreCall(this) ;

                // ShellApp.getInstance().getCoreApp().ignoreCallByID(getCallID(), false) ;

                for (int i=0; i<participants.length; i++)
                {
                    PConnectionEvent event = new PConnectionEvent(
                            this,
                            participants[i],
                            PtConnectionEvent.CONNECTION_DISCONNECTED) ;

                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "connectionDisconnected",
                                SipParser.stripParameters(event.getAddress()),
                                event.getCall().getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    for (int j=0; j<iNumListeners;j++)
                    {
                        try
                        {
                            listeners[j].connectionDisconnected(event) ;
                        }
                        catch (Exception e)
                        {
                            System.out.println("Uncaught exception from listener: " + e.toString()) ;
                            SysLog.log(e);
                        }
                    }
                }

                m_bIsDropping = false ;
                m_bWasDropped = true ;

                PConnectionEvent event = new PConnectionEvent(
                        this,
                        originalAddress,
                        PtConnectionEvent.CONNECTION_DISCONNECTED) ;

                if (Logger.isEnabled())
                {
                    Logger.post("stapi",
                            Logger.TRAIL_NOTIFICATION,
                            "callDestroyed",
                            SipParser.stripParameters(event.getAddress()),
                            event.getCall().getCallID(),
                            String.valueOf(event.getCause())) ;
                }

                for (int i=0; i<iNumListeners;i++)
                {
                    try
                    {
                        listeners[i].callDestroyed(event) ;
                    }
                    catch (Exception e)
                    {
                        System.out.println("Uncaught exception from listener: " + e.toString()) ;
                        SysLog.log(e);
                    }
                }

                // Don't bother polling if the call is null or duration is zero
                // The *ONLY* way the m_call becomes null is if we receive a
                // callInvalid event.  After which, the call is invalid and we
                // cannot make and requests upon it.
                PtCall droppedCall = m_call ;
                if (droppedCall != null && iDuration > 0)
                {
                    DropCallStateChangePoller poller = new DropCallStateChangePoller(droppedCall) ;
                    try
                    {
                        poller.beginPolling() ;
                        if (!poller.wasSuccessful())
                        {
                            throw new PCallStateException("Unable to drop call") ;
                        }
                    }
                    finally
                    {
                        poller.cleanup() ;
                    }
                }
            }
        }
    }


    /**
     * Hang up one participant in a conference call,
     * leaving the other conference call
     * participants connected.
     *
     * @param address The {@link PAddress} of the participant you wish to drop.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to drop this call.
     * @exception PCallStateException Thrown if the participant is not
     *            a member of this call, or if the call has already been disconnected.
     */
    public void disconnect(PAddress address)
        throws SecurityException, PCallStateException
    {
        // Find out address
        //Thread.dumpStack();
        Connection connection = findConnection(address) ;
        if (connection != null) {
            try {
                if (Logger.isEnabled())
                {
                    Logger.post("stapi",
                            Logger.TRAIL_COMMAND,
                            "disconnect",
                            SipParser.stripParameters(address.getAddress())) ;
                }
                connection.disconnect() ;
            } catch (Exception e) {
                SysLog.log(e) ;
            }
        } else {
            System.out.println("Trying to disconnect an unknown address: " + address) ;
        }
    }


    /**
     * Answer an incoming call. If multiple incoming calls are ringing,
     * one of the incoming calls is answered (picked at random).
     * This method should only be invoked as a response to a connectionInboundAlerting
     * event.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to answer a call.
     *
     * @exception PCallStateException Thrown if the call is not an incoming
     *            call.
     */
    public void answer()
        throws SecurityException, PCallStateException
    {
        if (m_call != null) {
            PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
            if (tcLocal != null) {
                try {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_COMMAND,
                                "answer") ;
                    }
                    tcLocal.answer() ;
                } catch (Exception e) {
                    Shell.getInstance().showMessageBox(MessageBox.TYPE_ERROR, "Error", "Unable to answer call: " + e.toString(), null) ;
                }
            } else {
                Shell.getInstance().showMessageBox(MessageBox.TYPE_ERROR, "Error", "Unable to answer call: cannot get terminal connection", null) ;
            }
        }
    }


    /**
     * Place all of the participants of a call on hold.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to place a call on hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or already on hold.
     */
    public void placeOnHold()
        throws SecurityException, PCallStateException{

        // To enable PLayMusicOnHold, only the local address should be on
        // hold. This is done by calling placeOnHold(null).

        if (m_bIsConference)
        {
            // Bridge the conference
            placeOnHold(null);
        }
        else
        {
            if (isHoldMusicCapable())
            {
                 // Limit the call to low-cpu codecs
                 m_call.setCodecCPULimit(
                        PtCallControlCall.CODEC_CPU_LOW, false) ;

                placeOnHold(null);

                if (m_musicOnHoldPlayer == null)
                {
                    initMusicOnHold();
                }

                if( m_musicOnHoldPlayer != null )
                {
                    playMusicOnHold();
                }
            }
            else
            {
                placeAllParticipantsOnHold();
            }
        }


    }


    /**
     * Place a single participant on hold without placing the entire call
     * on hold. Useful, for example, in conference calls.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to place a call on hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or already on hold.
     */
    public  void placeOnHold(PAddress address)
        throws SecurityException, PCallStateException
    {

        synchronized (m_objHold)
        {
            if (m_call != null)
            {
                // If the address is null, place the local call on hold.
                if (address == null)
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi", Logger.TRAIL_COMMAND, "placeOnHold") ;
                    }

                    synchronized (m_call)
                    {
                        m_call.hold(true) ;
                        // No need to poll for state here, local hold is instant.
                    }
                }
                else
                {
                    Connection connection = findConnection(address) ;
                    if (connection != null)
                    {
                        HeldConnectionStateChangePoller poller =
                                new HeldConnectionStateChangePoller(m_call, address, getSession(address)) ;
                        try
                        {
                            if (Logger.isEnabled())
                            {
                                Logger.post("stapi",
                                        Logger.TRAIL_COMMAND,
                                        "placeOnHold",
                                        SipParser.stripParameters(address.getAddress())) ;
                            }

                            synchronized (m_call)
                            {
                                try
                                {
                                    ((PtCallControlConnection) connection).hold() ;

                                }
                                catch (Exception e)
                                {
                                    throw new PCallStateException("Unexpected exception: " + e.toString()) ;
                                }
                            }

                            poller.beginPolling() ;
                            if (!poller.wasSuccessful())
                            {
                                throw new PCallStateException("Unable to place connection on hold") ;
                            }
                            else
                            {
                                m_cacheStates.setHeldRemote(address, true) ;
                            }
                        }
                        finally
                        {
                            poller.cleanup() ;
                        }
                    } else {
                        throw new PCallStateException("Unable to to find connection") ;
                    }
                }
            }
        }
    }

    /**
     * Take all of the participants in the call off hold.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to release this call from hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or no connections are on hold.
     */
    public void releaseFromHold()
        throws SecurityException, PCallStateException
    {
        /**
         * If the music on hold is playing, that means only the local
         * address was put on hold, so just release the local address from
         * hold. This is done by calling releaseFromHold(null);
         */
        if( m_bMusicOnHoldPlaying ){
            releaseFromHold(null);
            stopMusicOnHold();
        }else{
            releaseAllParticipantsFromHold();
        }
    }

    /**
     * Take all of the participants in the call off hold.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to release this call from hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or no connections are on hold.
     * @deprecated DO NOT EXPOSE
     */
    public void releaseAllParticipantsFromHold()
        throws SecurityException, PCallStateException
    {
        synchronized (m_objHold)
        {
            if (m_call != null)
            {
                ReleaseCallStateChangePoller poller = new ReleaseCallStateChangePoller(m_call, getSessions()) ;
                try
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi", Logger.TRAIL_COMMAND, "releaseFromHold") ;
                    }

                    synchronized (m_call)
                    {
                        m_call.unhold(m_bIsConference) ;
                    }

                    // If this is not a conference, then block until the state change
                    if (!m_bIsConference)
                    {
                        poller.beginPolling() ;
                        if (!poller.wasSuccessful())
                        {
                            throw new PCallStateException("Unable to take call off hold") ;
                        }
                    }
                }
                finally
                {
                    poller.cleanup() ;
                }
            }
        }
    }


    /**
     * Release a single participant from hold without releasing any other
     * participants.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to place a call on hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or already on hold.
     */
    public void releaseFromHold(PAddress address)
        throws SecurityException, PCallStateException
    {
        //debug("releaseFromHold with Adderess called ");
        synchronized (m_objHold)
        {
            //if address is null, release the local address from hold.
            if (address == null)
            {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi", Logger.TRAIL_COMMAND, "releaseFromHold") ;
                    }

                    synchronized (m_call)
                    {
                        m_call.unhold(true) ;
                        // No need to poll for state here, local unhold is instant.
                    }
            }else
            {
                // Find out address
                Connection connection = findConnection(address) ;
                if (connection != null) {
                    ReleaseConnectionStateChangePoller poller =
                            new ReleaseConnectionStateChangePoller(m_call, address, getSession(address)) ;

                    try
                    {
                        if (Logger.isEnabled())
                        {
                            Logger.post("stapi",
                                    Logger.TRAIL_COMMAND,
                                    "releaseFromHold",
                                    SipParser.stripParameters(address.getAddress())) ;
                        }

                        try
                        {
                            ((PtCallControlConnection) connection).unhold() ;
                        }
                        catch (Exception e)
                        {
                            throw new PCallStateException("Unexpected exception: " + e.toString()) ;
                        }

                        poller.beginPolling() ;
                        if (!poller.wasSuccessful())
                        {
                            throw new PCallStateException("Unable to take call off hold") ;
                        }
                        else
                        {
                            m_cacheStates.setHeldRemote(address, false) ;
                        }
                    }
                    finally
                    {
                        poller.cleanup() ;
                    }
                }
                else
                {
                    throw new PCallStateException("Unable to to find connection") ;
                }
            }
        }
    }



    /**
     * Transfer the participants of this call to another party.
     *
     * @param address The phone number of the party to whom the call
     *                should be tranferred.
     *
     * @exception SecurityException Thrown when a user or application does
     *            not have permission to transfer the call.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or not on hold.
     *
     * @exception PCallAddressException Thrown if the specified address is
     *            invalid or unsupported.
     *
     * @exception PCallNotSupportedException Thrown if the transferee or
     *            transfer target does not support transfer.
     *
     * @exception MalformedURLException Thrown if the phone number (URL) is
     *            not valid.
     */
    public PCall transfer(PAddress address)
        throws SecurityException,
               PCallStateException,
               MalformedURLException,
               PCallAddressException,
               PCallNotSupportedException
    {
        PCall callRC = null ;

        int iState = getConnectionState() ;
        if ((iState != CONN_STATE_DISCONNECTED) && (iState != CONN_STATE_FAILED))
        {
            try
            {
                if (m_call != null)
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                    Logger.TRAIL_COMMAND,
                                    "transfer",
                                    SipParser.stripParameters(address.getAddress())) ;
                    }

                    // Record last called address.
                    if (address != null)
                        setCallData("last_called", address.getAddress()) ;

                    synchronized (m_call)
                    {
                        Connection connection =
                            m_call.transfer(address.getAddress()) ;

                        if (connection != null)
                        {
                            Call call = connection.getCall() ;
                            String strNewCallId = ((PtCall)call).getCallID() ;

                            callRC = Shell.getCallManager().getCallByCallID(strNewCallId) ;
                            if (callRC == null)
                                callRC = new PCall(call, false) ;
                        }
                    }
                }
            } catch (InvalidPartyException ipe) {
                throw new PCallAddressException(
                        AppResourceManager.getInstance().getSystemString("lblErrorInvalidAddress")) ;
            } catch (InvalidStateException ise) {
                throw new PCallAddressException(
                        AppResourceManager.getInstance().getSystemString("lblErrorNoDirectoryServer")) ;
            } catch (IllegalStateException e) {
                throw e ;
            } catch (Exception e) {
                SysLog.log(e) ;
            }
        }
        return callRC ;
    }


    /**
     * @deprecated Do not expose
     */
    public void transfer(PCall call)
        throws PCallStateException
    {
        // Only continue if we have a valid connection state
        int iState = getConnectionState() ;
        if ((iState != CONN_STATE_DISCONNECTED) && (iState != CONN_STATE_FAILED))
        {
            try
            {
                synchronized (m_call)
                {
                    m_call.transfer(call.getJTAPICall()) ;
                }

                // Prime the call state
                PAddress addresses[] = call.getParticipants() ;
                if ((addresses != null) && (addresses.length > 0))
                {
                    setState(PCall.CONN_STATE_TRYING, addresses[0], PConnectionEvent.CAUSE_NORMAL, false, false) ;
                }

            }
            catch (Exception e)
            {
                throw new PCallStateException("transfer failed") ;
            }
        }
    }


    /**
     * @deprecated Do not expose
     */
    public void consult(PCall callTransferee, PAddress address)
        throws SecurityException,
            PCallStateException,
            MalformedURLException,
            PCallAddressException,
            PCallNotSupportedException
    {
        // Only continue if we have a valid connection state
        int iState = getConnectionState() ;
        if ((iState != CONN_STATE_DISCONNECTED) && (iState != CONN_STATE_FAILED))
        {
            try
            {
                if (m_call != null)
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                            Logger.TRAIL_COMMAND,
                            "consult",
                            SipParser.stripParameters(address.getAddress())) ;
                    }

                    // Prime the call state
                    setState(PCall.CONN_STATE_TRYING, address, PConnectionEvent.CAUSE_NORMAL, false, false) ;


                    // Record last called address.
                    if (address != null)
                        setCallData("last_called", address.getAddress()) ;

                    synchronized (m_call)
                    {
                        PtCall callJTAPI = (PtCall) callTransferee.getJTAPICall() ;
                        PtTerminalConnection tcLocal = (PtTerminalConnection) callJTAPI.getLocalTerminalConnection() ;

                        // Consult
                        m_call.consult(tcLocal, address.getAddress()) ;

                        // Force into focus
                        m_call.unhold(true) ;
                    }
                }
            }
            catch (InvalidPartyException ipe)
            {
                throw new PCallAddressException(
                        AppResourceManager.getInstance().getSystemString("lblErrorInvalidAddress")) ;
            }
            catch (InvalidStateException ise)
            {
                throw new PCallAddressException(
                        AppResourceManager.getInstance().getSystemString("lblErrorNoDirectoryServer")) ;
            }
            catch (IllegalStateException e)
            {
                throw e ;
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }
    }



    /**
     * Return the local address.  If this was originally an outbound called,
     * then this address represents the called-as address.  If this call was
     * originally an inbound call, then this address represents the called
     * address.
     *
     * @return The PAddress representing the local connection of this call.
     *
     * @deprecated Hide
     */
    public PAddress getLocalAddress()
    {
        PAddress address = null ;

        if ((!m_bWasDropped) && (m_call != null))
        {
            if (m_addressLocal != null)
            {
                address = m_addressLocal ;
            }
            else
            {
                synchronized (m_call)
                {
                    Connection connections[] = m_call.getConnections() ;
                    if (connections.length > 0)
                    {
                        Address addr = connections[0].getAddress() ;
                        String strAddress = addr.getName() ;
                        if ((strAddress != null) && (strAddress.length() > 0))
                        {
                            try
                            {
                                m_addressLocal = PAddressFactory.getInstance().createAddress(strAddress) ;
                                address = m_addressLocal ;
                            }
                            catch  (PCallAddressException e)
                            {
                                System.out.println("Invalid Address: " + strAddress) ;
                                SysLog.log(e) ;
                            }
                        }
                    }
                }
            }
        }
        return address ;
    }


    /**
     * Return the phone numbers of the participants in this call.
     *
     * @return The PAddress[] array of the phone numbers of the people
     * participating in the call. This array does not include the phone
     * number of this phone.
     */
    public PAddress[] getParticipants()
    {
        Vector vParticipants = new Vector() ;
        PAddress[] rc ;

        if ((!m_bWasDropped) && (m_call != null))
        {

            /*
             * Step 1: Build a list of all the non-local addresses and stuff
             *         them into a vector
             */
            Enumeration enum = m_cacheStates.getAddresses() ;
            while (enum.hasMoreElements())
            {
                PAddress address = (PAddress) enum.nextElement() ;
                int iConnectionState = m_cacheStates.getState(address) ;
                if ((iConnectionState != CONN_STATE_FAILED) && (iConnectionState != CONN_STATE_DISCONNECTED))
                {
                    vParticipants.addElement(address) ;
                }
            }
        }

        /*
         * Step 2: Massage our data into an array of PAddresses
         */
        rc = new PAddress[vParticipants.size()] ;
        for (int i=0; i<vParticipants.size(); i++) {
            rc[i] = (PAddress) vParticipants.elementAt(i) ;
        }


        return rc ;
    }


    /**
     * Return the phone numbers of the participants in this call.
     *
     * @return The PAddress[] array of the phone numbers of the people
     * participating in the call. This array does not include the phone
     * number of this phone.
     *
     * @deprecated do not expose
     */
    public PAddress[] getParticipantsUncached()
    {
        PAddress rc[] = null ;
        Vector vParticipants = new Vector () ;

        if ((!m_bWasDropped) && (m_call != null)) {

            /*
             *Step 1: Build a list of all the non-local addresses and stuff them
             *         into a vector
             */

            synchronized (m_call) {
                Connection connections[] = m_call.getConnections() ;

                // Record the local address
                if (connections.length > 0)
                {
                    try
                    {
                        PAddress addressLocal = PAddressFactory.getInstance().createAddress(connections[0].getAddress().getName()) ;
                        if (m_addressLocal == null)
                        {
                            m_addressLocal = addressLocal ;
                        }
                    }
                    catch (PCallAddressException e)
                    {
                        e.printStackTrace() ;
                    }
                }

                // Traverse other participants
                for (int i=1; i<connections.length; i++)
                {
                    // Make sure the address is valid
                    Address addr = connections[i].getAddress() ;
                    boolean bLocalConn = ((PtConnection)connections[i]).isLocal() ;
                    if (addr != null)
                    {
                        String strAddress = addr.getName() ;
                        if ((strAddress != null) && (strAddress.length() > 0))
                        {
                            try
                            {
                                PAddress address = PAddressFactory.getInstance().createAddress(strAddress) ;

                                if (!address.equals(getLocalAddress()))
                                    vParticipants.addElement(address) ;

                                // Update the connection state cache while we are here...
                                int iState = mapJTAPIStateToPTAPIState(connections[i].getState(), bLocalConn) ;
                                if ((iState != CONN_STATE_HELD) && !bLocalConn)
                                {

                                    // Only add connections to the cache if the
                                    // connection state is established.  The states
                                    // between JTAPI and STAPI don't actaully match
                                    // -- see the JTAPI connectionConnected handler
                                    // within this source for more information.
                                    int iLocal = getLocalConnectionState() ;
                                    if ( iLocal == PCall.CONN_STATE_CONNECTED ||
                                            iLocal == PCall.CONN_STATE_UNKNOWN)
                                    {
                                        addToStateCache(address, iState) ;
                                    }
                                }
                            }
                            catch (PCallAddressException e)
                            {
                                System.out.println("Invalid Address: " + strAddress) ;
                                SysLog.log(e) ;
                            }
                        }
                    }
                }
            }
        }


        /*
         * Step 2: Massage our data into an array of PAddressse
         */
        rc = new PAddress[vParticipants.size()] ;
        for (int i=0; i<vParticipants.size(); i++) {
            rc[i] = (PAddress) vParticipants.elementAt(i) ;
        }

        return rc ;
    }



    /**
     * Get the connection state of this call. If a call has multiple
     * connections, then 'best' state of the entire call is returned. The
     * 'best' state can be defined as the state closest to connected/
     * established. <p>
     * For example, if one connection is established/connected then the
     * entire call is considered connected, even if another connection is in a
     * TRYING or FAILED state. Likewise, if a connection is in a FAILED
     * state and a new connection is created and moves to TRYING, then the
     * call is trying. IDLE states are generally ignored.
     *
     * @return The state of the call. Please see CONN_STATE_* for the list
     *      of possible return values.
     * @see #CONN_STATE_UNKNOWN
     * @see #CONN_STATE_IDLE
     * @see #CONN_STATE_TRYING
     * @see #CONN_STATE_OUTBOUND_ALERTING
     * @see #CONN_STATE_INBOUND_ALERTING
     * @see #CONN_STATE_CONNECTED
     * @see #CONN_STATE_HELD
     * @see #CONN_STATE_FAILED
     * @see #CONN_STATE_DISCONNECTED
     */
    public int getConnectionState()
    {
        int        iState = CONN_STATE_UNKNOWN ;
        int        iNumConnections = 0 ;

        if (isDropping())
        {
            iState = CONN_STATE_DISCONNECTED ;
        }
        else
        {
            Enumeration enum = m_cacheStates.getAddresses() ;
            while (enum.hasMoreElements())
            {
                PAddress address = (PAddress) enum.nextElement() ;

                int iConnectionState = m_cacheStates.getState(address) ;
                if (iConnectionState > iState)
                {
                    iState = iConnectionState ;
                }
                iNumConnections++ ;
            }

            if ((m_datePlacedOnHold != null) && (iState > CONN_STATE_IDLE))
                iState = PCall.CONN_STATE_HELD ;

            if ((iNumConnections < 1) && (iState == CONN_STATE_UNKNOWN))
                if (m_dateConnected == null)
                    iState = CONN_STATE_IDLE ;
                else
                    iState = CONN_STATE_DISCONNECTED ;
        }

        return iState ;
    }


    /**
     * Return the state of the call for individual participants (connections)
     * in a conference call.
     *
     * @param address The {@link PAddress} of the participant.
     *
     * @return Transfer state constant representing the call state.
     */
    public int getConnectionState(PAddress address)
    {
        int iJTAPIState = Connection.UNKNOWN ;
        int iState = CONN_STATE_UNKNOWN ;

        if (m_bWasDropped)
        {
            iState = CONN_STATE_DISCONNECTED ;
        }
        else
        {
            iState = m_cacheStates.getState(address) ;

            if (iState == PCall.CONN_STATE_UNKNOWN)
            {
                // otherwise try to find it in our connection list and restore it.
                Connection connection = findConnection(address) ;
                if (connection != null) {
                    boolean bLocalConn = ((PtConnection)connection).isLocal() ;
                    iState = mapJTAPIStateToPTAPIState(connection.getState(), bLocalConn) ;
                    if ((iState != CONN_STATE_HELD) && !bLocalConn)
                    {
                        addToStateCache(address, iState) ;
                    }
                }
            }
        }

        return iState ;
    }



    /**
     * Obtain the actual connection state by polling the underlying framework.
     * This is relatively expensive and should be avoided if possible.
     *
     * @deprecated do not expose.
     */
    public int getConnectionStateUncached(PAddress address)
    {
        int iState = PCall.CONN_STATE_UNKNOWN ;
        // otherwise try to find it in our connection list and restore it.
        Connection connection = findConnection(address) ;
        if (connection != null) {
            iState = mapJTAPIStateToPTAPIState(connection.getState(), address.equals(getLocalAddress())) ;
        }

        return iState ;
    }


    /**
     * Play audio from a file. All tones being generated or sounds that are
     * playing are
     * implicitly stopped by invoking <i>playSound</i>.
     * <p>
     * The sound will optionally be played to all of the participants in the
     * call and the local audio device. If the call does not have control
     * of an audio device then no sound will be heard locally. Use <i>playSound</i>
     * in the {@link PMediaManager} class to play a sound locally regardless
     * of call states.
     * <p>
     * Audio files must have the following characteristics:
     * <p>
     * - The file format can be .WAV, Sun .AU, or .RAW audio.<br>
     * - All formats must be 16 bit signed PCM.<br>
     * - The maximum file size is 500 KB.<br>
     * - All formats must use a sampling rate of 8000 samples/second.<p>
     * In addition, .WAV files must be mono and .RAW files must be in
     * little-endian byte order.</P>
     *
     * @param strRAWFile The fully qualified file name of the desired RAW sound
     *        file. See method description for sound file specifications.
     *
     * @param bRepeat The repeat flag. If this flag is true, the audio clip will
     *        be played in a loop until stopped. If this flag is false, the
     *        audio clip is played once.
     *
     * @param bLocal The flag to specify if the audio should be played locally.
     *        If this flag is true, the local audio devices, such as the
     *        handset or speakerphone, are used to play the sound.
     *
     * @param bRemote The flag to specify if the audio should be played to the
     *        call participants. If this flag is true, the call participants
     *        will hear the audio being played.
     *
     *
     * @exception SecurityException Thrown when a user/application does not
     *            have permission to play the passed audio file.
     *
     * @exception PMediaNotSupportedException Thrown if the audio file is
     *            invalid or not supported.
     *
     * @exception PMediaUnavailableException Thrown if specified media is
     *            unavailable.
     *
     * @exception PCallStateException Thrown if a call is in an "Unknown" call
     *            state.
     *
     * @see PMediaManager#playSound
     */
    public void playSound(String strRAWFile, boolean bRepeat, boolean bLocal, boolean bRemote)
        throws SecurityException, PMediaNotSupportedException, PMediaUnavailableException, PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_playSound_file",
                    getCallID(),
                    strRAWFile,
                    String.valueOf(bRepeat),
                    "L="+String.valueOf(bLocal)+"|R="+String.valueOf(bRemote)) ;
        }

        if (strRAWFile == null)
            throw new IllegalArgumentException("null filename passed to playSound.");
        File soundFile = new File(strRAWFile);
        if (soundFile.exists() == false)
            throw new PMediaUnavailableException("Could not find file.");

        int iState = getConnectionState() ;
        if (iState != CONN_STATE_UNKNOWN)
        {
            if (m_call != null)
            {
                synchronized (m_call)
                {
                    PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
                    if (tcLocal != null)
                    {
                        stopTone() ;
                        stopSound() ;
                        tcLocal.playFile(strRAWFile, bRepeat, bLocal, bRemote) ;
                        setCallData("playsound.file", strRAWFile) ;

                        // Block and cleanup if not repeating
                        File file = new File(strRAWFile);
                        int iLength = (int)file.length();
//
//      WARNING
//      WARNING
//      WARNING:        THERE IS A BUG WHERE REPEAT DOESN"T WORK.
//      WARNING         FOR NOW WE WILL PLAY IT ONCE.
//      WARNING
//      WARNING         TAKE OUT NEXT TWO LINES WHEN IT"S FIXED

                        //commented out the following two lines
                        //as we were getting a long delay while receiving
                        // a call. ( Jan 22. 2002)

                        // if (bRepeat)
                        //    bRepeat = false;

                        if (!bRepeat)
                        {
                            // Wait time in ms: ((bytes / (8000 samples/sec)*(2 bytes/sample)) * 1000)
                            int iWaitTime = ((iLength / 16) + 200) + 50 ; // add 50 ms for buffer room
                            try
                            {
                                Thread.currentThread().sleep(iWaitTime) ;
                            }
                            catch (InterruptedException ie) { }


                            try
                            {
                                stopSound() ;
                            }
                            catch (PSTAPIException e)
                            {
                                SysLog.log(e) ;
                            }
                        }
                    }
                }
            }
        }
    }


    /**
     * Play audio from a stream. All tones being generated or playing sounds
     * are implicitly stopped by invoking <i>playSound</i>.
     * <p>
     * The sound will optionally be played to all of the participants in the
     * call and the local audio device. The local audio device includes the
     * speakerphone as well as the handset. If the call does not have control
     * of an audio device then no sound will be heard on the local audio devices.
     * Use <i>playSound</i> in the {@link PMediaManager} class to play a sound
     * locally regardless of call states.
     * <p>
     * To stop playing an audio clip before the clip has reached the end, use the
     * <i>stopSound</i> method. You may also use the <i>stopSound</i> method
     * to stop playing a looping audio clip.</p>
     * <p>
     * Audio files must have the following characteristics:
     * <p>
     * - The file format can be .WAV, Sun .AU, or .RAW audio.<br>
     * - All formats must be 16 bit signed PCM.<br>
     * - The maximum file size is 500 KB.<br>
     * - All formats must use a sampling rate of 8000 samples/second.<p>
     * In addition, .WAV files must be mono and .RAW files must be in
     * little-endian byte order.</P>
     *
     * @param inputStream The stream containing the audio clip. Use
     *      <pre>getApplication().getResourceAsStream(audioResourceName)</pre>
     *                    in your form to get the audio clip stream.
     *
     * @param bRepeat The repeat flag. If this flag is true, the audio clip is
     *                played in a loop until stopped.
     *                If this flag is false, the audio clip is
     *                played once.
     *
     * @param bLocal The flag to specify if the audio should be played locally. If
     *               this flag is true,
     *               the local audio devices, such as the handset or speakerphone,
     *               are used to play the sound.
     *
     * @param bRemote The flag to specify if the audio should be played to the
     *                call participants. If this flag is true, the call
     *                participants will hear the audio being played.
     *
     * @exception SecurityException Thrown when a user/application does not
     *            have permission to play the passed audio stream.
     *
     * @exception PMediaNotSupportedException Thrown if the audio stream is
     *            invalid or not supported.
     *
     * @exception PMediaUnavailableException Thrown if specified media is
     *            unavailable.
     *
     * @exception PCallStateException Thrown if a call is in an "Unknown" call
     *            state.
     *
     * @exception IllegalArgumentException Thrown if inputStream is null
     *
     * @see PMediaManager#playSound
     * @see org.sipfoundry.sipxphone.Application#getResourceAsStream
     */
    public void playSound(InputStream inputStream, boolean bRepeat, boolean bLocal, boolean bRemote)
        throws SecurityException, PMediaNotSupportedException, PMediaUnavailableException, PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_playSound_stream",
                    getCallID(),
                    inputStream.toString(),
                    String.valueOf(bRepeat),
                    "L="+String.valueOf(bLocal)+"|R="+String.valueOf(bRemote)) ;
        }

        if (inputStream == null)
            throw new IllegalArgumentException("null inputStream passed to playSound.");

        String strTempFile = PingerInfo.getInstance().getFlashFileSystemLocation() + File.separator +  "tmpSoundFile.raw";

        byte buf[] = new byte[1024*4] ;
        int  iLength = 0 ;
        try
        {
            File file = new File(strTempFile) ;
            FileOutputStream out = new FileOutputStream(file) ;

            // Dump the Contents of the File into a temp file
            int iBytes = inputStream.read(buf) ;
            while (iBytes != -1)
            {
                iLength += iBytes ;
                out.write(buf, 0, iBytes) ;
                iBytes = inputStream.read(buf) ;
            }

            out.close();

            playSound(strTempFile, bRepeat, bLocal, bRemote) ;

            if (!bRepeat)
                file.delete() ;

        }
        catch (IOException ioe)
        {
            Shell.getInstance().showMessageBox(MessageBox.TYPE_ERROR, "playSound", "Error creating temp file:\n\n " + ioe, null) ;
            SysLog.log(ioe) ;
        }
    }

    /**
     * Records to a filename for specified time.
     * Will terminate if silenceLength time of silence is reached.
     * Use stopRecording to stop before silence is reached.
     *
     * @since 2.1
     *
     *
     * @param fileName file to record audio to.
     * @param msRecordTime max length of recording.
     * @param msSilenceLength max length of silence before terminating record.
     *
     * @return void
     * @author dwinsor
     *
     * @deprecated do not expose
     *
     */
    public void startRecord(String fileName, int msRecordTime, int silenceLength)
    {
        if (m_call != null)
        {
            icRecordThread recordThread = new icRecordThread(fileName, m_strCallID, msRecordTime,silenceLength);
            recordThread.start();
        }
    }

    /**
     * Stops recording to a filename specified above.
     *
     * @since 2.1
     *
     * @return void
     * @author dwinsor
     *
     * @deprecated do not expose
     *
     */
    public void stopRecord()
    {
        if (m_call != null)
            stopRecording(m_strCallID);
    }


    /**
     * Stop playing all audio clips. If no audio clips are playing, this
     * method does nothing (silent error).
     *
     * @exception PCallStateException Thrown if the call is in an 'Unknown'
     *            call state.
     */
    public void stopSound()
        throws PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_stopSound",
                    getCallID()) ;
        }

        int iState = getConnectionState() ;
        if (iState != CONN_STATE_UNKNOWN)
        {
            String strRAWFile = (String) getCallData("playsound.file") ;
            if (strRAWFile != null)
            {
                removeCallData("playsound.file") ;
                PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
                if (tcLocal != null)
                {
                    tcLocal.stopPlay(true) ;
                }
            }
        }
    }


    /**
     * Play a DTMF tone until told to stop. The tone plays until
     * explicitly stopped by calling <i>stopTone</i>. All tones being
     * generated or sounds that are playing are implicitly stopped by invoking
     * the <i>playTone</i> method.
     * <p>
     * The tone is played to this call and local audio device. If the
     * call does not have control of an audio device then the tone will not
     * be heard locally. Use PMediaManager's <i>playTone</i> to play a tone
     * locally regardless of whether or not this phone is participating in a
     * call.
     *
     * @param iToneID Integer tone ID. See DTMF_* for the values to pass in
     *        to this method.
     *
     * @exception PCallStateException Thrown if the call is in an
     *            'unknown' call state
     * @exception IllegalArgumentException Thrown if an invalid tone id is
     *            specified.
     *
     * @see #stopTone
     * @see PMediaManager#playTone(int, int)
     */
    public void playTone(int iToneID)
        throws PCallStateException
    {
        playTone(iToneID, true, true) ;
    }


    /**
     * Play a DTMF tone until told to stop. The tone plays until
     * explicitly stopped by calling <i>stopTone</i>. All tones being
     * generated or playing sounds are implicitly stopped by invoking
     * the <i>playTone</i> method.
     * <p>
     * The tone will be played to this call and local audio device. If the
     * call does not have control of an audio device then the tone will not
     * be heard locally. Use PMediaManager's <i>playTone</i> to play a tone
     * locally regardless of whether or not this phone is participating in a
     * call.
     *
     * @param iToneID integer tone ID. See DTMF_* for the values to pass in
     *        to this method.
     * @param bLocal Should the tone be played to the local caller
     * @param bRemote Should the tone be played to the remote caller
     *
     * @exception PCallStateException Thrown if the call is in an
     *            'unknown' call state
     * @exception IllegalArgumentException Thrown if an invalid tone id is
     *            specified.
     *
     * @see #stopTone
     * @see PMediaManager#playTone(int, int)
     */
    public void playTone(int iToneID, boolean bLocal, boolean bRemote)
        throws PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_playTone",
                    getCallID(),
                    String.valueOf(iToneID),
                    "L="+String.valueOf(bLocal)+"|R="+String.valueOf(bRemote)) ;
        }

        if ((iToneID < DTMF_POUND || iToneID > DTMF_9) &&
            iToneID != PtTerminalConnection.DTMF_TONE_DIALTONE &&
            iToneID != PtTerminalConnection.DTMF_TONE_BUSY &&
            iToneID != PtTerminalConnection.DTMF_TONE_RINGBACK &&
            iToneID != PtTerminalConnection.DTMF_TONE_RINGTONE &&
            iToneID != PtTerminalConnection.DTMF_TONE_CALLFAILED &&
            iToneID != PtTerminalConnection.DTMF_TONE_SILENCE &&
            iToneID != PtTerminalConnection.DTMF_TONE_BACKSPACE &&
            iToneID != PtTerminalConnection.DTMF_TONE_CALLWAITING &&
            iToneID != PtTerminalConnection.DTMF_TONE_CALLHELD)
            throw new IllegalArgumentException("Invalid Tone ID passed.");

        int iState = getConnectionState() ;
        if (iState != CONN_STATE_UNKNOWN)
        {
            PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
            if (tcLocal != null)
            {
                Stack stack = (Stack) getCallData("tone.stack") ;
                if (stack == null)
                    stack = new Stack() ;

                icToneDescriptor toneDesc = new icToneDescriptor(iToneID, bLocal, bRemote) ;
                stack.push(toneDesc) ;
                setCallData("tone.stack", stack) ;

                postDtmfDown(toneDesc);
                tcLocal.startTone(iToneID, bLocal, bRemote) ;
            }
            else
                System.out.println("Unable to get local terminal connection in playTone") ;
        } else
            System.out.println("Refusing to playTone on a failed or disconnected call") ;
    }


    /**
     * Determines if the specified tone ID is being played by this call.
     *
     * @deprecated DO NOT EXPOSE
     */
    public boolean isPlayingTone(int iToneID)
    {
        boolean bPlaying = false ;

        Stack stack = (Stack) getCallData("tone.stack") ;
        if (stack != null)
        {
            icToneDescriptor desc = (icToneDescriptor) stack.peek() ;
            if ((desc != null) && (desc.iToneID == iToneID))
            {
                bPlaying = true ;
            }
        }

        return bPlaying ;
    }



    /**
     * Play a DTMF tone for a specified duration. The tone will be played for
     * the period specified by iDuration. Tones can be prematurely ended by
     * calling <i>stopTone</i>. All tones being generated or playing sounds
     * are implicitly stopped by invoking the <i>playTone</i> method.
     * <p>
     * The tone will be played to this call and the local audio device. If the
     * call does not have control of an audio device then the tone will not
     * be heard locally. Use PMediaManager's <i>playTone</i> method
     * to play a tone locally regardless of call states.
     *
     * @param iToneID Integer tone ID. See DTMF_* for the values to pass in
     *        to this method.
     *
     * @param iDuration How long the tone should be played in milliseconds.
     *
     * @exception PCallStateException Thrown if the call is in an "Unknown" call
     *            state.
     * @exception IllegalArgumentException Thrown if an invalid tone id is
     *            specified.
     *
     * @see #stopTone
     * @see PMediaManager#playTone(int, int)
      */
    public  void playTone(int iToneID, int iDuration)
        throws PCallStateException
    {
        playTone(iToneID, iDuration, true, true) ;
    }


    /**
     * Play a DTMF tone for a specified duration. The tone plays for
     * the period specified by iDuration. Tones can be prematurely ended by
     * calling <i>stopTone</i>. All tones being generated or playing sounds
     * are implicitly stopped by invoking the <i>playTone</i> method.
     * <p>
     * The tone will be played to this call and the local audio device. If the
     * call does not have control of an audio device then the tone will not
     * be heard locally. Use PMediaManager's <i>playTone</i> method
     * to play a tone locally regardless of call states.
     *
     * @param iToneID Integer tone ID. See DTMF_* for the values to pass in
     *        to this method.
     *
     * @param iDuration How long the tone should be played in milliseconds.
     *
     * @exception PCallStateException Thrown if the call is in an "Unknown" call
     *            state.
     * @exception IllegalArgumentException Thrown if an invalid tone id is
     *            specified.
     *
     * @see #stopTone
     * @see PMediaManager#playTone(int, int)
      */
    public void playTone(int iToneID, int iDuration, boolean bLocal, boolean bRemote)
        throws PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_playTone",
                    getCallID(),
                    String.valueOf(iToneID),
                    "L="+String.valueOf(bLocal)+"|R="+String.valueOf(bRemote)) ;
        }

        int iState = getConnectionState() ;
        if (iState != CONN_STATE_UNKNOWN) {
            PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
            if (tcLocal != null)
            {
                Stack stack = (Stack) getCallData("tone.stack") ;
                if (stack == null)
                    stack = new Stack() ;

                icToneDescriptor toneDesc = new icToneDescriptor(iToneID, bLocal, bRemote) ;
                stack.push(toneDesc) ;
                setCallData("tone.stack", stack) ;
                postDtmfDown(toneDesc);
                tcLocal.startTone(iToneID, bLocal, bRemote) ;
            }

            try
            {
                Thread.currentThread().sleep(iDuration) ;
            } catch (InterruptedException ie) { /* burp */ }

            stopTone() ;
        }
    }


    /**
     * Stop playing a DTMF tone. This method will stop tones started by
     * either of the <i>playTone</i> methods. If no tones are playing,
     * this method does nothing (silent error).
     *
     * @exception PCallStateException Thrown if the call is in an 'Unknown'
     *            call state.
     *
     * @see #playTone
     */
    public void stopTone()
        throws PCallStateException
    {
        if (Logger.isEnabled())
        {
            Logger.post("media",
                    Logger.TRAIL_NOTIFICATION,
                    "call_stopTone",
                    getCallID()) ;
        }

        Stack stack = (Stack) getCallData("tone.stack") ;
        if (stack != null)
        {
            try
            {
                icToneDescriptor descTone = (icToneDescriptor) stack.pop() ;
                if (descTone != null)
                {
                    int iState = getConnectionState() ;
                    if (iState != CONN_STATE_UNKNOWN)
                    {
                        PtTerminalConnection tcLocal = getLocalTerminalConnection() ;
                        if (tcLocal != null)
                        {
                            postDtmfUp(descTone);
                            tcLocal.stopTone() ;

                            descTone = (icToneDescriptor) stack.peek() ;
                            if (descTone != null)
                            {
                                postDtmfDown(descTone);
                                tcLocal.startTone(descTone.iToneID, descTone.bLocal, descTone.bRemote) ;
                            }
                        }
                    }
                }
                setCallData("tone.stack", stack) ;
            }
            catch (EmptyStackException ese)
            {
                removeCallData("tone.stack") ;
            }
        }
    }


    /**
    * Create a MediaPlayer to play the file located in the <i>sourceURL</i>, of
    * format <i>iSourceFormat</i>. The specified file plays locally, remotely,
    * or both, based on the value of <i>iTarget</i>.
    * Caching can be turned on by passing an appropriate <i>iFlags</i> value.
    *
    * @param sourceURL The URL of the audio file for the media player to play.<br><br>
    * @param iTarget Specifies whether to play the file locally, remotely, or both:
    *    <ul>
    *       <li>PMediaPlayer.TARGET_LOCAL to play locally
    *       <li>PMediaPlayer.TARGET_REMOTE to play remotely
    *       <li>(PMediaPlayer.TARGET_LOCAL | PMediaPlayer.TARGET_REMOTE) to
    *        play both locally and remotely
    *    </ul><br>
    * @param iSourceFormat Indicates the format of the data source:<br>
    *    <ul>
    *       <li>PMediaPlayer.SOURCE_FORMAT_AUTO to have the player automatically
    *         detect the format.<br> Only ".wav" and ".raw" format sources are
    *         supported in this release.
    *       <li>PMediaPlayer.SOURCE_FORMAT_RAW to indicate ".raw" format
    *       <li>PMediaPlayer.SOURCE_FORMAT_WAV to indicate ".wav" format
    *    </ul><br>
    * @param iFlags Controls caching:<br>
    *    <ul>
    *      <li>PMediaPlayer.FLAG_ENABLE_CACHE to turn caching on
    *      <li>PMediaPlayer.FLAG_NONE to pass no flags
    *    </ul><br>
    * @exception IllegalArgumentException Thrown if <i>sourceURL</i> is null.
    * @exception PMediaException Thrown if the player cannot be created.
    * @exception java.io.IOException Thrown if a problem was encountered connecting to
    *                                   <i>sourceURL</i>.
    */
   public PMediaPlayer createMediaPlayer
        (URL sourceURL, int iTarget, int iSourceFormat, int iFlags)
        throws java.io.IOException, PMediaException
   {
       if( sourceURL == null )
         throw new IllegalArgumentException("sourceURL is null");
       PMediaPlayer player =
           new PMediaPlayer(getLocalTerminalConnection(),
                            sourceURL,
                            iTarget, iSourceFormat, iFlags );


       return player ;
   }


    /**
     * @deprecated - do not expose (yet)
     */
    public PMediaPlaylistPlayer createMediaPlaylistPlayer()
    {
        PMediaPlaylistPlayer player = new PMediaPlaylistPlayer(getLocalTerminalConnection());

        return player ;
    }



    /**
     * Add a connection listener to this PCall. After it is added, the listener
     * will receive notification of all of the state changes of each connection
     * (participant) of the call.
     * <p>
     * Add a call listener to {@link PCallManager} for notification of all calls,
     * including incoming calls.
     *
     * @param listener The call listener that will receive notifications.
     * @see PConnectionListener
     */
    public void addConnectionListener(PConnectionListener listener)
    {
        if (m_vConnectionListenersList == null) {
            m_vConnectionListenersList = new Vector() ;
        }

        synchronized (m_vConnectionListenersList) {
            if (!m_vConnectionListenersList.contains(listener)) {
                m_vConnectionListenersList.addElement(listener) ;
            }
        }
    }


    /**
     * Remove a connection listener from this call.
     *
     * @param listener The call listener that no longer needs notifications from
     *                 this call.
     *
     * @see PConnectionListener
     */
    public void removeConnectionListener(PConnectionListener listener)
    {
        if (m_vConnectionListenersList != null) {
            synchronized (m_vConnectionListenersList) {
                m_vConnectionListenersList.removeElement(listener) ;
            }
        }
    }


    /**
     * How long has the call has been active? This method returns the
     * call duration in seconds, rounded down. If the call is no longer
     * connected, then the total call
     * duration is returned.
     * <p>
     * The duration of the call is defined as starting when the callee
     * picked up the
     * phone until one of the participants of the call hangs up.
     *
     * @return Duration of the call in seconds, or 0 if the call was never
     *         established or connected.
     */
    public int getDuration()
    {
        int iRC = m_iCallDuration ;

        if (m_dateConnected != null) {
            int iState = getConnectionState() ;
            if ( (iState == PCall.CONN_STATE_CONNECTED) || (iState == PCall.CONN_STATE_HELD)) {
                Date dateNow = Calendar.getInstance().getTime() ;

                long lDiff = dateNow.getTime() - m_dateConnected.getTime() ;
                m_iCallDuration = (int) (lDiff / 1000) ;
            }
        }
//        System.out.println("Call duration = " + iRC);
        return iRC ;
    }

    /**
     * only for incoming calls.
     * gives the duration from the time it started ringing.
     *
     * @deprecated  DO NOT EXPOSE
     */
    public int getDurationSinceFirstRang(){
        int iRC = 0;
        Date dateNow = Calendar.getInstance().getTime() ;
        long lDiff = dateNow.getTime() - m_dateStartedRinging.getTime() ;
        iRC = (int) (lDiff / 1000) ;
        return iRC;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Hidden Methods.  These are public for various reasons, however, we
// do not want to expose these to outside developers
////


    /**
     * Is premium sound enabled for this call?
     *
     * @deprecated DO NOT EXPOSE
     */
    public boolean isPremiumSoundEnabled()
    {
        return !m_bDisabledPremiumSound ;
    }


    /**
     * Obtain session information for a connection.  The actual Session
     * returned here is a tag, the user should check the session for it's
     * true runtime type and cast it appropriately.
     *
     * @param address The address whose session information will be returned.
     *
     * @return The session object for this connection or null if not
     *         available/supported.
     *
     * @deprecated Do not expose, yet.
     */
    public PSession getSession(PAddress address)
    {
        SipSession session = null ;

        if( address != null )
           session = (SipSession) m_htSessionCache.get(address.toString()) ;
        if (session == null)
        {
            PtConnection connection = (PtConnection) findConnection(address) ;
            if (connection != null)
            {
                session = connection.getSession() ;
            }

            if (session != null)
            {
                m_htSessionCache.put(address.toString(), session) ;
            }
        }

        return session ;
    }

    /**
     * gets the JAIN SIPLITE equivalent of this STAPI call.
     * @deprecated DO NOT EXPOSE
     */
    public javax.siplite.Call getJAINCall(){
        javax.siplite.Call jainCall = null;
        String strCallID = getCallID();
        if( strCallID != null ){
            jainCall =
                CallProviderImpl.getInstance().getCallByCallID( strCallID );
        }
        return jainCall;

    }



    /**
     * Obtain session information for all connections.  The actual session
     * returned here is actaully a tag, the user should check the session for
     * it's true runtime type and cast it appropriately.
     *
     * @deprecated Do not expose, yet.
     */
    public PSession[] getSessions()
    {
        PAddress addresses[] = getParticipants() ;

        PSession sessions[] = new PSession[addresses.length] ;
        for (int i=0; i<addresses.length; i++)
        {
            sessions[i] = getSession(addresses[i]) ;
        }
        return sessions ;
    }


    /**
     * Return the time when the call was established.
     *
     * @return The time/date stamp of when the call connected, or null
     *         if the call is not yet connected.
     *
     * @deprecated do not expose, yet.
     */
    public Date getStartTime()
    {
        return m_dateConnected ;
    }


    /**
     * Is this call considered a conference? A call is considered a
     * conference if it was ever part of a conference and doesn't
     * reflect the current number of connections/participants.
     *
     * @return boolean True if this call is considered a conference call,
     *         else false.
     *
     * @deprecated do not expose, yet.
     */
    public boolean isConferenceCall()
    {
        return m_bIsConference ;
    }


    /**
     * Is this an inbound call?
     *
     * @return True if the call was created as a response to an external
     *         invite.
     *
     * @deprecated do not expose, yet.
     */
    public boolean isInbound()
    {
        return !m_bWasOutboundCall ;
    }


    /**
     * Is this an outbound call?
     *
     * @return True if the call was created locally and not as a reaction to
     *         an external invite.
     *
     * @deprecated do not expose, yet.
     */
    public boolean isOutbound()
    {
        return m_bWasOutboundCall ;
    }



    /**
     * Is this an incoming call?
     *
     * @return True if this call is an inbound call.
     *
     * @deprecated do not expose, yet.
     */
    public boolean isInboundConnection(PAddress address)
    {
        boolean bRC = false ;

        Connection connection = findConnection(address) ;
        if (connection != null)
        {
            if (((PtConnection) connection).isLocal())
            {
                bRC = true ;
            }
        } else
            System.out.println("Unable to find connection: " + address) ;
        return bRC ;
    }


    /**
     * Is this an outgoing call?
     *
     * @return true if this call is an outbound call
     *
     * @deprecated do not expose, yet.
     */
    public boolean isOutboundConnection(PAddress address)
    {
        return !isInboundConnection(address) ;
    }


    /**
     * Return the elapsed time that this call has been on hold (in seconds).
     *
     * @return The number of seconds that this call has been on hold, or
     *         -1 if the call is not actually on hold.
     *
     * @see #placeOnHold
     * @see #releaseFromHold
     *
     * @deprecated not yet ready for prime time
     */
    public int getHoldDuration()
    {
        int iDuration = -1 ;

        if (m_datePlacedOnHold != null) {
            Date dateNow = Calendar.getInstance().getTime() ;

            long lDiff = dateNow.getTime() - m_datePlacedOnHold.getTime() ;
            iDuration = (int) (lDiff / 1000) ;
        }

        return iDuration ;
    }


    /**
     * @deprecated do not expose
     */
    public String toString()
    {
        String strCallID = getCallID() ;
        if (strCallID == null)
            strCallID = "UNKNOWN" ;

        return strCallID ;
    }


    /**
     * Ignore attempts to drop the call by developers.  Any call to
     * disconnect() will be ignored.
     *
     * @deprecated do not expose
     */
    public void ignoreExternalDrops(boolean bIgnore)
    {
        m_bIgnoreExternalDrops = bIgnore ;
    }


    /**
     * The Call ID for this call.  Each call has a unique ID that can be used
     * to differentiate calls.
     *
     * @deprecated do not exposes
     */
    public String getCallID()
    {
        if (m_strCallID == null) {

            if (m_call != null) {
                synchronized (m_call) {
                    m_strCallID = m_call.getCallID() ;
                }
            }
        }

        return m_strCallID ;
    }


    /**
     * Dump the internal cache of states.  This shouldn't be needed, however,
     * is sometimes useful when debugging.
     *
     * @deprecated do not expose
     */
    public void dumpConnectionsCache()
    {
        System.out.println("Dumping Connection Cache for " + toString() + " (" + toConnectionStateString(getConnectionState()) + ")") ;
// System.out.println("\tZombie: " + Shell.getCallManager().isZombieCall(this) + "  Hidden: " + Shell.getCallManager().isHiddenCall(this)) ;

        Enumeration enumKeys = m_cacheStates.getAddresses() ;
        while (enumKeys.hasMoreElements())
        {
            PAddress address = (PAddress) enumKeys.nextElement() ;
            if (address != null)
            {
                int iState = m_cacheStates.getState(address) ;
                System.out.println("\t" + address.toString() + ": " + iState + "/" + toConnectionStateString(iState)) ;
            }
        }
        System.out.println("") ;
    }


    /**
     * Dump the state of all our connections.
     *
     * @deprecated do not expose
     */
    public void dumpConnections()
    {
        System.out.println("Dumping Connections for " + toString()) ;
        if (m_call != null)
        {
            Connection connections[] = m_call.getConnections() ;
            for (int i=0; i<connections.length; i++)
            {
                System.out.println("  " + connections[i]) ;
                TerminalConnection tcs[] = connections[i].getTerminalConnections() ;
                for (int j=0; j<tcs.length; j++)
                {
                    System.out.println("    " + tcs[j]) ;
                }
            }
        }
        System.out.println("") ;
    }

    /**
     * This method gets called when the VM garbage collects this object.
     */
    protected void finalize()
        throws Throwable
    {
        if (m_musicOnHoldPlayer != null)
            m_musicOnHoldPlayer.close();
    }


    /**
     * Set call specfic data related.
     *
     * @param objKey the key for the data
     * @param objData the call specific data
     *
     * @return any previous data set via the designated key or null
     *
     * @see #getCallData
     *
     * @deprecated do not exposes
     */
    public Object setCallData(Object objKey, Object objData)
    {
        Object objOldData = null ;

        PCallData dataSource = PCallData.getInstance() ;
        if (dataSource != null)
        {
            objOldData = dataSource.setData(getCallID(), objKey, objData) ;
        }

        return objOldData ;
    }


    /**
     * Get call specific data
     *
     * @param objKey the key that the data was set under
     *
     * @return the object stored under the designated objKey or null if not
     *         found.
     *
     * @see #setCallData
     *
     * @deprecated do not exposes
     */
    public Object getCallData(Object objKey)
    {
        Object objRC = null ;

        PCallData dataSource = PCallData.getInstance() ;
        if (dataSource != null)
        {
            objRC = dataSource.getData(getCallID(), objKey) ;
        }

        return objRC ;
    }


    /**
     * Remove the designated piece of call specific data
     *
     * @param objKey the key that identifies the data to remove
     *
     * @return The object previously stored under the designed key or null if
     *         no data was stored.
     *
     * @deprecated do not exposes
     */
    public Object removeCallData(Object objKey)
    {
        Object objOldData = null ;

        PCallData dataSource = PCallData.getInstance() ;
        if (dataSource != null)
        {
            objOldData = dataSource.removeData(getCallID(), objKey) ;
        }

        return objOldData ;
    }


    /**
     * Set a hangup handler for this call
     *
     * @deprecated do not expose
     */
    public void setHangupHandler(ActionHandler handler)
    {
        m_hangupHandler = handler ;
    }


    /**
     * Get the hangup handler for this call.
     *
     * @deprecated do not expose
     */
    public ActionHandler getHangupHandler()
    {
        return m_hangupHandler ;

    }


    /**
     * @deprecated do not expose
     */
    public void setAutoDrop(boolean bEnabled)
    {
        m_bAutoDrop = bEnabled ;
    }


    /**
     * @deprecated do not expose
     */
    public boolean isAutoDropEnabled()
    {
        return m_bAutoDrop ;
    }


    /**
     * @deprecated do not expose
     */
    public boolean isHeld(PAddress address)
    {
        return m_cacheStates.isRemoteHeld(address) ;
    }


    /**
     * @deprecated do not expose
     */
    public boolean isPlayingHoldMusic()
    {
        return m_bMusicOnHoldPlaying ;
    }


    /**
     * Does this call already contain the max number of connections allow
     * for a conference?
     *
     * @deprecated do not expose
     */
    public boolean isConferenceLimitReached()
    {
        boolean bLimitReached = false ;

        int iMaxParticipants = Settings.getInt("MAX_CONFERENCE_PARTICIPANTS", 4) ;
        PAddress participants[] = getParticipants() ;
        if ((participants != null) && (participants.length >= iMaxParticipants))
        {
            bLimitReached = true ;
        }

        return bLimitReached ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Place all of the participants of a call on hold.
     *
     * @exception SecurityException Thrown when a user/application does
     *            not have permission to place a call on hold.
     *
     * @exception PCallStateException Thrown if the call is dropped,
     *            incoming, or already on hold.
     */
    protected  void placeAllParticipantsOnHold()
         throws SecurityException, PCallStateException
     {
         synchronized (m_objHold)
         {
             if (m_call != null)
             {
                 boolean bSuccessful = false ;
                 int iAttempts = 0 ;

                 while ((iAttempts < 3) && !bSuccessful)
                 {
                     iAttempts++ ;

                     HeldCallStateChangePoller poller = new HeldCallStateChangePoller(m_call, getSessions()) ;
                     try
                     {
                         if (Logger.isEnabled())
                         {
                             Logger.post("stapi", Logger.TRAIL_COMMAND, "placeOnHold") ;
                         }

                         synchronized (m_call)
                         {
                             m_call.hold(m_bIsConference) ;
                         }

                         // If this is not a conference, then block until the state change
                         if (!m_bIsConference)
                         {
                             poller.beginPolling() ;
                             if (!poller.wasSuccessful())
                             {
                                 if (poller.wasAborted())
                                 {
                                     try
                                     {
                                         Random rand = new Random() ;

                                         int iSleepPeriod = (int) Math.pow(2, 8+iAttempts) ;

                                         Thread.sleep(Math.abs(rand.nextInt() % iSleepPeriod)) ;
                                     }
                                     catch (InterruptedException e)
                                     {
                                         SysLog.log(e) ;
                                     }
                                 }
                                 else
                                 {
                                     throw new PCallStateException("Unable to place call on hold") ;
                                 }
                             } else
                                 bSuccessful = true ;
                         }
                         else
                             bSuccessful = true ;
                     }
                     finally
                     {
                         poller.cleanup() ;
                     }
                 }

                 if (!bSuccessful)
                     throw new PCallStateException("Unable to place call on hold") ;
             }
         }
     }


    /**
     * @deprecated Do not expose
     */
    public boolean isConference()
    {
        return m_bIsConference ;
    }


    /**
     * Determine if the passed candidate is a participant of the specified
     * member list.
     */
    private boolean isParticipantOf(PAddress candidate, PAddress[] members)
    {
        boolean bParticipant = false ;

        for (int i=0; i<members.length; i++)
        {
            if (candidate.equals(members[i]))
            {
                bParticipant = true ;
                break ;
            }
        }
        return bParticipant ;
    }


    /**
     * Calculate the duration for a connection, given the supplied starting
     * time and connection state.  The connection state must be connect or
     * held for the duration calculation to succeed.
     */
    private int getDuration( Date dateConnected, int iConnectionState ){
        int iRC = 0 ;

        if (dateConnected != null) {
            if ( (iConnectionState == PCall.CONN_STATE_CONNECTED) || (iConnectionState == PCall.CONN_STATE_HELD)) {
                Date dateNow = Calendar.getInstance().getTime() ;

                long lDiff = dateNow.getTime() - dateConnected.getTime() ;
                iRC = (int) (lDiff / 1000) ;
            }
        }
        return iRC ;
    }


    /**
     * Get the local connection state of the specified address.  This
     * information is pulled out the local cache.
     */
    protected int getCachedConnectionState(PAddress address)
    {
        int iJTAPIState = Connection.UNKNOWN ;
        int iState = CONN_STATE_UNKNOWN ;

        if (m_bWasDropped)
        {
            iState = CONN_STATE_DISCONNECTED ;
        }
        else
        {
            iState = m_cacheStates.getState(address) ;
        }
        return iState ;
    }


    /**
     * What it the local connection state?
     */
    protected int getLocalConnectionState()
    {
        return m_iLocalState ;
    }

    /**
     * Initializes the "music on hold" player.
     */
    protected synchronized void initMusicOnHold()
    {
        //debug("in initMusic on Hold ");
        URL musicOnHoldURL = Shell.getInstance().getMusicOnHoldURL();

/*
        if( musicOnHoldURL != null )
        {
            try
            {
                //debug("before doing openConnection().getInputStream()");
                musicOnHoldURL.openConnection().getInputStream();
            }catch( IOException e )
            {
                e.printStackTrace();
                musicOnHoldURL = null;
            }
        }
*/
        //debug("before creating media player if URL is not NULL");
        if( musicOnHoldURL != null )
        {
            // debug("before actually creating media player ");
            try
            {
                m_musicOnHoldPlayer = createMediaPlayer(
                        musicOnHoldURL,
                        PMediaPlayer.TARGET_REMOTE,
                        PMediaPlayer.SOURCE_FORMAT_WAV,
                        PMediaPlayer.FLAG_NONE);

                m_musicOnHoldPlayer.setLoopCount(-1);
                // debug("state before realize" + m_musicOnHoldPlayer.getState());
                m_musicOnHoldPlayer.realize();
                // debug("state after realize" + m_musicOnHoldPlayer.getState());
                m_musicOnHoldPlayer.prefetch();
                // debug("state after prefetch" + m_musicOnHoldPlayer.getState());
            }
            catch(Exception e)
            {
                m_musicOnHoldPlayer = null;
                e.printStackTrace();
            }
        }
    }

    /**
     * Plays the music on hold.
     */
    protected  synchronized void playMusicOnHold() {

        if (m_musicOnHoldPlayer != null )
        {
            try
            {
                //debug("player.getState() before play "+ m_musicOnHoldPlayer.getState() );
                // debug("before playing  player");
                m_musicOnHoldPlayer.setLoopCount(-1);
                m_musicOnHoldPlayer.start();
                m_bMusicOnHoldPlaying = true;
                // debug("after playing  player");
            }
            catch (Exception e) {
                //attempt
                m_musicOnHoldPlayer = null;
                e.printStackTrace();
            }

        }
        else
        {
            try
            {

                //try to recover and restart
                initMusicOnHold();
                m_musicOnHoldPlayer.setLoopCount(-1);
                m_musicOnHoldPlayer.start();
                m_bMusicOnHoldPlaying = true;
            }
            catch (Exception ee) {
                m_musicOnHoldPlayer = null;
                ee.printStackTrace();
            }
        }
    }

    /**
    * Stops the music on hold.
    */
    protected  synchronized void stopMusicOnHold() {

        if (m_musicOnHoldPlayer != null )
        {
            try {
                // debug("player.getState() before stop "+ m_musicOnHoldPlayer.getState() );
                if( m_musicOnHoldPlayer.getState()  == PMediaPlayer.STATE_STARTED )
                {
                    // debug("before stopping  player");
                   m_musicOnHoldPlayer.stop();
                    // debug("after stopping  player");
                    m_bMusicOnHoldPlaying = false;

                    m_musicOnHoldPlayer.close() ;
                    m_musicOnHoldPlayer = null ;
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                m_musicOnHoldPlayer = null;
            }
        }
    }



    /**
     * Map a JTAPI state to one of our internal PTAPI states
     */
    protected int mapJTAPIStateToPTAPIState(int iJTAPIState, boolean bIsLocal)
    {
        int iState = CONN_STATE_UNKNOWN ;

        // Convert the JTAPI state into a PTAPI state
        switch (iJTAPIState)
        {
            case Connection.IDLE:
            case CallControlConnection.IDLE:
                iState = CONN_STATE_IDLE ;
                break ;
            case Connection.INPROGRESS:
            case CallControlConnection.OFFERED:
            case CallControlConnection.NETWORK_REACHED:
            case CallControlConnection.DIALING:
            case CallControlConnection.INITIATED:
                iState = CONN_STATE_TRYING ;
                break ;
            case Connection.ALERTING:
            case CallControlConnection.NETWORK_ALERTING:
            case CallControlConnection.ALERTING:
                if (bIsLocal)
                    iState = CONN_STATE_INBOUND_ALERTING ;
                else
                    iState = CONN_STATE_OUTBOUND_ALERTING ;
                break ;
            case Connection.CONNECTED:
            case CallControlConnection.QUEUED:  //HACK: THIS IS WRONG.  Fix once
                                                // QUEUED is not used to denote
                                                // a held call.

            case CallControlConnection.ESTABLISHED:
                if (m_datePlacedOnHold == null) {
                    iState = CONN_STATE_CONNECTED ;
                } else {
                    iState = CONN_STATE_HELD ;
                }
                break ;
            case Connection.FAILED:
            case CallControlConnection.FAILED:
                iState = CONN_STATE_FAILED ;
                break ;
            case Connection.DISCONNECTED:
            case CallControlConnection.DISCONNECTED:
                iState = CONN_STATE_DISCONNECTED ;
                break ;
            case Connection.UNKNOWN:
            case CallControlConnection.UNKNOWN:
            default:
                iState = CONN_STATE_UNKNOWN ;
        }

        return iState ;
    }


    /**
     * Refresh the list of connection states.  Here we dump our internal cache
     * and then force a hard reload by querying the lower layers.
     */
    protected void refreshConnectionStates()
    {
        m_cacheStates.clear() ;
        PAddress participants[] = getParticipantsUncached() ;
// System.out.println("Refreshed Connection State:" + getCallID()) ;
// System.out.println("getLocalConnectionState()" + getLocalConnectionState()) ;
// dumpConnectionsCache() ;
// dumpConnections() ;
    }

    /**
     * adds internal dtmf listener( this is not the listneer the XDI API has/may
     * have exposed ).
     */
    protected void addInternalDtmfListener(String strCallId)
    {
        if( m_bInternalDtmfListenerAdded == false)
        {
            addDtmfListener(strCallId, new Object(), 0) ;
            m_bInternalDtmfListenerAdded = true;
        }
    }


    /**
     * Get all of the connection listeners registered to this call.
     *
     * @return PCallListener Array holding call listeners registered to this call.
     */
    protected PConnectionListener[] getConnectionListeners()
    {
        PConnectionListener[] rc = null ;

        // If we have a list return that list other wise return an empty array
        if (m_vConnectionListenersList != null) {
            synchronized (m_vConnectionListenersList) {

                int iItems = m_vConnectionListenersList.size() ;

                rc = new PConnectionListener[iItems] ;
                for (int i=0;i<iItems;i++) {
                    rc[i] = (PConnectionListener) m_vConnectionListenersList.elementAt(i) ;
                }
            }
        } else {
            rc = new PConnectionListener[0] ;
        }

        return rc ;
    }


    /**
     * Get the local terminal connection from the call member variable.
     */
    protected PtTerminalConnection getLocalTerminalConnection()
    {
        PtTerminalConnection connection = null ;

        int iState = getConnectionState() ;
        if (m_call != null) {
            synchronized (m_call) {
                connection = (PtTerminalConnection) m_call.getLocalTerminalConnection() ;
            }
        }

        return connection ;
    }


    /**
     * Attach an internal monitor to the JTAPI call.
     */
    protected void attachLocalCallMonitor()
    {
        if (m_icCallMonitor == null)
        {
            m_icCallMonitor = new icCallMonitor() ;
            try
            {
                if (m_call != null)
                {
                    synchronized (m_call)
                    {
                        // Exclude the following events: (not used)
                        //
                        //
                        //
                        // EMASK_TERM_CONN_CREATED    		0x00000001
                        // EMASK_TERM_CONN_IDLE       		0x00000002
                        // EMASK_TERM_CONN_RINGING    		0x00000004
                        // EMASK_TERM_CONN_DROPPED    		0x00000008
                        // EMASK_TERM_CONN_UNKNOWN    		0x00000010
                        // EMASK_TERM_CONN_IN_USE     		0x00000080
                        // EMASK_CONN_CREATED         		0x00000100
                        // EMASK_CONN_UNKNOWN         		0x00001000
                        // EMASK_CONN_DIALING         		0x00002000
                        // EMASK_CONN_INITIATED       		0x00008000
                        // EMASK_CONN_NETWORK_ALERTING		0x00010000
                        // EMASK_CONN_NETWORK_REACHED 		0x00020000
                        // EMASK_CONN_QUEUED          		0x00040000
                        // EMASK_CALL_ACTIVE          		0x00100000
                        // EMASK_CALL_META            		0x00400000
                        //                                  ----------
                        //                                  0x0057b19F
                        m_call.addCallListener(m_icCallMonitor, 0x0057b19F) ;
                    }
                }
            }
            catch (Exception e)
            {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * Detach all internal monitors from the JTAPI call.
     */
    protected void detachLocalCallMonitor()
    {
        // Remove the call monitor
        if (m_callOriginal != null) {
            if (m_icCallMonitor != null)
            {
                synchronized (m_callOriginal)
                {
                    m_callOriginal.removeCallListener(m_icCallMonitor) ;
                }
                m_icCallMonitor = null ;
            }
        }
    }



    /**
     * Sets the local state of this object.  This method was created so that
     * the call manager could set the state of this object externally.  This
     * was needed because in some cases (Blind Transfer), the local connection
     * state event is fired before the listener is added.
     *
     * @deprecated do not expose
     */
    protected void setLocalConnectionState(int iLocalState)
    {
    	m_iLocalState = iLocalState ;
    }


    /**
     * Verifies the iOldState to iNewState transition is valid.
     * set
     * @param iOldState the original state
     * @param iNewState the new state
     *
     * @return true if the state transition is valid; otherwise false.
     */
    protected boolean isValidStateChange(int iOldState, int iNewState)
    {
        boolean bRC = true ;
//        System.out.println("isValidStateChange : old=" +
//                toConnectionStateString(iOldState) + " new=" +
//                toConnectionStateString(iNewState)) ;

        switch (iOldState) {
            case CONN_STATE_DISCONNECTED:
            case CONN_STATE_FAILED:
                // Cannot move out of the failed or disconnected state
                System.out.println
                    ("ERROR: Connection cannot transition for "
                     +getCallID()+" from " +
                     toConnectionStateString(iOldState) + " to " +
                     toConnectionStateString(iNewState) + ".") ;
                bRC = false ;
                break ;

            case CONN_STATE_IDLE:
            case CONN_STATE_UNKNOWN:
            case CONN_STATE_TRYING:
            case CONN_STATE_OUTBOUND_ALERTING:
            case CONN_STATE_INBOUND_ALERTING:
            case CONN_STATE_HELD:
            case CONN_STATE_CONNECTED:
                // Allow movement into and out of all of the above states
                break ;
        }
        return bRC ;
    }


    protected void addToStateCache(PAddress address, int iState)
    {
        // Do not add the state if the connection is idle.  This was added
        // to avoid a problem in transfer where the transfer target gets
        // confused over an idle local connection state.  The connection
        // state should be established, but that event is not being fired.
        // Actually, the connection states don't match the events being
        // fired, at all.  It is a long story, but when receiving a call,
        // the remote site should move to established and the local TC should
        // ring.  Those events are coming through JTAPI, however the states
        // in the connections/TCs are the opposite.
        if (iState != PCall.CONN_STATE_IDLE)
        {
            if( (address != null) &&  (! TextUtils.isNullOrSpace(address.getAddress()) ))
            {
               m_cacheStates.setState(address, iState);
               //Thread.dumpStack();
               debug(" for callid " + getCallID()+ "and address " + address + ",iState " + toConnectionStateString(iState) +
                     " is added to state cache " );
            }else
               debug(" empty address " +  address +" is not put in m_cacheStates ");
        }
    }


    /**
     *  Set the state of our PCall Object. As a side effect the listeners will
     *  be notified of all state changes.
     *<br>
     * helper method to setState with call state, ConnectionEvent object
     * and cacheState. It gets the "address", "iCauseCode", "iResponseCode", and
     * "strResponseText" from the ConnectionEvent object and calls the
     * setState(int      iState,
                 PAddress address,
                 int      iCauseCode,
                 int      iResponseCode,
                 String   strResponseText,
                 boolean  bCacheState) method.
     * @see protected synchronized void setState(int      iState,
                                         PAddress address,
                                         int      iCauseCode,
                                         int      iResponseCode,
                                         String   strResponseText,
                                         boolean  bCacheState)
     */
     protected synchronized void setState(int      iState,
                                          ConnectionEvent event,
                                          boolean  bCacheState)
    {
         setState( iState,
                   generateAddress(event.getConnection().getAddress()),
                   event.getCause(),
                   ((PtConnectionEvent)event).getResponseCode(),
                   ((PtConnectionEvent)event).getResponseText(),
                   bCacheState,
                   ((PtConnectionEvent)event).isLocalConnection()) ;

    }

    /**
     *  Set the state of our PCall Object. As a side effect the listeners will
     *  be notified of all state changes.
     *<br>
     * helper method to setState with call state, address, causeCode,
     * cacheState. It sets the responseCode to PConnectionEvent.DEFAULT_RESPONSE_CODE,
     * and responseText to null and  calls the
     * setState(int      iState,
                 PAddress address,
                 int      iCauseCode,
                 int      iResponseCode,
                 String   strResponseText,
                 boolean  bCacheState)      method.
     * @see protected synchronized void setState(int      iState,
                                         PAddress address,
                                         int      iCauseCode,
                                         int      iResponseCode,
                                         String   strResponseText,
                                         boolean  bCacheState)
     */
    protected synchronized void setState(int      iState,
                                         PAddress address,
                                         int      iCauseCode,
                                         boolean  bCacheState,
                                         boolean  bLocalConnection)
    {
       setState( iState,
                 address,
                 iCauseCode,
                 PConnectionEvent.DEFAULT_RESPONSE_CODE,
                 null,
                 bCacheState,
                 bLocalConnection) ;
    }


    /**
     * Set the state of our PCall Object. As a side effect the listeners will
     * be notified of all state changes.
     */
    protected synchronized void setState(int      iState,
                                         PAddress address,
                                         int      iCauseCode,
                                         int      iResponseCode,
                                         String   strResponseText,
                                         boolean  bCacheState,
                                         boolean  bLocalConnection)
    {

        //Thread.dumpStack();

        int iOldState = PCall.CONN_STATE_UNKNOWN ;
        getDuration() ;

        if (bLocalConnection)
            iOldState = getLocalConnectionState() ;
        else
            iOldState = getCachedConnectionState(address) ;


        // Display state change for debugging purpose
        if ((address != null) && m_sCallDebug)
        {
          System.out.println
                ("\n Set State for " + address + " and callid "+ getCallID() +" : " + toConnectionStateString(iState) + ", oldState="+ toConnectionStateString(iOldState) + "\n\n") ;
        }


        // Cache state
        if (address != null)
        {
            // Only add cached state NOT trying, because the JAVA layers kick of
            // the trying event instead of the lower layers, thus missing the
            // tag.

            if (isValidStateChange(iOldState, iState))
            {
                if (bCacheState)
                {
                    if (!bLocalConnection)
                        addToStateCache(address, iState) ;
                }
                else
                {
                    // If we already have some state for this address, kick out
                    // because the non-cache state is used for priming states-
                    // the real state should trump.
                    if (isParticipantOf(address, getParticipants()))
                        if (!bLocalConnection)
                            return ;
                }
            }
            else
            {
              //if it is CONN_STATE_DISCONNECTEDstate, we need to let it continue
              //to clean up this call.
              if( iState != CONN_STATE_DISCONNECTED )
                 return ;
            }
        }


        PConnectionListener listeners[] = getConnectionListeners() ;
        int                 iNumListeners = listeners.length ;
        PConnectionEvent event =
            new PConnectionEvent(this, address, iCauseCode, iResponseCode, strResponseText) ;

        switch (iState) {
            case CONN_STATE_IDLE:
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "callCreated",
                                SipParser.stripParameters(event.getAddress()),
                                event.getCall().getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    // Broadcast event change to all listeners
                    for (int i=0; i<iNumListeners;i++) {
                        try {
                            listeners[i].callCreated(event) ;
                        } catch (Throwable t) {
                            Shell.getInstance().showUnhandledException(t, false) ;
                        }
                    }
                }
                break ;
            case CONN_STATE_TRYING:
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "connectionTrying",
                                SipParser.stripParameters(event.getAddress()),
                                event.getCall().getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    // Broadcast event change to all listeners
                    for (int i=0; i<iNumListeners;i++) {
                        try {
                            listeners[i].connectionTrying(event) ;
                        } catch (Throwable t) {
                            Shell.getInstance().showUnhandledException(t, false) ;
                        }
                    }
                }
                break ;
            case CONN_STATE_OUTBOUND_ALERTING:
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "connectionOutboundAlerting",
                                SipParser.stripParameters(event.getAddress()),
                                event.getCall().getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    // Broadcast event change to all listeners
                    for (int i=0; i<iNumListeners;i++) {
                        try {
                            listeners[i].connectionOutboundAlerting(event) ;
                        } catch (Throwable t) {
                            Shell.getInstance().showUnhandledException(t, false) ;
                        }
                    }
                }
                break ;
            case CONN_STATE_INBOUND_ALERTING:
                {
                    m_dateStartedRinging  = Calendar.getInstance().getTime() ;

                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "connectionInboundAlerting",
                                SipParser.stripParameters(event.getAddress()),
                                getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    // Broadcast event change to all listeners
                    for (int i=0; i<iNumListeners;i++) {
                        try {
                            listeners[i].connectionInboundAlerting(event) ;
                        } catch (Throwable t) {
                            Shell.getInstance().showUnhandledException(t, false) ;
                        }
                    }
                }
                break ;
            case CONN_STATE_CONNECTED:
                {
                    //only after a call is connected, add the internal
                    //dtmf listener as the call needs to know the
                    //codecs used before it can listen to dtmf tones.
                    //For outgoing calls, this can be probably added
                    //when the call is created, but for incoming calls,
                    //we have to wait until we know the codecs, that is until
                    //we get to "connected" state. Ask Feng for more info.
                    //--pradeep
                    addInternalDtmfListener(event.getCall().getCallID()) ;

                    addPremiumSoundDisabler() ;

                    if (m_dateConnected == null)
                        m_dateConnected = Calendar.getInstance().getTime() ;

                    // Broadcast event change to all listenersfs
                    if (iCauseCode == PConnectionEvent.CAUSE_UNHOLD)
                    {
                        // refreshConnectionStates() ;

                        if (Logger.isEnabled())
                        {
                            Logger.post("stapi",
                                    Logger.TRAIL_NOTIFICATION,
                                    "callReleased",
                                    SipParser.stripParameters(event.getAddress()),
                                    getCallID(),
                                    String.valueOf(event.getCause())) ;
                        }

                        for (int i=0; i<iNumListeners;i++)
                        {
                            try
                            {
                                listeners[i].callReleased(event) ;
                            }
                            catch (Throwable t)
                            {
                                Shell.getInstance().showUnhandledException(t, false) ;
                            }
                        }
                    }
                    else
                    {
                        if( m_bIsConference )
                        {
                            if (( m_originalAddress.getAddress().equals(address.getAddress())) &&
                               ( getConnectionState( address ) != CONN_STATE_HELD ))
                                m_bWasEstablishedCall = true;
                        }
                        else
                        {
                            if (getConnectionState() != CONN_STATE_HELD)
                                m_bWasEstablishedCall = true ;
                        }

                        // Do not fire connectionConnected for the local address
                        if (!bLocalConnection)
                        {
                            if (Logger.isEnabled())
                            {
                                Logger.post("stapi",
                                        Logger.TRAIL_NOTIFICATION,
                                        "connectionConnected",
                                        SipParser.stripParameters(event.getAddress()),
                                        getCallID(),
                                        String.valueOf(event.getCause())) ;
                            }


                            for (int i=0; i<iNumListeners;i++)
                            {
                                try
                                {
                                    listeners[i].connectionConnected(event) ;
                                }
                                catch (Throwable t)
                                {
                                    Shell.getInstance().showUnhandledException(t, false) ;
                                }
                            }
                        }
                    }
                }
                break ;
            case CONN_STATE_HELD:
                {
                    // Only fire off events for the location connection.  We
                    // *should* fire these off for the remote connections, but
                    // this would require change where.  TODO
                    if (bLocalConnection)
                    {
                        if (Logger.isEnabled())
                        {
                            Logger.post("stapi",
                                    Logger.TRAIL_NOTIFICATION,
                                    "callHeld",
                                    SipParser.stripParameters(event.getAddress()),
                                    getCallID(),
                                    String.valueOf(event.getCause())) ;
                        }

                        // Broadcast event change to all listeners
                        for (int i=0; i<iNumListeners;i++) {
                            try {
                                listeners[i].callHeld(event) ;
                            } catch (Throwable t) {
                                Shell.getInstance().showUnhandledException(t, false) ;
                            }
                        }
                    }
                }
                break ;
            case CONN_STATE_FAILED:
                {
                    if (Logger.isEnabled())
                    {
                        Logger.post("stapi",
                                Logger.TRAIL_NOTIFICATION,
                                "connectionFailed",
                                SipParser.stripParameters(event.getAddress()),
                                getCallID(),
                                String.valueOf(event.getCause())) ;
                    }

                    // Broadcast event change to all listeners
                    for (int i=0; i<iNumListeners;i++) {
                        try {
                            listeners[i].connectionFailed(event) ;
                        } catch (Throwable t) {
                            Shell.getInstance().showUnhandledException(t, false) ;
                        }
                    }

                    m_cacheStates.clearState(address);
                }
                break ;
            case CONN_STATE_DISCONNECTED:
                {
                    if (m_call != null)
                    {
                        if (Logger.isEnabled())
                        {
                            Logger.post("stapi",
                                    Logger.TRAIL_NOTIFICATION,
                                    "connectionDisconnected",
                                    SipParser.stripParameters(event.getAddress()),
                                    getCallID(),
                                    String.valueOf(event.getCause())) ;
                        }

                        // Broadcast event change to all listeners
                        for (int i=0; i<iNumListeners;i++) {
                            try {
                                listeners[i].connectionDisconnected(event) ;
                            } catch (Throwable t) {
                                Shell.getInstance().showUnhandledException(t, false) ;
                            }
                        }

                        m_cacheStates.clearState(address);
                    }
                }
                break ;
            case CONN_STATE_UNKNOWN:
                {
                    if (m_call != null)
                    {
                        if (Logger.isEnabled())
                        {
                            Logger.post("stapi",
                                    Logger.TRAIL_NOTIFICATION,
                                    "connectionUnknown",
                                    SipParser.stripParameters(event.getAddress()),
                                    getCallID(),
                                    String.valueOf(event.getCause())) ;
                        }

                        // Broadcast event change to all listeners
                        for (int i=0; i<iNumListeners;i++) {
                            try {
                                listeners[i].connectionUnknown(event) ;
                            } catch (Throwable t) {
                                Shell.getInstance().showUnhandledException(t, false) ;
                            }
                        }
                    }
                }
                break ;
        }
    }


    /**
     * Create a PSIPAddress from a JTAPI address.
     */
    protected static PAddress generateAddress(Address address)
    {
        PAddress rc = null ;

        try
        {
            if (address != null)
            {
                rc = PAddressFactory.getInstance().createAddress(address.getName()) ;
            }
        }
        catch (PCallAddressException e)
        {
            SysLog.log(e) ;
        }

        return rc ;
    }


    /**
     * Get a reference to this call object. Useful for gaining a reference to
     * the outer class from an inner class.
     */
    protected PCall getCall()
    {
        return this ;
    }

    /**
     * This cleans up everything related with this call.
     * When we are sure that this call is not necessary, we
     * should call this method.
     */
    protected void cleanCall()
    {
        removePremiumSoundDisabler();
        detachLocalCallMonitor();
        STAPIFactory.getInstance().getCallManager().deregisterCall(getCall());

        // clearing up the call from JAIN siplite call holder.
        org.sipfoundry.siplite.CallProviderImpl impl =
            org.sipfoundry.siplite.CallProviderImpl.getInstance();
        javax.siplite.Call call = impl.getCallByCallID(getCallID());
        if (call != null) {
            impl.removeCall(call);
        }

        // Null out references to our cached pieces and parts. This will
        // allow them to be garbaged collected, even if someone holds onto a
        // call reference.
        m_provider = null;
        m_terminal = null;
        m_call = null;
        m_callOriginal = null;
        m_phoneState = null;
        m_audioSourceControl = null;
        m_icCallMonitor = null;
        m_vConnectionListenersList = null;
        m_cacheStates.clear();
        m_originalAddress = null;

        if( m_musicOnHoldPlayer != null )
        {
            try{
                m_musicOnHoldPlayer.close();
                m_musicOnHoldPlayer = null;
            }catch(Exception e )
            {
                e.printStackTrace();
            }
        }

        PCallData dataSource = PCallData.getInstance();
        if (dataSource != null) {
            dataSource.removeAllData(getCallID());
        }

        PingerApp.checkLowMemoryCondition(false);
    }

    /**
     * Get a reference to the actual JTAPI call. There is a lot of rope here,
     * don't hang yourself.
     *
     * @deprecated Do not expose
     */
    public Call getJTAPICall()
    {
        return m_call ;
    }

     /**
     * Get a reference to the PTAPI call.
     * @deprecated Do not expose
     */
    public PtCall getPTAPICall()
    {
        return m_call ;
    }



    /**
     * Generate a displayable string for the given connection state
     */
    protected String toConnectionStateString(int iState)
    {
        String strRC = "Unknown" ;

        switch (iState) {
            case PCall.CONN_STATE_CONNECTED:
                strRC = "CONNECTED" ;
                break ;
            case PCall.CONN_STATE_TRYING:
                strRC = "TRYING" ;
                break ;
            case PCall.CONN_STATE_OUTBOUND_ALERTING:
                strRC = "ALERTING" ;
                break ;
            case PCall.CONN_STATE_INBOUND_ALERTING:
                strRC = "RINGING" ;
                break ;
            case PCall.CONN_STATE_IDLE:
                strRC = "IDLE" ;
                break ;
            case PCall.CONN_STATE_HELD:
                strRC = "HELD" ;
                break ;
            case PCall.CONN_STATE_FAILED:
                strRC = "FAILED" ;
                break ;
            case PCall.CONN_STATE_DISCONNECTED:
                strRC = "DROPPED" ;
                break ;
        }
        return strRC ;
    }


    /**
     * Determines if this call is in the process of being dropped (or has been dropped).
     *
     * @return true if the call is being dropped or has dropped, otherwise false.
     */
    protected boolean isDropping()
    {
        return (m_bIsDropping || m_bWasDropped) ;

    }


    /**
     * Find the connection identified by the specified address.
     *
     * @return The connection identfied by the specified address or null if
     *         not found.
     */
    protected Connection findConnection(PAddress address)
    {
        Connection rc = null ;
        SipParser parser ;
        Connection connections[] = null ;

        synchronized (m_call) {
            connections = m_call.getConnections() ;
        }

        for (int i=0;i<connections.length; i++) {

            // Clean connection address
            String strConnectionAddress = connections[i].getAddress().getName() ;
            parser = new SipParser(strConnectionAddress) ;
            parser.removeAllHeaderParameters() ;
            parser.removeAllURLParameters() ;
            parser.setDisplayName("") ;

            strConnectionAddress = parser.render() ;

            // Clean passed address
            String strPassedAddress = address.getAddress() ;
            parser = new SipParser(strPassedAddress) ;
            parser.removeAllHeaderParameters() ;
            parser.removeAllURLParameters() ;
            parser.setDisplayName("") ;
            strPassedAddress = parser.render() ;

            if (strConnectionAddress.equalsIgnoreCase(strPassedAddress)) {
                rc = connections[i] ;
                break ;
            }
        }
        return rc ;
    }


    /**
     * returns null if no displayname is provided.
     */
    private String getUserLabel(String strSIPURL ){
        SipParser parser = new SipParser(strSIPURL) ;
        return  (parser.getDisplayName()) ;
    }


    /**
     * returns true if the user label equals "anonymous"
     */
    private  boolean isUserLabelAnonymous( PAddress address){
        boolean bRet = false;
        String strLabel = getUserLabel(  address.getAddress() );
        if( strLabel != null ) {
            if( strLabel.equalsIgnoreCase(ANONYMOUS_DISPLAY_NAME) )
                bRet = true;
        }
        //debug("user label is " + strLabel );
        return bRet;
    }


    /**
     * Outputs a loud debug string that is more noticeable through the
     * console.
     */
    private void debug( String str )
    {
        if( m_sCallDebug )
        {
            StringBuffer buffer = new StringBuffer();
            buffer.append("\n");
            buffer.append("*******************************************");
            buffer.append("\n");
            buffer.append("PCall: DEBUG -->" + str);
            buffer.append("\n");
            buffer.append("*******************************************");
            buffer.append("\n");
            System.out.print(buffer.toString());
        }
    }


    /**
     * Set the DND flag for this call.  If a call is marked as in DND, then the
     * system tends to ignore it and will not allow it to interact with the user.
     * The call must also be dropped() whenever the call disconnects.
     */
    protected void setDND(boolean bDND)
    {
        m_bDNDing = bDND ;
    }


    /**
     * Is the DND flag set for this call?  If a call is marked as in DND, then
     * the system tends to ignore it and will not allow the it to interact
     * with the user.
     */
    protected boolean isDNDSet()
    {
        return m_bDNDing ;
    }


   /**
     * Helper routine that search through all of the connections/Terminal
     * connections for terminal connections that are not held.  If any are
     * found, the routine returns false, otherwise the routine returns
     * true.
     */
    private boolean areRemoteTerminalConnectionsHeld()
    {
        if (m_call != null)
        {
            synchronized (m_call)
            {
                Connection connections[] = m_call.getConnections() ;
                for (int i=0;i<connections.length; i++)
                {
                    TerminalConnection tcs[] = connections[i].getTerminalConnections() ;

                    for (int j=0;j<tcs.length; j++)
                    {
                        if (!((PtTerminalConnection) tcs[j]).isLocal())
                        {
                            if (tcs[j].getState() == CallControlTerminalConnection.HELD)
                            {
                                return true ;
                            }
                        }
                    }
                }
            }
        }
        else
            return false ;

        return false  ;
    }


    /**
     * Helper routine that search through all of the connections/Terminal
     * connections for terminal connections that are unheld.  If any are
     * found, the routine returns false, otherwise the routine returns
     * true.
     */
    private boolean areRemoteTerminalConnectionsUnheld()
    {
        if (m_call != null)
        {
            synchronized (m_call)
            {
                Connection connections[] = m_call.getConnections() ;
                for (int i=0;i<connections.length; i++)
                {
                    TerminalConnection tcs[] = connections[i].getTerminalConnections() ;

                    for (int j=0;j<tcs.length; j++)
                    {
                        if (!((PtTerminalConnection) tcs[j]).isLocal())
                        {
                            if (tcs[j].getState() == CallControlTerminalConnection.TALKING)
                            {
                                return true ;
                            }
                        }
                    }
                }
            }
        }
        else
            return false ;

        return false ;
    }


    /**
     * Helper routine that check to see if the terminal connection identifed
     * by the supplied address is in the HELD state.
     */
    private boolean isRemoteTerminalConnectionHeld(PAddress address)
    {
        boolean bHeld = false ;

        if (m_call != null)
        {
            synchronized (m_call)
            {
                TerminalConnection tc = m_call.getTerminalConnection(address.getAddress()) ;
                if ((tc != null) &&
                        (tc.getState() == CallControlTerminalConnection.HELD))
                {
                    bHeld = true ;
                }
            }
        }
        return bHeld ;
    }


    /**
     * If we are an ixpressa and a demo, then add something to disable the premium sound
     * for this call id.
     */
    private void addPremiumSoundDisabler()
    {
        boolean bFeatureShouldDisable = true;

        if (Shell.getXpressaSettings().getProductID() == XpressaSettings.PRODUCT_INSTANTXPRESSA)
        {
            Vector v = PingerInfo.getInstance().getLicenseFeatures();
            if (v != null)
            {
                Enumeration e = v.elements();
                while (e.hasMoreElements())
                {
                    String featureStr = (String)e.nextElement();
                    featureStr = featureStr.toUpperCase();
                    if (featureStr.equals("COMMERCIAL") ||
                        featureStr.equals("GIPS-IPCM") || featureStr.equals("GIPS-G711" ))
                    {
                        bFeatureShouldDisable = false;
                    }
                }
            }

            if ((m_premiumSoundDisabler == null) && !m_bDisabledPremiumSound && bFeatureShouldDisable)
                m_premiumSoundDisabler = new icPremiumSoundDisabler(DISABLE_PREMIUM_SOUND_SEC * 1000) ;
        }
    }


    /**
     * Clears/Removes the premium sound disabler.  This is invoked if the call is torn
     * down or after actually disabled.
     */
    private void removePremiumSoundDisabler()
    {
        if (m_premiumSoundDisabler != null)
        {
            m_premiumSoundDisabler.clear() ;
            m_premiumSoundDisabler = null ;
        }
    }


    /**
     * Determine if this call is capable for music on hold.
     */
    private boolean isHoldMusicCapable()
    {
        boolean bIsCapable = false ;

        /**
         * Verify:
         *    1) Music on hold URL is configured.
         *    2) This call is not a conference
         *    3) This call is not using an expensive codec (e.g. G729)
         *    4) No other calls are playing hold music.
         *    5) No other conferences exist.
         */
        if ((Shell.getInstance().getMusicOnHoldURL() != null) &&
                (!isConference()))
        {
            if (m_call.getCodecCPUCost() == PtCallControlCall.CODEC_CPU_LOW)
            {
                bIsCapable = true ;

                PCall calls[] = Shell.getCallManager().getAllCalls() ;
                if (calls != null)
                {
                    for (int i=0; i<calls.length; i++)
                    {
                        // ignore this call
                        if (calls[i] == this)
                            continue ;

                        if (calls[i].isPlayingHoldMusic())
                        {
                            bIsCapable = false ;
                            System.out.println("Not playing hold music: Music playing on another call") ;
                            break ;
                        }

                        if (calls[i].isConference())
                        {
                            System.out.println("Not playing hold music: Conference exists") ;
                            bIsCapable = false ;
                            break ;
                        }
                    }
                }
            }
            else
            {
                System.out.println("Not playing hold music: Expensive Codec") ;
            }

        }
        else if (isConference())
        {
            System.out.println("Not playing hold music: Call is Conference") ;
        }

        return bIsCapable ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner Classes
////
    protected class icPremiumSoundDisabler implements PActionListener
    {
        public icPremiumSoundDisabler(int iDelayInSeconds)
        {
            Timer.getInstance().addTimer(iDelayInSeconds, this, null) ;
        }

        public void actionEvent(PActionEvent event)
        {
            if (m_call != null)
                m_call.disablePremiumSound(getCallID()) ;

            m_bDisabledPremiumSound = true ;
            ShellApp.getInstance().getCoreApp().premiumSoundDisabled(getCallID()) ;

            removePremiumSoundDisabler() ;
        }

        public void clear()
        {
            Timer.getInstance().removeTimers(this) ;
        }
    }

    protected class icToneDescriptor
    {
        public int iToneID ;
        public boolean bLocal ;
        public boolean bRemote ;
        public long lStartTime ;
        public PAddress dtmfSource ;

        public icToneDescriptor(int iToneID, boolean bLocal, boolean bRemote)
        {
            this.iToneID = iToneID ;
            this.bLocal = bLocal ;
            this.bRemote = bRemote ;
        }
    }

    /**
     * This call monitor sits around listening for JTAPI events and then
     * converts the JTAPI events into PTAPI state changes.
     */
    protected class icCallMonitor extends PtTerminalConnectionAdapter
    {
        public void connectionOffered(ConnectionEvent event)
        {
            if (isDropping())
                return ;

            if (!((PtConnectionEvent) event).isLocalConnection())
            {
                // refreshConnectionStates() ;
                setState(CONN_STATE_TRYING, event, true);
            }
        }


        public void connectionAlerting(ConnectionEvent event)
        {
            if (isDropping())
                return ;

            /*
             * In JTAPI, the local connection moves between alerting/connected
             * for inbound calls while outbound calls move the actual address.
             *
             * In STAPI, there is no concept of the local connection, so we
             * map those events to CONN_STATE_INBOUND_ALERTING and
             * CONN_STATE_OUTBOUND_ALERTING on the remote connection.
             */

            if (((PtConnectionEvent) event).isLocalConnection())
            {
                // Receiving an inbound connection

                CallControlCall call = (CallControlCall) event.getCall() ;
                Address address = call.getCallingAddress() ;

                setLocalConnectionState(CONN_STATE_INBOUND_ALERTING) ;

                setState(CONN_STATE_INBOUND_ALERTING, generateAddress(address),
                        event.getCause(),
                        ((PtConnectionEvent)event).getResponseCode(),
                        ((PtConnectionEvent)event).getResponseText(),
                        true,
                        false);
            }
            else
            {
                // Processing an outbound connection
                setState(CONN_STATE_OUTBOUND_ALERTING, event, true);
            }
        }


        public void connectionConnected(ConnectionEvent event)
        {
           	if (isDropping())
                return ;

            /*
             * In JTAPI, the local connection moves between alerting/connected
             * for inbound calls while outbound calls move the actual address.
             *
             * In STAPI, there is no concept of the local connection, so we
             * map those events to a remove address.
             */

            if (((PtConnectionEvent) event).isLocalConnection())
            {
                // Only fire off this event to STAPI if we have a calling adress
                // and that calling address is not local and established
                CallControlCall call = (CallControlCall) event.getCall() ;
                Address address = call.getCallingAddress() ;

                // HACK: When adding a conference participant, the local
                //       is moved to established (again).  This is bad,
                //       because the local connection should only move to
                //       established when an inbound call is answered.  The
                //       following code hacks around this by comparing the
                //       addresses.

                String strCallingAddress = SipParser.cleanAddress(
                        generateAddress(address), true) ;
                String strLocalAddress = SipParser.cleanAddress(
                        generateAddress(event.getConnection().getAddress()), true) ;

                if (!strCallingAddress.equalsIgnoreCase(strLocalAddress))
                {
                    if (getLocalConnectionState() != CONN_STATE_UNKNOWN)
                    {
                        setState(CONN_STATE_CONNECTED, generateAddress(address),
                            event.getCause(),
                            ((PtConnectionEvent)event).getResponseCode(),
                            ((PtConnectionEvent)event).getResponseText(),
                            true,
                            false);

                    }
                }

                PAddress addresses[] = getParticipants() ;
                if ((addresses.length == 0) && !m_bWasEstablishedCall)
                {
                    m_bWasOutboundCall = true ;
                }
				setLocalConnectionState(CONN_STATE_CONNECTED) ;
            }
            else
            {
                // Only fire off this event if the local connection is established
                int iLocalState = getLocalConnectionState() ;

                if ((iLocalState == CONN_STATE_CONNECTED) ||
                        (iLocalState == CONN_STATE_HELD))
                {
                    setState(CONN_STATE_CONNECTED, event, true);
                }
            }
        }


        public void connectionDisconnected(ConnectionEvent event)
        {
            if (isDropping())
                return ;

            if (((PtConnectionEvent) event).isLocalConnection())
            {
				setLocalConnectionState(CONN_STATE_DISCONNECTED) ;
                setState( CONN_STATE_DISCONNECTED, event, false);
            }
            else
            {
                setState( CONN_STATE_DISCONNECTED, event, true);
            }
        }


        public void connectionFailed(ConnectionEvent event)
        {
            if (isDropping())
                return ;

            if (((PtConnectionEvent) event).isLocalConnection())
            {
				setLocalConnectionState(CONN_STATE_FAILED) ;
                setState( CONN_STATE_FAILED, event, false);
            }
            else
            {
                setState( CONN_STATE_FAILED, event, true);
            }
        }


        public void terminalConnectionHeld(TerminalConnectionEvent event)
        {
            if (isDropping())
                return ;

            if (((PtTerminalConnectionEvent) event).isLocalTerminalConnection())
            {
                m_datePlacedOnHold = Calendar.getInstance().getTime() ;
                setState(CONN_STATE_HELD, null, event.getCause(), false, true) ;
				setLocalConnectionState(CONN_STATE_HELD) ;
            }
            else
            {
                // We should do something with this?
            }
        }


        public void terminalConnectionTalking(TerminalConnectionEvent event)
        {
            if (isDropping())
                return ;


            if (((PtTerminalConnectionEvent) event).isLocalTerminalConnection())
            {
                PAddress addressID = null ;
                boolean bWasOnHold = (m_datePlacedOnHold != null) ;
                m_datePlacedOnHold = null ;

                if (bWasOnHold)
                {
                    setState(CONN_STATE_CONNECTED, getLocalAddress(), event.getCause(), false, true) ;
					setLocalConnectionState(CONN_STATE_CONNECTED) ;
                }
            }
            else
            {
                // We should do something more with this??  Like fire it off?
            }
        }

        public void callInvalid(CallEvent event)
        {
            debug("Call Invalid called for callid" + getCallID());
            cleanCall();
        }
    }


    /**
     * Debug class that dumps all all of the connection state info.
     */
    protected class icConnectionStateDebugger implements PConnectionListener
    {
        public void callCreated(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("callCreated") ;
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString()) ;
        }


        public void callDestroyed(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("callDestroyed") ;
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString()) ;
        }


        public void connectionTrying(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionTrying");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionOutboundAlerting(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionOutboundAlerting");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionInboundAlerting(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionInboundAlerting");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionConnected(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionConnected");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionFailed(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionFailed");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionUnknown(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionUnknown");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void connectionDisconnected(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("connectionDisconnected");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void callHeld(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("callHeld");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }

        public void callReleased(PConnectionEvent event)
        {
          StringBuffer buffer = new StringBuffer();
          buffer.append("\n");
          buffer.append("callReleased");
          buffer.append("\n");
          buffer.append(event);
          buffer.append("\n");
          System.out.println(buffer.toString());
        }
    }

    protected class icRecordThread extends Thread
    {
        String mFileName;
        String mCallID;
        int mRecordTime;
        int mSilenceLength;
        public icRecordThread(String fileName, String strCallID, int msRecordTime,int silenceLength)
        {
            mFileName = fileName;
            mCallID = strCallID;
            mRecordTime = msRecordTime;
            mSilenceLength = silenceLength;

        }

        public void run()
        {
             startRecording(mFileName, mCallID, mRecordTime,mSilenceLength);
             m_bWasRecording = true;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Native Methods
////

    protected static native void addDtmfListener(String strCallId,
                                                 Object objListener,
                                                 long lhashCode) ;

   protected static native void startRecording(String fileName,
                                                   String strCallId,
                                                   int msLength,
                                                   int msSilenceLength);
   protected static native void stopRecording(String callID);
}
