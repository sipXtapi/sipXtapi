/*
 * DialogListener.java
 *
 * Created on 24 October 2001, 15:46
 */

package javax.siplite;

/**
 * The DialogListener Interface listens for all incoming requests and 
 * responses for the specific Dialog it is registered with.<br><br>
 * Incoming messages are routed through the appropriate method, depending on.
 * whether they are requests or responses.<br><br>
 * A DialogListener implementation must be registered with the Dialog in 
 * order that incoming messages can be recieved by the listening 
 * application. <br><br>
 *
 * <blockquote><code> dialog.addDialogListener(dialogListener) </blockquote></code> <br>
 *
 * Multiple DialogListeners can be supported by the Dialog so that 
 * Listeners which perform different tasks can be added or removed as
 * neccessary.
 */
public interface DialogListener 
{
    /**
     * This method will be invoked by the Dialog when an incoming
     * response is received from the stack.
     * @param    incomingMessage	the incoming response message
     * @param    method          	the method type of the message which 
     *                           	resulted in this response.
     */
    void incomingResponse(javax.siplite.SIPMessage incomingMessage);
    
    /**
     * This method will be invoked by the Dialog when an incoming
     * request is received from the stack.
     * @param    incomingMessage	The incoming request message
     */
    void incomingRequest(javax.siplite.SIPMessage incomingMessage);
    
    /**
     * This method will be invoked by the Dialog when a request that has
     * previously been passed to the DialogListener is cancelled.
     * @param Message   The original reqeust message that has been cancelled
     */
    void cancelRequest(javax.siplite.SIPMessage originalIncomingMessage);

    /**
     * This method will be invoked when an outgoing request message is 
     * required to be authenticated. To resend the request, first add
     * the authorisation using the authoriseRealm method within the
     * User and then send the message again. You will not receive
     * any responses to an outgoing message that requires authentication.
     * @param originalOutgoingMessage	The original request message that 
     * 					requires authentication
     * @param realm                     The realm of authentication.
     */
    void authenticationRequired(
			javax.siplite.SIPMessage originalOutgoingMessage, 
			java.lang.String realm);
}

