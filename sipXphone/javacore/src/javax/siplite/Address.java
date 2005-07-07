package javax.siplite;


/**
 * The Address is used to indicate the originator of a request, the
 * destination of a request and to specify any redirections of the request.
 * The Address interface encapsulates the username, the host, the port, extra
 * parameters and the display name by providing accessor methods for all.
 * Address also allows the scheme to be set i.e. "sip:" or "tel:". If a scheme
 * is not set, the default scheme, "sip:", will be used.
 * 
 * An Address is created using the Factory. The Factory creates
 * an address with the default port of 5060. This can be changed if 
 * neccessary by the
 * <code> setPort </code> method<br>
 *<br>
 * An example of an address is : <br>
 * <br>
 *     <code> A Jones &lt;ajones@somewhere.com:5060&gt; </code><br>
 *<br>
 * where<blockquote> username = ajones<br>
 * domain = somewhere.com<br>
 * display name = A Jones<br>
 * port = 5060</blockquote>
 **/

public interface Address
{
    /**
     * Parses the supplied SIP address to set up all the attributes of 
     * an address.  If the SIP Address is malformed then an
     * InvalidArgumentException is thrown
     **/
     public void parseSIPAddress(java.lang.String sipAddress) throws 
        InvalidArgumentException;
    
    /**
     * Set the user name of an address. An example of a user name would be
     * ajones
     * @param    userName    The username of an address
     **/
    void setUserName(java.lang.String userName);
    
    /**
     * Set the host of an address. An example of a host would be a fully-
     * qualified domain name e.g "somewhere.com" or numeric IP address e.g.
     * 111.111.111.11.
     * @param    host    The host of an address
     **/
    void setHost(java.lang.String host);
    
    /**
     * Set the port of an address. An example of a port would be 5060
     * @param    port    The port of an address
     **/
    void setPort(int port);
    
    /**
     * Set the display name of an address. An example of a display name would
     * be A Jones
     * @param    displayName    The displayName of an address
     **/
    void setDisplayName(java.lang.String displayName);

    /**
     * Used to add a parameter to the URL. An example parameter could be 
     * "user=phone"
     * @param   parameter   The parameter to be added
     * @throws InvalidArgumentException This is thrown when malformed Parameter
     *                                  is given,
     **/
    void addParameter(java.lang.String parameter) throws InvalidArgumentException;

    /**
     * Used to add a parameter to the URL. 
     * @param   sipParameter   The parameter to be added
     **/
    void addParameter(javax.siplite.SIPParameter sipParameter);
    
    /**
     * Sets the scheme of the address. i.e. sip or tel. If the scheme is not 
     * supported then an Invalid Argument is thrown.
     * @param   scheme  The scheme (as a string) to use.
     * @throws InvalidArgumentException This is thrown when an unsupported 
     *	scheme is given.
     */
    void setScheme(java.lang.String scheme) throws InvalidArgumentException;
    
    /**
     * Retrieve the user name of an address.
     * @returns    the user name of an address
     **/
    java.lang.String getUserName();
    
    /**
     * Retrieve the host of an address
     * @returns    the host of an address
     **/
    java.lang.String getHost();
    
    /**
     * Retrieve the port of an address
     * @returns    the port of an address
     **/
    int getPort();
    
    /**
     * Retrieve the displayName of an address.
     * @returns    the displayName of an address
     **/
    java.lang.String getDisplayName();

    /**
     * Retrieves the List of parameters of an address.
     * @return  The list of parameters
     */
    javax.siplite.SIPParameter[] getParameters();
    
    /**
     * Gets the scheme of the address. i.e. sip or tel. 
     * @return  The scheme as a string.
     */
    java.lang.String getScheme();

    /**
     * Test to see if two addresses are equal. Two addresses are deemed to
     * be equal if the username, host and port are the same.
     * @return true if the object address is equal
     **/
    boolean equals(Object address);
    /**
     * This method is supported for the benefit of hashtable, because equals is 
     * defined by the interface
     * If two objects are equal according to the equals(Object) method, 
     * then calling the hashCode method on each of the two objects must produce 
     * the same integer result
     * @return a hash code value for this object
     **/
    int hashCode();
}
