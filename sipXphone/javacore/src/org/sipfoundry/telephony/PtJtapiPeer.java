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


package org.sipfoundry.telephony ;

import javax.telephony.* ;

public class PtJtapiPeer implements JtapiPeer 
{
    /**
     * Returns the name of this JtapiPeer object instance. 
     * 
     * This name is the same name used as an argument to 
     * JtapiPeerFactory.getJtapiPeer() method. 
     */   
    public String getName()
    {
        return this.getClass().getName() ;
    }
    

    /**
     * Returns the services that this implementation supports. This method 
     * returns null if no services as supported. 
     */
    public String[] getServices()
    {
        return null ;
    }
    

    /**
     * Returns an instance of a Provider object given a string argument which
     * contains the desired service name.
     */
    public Provider getProvider(String providerString)
        throws ProviderUnavailableException
    {
        PtProvider provider = PtProvider.getProvider("username", "password", "10.1.1.17:9000") ;
        
        return provider ;        
    }   
}
