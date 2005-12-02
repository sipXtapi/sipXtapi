/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/PAddressFactory.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.stapi ;

import org.sipfoundry.util.* ;

/**
 * The address factory is responsible for creating the appropriate address
 * object given an address string.
 *
 * This class is a singelton, use the getInstance() method to gain a
 * reference to the class.
 *
 * <p>Example:</p>
 * <code>
 * PAddressFactory factory = PAddressFactory.getInstance() ;
 * PAddress address;
 *
 * address = factory.createAddress("sip:user@host:port") ;
 * address = factory.createAddress("tel:18005551212") ;
 * </code>
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PAddressFactory
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static PAddressFactory m_instance ; // The singleton instance

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Private constructor guards against 3rd party instantiation.
     */
    private PAddressFactory()
    {

    }


    /**
     * Singleton accessor to obtain a reference to this class.
     *
     * @return The address factory.
     */
    public static PAddressFactory getInstance()
    {
        if (m_instance == null)
            m_instance = new PAddressFactory() ;

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Create an address object for the supplied string based address.  It is
     * assumed that the address contains either a "sip:" or "tel:" based
     * scheme.  See the example in the class definition.
     *
     * @param strAddress The string based representation of an address.
     *
     * @return An address object constructed from the supplied address string.
     */
    public PAddress createAddress(String strAddress)
        throws PCallAddressException
    {
        PAddress rc = null ;   // newly created address (return code)

        // Sanitize address
        if (strAddress == null)
            throw new PCallAddressException("null address") ;

        // Build the address object
        if (SipParser.isSIPURL(strAddress))
        {
            try
            {
                rc = new PSIPAddress(strAddress) ;
            }
            catch (Exception e)
            {
                throw new PCallAddressException("invalid sip url") ;
            }
        }
        else
        {
            rc = new PAddress(strAddress) ;
        }

        return rc ;
    }
}
