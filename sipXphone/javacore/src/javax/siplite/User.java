/*
 * User.java
 *
 * Created on 21 November 2001, 16:37
 */

package javax.siplite;

/**
 * Encapsulates all the details that are user specific. Each call must be tied 
 * to a User before the Call can be used to create Dialogs and Messages. The 
 * User contains both the contact address and the list of authentication
 * realm details.
 * @author  Ben Smith
 * @version 
 */
public interface User 
{

    /**
     * This will add a new username and password or update a password for an 
	 * existing username  and realm
     * @param realm     The realm the username and password is for
     * @param username  The username to use.
     * @param password  The password to use.
     * @throws InvalidArgumentException if the realm or username are null or blank
     */
    void addRealm(java.lang.String realm, 
                  java.lang.String username, 
                  java.lang.String password)
        throws InvalidArgumentException;
    
    /**
     * This will remove the username and password assigned to the given realm
     * @param realm     The realm the username and password is for
     * @return Whether or not the remove was successfull
     * @throws InvalidArgumentException if the realm is null or blank
     */
    boolean removeRealm(java.lang.String realm)
        throws InvalidArgumentException;
    
    /**
     * Used to determine if a realms detail exists.
     * @return Whether or not the realm exists.
     * @throws InvalidArgumentException if the realm is null or blank
     */
    boolean realmExists(java.lang.String realm)
        throws InvalidArgumentException;
    
    /**
     * This will return a String array of realms that username and password 
	 * combinations exist for.
     * @return the String array of realms.
     */
    java.lang.String[] getRealms();

    /**
     * This will return a String array of username and password pairs for a realm
     * @param realm Retrieve usernames and passwords for this realm
     * @return String array of usernames and passwords existing for a realm
     *  were element 0 is the first username and element 1 is its corresponding 
     *  password
     * @throws InvalidArgumentException if the realm is null or blank
     */
    String[] getRealmAuthenticationDetails(String realm)
        throws InvalidArgumentException;

    /**
     * Set's the contact addresses of the User. This must be set before
     * any messages can be created.
     * @param contactAddresses  The contact address.
     * @throws InvalidArgumentException if the array contains null or blank elements
     */
    void setContactAddresses(javax.siplite.Address[] contactAddress)
        throws InvalidArgumentException;
    
    /**
     * returns an array of the current contact addresses.
     * @return the current contact addresses of this user as Addresses
     */
    javax.siplite.Address[] getContactAddresses();
}
