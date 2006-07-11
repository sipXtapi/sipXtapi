package javax.siplite;

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

public interface Dialog
{
    
    
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
    javax.siplite.SIPMessage createRequest(java.lang.String method, java.lang.String contentType,
        					byte[] payload) throws InvalidArgumentException;
    
    /**
     * Create a Register request.  The From and To addresses set up
     * for the SIPMessage  will be taken from the Dialog. The 
     * <code>registrar</code> identifies the  destination of the message.
     * @return   A SIPMessage ready to send or edit
     * @throws   InvalidArgumentException	If an argument value is
     * 						incorrect and thus the message
     * 						cannot be created
     */
    javax.siplite.SIPMessage createRegisterRequest( javax.siplite.Address registrar )
        throws InvalidArgumentException;
    
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
    javax.siplite.SIPMessage createResponse( javax.siplite.SIPMessage incomingRequest, int statusCode,
        				     java.lang.String reason, java.lang.String contentType,
        					byte[] payload) throws InvalidArgumentException;

    /**
     * Cancelling a sent request.
     * @param   originalRequestMessage  The request message that has 
     * 					previously been 
     *                                  sent and is to be cancelled.
     * @throws   InvalidArgumentException	If an argument value is
     *						incorrect and the request cannot
     * 						be cancelled
     */
    void cancelRequest(javax.siplite.SIPMessage originalRequestMessage)
        						throws InvalidArgumentException;

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
    javax.siplite.SIPMessage getOriginalRequest(javax.siplite.SIPMessage incomingResponseMessage)
        						throws InvalidArgumentException;

    /**
     * Returns the Dialog's id. This id will not change over the lifetime
     * of the Dialog.
     * @return A string representation of the id
     */
    java.lang.String getDialogId();
    
    /**
     * Send the request / response message
     * @param	message	The SIPMessage to be sent
     * @throws	MessageNotSentException	This exception is thrown if the
     *          			message could not be sent
     **/
    void sendMessage(javax.siplite.SIPMessage message)
                     throws MessageNotSentException;
    
    /**
     * Add a DialogListener implementation to be notified of incoming 
     * requests or responses for the Dialog. The DialogListener interface
     * is implemented by the application.
     * @param dialogListener	the DialogListener interface implementation
     */
    void addDialogListener( javax.siplite.DialogListener dialogListener );
    
    
    /**
     * Remove a DialogListener implementation that is registered with the 
     * Dialog.
     * @param dialogListener	the DialogListener interface implementation
     */
    void removeDialogListener( javax.siplite.DialogListener dialogListener );
    
   /**
     * Method returns the local address of the Dialog.
     * @return from local address used by this Dialog
     */
    public javax.siplite.Address getLocalAddress();
    
    /**
     * Method returns the remote address of the Dialog.
     * @return from remote address used by this Dialog
     */
    public javax.siplite.Address getRemoteAddress();
    
} // end of interface Dialog
