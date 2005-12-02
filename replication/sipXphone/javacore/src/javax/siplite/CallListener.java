/*
 * CallListener.java
 *
 * Created on 24 October 2001, 15:46
 */

package javax.siplite;

/**
 * The CallListener Interface listens for all new incoming Dialogs. <br><br>
 * The CallListener Interface is implemented by the application so that all
 * new Dialogs within a call can be handled as neccessary. <br><br>
 * A CallListener implementation must be registered with the Call in order that
 * events can be recieved by the listening application. <br><br>
 *
 * <blockquote><code> call.addCallListener(callListener) </blockquote></code> 
 * <br>
 *
 * Multiple CallListeners can be supported by the Call so that Listeners which
 * perform different tasks can be added or removed as neccessary.
 */
public interface CallListener 
{
    /**
     * This method will be invoked by the Call when a new incoming Dialog is
     * received from the stack.
     * @param    dialog	 		The new Dialog that has been created
     * @param    incomingMessage	The message that was responsible for 
	 *								creating the Dialog.
     */
    void incomingDialog(javax.siplite.Dialog dialog, 
                  	javax.siplite.SIPMessage incomingMessage);
}

