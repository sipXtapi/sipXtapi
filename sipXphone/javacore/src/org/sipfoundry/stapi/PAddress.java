/*
 * $Id$
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

import java.io.* ;
import java.net.* ;

import org.sipfoundry.stapi.event.* ;


/**
 * PAddress is an abstraction for a phone number.
 * <p>
 * An address cannot be changed once it is constructed.
 */
public class PAddress implements Serializable
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    String m_strAddress ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Protected no-argument constructor provided for serialization
     */
    protected PAddress()
    {

    }


    /**
     * Constructor, requires an address.
     *
     * @param strAddress Address of call.
     */
    public PAddress(String strAddress)
    {
        m_strAddress = strAddress ;

    }


    /**
     * Query the flat string representation of this address.
     *
     * @return Flat string representation of this address.
     */
    public String getAddress()
    {
        return m_strAddress ;
    }

    /**
     * setting an address is protected and restricted to classes living
     * within the stapi package.
     *
     * @param strAddress Address of call.
     */
    protected void setAddress(String strAddress)
    {
        m_strAddress = strAddress ;
    }


    /**
     * @deprecated do not expose
     */
    public String toString()
    {
        return getAddress() ;
    }


    /**
     * @deprecated do not expose
     *
     * Determine if these addresses are equal by comparing the actual address
     * strings.  This is a short term solution.  We should pull the URLs apart
     * and strip all of the useless data (i.e. Display name).
     */
    public boolean equals(Object obj)
    {
        boolean bEquals = false ;

        PAddress addr = (PAddress) obj ;
        if (addr != null) {
            String strAddr1 = getAddress() ;
            String strAddr2 = addr.getAddress() ;

            // Be ultra anal about null pointers
            if ((strAddr1 == null) && (strAddr2 == null))  {
                bEquals = true ;
            } else if ((strAddr1 != null) && (strAddr2 != null))  {
                bEquals = strAddr1.equals(strAddr2) ;
            }
        }
        return bEquals ;
    }


    /**
     * Determine the hash code of this object.
     *
     * @deprecated do not expose
     */
    public int hashCode()
    {
        String strAddress = getAddress() ;
        if (strAddress == null)
            strAddress = "" ;

        return strAddress.hashCode() ;
    }
}
