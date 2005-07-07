/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/DialogImpl.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.siplite ;

import java.util.* ;

import javax.siplite.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sip.*;
import org.sipfoundry.sip.event.*;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * This class is responsible for creating, sending and managing messages
 * that have the same From, To and call id values. The application developer
 * does not need to be concerned with the call id, this is implemented by the
 * Dialog object. The Dialog creates both requests and responses. To create
 * a request<br>
 * <blockquote><code> createRequest(method, contentType, payload)</blockquote>
 * </code>
 * Message creation is very simple. The Dialog object has the knowledge
 * concerning the originator and destination of the message. Therefore,
 * creation of a request message only requires parameters concerning the
 * type of request and the payload.<br>
 * To create a response    <br>
 *  <blockquote><code> createResponse(incomingRequest, statusCode,
 * reason, contentType, payload)</blockquote></code>
 * The incoming request provides the callid, from and to addresses for the
 * request response.
 */

public class DialogImpl implements Dialog, NewSipMessageListener
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** SIP VERSION STRING */
    protected static String SIP_VERSION_STRING="SIP/2.0";

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** local address of this call leg */
    Address m_localAddress;

    /** remote address of this call leg.*/
    Address m_remoteAddress;

    /** CallImpl associatd with this dialog.*/
    CallImpl m_call;

    /** when the transaction is supposed to expire  */
    private long m_iTimeToExpire = -1;

    /** calllegid of this call.*/
    String  m_dialogID;

    /** vector to store the call leg listeneners that are added to this callleg.*/
    private Vector m_vListenerList = new Vector();

    /** counter used for creating cseq number */
    private static int m_sCSeqCounter = 1;

    /** filter criteria used for listening to incoming calllegs. */
    private SipMessageFilterCriteria m_criteria;

    /** last outgoing message */
    private SIPMessage lastOutgoingMessage;

    /** vector to hold outgoing requests and incoming responses */
    private Vector  m_vMessages = new Vector();


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * constructor. Takes a localAddress, remoteAddress and the call
     * that was used to create this dialog.
     */
    public DialogImpl( Address localAddress, Address remoteAddress, Call call){
        m_localAddress = localAddress;
        m_remoteAddress = remoteAddress;
        m_call = (CallImpl)call;
        //Util.debug(    "checking for seesion incoming: "+((CallImpl)(call)).getCallID() +"  "+
        //                   localAddress     +"  "+
        //                   remoteAddress             );
        SipSession session = new SipSession(call.getCallId(),
                remoteAddress.toString(), localAddress.toString());
        m_criteria = new SipMessageFilterCriteria(session) ;

        m_dialogID = localAddress.toString() + "-" + call.getCallId() + "-" + remoteAddress.toString() ;
    }

    /**
     * Get the time this transaction will expire
     */
    public long getExpireTime()
    {
        return m_iTimeToExpire;
    }

    /**
     * Set the time this transaction will expire
     * @param    nExpireTime   time in ms
     */
    public void setExpireTime(long nExpireTime)
    {
        m_iTimeToExpire = nExpireTime;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Create a request message.
     * @param    method        The type of message that is being sent,
     *                         for example, an INVITE message.
     * @param    contentType   A string which identifies the payload type
     * @param    payload       The payload
     * @return   A SIPMessage that is ready to send or edit.
     * @throws   InvalidArgumentException	If an argument value is
     * 						incorrect and thus the message
     * 						cannot be created
     */
    public SIPMessage createRequest(String method, String contentType,
                            byte[] payload) throws InvalidArgumentException
    {
       //should be changed.

        SIPMessageImpl sipMessage = new SIPMessageImpl();
        if( method == null ){
            throw new InvalidArgumentException("method cannot be null");
        }
        if( ! SIPPermissions.getInstance().isMethodPermitted( method )){
            throw new InvalidArgumentException
                    ("method "+ method +" is not permitted to be used when making a request");
        }
        boolean bForce = true;
        sipMessage.setStartLine(method+" "+m_remoteAddress+" "+SIP_VERSION_STRING);
        sipMessage.addHeader(new SIPHeaderImpl("Content-Type", contentType), bForce);
        sipMessage.addHeader(new SIPHeaderImpl("Cseq", getNewCSeqNumber(m_remoteAddress.toString())+" "+ method), bForce);
        sipMessage.addHeader(new SIPHeaderImpl("From", m_localAddress.toString()), bForce);
        sipMessage.addHeader(new SIPHeaderImpl("To", m_remoteAddress.toString()), bForce);

        //generate a call leg id and set the value.
        sipMessage.addHeader(new SIPHeaderImpl("Call-Id", m_call.getCallId()) , bForce);
        sipMessage.setPayload( payload );
        return sipMessage;
    }


    /**
     * Create a Register request.  The From and To addresses set up
     * for the SIPMessage  will be taken from the Dialog. The
     * <code>registrar</code> identifies the  destination of the message.
     * @return   A SIPMessage ready to send or edit
     * @throws   InvalidArgumentException	If an argument value is
     * 						incorrect and thus the message
     * 						cannot be created
     */
    public SIPMessage createRegisterRequest( Address registrar )
        throws InvalidArgumentException
    {
        /* In section 10.2 of draft-ietf-sip-rfc2543bis-08.txt,
         * its stated that:
         *       The Request-URI names the domain of the location
         *       service for which the registration is meant (for example,
         *       "sip:chicago.com"). The "userinfo" and "@" components of
         *       the SIP URI MUST NOT be present.
         */

        String method = "REGISTER";
        SIPMessageImpl sipMessage =
            (SIPMessageImpl)(createRequest( method, "application/sdp", null));
        sipMessage.setStartLine(method+" "+registrar.getHost()+":"+registrar.getPort()+" "+SIP_VERSION_STRING);
        boolean bForce = true;
        sipMessage.addHeader(new SIPHeaderImpl("Cseq", getNewCSeqNumber(m_remoteAddress.toString())+" "+ method), bForce);

        return sipMessage;
    }

    /**
     * Create a response message
     * @param    incomingRequest	The message to which we want to
     *                              respond.
     * @param    statusCode        	The status code of the response.
     * @param    reason             The reason-phrase
     * @param    contentType        A string which identifies the payload type
     * @param    payload            The payload
     * @return   A SIPMessage that can edited and/or sent.
     * @throws   InvalidArgumentException	If an argument value is
     *                                      incorrect and thus the message
     *                                      cannot be created
     **/
    public SIPMessage createResponse( javax.siplite.SIPMessage incomingRequest, int statusCode,
                            String reason, String contentType,
                            byte[] payload) throws InvalidArgumentException
    {

        SipRequest sipRequest = new SipRequest(incomingRequest.toString());
        SipResponse sipResponse = sipRequest.buildResponse( statusCode, reason );

        SIPMessageImpl sipMessage = new SIPMessageImpl();
        sipMessage.setStartLine("SIP/2.0 "+statusCode+" "+reason);
        boolean bForce = true;
        sipMessage.addHeader(new SIPHeaderImpl("Content-Type", contentType),bForce);
        sipMessage.addHeader(new SIPHeaderImpl("From", sipResponse.getHeaderFieldValue("From")),bForce);
        sipMessage.addHeader(new SIPHeaderImpl("To", sipResponse.getHeaderFieldValue("To")),bForce);
        sipMessage.addHeader(new SIPHeaderImpl("Call-Id", sipResponse.getHeaderFieldValue("Call-Id")),bForce);
        sipMessage.addHeader(new SIPHeaderImpl("Cseq", sipResponse.getHeaderFieldValue("Cseq")),bForce);
        sipMessage.setPayload( payload );
        return sipMessage;
    }

    /**
     * Cancelling a sent request.
     * @param   originalRequestMessage  The request message that has
     * 					previously been
     *                                  sent and is to be cancelled.
     * @throws   InvalidArgumentException	If an argument value is
     *						incorrect and the request cannot
     * 						be cancelled
     */
    public void cancelRequest(javax.siplite.SIPMessage originalRequestMessage)
                                throws InvalidArgumentException
    {
        //cancel request is basically constructing a request with CANCEL
        //and posting that request.
        String method = "CANCEL";
        if(! SIPPermissions.getInstance().isMethodPermitted( method )){
            throw new InvalidArgumentException
                    ("method "+ method +" is not permitted to be used when making a request");
        }
        SIPMessageImpl originalRequest = (SIPMessageImpl)originalRequestMessage;
        originalRequest.setStartLine(method+" "+m_remoteAddress+" "+SIP_VERSION_STRING);
        boolean bForce = true;
        originalRequest.addHeader( new SIPHeaderImpl("Cseq", getNewCSeqNumber(m_remoteAddress.toString())+" "+ method),
                                   bForce );
        try{
            sendMessage( originalRequest );
        }catch( MessageNotSentException e ){
            SysLog.log(e);
        }
        for (Enumeration e = m_vListenerList.elements() ; e.hasMoreElements() ;) {
            DialogListener listener = (DialogListener) e.nextElement() ;
            if (listener != null) {
                listener.cancelRequest( originalRequestMessage );
            }
        }

    }

    /**
     * Will match an incoming response with its corresponding outgoing request.
     * @param	incomingResponseMessage	The response that has beend received
     * 					and is to be used to determine the
     *                                  is to be used to determine the
     *					original request
     * @throws	InvalidArgumentException	If an argument value is
     *						incorrect and the Request cannot
     *						be found
     * @return  The original outgoing request message.
     */
    public SIPMessage getOriginalRequest(SIPMessage incomingResponseMessage)
                                throws InvalidArgumentException
    {
        SIPMessage jainRequest = null;
        synchronized( m_vMessages ){
            int index = m_vMessages.indexOf(incomingResponseMessage);
            while( index >= 0 ){
                SIPMessage messageInVector =
                    (SIPMessage)m_vMessages.elementAt( index-- );
                if( ! isResponse(messageInVector)  ){
                    jainRequest = messageInVector;
                    break;
                }
            }
        }
        return jainRequest;
    }

    /**
     * Returns the Dialog's id. This id will not change over the lifetime
     * of the Dialog.
     * @return A string representation of the id
     */
    public String getDialogId()
    {
        return m_dialogID ;
    }

    /**
     * Send the request / response message
     * @param	message	The SIPMessage to be sent
     * @throws	MessageNotSentException	This exception is thrown if the
     *          			message could not be sent
     **/
    public void sendMessage(javax.siplite.SIPMessage message)
                     throws MessageNotSentException
    {
        //converting to org.sipfoundry.sip.SipMessage
        //this gets confusing, probably should copy the JNI methods
        //over here.
        lastOutgoingMessage = message;
        SipMessage sipMessage = new SipMessage(message.toString());
        try{
            //if this is a outgoing request, then add it to the messages holder.
            if( !isResponse(message) )
                m_vMessages.addElement( message );

            SipUserAgent.getInstance().postMessage(sipMessage);
        }catch( Exception e ){
            SysLog.log(e);
        }
    }

    /**
     * Add a DialogListener implementation to be notified of incoming
     * requests or responses for the Dialog. The DialogListener interface
     * is implemented by the application.
     * @param dialogListener	the DialogListener interface implementation
     */
    public void addDialogListener( DialogListener dialogListener )
    {
        synchronized (m_vListenerList) {
            if (!m_vListenerList.contains(dialogListener)) {
                 m_vListenerList.addElement(dialogListener) ;
                 SipUserAgent.getInstance().addIncomingMessageListener(m_criteria, this) ;
            }
        }
    }


    /**
     * Remove a DialogListener implementation that is registered with the
     * Dialog.
     * @param dialogListener	the DialogListener interface implementation
     */
    public void removeDialogListener( DialogListener dialogListener )
    {
        if (m_vListenerList != null) {
            synchronized (m_vListenerList) {
                m_vListenerList.removeElement(dialogListener) ;
                SipUserAgent.getInstance().removeIncomingMessageListener(this) ;
            }
        }
    }

   /**
     * Method returns the local address of the Dialog.
     * @return from local address used by this Dialog
     */
    public Address getLocalAddress()
    {
        return m_localAddress ;
    }


    /**
     * Method returns the remote address of the Dialog.
     * @return from remote address used by this Dialog
     */
    public Address getRemoteAddress()
    {
        return m_remoteAddress ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     *  sets the local addresss of this callleg.
     *  This method should never be public, as we
     *  want only the CallProvider to change the address
     *  in case, the session has added any "tag" in the address.
     */
    protected void setLocalAddress(Address localAddress)
    {
        m_localAddress = localAddress;
    }

    /**
     *  sets the remote addresss of this callleg.
     *  This method should never be public, as we
     *  want only the CallProvider to change the address
     *  in case, the session has added any "tag" in the address.
    */
    protected void setRemoteAddress(Address remoteAddress)
    {
        m_remoteAddress = remoteAddress;
    }

    //implementation of NewSipMessageListener methods
    /**
     * Exposed as a side effect of implementing the NewSipMessageListener
     * interface.
     * @deprecated so that this method doesn't show up in the javadocs.
     */
    public void newMessage(NewSipMessageEvent event)
    {
      try{
        SipMessage incomingMessage =  event.getMessage() ;
        SIPMessageImpl jainIncomingMessage =
                        new SIPMessageImpl(incomingMessage.toString());
        String  strCallID      = jainIncomingMessage.getHeaderValue("Call-Id");
        String  strFromAddress = jainIncomingMessage.getHeaderValue("From");
        String  strToAddress   = jainIncomingMessage.getHeaderValue("To");
        String  method         = jainIncomingMessage.getMethod();
        int statusCode = -1;
        String statusString = null;
        if( incomingMessage instanceof SipResponse ){
            statusCode   = ((SipResponse)incomingMessage).getStatusCode();
            statusString = ((SipResponse)incomingMessage).getStatusText();
        }
        Address fromAddress    = new AddressImpl( strFromAddress );
        Address toAddress      = new AddressImpl( strToAddress );
        Util.debug("before servicing call leg listeners " );
        Util.debug(    "incoming: "+this.getCall().getCallId() +"  "+
                       this.getLocalAddress()     +"  "+
                       this.getRemoteAddress()    +"  \nthis    :"+
                       strCallID +"  "+
                       fromAddress     +"  "+
                       toAddress             );
        Util.debug( "incoming message by  "+getCall().getCallId() +" is "+  incomingMessage );

        //fire the listeners.
        for (Enumeration e = m_vListenerList.elements() ; e.hasMoreElements() ;) {
            DialogListener listener = (DialogListener) e.nextElement() ;
            if (listener != null) {
                if( incomingMessage instanceof SipRequest ){
                    // Bob: Changed to compile against latest sources
                    // listener.incomingRequest ( jainIncomingMessage, method);
                    listener.incomingRequest ( jainIncomingMessage );
                }else if ( incomingMessage instanceof SipResponse ){
                    //add the message to the messagesHolder
                    m_vMessages.addElement( jainIncomingMessage );
                    // Bob: Modified to compile against latest sources
                    // listener.incomingResponse
                    //    ( jainIncomingMessage, method, statusCode, statusString);
                    listener.incomingResponse( jainIncomingMessage);


                    //401 (Unauthorized) and
                    //407 (Proxy Authentication Required)
                    SipResponse response =(SipResponse)(incomingMessage);
                    if( (response.getStatusCode() == 401 ) ||
                        (response.getStatusCode() == 407 ) )
                    {
                        listener.authenticationRequired(lastOutgoingMessage, response.getRealm() );
                    }
                }
            }
        }


    }catch( Exception e ){
        SysLog.log(e);
    }


    }


    /**
     *gets the call that created it.
     */
    protected CallImpl getCall(){
        return m_call;
    }

    /**
     * gets the new CSeq number.
     */
    private int getNewCSeqNumber(String strRemoteURL){
        return org.sipfoundry.sip.SipSession.getNextCSeqNumber
                        ( getCall().getCallId(),  strRemoteURL);
    }


    /**
     * Is the message a response?
     */
    protected boolean isResponse(SIPMessage message){
         boolean bRet = false;
         String startLine = ((SIPMessageImpl)(message)).getStartLine();
         if(( startLine != null ) && ( startLine.startsWith(SIP_VERSION_STRING) ))
            bRet = true;

        return bRet;
    }
}
