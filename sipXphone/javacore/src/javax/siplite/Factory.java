package javax.siplite;


/**
 * A singleton factory class which applications can use to obtain a
 * SIPStack object, Address objects, SIPHeader objects, SIPValue objects,
 * SIPParameter objects and User objects.
 * The SIPStack object must be created before the any other objects
 * as the supplied path specifies the location of the objects<br><br>
 * <code> getSIPStack(path) </code><br><br>
 * The proprietary Address objects can be created in by<br><br>
 * <code> createAddress(sipAddress) </code><br><br>
 * or<br><br>
 * <code> createAddress(username, host) </code><br><br>
 * The proprietary SIPHeader objects can be created in by<br><br>
 * <code> createSIPHeader(type) </code><br><br>
 * The proprietary SIPValue objects can be created in by<br><br>
 * <code> createSIPValue() </code><br><br>
 * The proprietary SIPParameter objects can be created in by<br><br>
 * <code> createSIPParameter() </code><br><br>
 * A User object can be created by <br><br>
 * <code> createUser(contact) </code><br><br>
 * It is important to note that when an Address object is created the port is
 * set to the default port number of 5060. If this port needs to be changed,
 * the setPort method in Address interface can be used.<br><br>
 * It is also important to note that any object that implements<br>
 * - the Address Interface is referred to as AddressImpl<br>
 * - the SIPStack Interface is referred to as SIPStackImpl<br><br>
 * - the User Interface is referred to as UserImpl<br><br>
 **/

public class Factory
{
	/** lock object */
	private static final Object LOCK = new Object();

    /**
     * The singleton instance of AddressFactory
     */
    private static Factory factory = null;

    /**
     * The single instance of SIPStack
     */
    private static javax.siplite.SIPStack sipStack = null;

    /**
     * The high level path to the proprietary SIPStackImpl class.
     */
    private java.lang.String highLevelPath;

    /**
     * The lower level package structure and class name of any class
     * that implements Address.  This will be added to the highLevelPath
     * that is set in order to find the AddressImpl class.
     */
    private static final java.lang.String lowLevelAddressPath = "AddressImpl";

    /**
     * The lower level package structure and class name of any class
     * that implements SIPHeader.  This will be added to the highLevelPath
     * that is set in order to find the SIPHeaderImpl class.
     */
    private static final java.lang.String lowLevelSIPHeaderPath = "SIPHeaderImpl";

    /**
     * The lower level package structure and class name of any class
     * that implements SIPValue.  This will be added to the highLevelPath
     * that is set in order to find the SIPValueImpl class.
     */
    private static final java.lang.String lowLevelSIPValuePath = "SIPValueImpl";

    /**
     * The lower level package structure and class name of any class
     * that implements SIPParameter.  This will be added to the highLevelPath
     * that is set in order to find the SIPParameterImpl class.
     */
    private static final java.lang.String lowLevelSIPParameterPath =
        "SIPParameterImpl";

    /**
     * The lower level package structure and class name of any class
     * that implements SIPStack.  This will be added to the highLevelPath
     * that is set in order to find the SIPStackImpl class.
     */
    private static final java.lang.String lowLevelSIPStackPath = "SIPStackImpl";

    /**
     * The lower level package structure and class name of any class
     * that implements User.  This will be added to the highLevelPath
     * that is set in order to find the UserImpl class.
     */
    private static final java.lang.String lowLevelUserPath = "UserImpl";

    // ****************************
    // *****   CONSTRUCTORS   *****
    // ****************************

    /**
     * A private constructor to ensure that this is a singleton class.
     * Use the getInstance() method to get an instance of Factory.
     */
    private Factory()
    {
    }

    // *****************************
    // *****   GET Methods     *****
    // *****************************

    /**
     * Returns an instance of the Factory. This is a singleton
     * class and therefore there can only be one instance of this class.
     * This method is the global access point to this class.
     * @returns    the single instance of this class
     */
    public static Factory getInstance()
    {
		if (factory == null)
		{
			synchronized (LOCK)
			{
				if (factory == null)
				{
		            factory = new Factory();
		        }
            }
        }

        return factory;

    } // end of getInstance

    /**
     * Returns the instance of the SIPSack.
     * @param stackPath the path to the proprietary implementation
     * @returns the instance of the SIPSack implementation
     * @throws   PeerUnavailableException	if an error occurs such as the
     *						apropriate class cannot be found
     * 						and thus the SIPStack cannot be
     *						created
     */
    public javax.siplite.SIPStack getSIPStack(java.lang.String stackPath)
            throws PeerUnavailableException
    {
        highLevelPath = stackPath;
        // if high level path is not set, throw a PeerUnavailableException
        if ( null == highLevelPath )
        {
            throw new PeerUnavailableException( "SIPStack path name not set." );
        }

        if ( null == sipStack )
        {
			synchronized (LOCK)
			{
                          if (null == sipStack)
                          {
		            String className = highLevelPath + "." + lowLevelSIPStackPath;
		            try
		            {
		                Object o = Class.forName( className ).newInstance();
		                sipStack = (SIPStack) o;
		            }
                            catch(ClassNotFoundException e)
		            {
		                throw new PeerUnavailableException( "Unable to find class "
		                + className + " : " + e );
		            }
		            catch(InstantiationException e)
		            {
		                throw new PeerUnavailableException( "Could not instantiate "
		                + "class : " + e );
		            }
		            catch(IllegalAccessException e)
		            {
		                throw new PeerUnavailableException( "Illegal Access : " + e );
		            }
		       }
           }
        }
        return sipStack;

    } // end of getSIPStack

