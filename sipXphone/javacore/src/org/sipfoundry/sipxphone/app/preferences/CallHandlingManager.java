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

package org.sipfoundry.sipxphone.app.preferences ;

import javax.telephony.* ;
import javax.telephony.callcontrol.* ;
import java.util.Vector;
import org.sipfoundry.telephony.callcontrol.* ;


import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.hook.*;
import org.sipfoundry.sipxphone.sys.HookManager;
import org.sipfoundry.sipxphone.sys.Shell;
/**
 *  Call Handling Manager is the container class for the individual
 *  call handling preferences that can be set.
 */
public class CallHandlingManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public final static int RINGER_BOTH = 0 ;
    public final static int RINGER_AUDIBLE = 1 ;
    public final static int RINGER_VISUAL = 2 ;


    public static final String FORWARD_ON_NO_ANSWER = "FORWARD_ON_NO_ANSWER";
    public static final String FORWARD              = "FORWARD";
    public static final String BUSY                 = "BUSY";
    public static final String RING                 = "RING";
    public static final String ENABLE               = "ENABLE";
    public static final String DISABLE              = "DISABLE";


    /** If set, the phone will instantly reject the incoming call as busy.
        A proxy server or something could then do something more interesting-
        such as forwarding the call to voice mail.*/
    public final static int DND_SEND_BUSY = 0 ;
    /** If set, the phone will instantly redirect the incoming call to the
        user's forward on busy target.  If no forwarding target is set, then
        the phone will reject the incoming call as busy. */
    public final static int DND_FORWARD_ON_BUSY = 1 ;
    /** If set, the phone will instantly redirect the incoming call to the
        user's forward on answer target.  If no forwarding target is set, then
        the phone will ring for ever.  This is interesting if the user has
        different voice mail message for "I'm on the phone" vs "I'm away
        from my desk". */
    public final static int DND_FORWARD_ON_NO_ANSWER = 2 ;

    /** default  value of phoneset_no_answer_timoeut */
    public static final int  PHONESET_NO_ANSWER_TIMEOUT_DEFAULT = 24;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    //private CallControlAddress m_address ;

    /**
     * Destination for forwarding all incoming calls
     *  This is the value the user entered.
     */
    private String m_strSipForwardingAll ;

    /**
     * Destination for forwarding all busy calls
     *  This is the value the user entered.
     */
    private String m_strSipForwardingBusy ;

    /**
     * Destination for forwarding call when no answer
     *  This is the value the user entered.
     */
    private String m_strSipForwardingNoAnswer ;


    /**
     * Destination for forwarding all incoming calls.
     * If it is a digit-mapped number, this address is the
     * translated address.
     */
    private String m_strTranslatedSipForwardingAll ;

    /**
     * Destination for forwarding all busy calls.
     * If it is a digit-mapped number, this address is the
     * translated address.
     */
    private String m_strTranslatedSipForwardingBusy ;

    /**
     * Destination for forwarding call when no answer.
     * If it is a digit-mapped number, this address is the
     * translated address.
     */
    private String m_strTranslatedSipForwardingNoAnswer ;


    /** value of PHONESET_AVAILABLE_BEHAVIOUR */
    private String m_strPhonesetAvailableBehavior  ;
    /** value of PHONESET_BUSY_BEHAVIOUR */
    private String m_strPhonesetBusyBehavior;
    /** Is forward unconditional enabled? default is false. */
    private boolean m_bUnconditionalForwarding;

    /** Is call waiting enabled? */
    private boolean m_bCallWaiting ;
    /** are visual call indications enabled? */
    private int m_iCallIndication ;
    /** Should we put the phone in DND mode?  If so, the user will not
        receive any calls. */
    private boolean m_bDoNotDisturb ;
    /** The DND method (see DND_* constants above */
    private int     m_iDNDMethod ;

    private int  m_iPhonesetNoAnswerTimeout;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * For each address, determine the current call handling preferences
     */
    public CallHandlingManager()
    {
        PingerConfig config = PingerConfig.getInstance() ;
        Terminal terminal = ShellApp.getInstance().getTerminal() ;

        /*
        if (terminal != null) {
            Address addresses[] = ShellApp.getInstance().getTerminal().getAddresses() ;
            m_address = (CallControlAddress) addresses[0] ;
        } else
            m_address = null ;
        */

        // Initialize Unconditional Call Forwarding behavior
        String strUnconditionalForwarding =
                config.getValue(PingerConfig.PHONESET_FORWARD_UNCONDITIONAL) ;
        if ((strUnconditionalForwarding != null) &&
            strUnconditionalForwarding.equalsIgnoreCase(ENABLE))
            m_bUnconditionalForwarding = true ;
        else
            m_bUnconditionalForwarding = false ;

        m_strSipForwardingAll = config.getValue(PingerConfig.SIP_FORWARD_UNCONDITIONAL) ;
        m_strTranslatedSipForwardingAll= getTranslatedAddressIfAny( m_strSipForwardingAll );
        //all other  only comes into scene if Unconditional Call Forwarding
        //behavior is disabled.

        //Initializing Phoneset available behaviour
        m_strPhonesetAvailableBehavior =
            config.getValue(PingerConfig.PHONESET_AVAILABLE_BEHAVIOR) ;
        m_strSipForwardingNoAnswer = config.getValue(PingerConfig.SIP_FORWARD_ON_NO_ANSWER) ;
        m_strTranslatedSipForwardingNoAnswer= getTranslatedAddressIfAny( m_strSipForwardingNoAnswer );


        // For busy, the BUSY_BEHAVIOR must be FORWARD, otherwise CLEAR
        // For busy, the SIP_FORMWARD_ON_BUSY must be set
        m_strPhonesetBusyBehavior = config.getValue(PingerConfig.PHONESET_BUSY_BEHAVIOR) ;
        m_strSipForwardingBusy = config.getValue(PingerConfig.SIP_FORWARD_ON_BUSY) ;
        m_strTranslatedSipForwardingBusy = getTranslatedAddressIfAny( m_strSipForwardingBusy );


        // Initialize Call Waiting behavior
        String strCallWaiting = config.getValue(PingerConfig.PHONESET_CALL_WAITING_BEHAVIOR) ;
        if ((strCallWaiting != null) && strCallWaiting.equalsIgnoreCase(BUSY))
            m_bCallWaiting = false ;
        else
            m_bCallWaiting = true ;



        // Initialize Call Indication
        String strRinger = config.getValue(PingerConfig.PHONESET_RINGER) ;
        m_iCallIndication = RINGER_BOTH ;
        if (strRinger != null) {
            if (strRinger.equalsIgnoreCase("VISUAL")) {
                m_iCallIndication = RINGER_VISUAL ;
            } else if (strRinger.equalsIgnoreCase("AUDIBLE")) {
                m_iCallIndication = RINGER_AUDIBLE ;
            } else {
                m_iCallIndication = RINGER_BOTH ;
            }
        }

        //get the value of phoneset_no_answer_timeout.
        String strPhonesetNoAnswerTimeout =
            config.getValue(PingerConfig.PHONESET_NO_ANSWER_TIMEOUT);
         m_iPhonesetNoAnswerTimeout =  PHONESET_NO_ANSWER_TIMEOUT_DEFAULT;
         if( ! TextUtils.isNullOrSpace(strPhonesetNoAnswerTimeout)  ){
            try{
              m_iPhonesetNoAnswerTimeout =
                Integer.parseInt(strPhonesetNoAnswerTimeout);
            }catch( NumberFormatException e ){
               SysLog.log(e);
            }
         }

        // Initialize the DND Setting
        String strDNDSetting = config.getValue(PingerConfig.PHONESET_DND) ;
        if ((strDNDSetting != null) && strDNDSetting.equalsIgnoreCase(ENABLE))
            m_bDoNotDisturb = true ;
        else
            m_bDoNotDisturb = false ;

        // Initialize DND Method
        String strDNDMethod = config.getValue(PingerConfig.PHONESET_DND_METHOD) ;
        if (strDNDMethod != null) {
            if (strDNDMethod.equalsIgnoreCase("FORWARD_ON_BUSY")) {
                m_iDNDMethod = DND_FORWARD_ON_BUSY ;
            } else if (strDNDMethod.equalsIgnoreCase(FORWARD_ON_NO_ANSWER)) {
                m_iDNDMethod = DND_FORWARD_ON_NO_ANSWER ;
            } else {
                m_iDNDMethod = DND_SEND_BUSY ;
            }
        } else{
            m_iDNDMethod = DND_SEND_BUSY ;
        }
        dump() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     *
     */
    public void setCallIndication(int iIndication)
    {
        m_iCallIndication = iIndication ;

        switch (m_iCallIndication) {
            case RINGER_AUDIBLE:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_RINGER, "AUDIBLE") ;
                break ;
            case RINGER_VISUAL:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_RINGER, "VISUAL") ;
                break ;
            default:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_RINGER, "BOTH") ;
                break ;
        }
    }


    /**
     *
     */
    public void setCallWaiting(boolean bEnable)
    {
        m_bCallWaiting = bEnable ;

        if (m_bCallWaiting){
            PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                            PingerConfig.PHONESET_CALL_WAITING_BEHAVIOR,
                                            "ALERT") ;
        }else{
            PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                            PingerConfig.PHONESET_CALL_WAITING_BEHAVIOR,
                                            "BUSY") ;
        }
    }

    /**
     * is call waiting enabled?
     */
    public boolean  isCallWaitingEnabled()
    {
        return m_bCallWaiting ;
    }

    /**
     * get the call indication
     */
    public int getCallIndication()
    {
        return m_iCallIndication ;
    }



    /**
     * Get the forward all address
     */
    public String getSipForwardingAll()
    {
        return m_strSipForwardingAll ;
    }

    /**
     * Get the translated forward all address
     */
    public String getTranslatedSipForwardingAll()
    {
        return m_strTranslatedSipForwardingAll ;
    }

    /**
     * is forwarding all enabled?
     */
    public boolean isForwardingAllEnabled(){
        return m_bUnconditionalForwarding;
    }

    /**
     * is forward_on_no_answer enabled?
     */
    public boolean isForwardOnNoAnswerEnabled(){
        if(( m_strPhonesetAvailableBehavior != null ) &&
            m_strPhonesetAvailableBehavior.equals(FORWARD_ON_NO_ANSWER))
            return true;
         else
            return false;
    }

    /**
     * is forward_on_busy enabled?
     */
    public boolean isForwardOnBusyEnabled(){
        if(( m_strPhonesetBusyBehavior != null ) &&
            m_strPhonesetBusyBehavior.equals(FORWARD))
            return true;
         else
            return false;
    }

    /**
     * Get the forward on busy address
     */
    public String getSipForwardingBusy()
    {
        return m_strSipForwardingBusy ;
    }

    /**
     * Get the translated forward on busy address
     */
    public String getTranslatedSipForwardingBusy()
    {
        return m_strTranslatedSipForwardingBusy ;
    }


    /**
     * Get the forward on no answer address
     */
    public String getSipForwardingNoAnswer()
    {
        return m_strSipForwardingNoAnswer ;
    }

    /**
     * Get the forward on no answer address
     */
    public String getTranslatedSipForwardingNoAnswer()
    {
        return m_strTranslatedSipForwardingNoAnswer ;
    }

    /**
     * Get the PhonesetAvailableBehavior
     */
    public String getPhonesetAvailableBehavior()
    {
        return m_strPhonesetAvailableBehavior ;
    }


    /**
     * Get the forward on no answer address
     */
    public String getPhonesetBusyBehavior()
    {
        return m_strPhonesetBusyBehavior ;
    }

    /**
     * gets the PHONESET_NO_ANSWER_TIMEOUT value
     */
    public int getPhonesetNoAnswerTimeout(){
       return m_iPhonesetNoAnswerTimeout;
    }

    /**
     * Set the forward all address.
     * It sets the SIP_FORWARD_UNCONDITIONAL value to be used
     * to forward all inbound calls if PHONESET_FORWARD_UNCONDITIONAL
     * is enabled.
     */
    public synchronized void setSipForwardingAll(String strDest)
    {
        // Turn blank strings into nulls
        if ((strDest != null) && (strDest.trim().length() == 0))
            strDest = null ;

        m_strSipForwardingAll = strDest;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.SIP_FORWARD_UNCONDITIONAL,
                                        m_strSipForwardingAll) ;
        m_strTranslatedSipForwardingAll= getTranslatedAddressIfAny( m_strSipForwardingAll );

    }

    /**
     * sets the PHONESET_FORWARD_UNCONDITIONAL behaviour to
     * enabled or disabled. If it is enabled, it will override all other
     * call handling settings and forward all inbound calls to the
     * SIP_FORWARD_UNCONDITIONAL address.
     */
    public synchronized void setPhonesetForwardingAll(boolean bEnabled)
    {
        String  strPhonesetForwardingAll = DISABLE;
        if( bEnabled )
        {
            strPhonesetForwardingAll = ENABLE;
        }

        m_bUnconditionalForwarding = bEnabled ;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.PHONESET_FORWARD_UNCONDITIONAL,
                                        strPhonesetForwardingAll) ;
    }


    /**
     * Set the forward on busy address to be used by
     * if PHONESET_BUSY_BEHAVIOR is set to BUSY.
     */
    public synchronized void setSipForwardingBusy(String strDest)
    {
        // Turn blank strings into nulls
        if ((strDest != null) && (strDest.trim().length() == 0))
            strDest = null ;

        m_strSipForwardingBusy = strDest;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.SIP_FORWARD_ON_BUSY,
                                        m_strSipForwardingBusy) ;
        m_strTranslatedSipForwardingBusy= getTranslatedAddressIfAny( m_strSipForwardingBusy );

    }

    /**
     * sets the PHONESET_BUSY_BEHAVIOR to BUSY , QUEUE or FORWARD.
     * @throws IllegalArgumentException if strBehavior is null or
     * not one of "BUSY", "QUEUE" or "FORWARD".
     */
    public synchronized void setPhonesetBusyBehavior(String strBehavior)
      throws IllegalArgumentException{

         if( ( strBehavior == null ) ||
             !(( strBehavior.equals(BUSY)) ||
               ( strBehavior.equals("QUEUE")) ||
               ( strBehavior.equals(FORWARD))  )  ){
                throw new IllegalArgumentException
                    ("PHONESET_BUSY_BEHAVIOUR can be only BUSY, QUEUE or FORWARD");
         }
        m_strPhonesetBusyBehavior = strBehavior;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.PHONESET_BUSY_BEHAVIOR,
                                        strBehavior) ;

    }

    /**
     *Set the forward on no answer address.
     *It sets the value of PHONESET_NO_ANSWER_TIMEOUT
     *to PHONESET_NO_ANSWER_TIMEOUT_DEFAULT.
     *
     */
    public synchronized void setSipForwardingNoAnswer(String strDest)
    {
        //set the default to 24.
        setSipForwardingNoAnswer( strDest,  PHONESET_NO_ANSWER_TIMEOUT_DEFAULT );
    }


    /**
     * Set the forward on no answer address
     * and the PHONESET_NO_ANSWER_TIMEOUT key.
     */
    public synchronized void setSipForwardingNoAnswer
              (String strDest, int iPhonesetNoAnswerTimeout)
    {
        // Turn blank strings into nulls
        if ((strDest != null) && (strDest.trim().length() == 0))
            strDest = null ;

        m_strSipForwardingNoAnswer = strDest ;
        PingerConfigFileParser.setValue
          (PingerConfig.USER_CONFIG,
            PingerConfig.SIP_FORWARD_ON_NO_ANSWER, m_strSipForwardingNoAnswer) ;
        m_strTranslatedSipForwardingNoAnswer= getTranslatedAddressIfAny( m_strSipForwardingNoAnswer );


        m_iPhonesetNoAnswerTimeout = iPhonesetNoAnswerTimeout;
        PingerConfig config = PingerConfig.getInstance() ;
        PingerConfigFileParser.setValue
          (PingerConfig.USER_CONFIG,
            PingerConfig.PHONESET_NO_ANSWER_TIMEOUT, ""+iPhonesetNoAnswerTimeout) ;


    }

    /**
     * sets the PHONESET_AVAILABLE_BEHAVIOR to FORWARD_ON_NO_ANSWER or RING.
     * @throws IllegalArgumentException if strBehavior is null or
     * not one of "FORWARD_ON_NO_ANSWER" or "RING". "FORWARD" is treated
     * as "FORWARD_ON_NO_ANSWER" for backward compatibility reasons.
     */
    public synchronized void setPhonesetAvailableBehavior(String strBehavior)
        throws IllegalArgumentException{

         if( ( strBehavior == null ) ||
             !(( strBehavior.equals(FORWARD_ON_NO_ANSWER)) ||
               ( strBehavior.equals(RING)) ||
               ( strBehavior.equals(FORWARD))  )  ){
                throw new IllegalArgumentException
                    ("PHONESET_AVAILABLE_BEHAVIOUR can be only FORWARD_ON_NO_ANSWER or RING ");
         }
         m_strPhonesetAvailableBehavior = strBehavior;
        //FORWARD is deprecated now.
        if( m_strPhonesetAvailableBehavior.equals(FORWARD))
            m_strPhonesetAvailableBehavior = FORWARD_ON_NO_ANSWER;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.PHONESET_AVAILABLE_BEHAVIOR,
                                        strBehavior) ;


    }



    /**
     * This method primes the phone with the various SIP_FORWARDing rules and
     * whatnot.  This method should be called upon phone start up after the
     * JTAPI world has had a change to initialize.
     */
    public synchronized void initialize()
    {
        PingerConfig config = PingerConfig.getInstance() ;

        String strForwardUnconditional =
                    config.getValue(PingerConfig.SIP_FORWARD_UNCONDITIONAL) ;
        String strForwardOnBusy =
                    config.getValue(PingerConfig.SIP_FORWARD_ON_BUSY) ;
        String strForwardOnNoAnswer =
                    config.getValue(PingerConfig.SIP_FORWARD_ON_NO_ANSWER) ;

        // Set Unconditional forwarding
        if ((strForwardUnconditional != null) && (strForwardUnconditional.length() > 0))
            setSipForwardingAll(strForwardUnconditional) ;

        if ((strForwardOnBusy != null) && (strForwardOnBusy.length() > 0)) {
            setSipForwardingBusy(strForwardOnBusy) ;
        }

        if ((strForwardOnNoAnswer != null) && (strForwardOnNoAnswer.length() > 0))
            setSipForwardingNoAnswer(strForwardOnNoAnswer) ;

        dump() ;
    }


    /**
     *
     */
     /*
    public String getJTAPIForwarding(int iType)
    {
        String strRC = null ;

        try {
            // Get our JTAPI Call Forwarding settings
            CallControlForwarding rules[] = m_address.getForwarding() ;

            for (int i=0; i<rules.length; i++) {
                if (rules[i].getType() == iType) {
                    // If found, set internal data
                    strRC = rules[i].getDestinationAddress() +" timeout: "+
                        ((PtCallControlForwarding)(rules[i])).getNoAnswerTimeout();
                }
            }
        } catch (MethodNotSupportedException e) {
            System.out.println("CallHandlingManager:: method not supported") ;
            SysLog.log(e) ;
        } catch (Exception e) {
            System.out.println("CallHandlingManager:: exception caught") ;
            SysLog.log(e) ;
        }
        return strRC ;
    }
    */



    /**
     * Enable the do not disturb feature of our phone.  If enabled, the phone
     * will stop alerting the user in mechanism/mannor described by getDNDMethod.
     */
    public void enableDND(boolean bEnable)
    {
        m_bDoNotDisturb = bEnable ;
        PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG,
                                        PingerConfig.PHONESET_DND,
                                        m_bDoNotDisturb ? "ENABLE" : "DISABLE") ;
    }


    /**
     * Is DND Enabled?
     */
    public boolean isDNDEnabled()
    {
        return m_bDoNotDisturb ;
    }


    /**
     * Set the method of DND.  Please see the DND_* constants listed above.
     */
    public void setDNDMethod(int iMethod)
    {
        switch (iMethod) {
            case DND_SEND_BUSY:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_DND_METHOD, "SEND_BUSY") ;
                break ;
            case DND_FORWARD_ON_NO_ANSWER:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_DND_METHOD, "FORWARD_ON_NO_ANSWER") ;
                break ;
            case DND_FORWARD_ON_BUSY:
                PingerConfigFileParser.setValue(PingerConfig.USER_CONFIG, PingerConfig.PHONESET_DND_METHOD, "FORWARD_ON_BUSY") ;
                break ;
            default:
                throw new IllegalArgumentException("Invalid DND method") ;
        }

    }


    /**
     * Get the method of DND.
     */
    public int getDNDMethod()
    {
        return m_iDNDMethod ;
    }



    public void dump()
    {
        // Dump Debugging Information
        StringBuffer dumpBuffer = new StringBuffer();
        dumpBuffer.append("\n") ;
        dumpBuffer.append("Call Handling Settings:") ;
        dumpBuffer.append("\n") ;

        dumpBuffer.append("  Unconditional Forwarding                   : " +( isForwardingAllEnabled() ? "*Enabled*" : "Disabled")) ;
        dumpBuffer.append("\n") ;

        dumpBuffer.append("  Unconditional Forwarding address if enabled: " + m_strSipForwardingAll ) ;
        dumpBuffer.append("\n\n") ;

        dumpBuffer.append("  On Busy Forwarding                         : " +( isForwardOnBusyEnabled() ? "*Enabled*" : "Disabled")) ;
        dumpBuffer.append("\n") ;

        dumpBuffer.append("  On Busy Forwarding address if enabled      : " + m_strSipForwardingBusy ) ;
        dumpBuffer.append("\n\n") ;

        dumpBuffer.append("  On No Answer Forwarding                    : " +( isForwardOnNoAnswerEnabled() ? "*Enabled*" : "Disabled")) ;
        dumpBuffer.append("\n") ;

        dumpBuffer.append("  On No Answer Forwarding address if enabled : " +m_strSipForwardingNoAnswer) ;
        dumpBuffer.append("\n\n") ;

        dumpBuffer.append("  Call Waiting                               : " +( isCallWaitingEnabled() ? "*Enabled*" : "*Disabled*") ) ;
        dumpBuffer.append("\n") ;
        String strDNDMethod = "SEND_BUSY" ;
        switch (m_iDNDMethod) {
            case DND_FORWARD_ON_NO_ANSWER:
                strDNDMethod = "FORWARD_ON_NO_ANSWER" ;
                break ;
            case DND_FORWARD_ON_BUSY:
                strDNDMethod = "FORWARD_ON_BUSY" ;
                break ;
        }
        dumpBuffer.append("            Do Not Disturb                   : " +
                ((m_bDoNotDisturb == false) ? "*Disabled*" : strDNDMethod)) ;
        dumpBuffer.append("\n") ;

        String strRinger = "BOTH" ;
        switch (m_iCallIndication) {
            case RINGER_AUDIBLE:
                strRinger = "AUDIBLE" ;
                break ;
            case RINGER_VISUAL:
                strRinger = "VISUAL" ;
                break ;
            default:
                strRinger = "BOTH" ;
                break ;
        }
        dumpBuffer.append("              Ringer                         : " + strRinger) ;
        dumpBuffer.append("\n") ;
        System.out.println(dumpBuffer.toString());
    }

    //////////////////////////////////////////////////////////////////////////////
// Implementation
////


    /**
     * Gets the digit-mapped address if nay for the givebn string. So for example,
     * if there is a digit map set as 123 : sip:10.1.1.194, passing 123 will return
     * you sip:10.1.1.194 .
     */
    private String getTranslatedAddressIfAny(String strDigitMap ){
        String strRet = strDigitMap;
        if( strDigitMap != null ){
            MatchDialplanHookData digitMapdata =
                        new MatchDialplanHookData( strDigitMap ) ;
            Shell.getHookManager().executeHook(HookManager.HOOK_MATCH_DIALPLAN, digitMapdata) ;
            if(  digitMapdata.getMatchState() == MatchDialplanHookData.MATCH_SUCCESS )
                strRet = digitMapdata.getAddress();
       }
       return strRet;
    }


    private static final boolean m_sDebug = false;
    private void debug( String str )
    {
         if( m_sDebug )
          System.out.println("---------------CallHandlingManager: DEBUG -->"+str );
    }

}

