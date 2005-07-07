package javax.siplite;

/**
 * This class is responsible for creating and managing call legs. When a
 * call is created, this call is identified by a call id. This call id will
 * not change over the lifetime of the call and therefore each call leg created
 * by the call will inherit the call id.
 **/
public interface Call
{
    /**
     * This method creates a Dialog with the from and to address and these
     * will remain constant over the length of the Dialog. The Dialog will
     * have the same call-id as the Call that created it.
     * @param	from	The address of whom the message is from
     * @param	to      The address of whom the message is to
     * @throws  InvalidArgumentException	If an argument value is
     *                                      incorrect and thus the Dialog
     *                                      cannot be created
     **/
    javax.siplite.Dialog createDialog(javax.siplite.Address from, javax.siplite.Address to)
        throws InvalidArgumentException;

    /**
     * This method returns an array of the current Dialogs being used in this call.
     * @return an array of the current Dialogs
     **/
    javax.siplite.Dialog[] getDialogs();

    /**
     * Add a CallListener implementation to be notified of new Dialogs. The
     * CallListener interface is implemented by the application.
     * @param callListener	the CallListener interface implementation
     */
    void addCallListener( javax.siplite.CallListener callListener );


    /**
     * Remove a CallListener implementation that is registered with the Call.
     * @param 	callListener	the CallListener interface implementation
     */
    void removeCallListener( javax.siplite.CallListener callListener );

    /**
     * Returns the User object that is currently being used in this call.
     * The User must be set in the Call before Dialogs can be created.
     * @return the current User object being used in this call
     */
    javax.siplite.User getUser();

    /**
     * Set the current User details. The User must be set when a new incoming
     * or outgoing Call is created.
     * @param user  The User to associate this Call with.
     * @throws AttributeSetException if the user has already been set
     */
    void setUser(javax.siplite.User user)
        throws AttributeSetException;

    /**
     * Returns the Call's id. This id will not change over the lifetime
     * of the Call.
     * @return A string representation of the id
     */
    java.lang.String getCallId();
}