    /**
     * Returns an instance of Address based on the SIP address supplied
     * @param    sipAddress    sip Address to base address on.
     * @throws   InvalidAddressException	if an error occurs while creating
     *                                   the proprietary implementation.
     */
    public javax.siplite.Address createAddress( String sipAddress ) throws InvalidAddressException
    {
        javax.siplite.Address address = null;
        try
        {
            address = getAddress();
        }
        catch ( InvalidAddressException e )
        {
            throw e;
        }

	try
	{
            address.parseSIPAddress( sipAddress );
	}
	catch ( InvalidArgumentException e)
	{
            throw new InvalidAddressException("Invalid Argument: " + e) ;
	}

        return address;

    } // end of createAddress( String )


    /**
     * Returns an instance of Address with the host and user name set as
     * specified in the parameters.  If the username is null only the
     * host is used.  The port value will be the default 5060.
     * @param    username    the username.
     * @param    host        the name of the host.
     * @throws   InvalidAddressException	if an error occurs while creating
     *                                   the proprietary implmentation.
     */
    public javax.siplite.Address createAddress( java.lang.String username,
                                                java.lang.String host )
    throws InvalidAddressException
    {
        javax.siplite.Address address = null;

        try
        {
            address = getAddress();
        }
        catch (InvalidAddressException e )
        {
            throw e;
        }

        address.setHost( host );
        address.setUserName( username );

        return address;

    } // end of createAddress( String, String )

    /**
     * Returns an instance of the Address.
     * @returns an instance of the Address implementation
     * @throws    InvalidAddressException	if an error occurs at this point
     *                                 		such as cannot find the class
     */
    private javax.siplite.Address getAddress() throws InvalidAddressException
    {
        // if high level path is not set, throw a InvalidAddressException
        if ( null == highLevelPath )
        {
            if ( null == highLevelPath )
                throw new InvalidAddressException( "Address path name not set.");
        }

        String className = highLevelPath + "." + lowLevelAddressPath;
        try
        {
            Object o = Class.forName( className ).newInstance();
            return (Address) o;
        }
        catch(ClassNotFoundException e)
        {
            throw new InvalidAddressException( "Unable to find class "
            + className + " : " + e );
        }
        catch(InstantiationException e)
        {
            throw new InvalidAddressException( "Could not instantiate "
            + "class : " + e );
        }
        catch(IllegalAccessException e)
        {
            throw new InvalidAddressException( "Illegal Access : " + e );
        }

    } // end of getAddress

    /**
     * Returns an instance of SIPHeader based on the name supplied
     * @param    name    type of the header to be created.
     * @throws   InvalidArgumentException	if an error occurs while creating
     *                                   the proprietary implementation.
     */
    public javax.siplite.SIPHeader createSIPHeader( java.lang.String type )
        throws InvalidArgumentException
    {
        javax.siplite.SIPHeader sipHeader = null;
        try
        {
            sipHeader = getSIPHeader();
        }
        catch ( InvalidArgumentException e )
        {
            throw e;
        }

	try
	{
            sipHeader.setType( type );
	}
	catch (AttributeSetException e)
	{
	    throw new InvalidArgumentException("Unexpected exception: " + e);
	}

        return sipHeader;

    } // end of createSIPHeader( String )

    /**
     * Returns an instance of the SIPHeader.
     * @returns an instance of the Address implementation
     * @throws    InvalidArgumentException	if an error occurs at this point
     *                                 		such as cannot find the class
     */
    private javax.siplite.SIPHeader getSIPHeader() throws InvalidArgumentException
    {
        // if high level path is not set, throw a InvalidImplementationException
        if ( null == highLevelPath )
        {
            if ( null == highLevelPath )
                throw new InvalidArgumentException( "SIPHeader path name not set.");
        }

        String className = highLevelPath + "." + lowLevelSIPHeaderPath;
        try
        {
            Object o = Class.forName( className ).newInstance();
            return (SIPHeader) o;
        }
        catch(ClassNotFoundException e)
        {
            throw new InvalidArgumentException( "Unable to find class "
            + className + " : " + e );
        }
        catch(InstantiationException e)
        {
            throw new InvalidArgumentException( "Could not instantiate "
            + "class : " + e  );
        }
        catch(IllegalAccessException e)
        {
            throw new InvalidArgumentException( "Illegal Access : " + e );
        }

    } // end of getSIPHeader

