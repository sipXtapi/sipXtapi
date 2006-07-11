package javax.siplite;

/**
 * The CallProviderListener Interface listens for all new incoming Calls.
 * <br><br>
 * The CallProviderListener Interface is implemented by the application so 
 * that all new Calls can be handled as neccessary. <br><br>
 * A CallProviderListener implementation must be registered with the 
 * CallProvider in order that events can be recieved by the listening 
 * application. <br><br>
 *
 * <blockquote><code> callProvider.addCallProviderListener(callProviderListener) * </blockquote></code> <br>
 *
 * Multiple CallProviderListeners can be supported by the CallProvider so 
 * that CallProviderListeners which perform different tasks can be added or
 * removed as neccessary.
 */

public interface CallProviderListener
{
    /**
     * This method will be invoked by the CallProvider when a new incoming 
	 * Call is received from the stack.
     * It is recommended that you set the DialogListener in the given Dialog
     * to allow for events which occur immediatly.  
     * @param    call            The new incoming Call 
     * @param    dialog         The new incoming Dialog
     * @param    incomingMessage The message that created this Call/Dialog. 
     *                           You must respond to this through the Dialog
     * 							 object.
     */
    void incomingCall(javax.siplite.Call call, 
                      javax.siplite.Dialog dialog, 
                      javax.siplite.SIPMessage incomingMessage);

}  // end of interface CallProviderListener
