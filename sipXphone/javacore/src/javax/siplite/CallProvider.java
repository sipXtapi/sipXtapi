package javax.siplite;


/**
 * The role of the CallProvider object is that of creating Call objects. 
 * The Call Object is responsible for creating and managing Dialogs. 
 * Any number of Calls can be created. For an incoming request or response, 
 * the Call object associated with this message can be obtained.
 **/

public interface CallProvider
{
    
    /**
     * Add a CallProviderListener implementation for receiving new incoming 
	 * Calls. The CallProviderListener interface is implemented by the 
     * application.
     * @param callProviderListener	the CallProviderListener interface 
	 * 								implementation
     */
    void addCallProviderListener( javax.siplite.CallProviderListener callProviderListener );
    
    
    /**
     * Remove a CallProviderListener implementation that is registered with  
	 * the CallProvider.
     * @param callProviderListener	the CallProviderListener interface 
	 * 								implementation
     */
    void removeListener( javax.siplite.CallProviderListener callProviderListener );
    
  
    /**
     * This method is used to create a Call. On creation, a Call will be
     * associated with call id which will remain constant over the lifetime of
     * the Call.
     * @param user	The User associated with the Call.
     * @return 	The created Call 
     **/
    javax.siplite.Call createCall(javax.siplite.User user);

    /**
     * This method returns an array of the current calls known to the CallProvider.
     * @return an array of the current Calls 
     **/
    javax.siplite.Call[] getCalls();
   
}
