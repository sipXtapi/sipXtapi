/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/siplite/UserImpl.java#2 $
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
import org.sipfoundry.stapi.*;

/**
 * Encapsulates all the details that are user specific. Each call must be tied
 * to a User before the Call can be used to create Dialogs and Messages. The
 * User contains both the contact address and the list of authentication
 * realm details.
 */
public class UserImpl implements User
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    //handle for this user.
    private SipLine m_sipLines[] ;

    /** contact addresses */
    private Address m_addresses[];


//////////////////////////////////////////////////////////////////////////////
// Contruction
////
    public UserImpl()
    {

    }

    public UserImpl(Address address)
    {
        m_addresses = new Address[1] ;
        m_addresses[0] = address;

        try
        {
            setContactAddresses(m_addresses) ;
        }
        catch (InvalidArgumentException e)
        {
            // TODO: How should we handle this?
            SysLog.log(e);
        }
   }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * This will add a new username and password or update a password for an
     * existing username  and realm
     * @param realm     The realm the username and password is for
     * @param username  The username to use.
     * @param password  The password to use.
     */
    public void addRealm(String realm, String username, String password)
        throws InvalidArgumentException
    {
        // Validate Input
        if ((realm == null) || (username == null) || (password == null))
            throw new InvalidArgumentException() ;

        // Add Realm to all our lines
        if (m_sipLines != null)
        {
            for (int i=0; i<m_sipLines.length; i++)
            {
                m_sipLines[i].addRealm(realm, username, password);
            }
        }
    }


    /**
    * This will remove the username and password assigned to the given realm
    * @param realm     The realm the username and password is for
    * @return Whether or not the remove was successfull
    * @throws InvalidArgumentException if the realm is null or blank
    */
   public boolean removeRealm(java.lang.String realm)
       throws InvalidArgumentException
   {
       if( TextUtils.isNullOrSpace(realm) )
           throw new InvalidArgumentException("realm cannot be null or blank");
        boolean bRemoved = false ;

        if (m_sipLines != null)
        {
            bRemoved = true ;
            for (int i=0; i<m_sipLines.length; i++)
            {
                m_sipLines[i].removeRealm(realm) ;
            }
        }

        return bRemoved ;
    }


    /**
     * Used to determine if a realms detail exists.
     * @return Whether or not the realm exists.
     * @throws InvalidArgumentException if the realm is null or blank
     */
    public boolean realmExists(java.lang.String realm)
        throws InvalidArgumentException
    {
        if( TextUtils.isNullOrSpace(realm) )
           throw new InvalidArgumentException("realm cannot be null or blank");

        boolean bFound = false;

        if (m_sipLines != null)
        {
            for (int i=0; i<m_sipLines.length && !bFound; i++)
            {
                String[] realms = m_sipLines[i].getRealms();
                for (int j=0; j<realms.length; j++)
                {
                    if (realms[j].equals(realm))
                    {
                        bFound = true;
                        break;
                    }
                }
            }
        }
        return bFound ;
    }


    /**
     * This will return a String array of realms that username and password
     * combinations exist for.
     * @return the String array of realms.
     */
    public String[] getRealms()
    {
        // coming in the reverse order for some reasons
        return m_sipLines[0].getRealms() ;
    }



    /**
     * This will return a String array of username and password pairs for a realm
     * @param realm Retrieve usernames and passwords for this realm
     * @return String array of usernames and passwords existing for a realm
     *  were element 0 is the first username and element 1 is its corresponding
     *  password
     * @throws InvalidArgumentException if the realm is null or blank
     */
    public String[] getRealmAuthenticationDetails(String realm) throws
        InvalidArgumentException
    {
        // TODO: Must figure out HOW this works and implement.

        if( TextUtils.isNullOrSpace(realm) )
           throw new InvalidArgumentException("realm cannot be null or blank");

        return null ;
    }


    /**
     * Set's the contact addresses of the User. This must be set before
     * any messages can be created.
     * @param contactAddresses  The contact address.
     */
    public void setContactAddresses(javax.siplite.Address[] contactAddresses)
        throws InvalidArgumentException
    {
        SipLineManager lineManager = SipLineManager.getInstance() ;

        // Validate input
        if (contactAddresses == null)
            throw new InvalidArgumentException() ;

        // Init Data Structures
        m_addresses = contactAddresses ;
        m_sipLines = new SipLine[contactAddresses.length] ;

        // Create SIP Lines (if needed)
        for (int i=0; i< m_addresses.length; i++)
        {
            // Generate address
            PAddress addressLine = null;
            try
            {
                addressLine = PAddressFactory.getInstance().createAddress(
                        m_addresses[i].toString()) ;
            }
            catch(PCallAddressException e)
            {
                SysLog.log(e);
            }


            // Find / Create line
            if (addressLine != null)
            {
                m_sipLines[i] = lineManager.getLineByAddress(addressLine);
                if (m_sipLines[i] == null)
                {
                    m_sipLines[i] = lineManager.createLine(
                            contactAddresses[i].toString(),
                            "User",
                            true,   //provisioned
                            false,  // show Line
                            true,   // autoEnable
                            false) ; //callHandlingSettings
                }
            }
        }
    }


    /**
     * returns an array of the current contact addresses.
     * @return the current contact addresses of this user as Addresses
     */
    public javax.siplite.Address[] getContactAddresses()
    {
        return m_addresses ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Set's the contact address of the Call Provider. This must be set before
     * any messages can be created.
     * @param contactAddress  The contact address.
     */
    protected void setContactAddress(Address contactAddress)
        throws InvalidArgumentException
    {
        // Validate Data
        if (contactAddress == null)
            throw new InvalidArgumentException() ;

        Address addresses[] = new Address[1] ;
        addresses[0] = contactAddress;

        setContactAddresses(addresses) ;
    }


    /**
     * returns the current contact address.
     * @return the current contact address of this user as an Address
     */
    protected Address getContactAddress()
    {
        Address rc = null ;

        if ((m_addresses != null) && (m_addresses.length > 0))
            rc = m_addresses[0];

        return rc ;
    }
}