    /**
     * Returns an instance of SIPValue
     * @throws   InvalidArgumentException	if an error occurs while creating
     *                                   the proprietary implementation.
     */
    public javax.siplite.SIPValue createSIPValue( )
        throws InvalidArgumentException
    {
        javax.siplite.SIPValue sipValue = null;
        try
        {
            sipValue = getSIPValue();
        }
        catch ( InvalidArgumentException e )
        {
            throw e;
        }

        return sipValue;

    } // end of createSIPValue( )

    /**
     * Returns an instance of the SIPValue.
     * @returns an instance of the SIPValue implementation
     * @throws    InvalidArgumentException	if an error occurs at this point
     *                                 		such as cannot find the class
     */
    private javax.siplite.SIPValue getSIPValue() throws InvalidArgumentException
    {
        // if high level path is not set, throw a InvalidArgumentException
        if ( null == highLevelPath )
        {
            if ( null == highLevelPath )
                throw new InvalidArgumentException( "SIPValue path name not set.");
        }

        String className = highLevelPath + "." + lowLevelSIPValuePath;
        try
        {
            Object o = Class.forName( className ).newInstance();
            return (SIPValue) o;
        }
        catch(ClassNotFoundException e)
        {
            throw new InvalidArgumentException( "Unable to find class "
            + className + " : " + e );
        }
        catch(InstantiationException e)
        {
            throw new InvalidArgumentException( "Could not instantiate "
            + "class : " + e  );
        }
        catch(IllegalAccessException e)
        {
            throw new InvalidArgumentException( "Illegal Access : " + e );
        }

    } // end of getSIPValue

    /**
     * Returns an instance of SIPParameter
     * @throws   InvalidArgumentException	if an error occurs while creating
     *                                   the proprietary implementation.
     */
    public javax.siplite.SIPParameter createSIPParameter( )
        throws InvalidArgumentException
    {
        javax.siplite.SIPParameter sipParameter = null;
        try
        {
            sipParameter = getSIPParameter();
        }
        catch ( InvalidArgumentException e )
        {
            throw e;
        }

        return sipParameter;

    } // end of createSIPParameter( )

    /**
     * Returns an instance of the SIPParameter.
     * @returns an instance of the SIPParameter implementation
     * @throws    InvalidArgumentException	if an error occurs at this point
     *                                 		such as cannot find the class
     */
    private javax.siplite.SIPParameter getSIPParameter()
        throws InvalidArgumentException
    {
        // if high level path is not set, throw a InvalidArgumentException
        if ( null == highLevelPath )
        {
            if ( null == highLevelPath )
                throw new InvalidArgumentException( "SIPParameter path name not set.");
        }

        String className = highLevelPath + "." + lowLevelSIPParameterPath;
        try
        {
            Object o = Class.forName( className ).newInstance();
            return (SIPParameter) o;
        }
        catch(ClassNotFoundException e)
        {
            throw new InvalidArgumentException( "Unable to find class "
            + className + " : " + e );
        }
        catch(InstantiationException e)
        {
            throw new InvalidArgumentException( "Could not instantiate "
            + "class : " + e  );
        }
        catch(IllegalAccessException e)
        {
            throw new InvalidArgumentException( "Illegal Access : " + e );
        }

    } // end of getSIPParameter

    /**
     * Returns an instance of User with the contact set to the given
     * contact addresses.
     * @param    Address    the contact addresses
     * @throws   InvalidArgumentException	if an error occurs while creating
     *                                          the proprietary implementation.
     */
    public javax.siplite.User createUser( javax.siplite.Address[] contactAddresses)
        throws InvalidArgumentException
    {
        javax.siplite.User user = null;
        try
        {
            user = getUser();
        }
        catch ( InvalidArgumentException e )
        {
            throw e;
        }

        user.setContactAddresses( contactAddresses);

        return user;

    } // end of createAddress( String )


    /**
     * Returns an instance of the User Object.
     * @returns an instance of the User implementation
     * @throws    InvalidAddressException	if an error occurs at this point
     *                                 		such as cannot find the class
     */
    private javax.siplite.User getUser() throws InvalidArgumentException
    {
        // if high level path is not set, throw a InvalidAddressException
        if ( null == highLevelPath )
        {
            if ( null == highLevelPath )
                throw new InvalidArgumentException( "Address path name not set.");
        }

        String className = highLevelPath + "." + lowLevelUserPath;
        try
        {
            Object o = Class.forName( className ).newInstance();
            return (User) o;
        }
        catch(ClassNotFoundException e)
        {
            throw new InvalidArgumentException( "Unable to find class "
            + className + " : " + e );
        }
        catch(InstantiationException e)
        {
            throw new InvalidArgumentException ("Could not instantiate "
            + "class : " + e  );
        }
        catch(IllegalAccessException e)
        {
            throw new InvalidArgumentException( "Illegal Access : " + e );
        }

    } // end of createAddress

}
